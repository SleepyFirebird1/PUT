#ifndef BACKEND_H
#define BACKEND_H

// --- Zintegrowane zależności ze wszystkich plików ---
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <stdexcept>
#include <sstream>
#include <ctime>
#include <limits>
#include <chrono>

// --- Zależność od JSON (musi być dostępna) ---
#include "json.hpp" // Upewnij się, że masz ten plik!

// --- Definicje globalne i using ---
using namespace std;
using json = nlohmann::json;
namespace fs = filesystem;

// Ścieżki (można je przenieść do konfiguracji)
const string dbPath = "DataBase/CashStorage.txt";
const string logPath = "DataBase/CashStorage_log.txt";
const string receiptPath = "Receipt.txt";
const string bankdbPath = "DataBase/BankDatabase.json";


// --- Klasa CashStorage (z CashStorage.cpp) ---
class CashStorage {
protected:
    vector<int> denominations{500, 200, 100, 50}; // zawsze malejąco
    vector<long long> quantities{0,0,0,0};

    bool ensureParentDir(const string &path) {
        try {
            fs::path p(path);
            fs::path parent = p.parent_path();
            if (!parent.empty() && !fs::exists(parent)) {
                fs::create_directories(parent);
            }
            return true;
        } catch (...) {
            return false;
        }
    }

public:
    CashStorage() = default;
    virtual ~CashStorage() = default;

    bool loadFromFile(const string &path) {
        ifstream in(path);
        if (!in.is_open()) {
            cerr << "Nie mozna otworzyc pliku: " << path << " (utworzę plik z zerami)." << endl;
            if (!ensureParentDir(path) || !saveToFile(path)) {
                cerr << "Nie udalo sie utworzyc pliku: " << path << endl;
                return false;
            }
            return true;
        }
        fill(quantities.begin(), quantities.end(), 0);
        int denom;
        long long qty;
        while (in >> denom >> qty) {
            auto it = find(denominations.begin(), denominations.end(), denom);
            if (it != denominations.end()) {
                size_t idx = distance(denominations.begin(), it);
                if (qty < 0) {
                    cerr << "Ujemna ilosc w pliku dla nominału " << denom << " - ustawiam 0." << endl;
                    quantities[idx] = 0;
                } else {
                    quantities[idx] = qty;
                }
            } else {
                cerr << "Ostrzeżenie: nieznany nominał w pliku: " << denom << " - pomijam." << endl;
            }
        }
        if (in.bad()) {
            cerr << "Blad podczas czytania pliku " << path << endl;
            return false;
        }
        in.close();
        return true;
    }

    bool saveToFile(const string &path) {
        if (!ensureParentDir(path)) {
            cerr << "Nie mozna utworzyc katalogu dla pliku: " << path << endl;
            return false;
        }
        ofstream out(path);
        if (!out.is_open()) {
            cerr << "Nie mozna otworzyc pliku do zapisu: " << path << endl;
            return false;
        }
        for (size_t i = 0; i < denominations.size(); ++i) {
            out << denominations[i] << ' ' << quantities[i] << '\n';
        }
        out.close();
        return true;
    }

    const vector<int>& getDenominations() const { return denominations; }
    const vector<long long>& getQuantities() const { return quantities; }

    void addNotes(size_t index, long long count) {
        if (index >= quantities.size()) throw out_of_range("Index poza zasiegiem");
        long long newVal = quantities[index] + count;
        if (newVal < 0) {
            throw runtime_error("Operacja prowadzi do ujemnej liczby banknotow dla nominalu " + to_string(denominations[index]));
        }
        quantities[index] = newVal;
    }

    void printStorage() const {
        cout << "Aktualny stan magazynu banknotow:\n";
        for (size_t i = 0; i < denominations.size(); ++i) {
            cout << denominations[i] << " : " << quantities[i] << '\n';
        }
    }
};

// --- Klasa BankDatabaseHandler (z DatabaseHandler.cpp) ---
class BankDatabaseHandler {
public:
    json db;

    bool loadData(const string &filename) {
        ifstream file(filename);
        if (file.is_open()) {
            try {
                file >> db;
            } catch (json::parse_error& e) {
                cerr << "Blad parsowania JSON: " << e.what() << "\nTworze nowa, pusta baze.\n";
                db = json::object(); 
            }
            file.close();
            return true;
        } else {
            cerr << "Błąd otwierania pliku, wczytywanie. Tworze nowy plik.\n";
            db = json::object(); 
            return true; 
        }
    }

    bool saveData(const string &filename) {
        ofstream file(filename);
        if (file.is_open()) {
            file << db.dump(4);
            file.close();
            return true;
        } else {
            cerr << "Błąd otwierania pliku, zapisywanie\n";
            return false;
        }
    }

    bool addData(const string &accountId, const string &cardNumber, const string &pin, const long long &balance) {
        json account = {
            {"accountId", accountId},
            {"cardNumber", cardNumber},
            {"pin", pin},
            {"balance", balance},
            {"isBlocked", false} 
        };
        db[cardNumber] = account;
        return true;
    }

