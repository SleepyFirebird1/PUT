#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class BankDatabaseHandler {
public:
    json db;

    void loadData(const std::string &filename) {
        std::ifstream file(filename);
        if (file.is_open()) {
            file >> db;
        } else {
            std::cerr << "Błąd otwierania pliku, wczytywanie" << std::endl;
        }
    }

    void saveData(const std::string &filename) {
        std::ofstream file(filename);
        if (file.is_open()) {
            file << db.dump(4);
        } else {
            std::cerr << "Błąd otwierania pliku, zapisywanie" << std::endl;
        }
    }


    void addData(const std::string &accountId, const std::string &cardNumber, const std::string &pin, const double &balance) {
        json account = {
            {"accountId", accountId},
            {"cardNumber", cardNumber},
            {"pin", pin},
            {"balance", balance}
        };
        db[cardNumber] = account;
    }

    void deleteData(const std::string &cardNumber) {
        if (db.contains(cardNumber)) {
            db.erase(cardNumber);
        } else {
            std::cerr << "Błąd numeru karty" << std::endl;
        }
    }

    double getBalance(const std::string &cardNumber) {
        if (db.contains(cardNumber)) {
            return db[cardNumber]["balance"];
        } else {
            std::cerr << "Błąd numeru karty" << std::endl;
            return -1;
        }
    }

    void changeBalance(const std::string &cardNumber, double amount) {
        if (db.contains(cardNumber)) {
            double currentBalance = db[cardNumber]["balance"];
            if (amount < 0 && -amount <= currentBalance) {
                db[cardNumber]["balance"] = currentBalance + amount;
            } else if (amount >= 0) {
                db[cardNumber]["balance"] = currentBalance + amount;
            } else {
                std::cerr << "Nie wystarczająca ilość środkow" << std::endl;
            }
        } else {
            std::cerr << "Błąd numeru karty" << std::endl;
        }
    }

    void changePin(const std::string &cardNumber, const std::string &newPin) {
        if (db.contains(cardNumber)) {
            db[cardNumber]["pin"] = newPin;
        } else {
            std::cerr << "Błąd numeru karty" << std::endl;
        }
    }

    int checkPin(const std::string &cardNumber, const std::string &pinCheck) {
        if (db.contains(cardNumber)) {
            if (db[cardNumber]["pin"] == pinCheck) {
                return 1;
            } else {
                return 0;
            }
        } else {
            std::cerr << "Błąd numeru karty" << std::endl;
            return -1;
        }
    }
};