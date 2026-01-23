#ifndef CASHMACHINE_H
#define CASHMACHINE_H

#include "CashStorage.h"
#include "ReceiptStrategy.h"
#include <string>
#include <vector>

class CashDispenser : public CashStorage {
protected:
    IReceiptStrategy* receiptStrategy = nullptr;

    bool appendLog(const std::string &logPath, const std::string &operation, const std::vector<long long> &changed, long long totalAmount);

public:
    CashDispenser() = default;
    
    // Ustawianie strategii
    void setReceiptStrategy(IReceiptStrategy* strategy);

    // Drukowanie paragonu
    bool printReceipt(const std::string &filePath, const std::string &operation, long long totalAmount, std::string accountId);

    // Metody operacyjne
    bool depositAmount(long long amount, long long number, const std::string &dbPath, const std::string &logPath);
    bool withdrawAmount(long long amount, const std::string &dbPath, const std::string &logPath);
    bool canDispense(long long amount);
};

// Pomocnicza funkcja (moze byc w util, ale byla tu)
bool readLongLong(long long &out);

#endif // CASHMACHINE_H
