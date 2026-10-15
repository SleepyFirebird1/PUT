#ifndef DATABASEHANDLER_H
#define DATABASEHANDLER_H

#include <string>
#include <nlohmann/json.hpp>

class BankDatabaseHandler {
public:
    nlohmann::json db;

    bool loadData(const std::string &filename);
    bool saveData(const std::string &filename);
    
    bool addData(const std::string &accountId, const std::string &cardNumber, const std::string &pin, const long long &balance);
    
    bool deleteData(const std::string &cardNumber);
    
    long long getBalance(const std::string &cardNumber);
    
    bool changeBalance(const std::string &cardNumber, long long amount);
    
    bool changePin(const std::string &cardNumber, const std::string &newPin);

    bool changeBlockStatus(const std::string &cardNumber, bool shouldBeBlocked);  

    int checkPin(const std::string &cardNumber, const std::string &pinCheck);

    bool existenceOfAccount(const std::string &cardNumber);
};

#endif // DATABASEHANDLER_H
