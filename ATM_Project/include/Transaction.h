#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "CashMachine.h"
#include "DatabaseHandler.h"
#include <string>

class Transaction : public CashDispenser, public BankDatabaseHandler {
public:
    bool deposit(const std::string &cardNumber, const long long &amount, const long long &number);
    bool canWithdrawal(const long long &amount, const std::string &cardNumber);
    bool withdrawal(const std::string &cardNumber, const long long &amount);
};

// Globalna/pomocnicza funkcja obsługi menu 
void showLoggedInMenu(std::string cardNumber);

#endif // TRANSACTION_H