    bool deleteData(const string &cardNumber) {
        if (db.contains(cardNumber)) {
            db.erase(cardNumber);
            return true;
        } else {
            cerr << "Błąd numeru karty\n";
            return false;
        }
    }

    long long getBalance(const string &cardNumber) {
        if (db.contains(cardNumber)) {
            return db[cardNumber]["balance"];
        } else {
            cerr << "Błąd numeru karty\n";
            return -1;
        }
    }

    bool changeBalance(const string &cardNumber, long long amount) {
        if (db.contains(cardNumber)) {
            long long currentBalance = db[cardNumber]["balance"]; // Zmieniono na long long
            if (amount < 0 && -amount <= currentBalance) {
                db[cardNumber]["balance"] = currentBalance + amount;
                return true;
            } else if (amount >= 0) {
                db[cardNumber]["balance"] = currentBalance + amount;
                return true;
            } else {
                cerr << "Nie wystarczająca ilość środkow\n";
                return false;
            }
        } else {
            cerr << "Błąd numeru karty\n";
            return false;
        }
    }

    bool changePin(const string &cardNumber, const string &newPin) {
        if (db.contains(cardNumber)) {
            db[cardNumber]["pin"] = newPin;
            return true;
        } else {
            cerr << "Błąd numeru karty\n";
            return false;
        }
    }

    bool changeBlockStatus(const string &cardNumber, bool shouldBeBlocked) {
        if (db.contains(cardNumber)) {
            db[cardNumber]["isBlocked"] = shouldBeBlocked;
            return true;
        } else {
            cerr << "Błąd numeru karty\n";
            return false;
        }
    }
    
    int checkPin(const string &cardNumber, const string &pinCheck) {
        if (db.contains(cardNumber)) {
            if (db[cardNumber].value("isBlocked", false) == true) {
                return -2; // Konto jest zablokowane
            }
            if (db[cardNumber]["pin"] == pinCheck) {
                return 1; // PIN poprawny
            } else {
                return 0; // Zły PIN
            }
        } else {
            cerr << "Błąd numeru karty\n";
            return -1; // Nie ma takiego konta
        }
    }
    
    bool existenceOfAccount(const string &cardNumber) {
        return db.contains(cardNumber);
    }
};

// --- Funkcja pomocnicza (z CashMachine.cpp) ---
inline bool readLongLong(const string& s, long long &out) {
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


// --- Klasa CashDispenser (z CashMachine.cpp) ---
class CashDispenser : public CashStorage {
public:
    CashDispenser() = default;

    // Zmodyfikowałem, aby zwracały string z błędem zamiast pisać do cerr
    // W GUI chcemy wyświetlać błędy na ekranie, a nie w konsoli.
    string depositAmount(long long amount, long long number, const string &dbPath, const string &logPath) {
        if (amount <= 0) return "Blad: kwota do depozytu musi byc dodatnia.";
        if (number <= 0) return "Blad: liczba banknotow musi byc dodatnia.";

        const long long MAX_BANKNOTS = 200;
        const long long MAX_AMOUNT = 100000;
        if (amount > MAX_AMOUNT) return "Blad: przekroczono maksymalna kwote (" + to_string(MAX_AMOUNT) + ")";
        if (number > MAX_BANKNOTS) return "Blad: przekroczono maksymalna ilosc banknotow (" + to_string(MAX_BANKNOTS) + ")";

        int smallest = denominations.back();
        if (amount % smallest != 0) return "Blad: kwota musi byc podzielna przez najmniejszy nominał (" + to_string(smallest) + ").";

        int unit = smallest;
        int m = denominations.size();
        vector<int> denomUnits(m);
        for (size_t i = 0; i < m; ++i) denomUnits[i] = denominations[i] / unit;

        int amountUnits = static_cast<int>(amount / unit);
        int num = static_cast<int>(number);

        if (num > amountUnits) return "Blad: podana liczba banknotow jest za duza.";
        int maxUnitPerNote = denomUnits.front();
        if ((long long)num * maxUnitPerNote < amountUnits) return "Blad: podana liczba banknotow jest za mala do uzyskania tej kwoty.";

        vector<vector<int>> prev(num + 1, vector<int>(amountUnits + 1, -1));
        prev[0][0] = -2;

        for (int cnt = 1; cnt <= num; ++cnt) {
            for (int u = 0; u <= amountUnits; ++u) {
                if (prev[cnt][u] != -1) continue;
                for (size_t j = 0; j < (size_t)m; ++j) {
                    int d = denomUnits[j];
                    if (u - d < 0) continue;
                    if (prev[cnt - 1][u - d] != -1) {
                        prev[cnt][u] = static_cast<int>(j);
                        break;
                    }
                }
            }
        }

        if (prev[num][amountUnits] == -1) {
            return "Nie znaleziono kombinacji " + to_string(number) + " banknotow, ktore daja kwote " + to_string(amount) + " PLN.";
        }

        vector<long long> toAdd(m, 0);
        int curU = amountUnits;
        int curCnt = num;
        while (curCnt > 0) {
            int j = prev[curCnt][curU];
            if (j < 0 || j >= m) return "Blad odtwarzania rozwiazania.";
            toAdd[j] += 1;
            curU -= denomUnits[j];
            curCnt -= 1;
        }

        try {
            for (size_t i = 0; i < toAdd.size(); ++i) {
                if (toAdd[i] != 0) addNotes(i, toAdd[i]);
            }
        } catch (const exception &e) {
            return "Blad przy aktualizacji stanu: " + string(e.what());
        }

        if (!saveToFile(dbPath)) return "Blad: nie udalo sie zapisac stanu do bazy.";
        if (!appendLog(logPath, "WPŁATA", toAdd, amount)) cerr << "Uwaga: nie udalo sie zapisac logu.\n";

        // GUI obsłuży wyświetlanie sukcesu
        return "OK";
    }

    string withdrawAmount(long long amount, const string &dbPath, const string &logPath) {
        if (amount <= 0) return "Blad: kwota do wyplaty musi byc dodatnia.";

        int smallest = denominations.back();
        if (amount % smallest != 0) return "Blad: kwota musi byc podzielna przez najmniejszy nominał (" + to_string(smallest) + ").";

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
            return "Nie mozna wydac żądanej kwoty z dostępnych banknotów.";
        }

        try {
            for (size_t i = 0; i < toRemove.size(); ++i) {
                if (toRemove[i] != 0) addNotes(i, -toRemove[i]);
            }
        } catch (const exception &e) {
            return "Blad przy aktualizacji stanu: " + string(e.what());
        }

        if (!saveToFile(dbPath)) return "Blad: nie udalo sie zapisac stanu do bazy.";
        if (!appendLog(logPath, "WYPŁATA", toRemove, amount)) cerr << "Uwaga: nie udalo sie zapisac logu.\n";

        return "OK";
    }

    bool canDispense(long long amount) {
        if (amount <= 0) return false;
        vector<long long> temp_quantities = quantities;
        long long remaining = amount;
        for (size_t i = 0; i < denominations.size(); ++i) {
            if (remaining == 0) break;
            if (denominations[i] > remaining) continue;
            long long needed = remaining / denominations[i];
            long long can_give = min(needed, temp_quantities[i]);
            if (can_give > 0) {
                remaining -= can_give * denominations[i];
            }
        }
        return remaining == 0;
    }

    bool printReceipt(const string &filePath, const string &operation, long long totalAmount, string accountId) {
        if (!ensureParentDir(filePath)) return false;
        ofstream out(filePath, ios::trunc);
        if (!out.is_open()) return false;

        time_t t = time(nullptr);
        char buf[64];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t));

        out << "=== " << buf << " ===\n";
        out << "Konto: " << accountId << '\n';
        out << "Operacja: " << operation << '\n';
        out << "Kwota: " << totalAmount << " PLN\n";
        out << '\n';
        out.close();
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

