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

    bool depositAmount(long long amount, const string &dbPath, const string &logPath) {
        if (amount <= 0) {
            cerr << "Blad: kwota do depozytu musi byc dodatnia.\n";
            return false;
        }

        int smallest = denominations.back();
        if (amount % smallest != 0) {
            cerr << "Blad: kwota musi byc podzielna przez najmniejszy nominał (" << smallest << ").\n";
            return false;
        }

        vector<long long> toAdd(denominations.size(), 0);
        long long remaining = amount;
        for (size_t i = 0; i < denominations.size(); ++i) {
            long long cnt = remaining / denominations[i];
            if (cnt > 0) {
                toAdd[i] = cnt;
                remaining -= cnt * denominations[i];
            }
        }

        if (remaining != 0) {
            cerr << "Nie mozna rozlozyc kwoty przy uzyciu dostepnych nominałów.\n";
            return false;
        }

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

        cout << "Przyjeto depozyt: " << amount << " PLN\n";
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
            long long can = std::min(want, quantities[i]);
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
        std::ofstream log(logPath, ios::app);
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
    std::string s;
    if (!(std::cin >> s)) return false;
    try {
        size_t pos;
        long long val = std::stoll(s, &pos);
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
            std::cin.clear();
            continue;
        }

        if (choice == 1) {
            cout << "Podaj kwote do wpłaty (liczba całkowita, bez PLN): ";
            long long amount;
            if (!readLongLong(amount)) {
                cerr << "Niepoprawne dane wejsciowe. Oczekiwano liczby calkowitej.\n";
                continue;
            }
            if (!dispenser.depositAmount(amount, dbPath, logPath)) {
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
