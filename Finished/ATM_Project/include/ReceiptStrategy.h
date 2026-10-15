#ifndef RECEIPTSTRATEGY_H
#define RECEIPTSTRATEGY_H

#include <string>

class IReceiptStrategy {
public:
    virtual ~IReceiptStrategy() = default;
    virtual void generate(const std::string &filePath, const std::string &operation, 
                          long long totalAmount, std::string accountId) = 0;
};

class StandardReceipt : public IReceiptStrategy {
public:
    void generate(const std::string &filePath, const std::string &operation, 
                  long long totalAmount, std::string accountId) override;
};

class PrivacyReceipt : public IReceiptStrategy {
public:
    void generate(const std::string &filePath, const std::string &operation, 
                  long long totalAmount, std::string accountId) override;
};

#endif // RECEIPTSTRATEGY_H
