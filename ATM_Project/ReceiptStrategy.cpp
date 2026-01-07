#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

using namespace std;

// --- INTERFEJS STRATEGII (Polimorfizm) ---
class IReceiptStrategy
{
public:
    virtual ~IReceiptStrategy() = default;

    virtual void generate(const string &filePath, const string &operation,
                          long long totalAmount, string accountId) = 0;
};

// ---  STRATEGIA 1: Standardowy Paragon ---
class StandardReceipt : public IReceiptStrategy
{
public:
    void generate(const string &filePath, const string &operation,
                  long long totalAmount, string accountId) override
    {
        ofstream out(filePath, ios::trunc);
        if (out.is_open())
        {
            time_t t = time(nullptr);
            char buf[64];
            strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t));

            out << "=== BANKOMAT - POTWIERDZENIE ===\n";
            out << "Data: " << buf << "\n";
            out << "Konto: " << accountId << "\n";
            out << "Operacja: " << operation << "\n";
            out << "Kwota: " << totalAmount << " PLN\n";
            out << "================================\n";
            out.close();
        }
    }
};

// ---  STRATEGIA 2: Bezpieczny Paragon (Privacy Mode) ---
class PrivacyReceipt : public IReceiptStrategy
{
public:
    void generate(const string &filePath, const string &operation,
                  long long totalAmount, string accountId) override
    {
        ofstream out(filePath, ios::trunc);
        if (out.is_open())
        {
            time_t t = time(nullptr);
            char buf[64];
            strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t));

            // Maskowanie numeru konta (zostawiamy 4 ostatnie cyfry)
            string maskedAccount = "****";
            if (accountId.length() > 4)
            {
                maskedAccount += accountId.substr(accountId.length() - 4);
            }
            else
            {
                maskedAccount = "****";
            }

            out << "=== BANKOMAT - RODO/PRIVACY ===\n";
            out << "Data: " << buf << "\n";
            out << "Konto: " << maskedAccount << "\n"; // ZMASKOWANY NUMER
            out << "Operacja: " << operation << "\n";
            out << "Kwota: " << totalAmount << " PLN\n";
            out << "Status: ZATWIERDZONO\n";
            out << "===============================\n";
            out.close();
        }
    }
};