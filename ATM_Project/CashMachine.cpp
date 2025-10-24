// CashMachine.cpp
#include "CashStorage.cpp"

#include <iostream>
#include <sstream>
#include <vector>
#include <ctime>
#include <fstream>
#include <limits>
#include <string>

using namespace std;

class CashDispenser : public CashStorage {
public:
    CashDispenser() = default;

    bool depositAmount(long long amount, long long number, const string &dbPath, const string &logPath) {
    if (amount <= 0) {
        cerr << "Blad: kwota do depozytu musi byc dodatnia.\n";
        return false;
    }
    if (number <= 0) {
        cerr << "Blad: liczba banknotow musi byc dodatnia.\n";
        return false;
    }

    const long long MAX_BANKNOTS = 200; // możesz zmienić limit
    const long long MAX_AMOUNT = 100000; // zgodnie z main()
    if (amount > MAX_AMOUNT) {
        cerr << "Blad: przekroczono maksymalna kwote (" << MAX_AMOUNT << ")\n";
        return false;
    }
    if (number > MAX_BANKNOTS) {
        cerr << "Blad: przekroczono maksymalna ilosc banknotow (" << MAX_BANKNOTS << ")\n";
        return false;
    }

    int smallest = denominations.back();
    if (amount % smallest != 0) {
        cerr << "Blad: kwota musi byc podzielna przez najmniejszy nominał (" << smallest << ").\n";
        return false;
    }

    // Zamień wartości na "jednostki" 50zl, by zmniejszyć DP (np. 500 -> 10)
    int unit = smallest; // 50
    int m = denominations.size();
    vector<int> denomUnits(m);
    for (size_t i = 0; i < m; ++i) denomUnits[i] = denominations[i] / unit;

    int amountUnits = static_cast<int>(amount / unit);
    int num = static_cast<int>(number);

    // Szybkie warunki niepowodzenia:
    // - zbyt wiele banknotów (nawet gdy wszystkie to 50) -> każdy banknot przynosi min 1 jednostkę
    if (num > amountUnits) {
        cerr << "Blad: podana liczba banknotow jest za duza - nawet wszystkie 50 PLN nie daja takiej liczby banknotow.\n";
        return false;
    }
    // - zbyt malo banknotów (nawet gdy wszystkie to największy nominał)
    int maxUnitPerNote = denomUnits.front(); // np 500->10
    if ((long long)num * maxUnitPerNote < amountUnits) {
        cerr << "Blad: podana liczba banknotow jest za mala do uzyskania tej kwoty (brakuje miejsca na nominały).\n";
        return false;
    }

    // DP: prev[count][units] = indeks nominału użytego ostatnio lub -1 (nieosiągalne),
    // bazowy prev[0][0] = -2 (oznacza osiągalne bez wyboru)
    vector<vector<int>> prev(num + 1, vector<int>(amountUnits + 1, -1));
    prev[0][0] = -2;

    // Wypełniamy warstwowo po liczbie banknotów; przeglądamy nominały w kolejności malejącej,
    // aby preferować większe nominały przy pierwszym znalezieniu rozwiązania.
    for (int cnt = 1; cnt <= num; ++cnt) {
        for (int u = 0; u <= amountUnits; ++u) {
            if (prev[cnt][u] != -1) continue; // już oznaczony (opcjonalnie)
            for (size_t j = 0; j < (size_t)m; ++j) {
                int d = denomUnits[j];
                if (u - d < 0) continue;
                if (prev[cnt - 1][u - d] != -1) {
                    prev[cnt][u] = static_cast<int>(j); // użyto nominału j jako ostatniego
                    break; // preferujemy pierwszy znaleziony nominał (większe są na początku)
                }
            }
        }
    }

    if (prev[num][amountUnits] == -1) {
        cerr << "Nie znaleziono kombinacji " << number << " banknotow, ktore daja kwote " << amount << " PLN.\n";
        return false;
    }

    // Odtwórz rozwiązanie
    vector<long long> toAdd(m, 0);
    int curU = amountUnits;
    int curCnt = num;
    while (curCnt > 0) {
        int j = prev[curCnt][curU];
        if (j < 0 || j >= m) {
            // to nie powinno się zdarzyć
            cerr << "Blad odtwarzania rozwiazania.\n";
            return false;
        }
        toAdd[j] += 1;
        curU -= denomUnits[j];
        curCnt -= 1;
    }

    // Zastosuj zmiany (dodajemy banknoty do magazynu)
    try {
        for (size_t i = 0; i < toAdd.size(); ++i) {
            if (toAdd[i] != 0) addNotes(i, toAdd[i]);
        }
    } catch (const exception &e) {
        cerr << "Blad przy aktualizacji stanu: " << e.what() << endl;
        return false;
    }

    if (!saveToFile(dbPath)) {
        cerr << "Blad: nie udalo sie zapisac stanu do bazy (" << dbPath << ").\n";
        return false;
    }

    if (!appendLog(logPath, "WPŁATA", toAdd, amount)) {
        cerr << "Uwaga: nie udalo sie zapisac logu.\n";
    }

    cout << "Przyjeto depozyt: " << amount << " PLN w " << number << " banknotach\n";
    cout << "Rozklad dodanych banknotow:\n";
    for (size_t i = 0; i < denominations.size(); ++i)
        if (toAdd[i] > 0) cout << "  " << denominations[i] << " PLN : " << toAdd[i] << " szt.\n";

    return true;
}

