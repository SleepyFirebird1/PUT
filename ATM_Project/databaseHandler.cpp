#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

class BankDatabaseHandler {
public:
    json db;

    bool loadData(const string &filename) {
        ifstream file(filename);
        if (file.is_open()) {
            file >> db;
            return true;
        } else {
            cerr << "Błąd otwierania pliku, wczytywanie\n";
            return false;
        }
    }

    bool saveData(const string &filename) {
        ofstream file(filename);
        if (file.is_open()) {
            file << db.dump(4);
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
            {"balance", balance}
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
            double currentBalance = db[cardNumber]["balance"];
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

    int checkPin(const string &cardNumber, const string &pinCheck) {
        if (db.contains(cardNumber)) {
            if (db[cardNumber]["pin"] == pinCheck) {
                return 1;
            } else {
                return 0;
            }
        } else {
            cerr << "Błąd numeru karty\n";
            return -1;
        }
    }
    bool existenceOfAccount(const string &cardNumber) {
        return db.contains(cardNumber);
    }
};