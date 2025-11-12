#include <iostream>
#include <string>
#include "DatabaseHandler.cpp"
#include "CashMachine.cpp"

using namespace std;



const string dbPath = "DataBase/CashStorage.txt";
const string logPath = "DataBase/CashStorage_log.txt";
const string bankdbPath = "DataBase/BankDatabase.json";

class Service : public BankDatabaseHandler, public CashDispenser {
public:
    bool service_collect(const long long &amount) {
        if (!loadFromFile(dbPath)){
            cerr << "Nie mozna wczytac bazy, korzystam ze stanu zerowego\n";
        }
        if (!canDispense(amount)) {
            cerr << "Nie mozna wypłacić takiej kwoty\n";
            return false;
        }
        if (!withdrawAmount(amount, dbPath, logPath)){
            return false;
        }
        return true;
    }

    bool service_deposit(const long long &amount, const long long &number) {
        if (!loadFromFile(dbPath)){
            cerr << "Nie mozna wczytac bazy, korzystam ze stanu zerowego\n";
        }
        if (!depositAmount(amount, number, dbPath, logPath)) {
            return false;
        }
        return true;
    }

    bool unblock_card(const string &cardNumber){
        if (!loadData(bankdbPath)) {
            return false;
        }
        if (!changeBlockStatus(cardNumber, false)) {
            return false;
        }
        if (!saveData(bankdbPath)) {
            return false;
        }
        return true;
    }
};