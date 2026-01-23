#include "DatabaseHandler.h"
#include <iostream>
#include <fstream>

using namespace std;
using json = nlohmann::json;

bool BankDatabaseHandler::loadData(const string &filename) {
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

bool BankDatabaseHandler::saveData(const string &filename) {
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

bool BankDatabaseHandler::addData(const string &accountId, const string &cardNumber, const string &pin, const long long &balance) {
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

bool BankDatabaseHandler::deleteData(const string &cardNumber) {
    if (db.contains(cardNumber)) {
        db.erase(cardNumber);
        return true;
    } else {
        cerr << "Błąd numeru karty\n";
        return false;
    }
}

long long BankDatabaseHandler::getBalance(const string &cardNumber) {
    if (db.contains(cardNumber)) {
        return db[cardNumber]["balance"];
    } else {
        cerr << "Błąd numeru karty\n";
        return -1;
    }
}

bool BankDatabaseHandler::changeBalance(const string &cardNumber, long long amount) {
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

bool BankDatabaseHandler::changePin(const string &cardNumber, const string &newPin) {
    if (db.contains(cardNumber)) {
        db[cardNumber]["pin"] = newPin;
        return true;
    } else {
        cerr << "Błąd numeru karty\n";
        return false;
    }
}

bool BankDatabaseHandler::changeBlockStatus(const string &cardNumber, bool shouldBeBlocked) {
    if (db.contains(cardNumber)) {
        db[cardNumber]["isBlocked"] = shouldBeBlocked;
        return true;
    } else {
        cerr << "Błąd numeru karty\n";
        return false;
    }
}

int BankDatabaseHandler::checkPin(const string &cardNumber, const string &pinCheck) {
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

bool BankDatabaseHandler::existenceOfAccount(const string &cardNumber) {
    return db.contains(cardNumber);
}