// --- Klasa Transaction (z Account.cpp) ---
class Transaction : public CashDispenser, public BankDatabaseHandler {
public:
    string deposit(const string &cardNumber, const long long &amount, const long long &number) {
        if (!loadFromFile(dbPath)) {
            cerr << "Blad: nie mozna wczytac bazy magazynu.\n";
        }
        if (!loadData(bankdbPath)) return "Błąd wczytania bazy banku";
        
        string depositResult = depositAmount(amount, number, dbPath, logPath);
        if (depositResult != "OK") {
            return depositResult; // Zwróć błąd z depositAmount
        }
        
        if (!changeBalance(cardNumber, amount)) {
            // To jest trudny stan - pieniądze przyjęte, ale saldo nie zaktualizowane.
            // Wymagałoby to transakcji bazodanowych. Na razie zwracamy błąd.
            return "Błąd aktualizacji salda po wpłacie.";
        }
        if (!saveData(bankdbPath)) return "Błąd zapisu bazy banku";
        return "OK";
    }

    string canWithdrawal(const long long &amount, const string &cardNumber) {
        if (!canDispense(amount)) return "Bankomat nie może wydać tej kwoty.";
        if (getBalance(cardNumber) < amount) return "Niewystarczające środki na koncie.";
        return "OK";
    }
  
    string withdrawal(const string &cardNumber, const long long &amount) {
        if (!loadFromFile(dbPath)) {
            cerr << "Blad: nie mozna wczytac bazy magazynu.\n";
        }
        if (!loadData(bankdbPath)) return "Błąd wczytania bazy banku";

        string canWithdrawResult = canWithdrawal(amount, cardNumber);
        if (canWithdrawResult != "OK") {
            return canWithdrawResult;
        }

        string withdrawResult = withdrawAmount(amount, dbPath, logPath);
        if (withdrawResult != "OK") {
            return withdrawResult;
        }

        if (!changeBalance(cardNumber, -amount)) {
            // Znowu trudny stan - wydano pieniądze, ale nie zaktualizowano salda.
            return "Błąd aktualizacji salda po wypłacie.";
        }
        if (!saveData(bankdbPath)) return "Błąd zapisu bazy banku";
        return "OK";     
    }
};


#endif // BACKEND_H