    bool withdrawAmount(long long amount, const string &dbPath, const string &logPath) {
        if (amount <= 0) {
            cerr << "Blad: kwota do wyplaty musi byc dodatnia.\n";
            return false;
        }

        int smallest = denominations.back();
        if (amount % smallest != 0) {
            cerr << "Blad: kwota musi byc podzielna przez najmniejszy nominał (" << smallest << ").\n";
            return false;
        }

        vector<long long> toRemove(denominations.size(), 0);
        long long remaining = amount;

        for (size_t i = 0; i < denominations.size(); ++i) {
            long long want = remaining / denominations[i];
            long long can = min(want, quantities[i]);
            if (can > 0) {
                toRemove[i] = can;
                remaining -= can * denominations[i];
            }
        }

        if (remaining != 0) {
            cerr << "Nie mozna wydac żądanej kwoty z dostępnych banknotów.\n";
            return false;
        }

        try {
            for (size_t i = 0; i < toRemove.size(); ++i) {
                if (toRemove[i] != 0) addNotes(i, -toRemove[i]);
            }
        } catch (const exception &e) {
            cerr << "Blad przy aktualizacji stanu: " << e.what() << endl;
            return false;
        }

        if (!saveToFile(dbPath)) {
            cerr << "Blad: nie udalo sie zapisac stanu do bazy (" << dbPath << ").\n";
            return false;
        }

        if (!appendLog(logPath, "WYPŁATA", toRemove, amount)) {
            cerr << "Uwaga: nie udalo sie zapisac logu.\n";
        }

        cout << "Wydano: " << amount << " PLN\n";
        cout << "Rozklad wydanych banknotow:\n";
        for (size_t i = 0; i < denominations.size(); ++i)
            if (toRemove[i] > 0) cout << "  " << denominations[i] << " PLN : " << toRemove[i] << " szt.\n";

        return true;
    }

private:
    bool appendLog(const string &logPath, const string &operation, const vector<long long> &changed, long long totalAmount) {
        if (!ensureParentDir(logPath)) return false;
        ofstream log(logPath, ios::app);
        if (!log.is_open()) return false;

        time_t t = time(nullptr);
        char buf[64];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t));

        log << "=== " << buf << " ===\n";
        log << "Operacja: " << operation << '\n';
        log << "Kwota: " << totalAmount << " PLN\n";
        log << (operation == "WPŁATA" ? "Dodano:\n" : "Pobrano:\n");
        for (size_t i = 0; i < denominations.size(); ++i) {
            if (changed[i] != 0) log << denominations[i] << ' ' << changed[i] << '\n';
        }
        log << "Stan po operacji:\n";
        for (size_t i = 0; i < denominations.size(); ++i) {
            log << denominations[i] << ' ' << quantities[i] << '\n';
        }
        log << '\n';
        log.close();
        return true;
    }
};

// Pomocniczna funkcja wczytania poprawnej liczby całkowitej z wejścia
bool readLongLong(long long &out) {
    string s;
    if (!(cin >> s)) return false;
    try {
        size_t pos;
        long long val = stoll(s, &pos);
        if (pos != s.size()) return false;
        out = val;
        return true;
    } catch (...) {
        return false;
    }
}

int main() {
    const string dbPath = "DataBase/CashStorage.txt";
    const string logPath = "DataBase/CashStorage_log.txt";

    CashDispenser dispenser;

    if (!dispenser.loadFromFile(dbPath)) {
        cerr << "Blad: nie mozna wczytac bazy. Sprawdz uprawnienia katalogu DataBase.\n";
        // kontynuujemy z zerowym stanem
    }

    cout << "Wczytano stan magazynu.\n";

    while (true) {
        cout << "\n=== MENU ===\n";
        cout << "1) WPŁATA (dodaj pieniądze)\n";
        cout << "2) WYPŁATA (wydaj pieniądze)\n";
        cout << "3) POKAŻ STAN MAGAZYNU\n";
        cout << "4) WYJŚCIE\n";
        cout << "Wybierz opcję (1-4): ";

        long long choice;
        if (!readLongLong(choice)) {
            cerr << "Niepoprawne wejscie. Wpisz liczbę 1-4.\n";
            cin.clear();
            continue;
        }

        if (choice == 1) {
            cout << "Podaj kwote do wpłaty (liczba całkowita, bez PLN maks 100 000PLN): ";
            long long number;
            long long amount;
            if (!readLongLong(amount)) {
                cerr << "Niepoprawne dane wejsciowe. Oczekiwano liczby calkowitej.\n";
                continue;
            }
            if(amount>100000){
                cerr << "Niepoprawne dane wejsciowe. Maksymalna kwota wpłaty to 100 000 PLN.\n";
                continue;
            }
            cout << "Podaj liczbe banknotów do wpłaty (max 200szt): ";
            if (!readLongLong(number)) {
                cerr << "Niepoprawne dane wejsciowe. Oczekiwano liczby calkowitej.\n";
                continue;
            }

            if (!dispenser.depositAmount(amount,number, dbPath, logPath)) {
                cerr << "Operacja wpłaty nie powiodła się.\n";
            }
        } else if (choice == 2) {
            cout << "Podaj kwote do wypłaty (liczba całkowita, bez PLN): ";
            long long amount;
            if (!readLongLong(amount)) {
                cerr << "Niepoprawne dane wejsciowe. Oczekiwano liczby calkowitej.\n";
                continue;
            }
            if (!dispenser.withdrawAmount(amount, dbPath, logPath)) {
                cerr << "Operacja wypłaty nie powiodła się.\n";
            }
        } else if (choice == 3) {
            dispenser.printStorage();
        } else if (choice == 4) {
            cout << "Koniec programu.\n";
            break;
        } else {
            cerr << "Niepoprawna opcja. Wybierz 1-4.\n";
        }
    }

    return 0;
}
