#include "ReceiptStrategy.h"
#include <fstream>
#include <ctime>

using namespace std;

void StandardReceipt::generate(const string &filePath, const string &operation, 
              long long totalAmount, string accountId) {
    ofstream out(filePath, ios::trunc);
    if (out.is_open()) {
        time_t t = time(nullptr);
        char buf[64];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t));

        out << "=== " << buf << " ===\n";
        out << "Konto: " << accountId << '\n';
        out << "Operacja: " << operation << '\n';
        out << "Kwota: " << totalAmount << " PLN\n";
        out << '\n';
        out.close();
    }
}

void PrivacyReceipt::generate(const string &filePath, const string &operation, 
              long long totalAmount, string accountId) {
    ofstream out(filePath, ios::trunc);
    if (out.is_open()) {
        time_t t = time(nullptr);
        char buf[64];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t));

        string masked = (accountId.length() > 4) ? "****" + accountId.substr(accountId.length() - 4) : "****";

        out << "=== " << buf << " ===\n";
        out << "Konto: " << masked << " (RODO)\n";
        out << "Operacja: " << operation << '\n';
        out << "Kwota: " << totalAmount << " PLN\n";
        out << "Status: ZATWIERDZONO\n";
        out << '\n';
        out.close();
    }
}
