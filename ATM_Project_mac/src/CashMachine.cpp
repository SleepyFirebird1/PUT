#include "CashMachine.h"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <ctime>

using namespace std;

void CashDispenser::setReceiptStrategy(IReceiptStrategy* strategy) {
    receiptStrategy = strategy;
}

bool CashDispenser::printReceipt(const string &filePath, const string &operation, long long totalAmount, string accountId) {
    if (!ensureParentDir(filePath)) return false;
    
    if (receiptStrategy != nullptr) {
        receiptStrategy->generate(filePath, operation, totalAmount, accountId);
    } else {
        StandardReceipt defaultStrat;
        defaultStrat.generate(filePath, operation, totalAmount, accountId);
    }
    return true;
}

bool CashDispenser::appendLog(const string &logPath, const string &operation, const vector<long long> &changed, long long totalAmount)
{
    if (!ensureParentDir(logPath)) return false;
    ofstream log(logPath, ios::app);
    if (!log.is_open()) return false;

    time_t t = time(nullptr);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t));

    log << "=== " << buf << " ===\n";
    log << "Operacja: " << operation << '\n';
    log << "Kwota: " << totalAmount << " PLN\n";
    log << (operation == "WPŁATA" ? "Dodano:\n" : "Pobrano:\n");
    for (size_t i = 0; i < denominations.size(); ++i) {
        if (changed[i] != 0)
            log << denominations[i] << ' ' << changed[i] << '\n';
    }
    log << "Stan po operacji:\n";
    for (size_t i = 0; i < denominations.size(); ++i) {
        log << denominations[i] << ' ' << quantities[i] << '\n';
    }
    log << '\n';
    log.close();
    return true;
}

bool CashDispenser::canDispense(long long amount)
{
    if (amount <= 0) return false;

    vector<long long> temp_quantities = quantities;
    long long remaining = amount;

    for (size_t i = 0; i < denominations.size(); ++i) {
        if (remaining == 0) break;
        if (denominations[i] > remaining) continue;

        long long needed = remaining / denominations[i];
        long long can_give = min(needed, temp_quantities[i]);

        if (can_give > 0) {
            remaining -= can_give * denominations[i];
        }
    }

    return remaining == 0;
}

bool CashDispenser::withdrawAmount(long long amount, const string &dbPath, const string &logPath)
{
    if (amount <= 0) {
        cerr << "Blad: kwota do wyplaty musi byc dodatnia.\n";
        return false;
    }

    int smallest = denominations.back();
    if (amount % smallest != 0) {
        cerr << "Blad: kwota musi byc podzielna przez najmniejszy nominał (" << smallest << ").\n";
        return false;
    }

    vector<long long> toRemove(denominations.size(), 0);
    long long remaining = amount;

    for (size_t i = 0; i < denominations.size(); ++i) {
        long long want = remaining / denominations[i];
        long long can = min(want, quantities[i]);
        if (can > 0) {
            toRemove[i] = can;
            remaining -= can * denominations[i];
        }
    }

    if (remaining != 0) {
        cerr << "Nie mozna wydac żądanej kwoty z dostępnych banknotów.\n";
        return false;
    }

    try {
        for (size_t i = 0; i < toRemove.size(); ++i) {
            if (toRemove[i] != 0)
                addNotes(i, -toRemove[i]);
        }
    } catch (const exception &e) {
        cerr << "Blad przy aktualizacji stanu: " << e.what() << endl;
        return false;
    }

    if (!saveToFile(dbPath)) {
        cerr << "Blad: nie udalo sie zapisac stanu do bazy (" << dbPath << ").\n";
        return false;
    }

    if (!appendLog(logPath, "WYPŁATA", toRemove, amount)) {
        cerr << "Uwaga: nie udalo sie zapisac logu.\n";
    }

    cout << "Wydano: " << amount << " PLN\n";
    return true;
}

bool CashDispenser::depositAmount(long long amount, long long number, const string &dbPath, const string &logPath)
{
    if (amount <= 0) {
        cerr << "Blad: kwota do depozytu musi byc dodatnia.\n";
        return false;
    }
    if (number <= 0) {
        cerr << "Blad: liczba banknotow musi byc dodatnia.\n";
        return false;
    }

    const long long MAX_BANKNOTS = 200;
    const long long MAX_AMOUNT = 100000;
    if (amount > MAX_AMOUNT) {
        cerr << "Blad: przekroczono maksymalna kwote (" << MAX_AMOUNT << ")\n";
        return false;
    }
    if (number > MAX_BANKNOTS) {
        cerr << "Blad: przekroczono maksymalna ilosc banknotow (" << MAX_BANKNOTS << ")\n";
        return false;
    }

    int smallest = denominations.back();
    if (amount % smallest != 0) {
        cerr << "Blad: kwota musi byc podzielna przez najmniejszy nominał (" << smallest << ").\n";
        return false;
    }

    int unit = smallest; 
    int m = denominations.size();
    vector<int> denomUnits(m);
    for (size_t i = 0; i < (size_t)m; ++i)
        denomUnits[i] = denominations[i] / unit;

    int amountUnits = static_cast<int>(amount / unit);
    int num = static_cast<int>(number);

    if (num > amountUnits) {
        cerr << "Blad: podana liczba banknotow jest za duza - nawet wszystkie 50 PLN nie daja takiej liczby banknotow.\n";
        return false;
    }
    
    int maxUnitPerNote = denomUnits.front(); 
    if ((long long)num * maxUnitPerNote < amountUnits) {
        cerr << "Blad: podana liczba banknotow jest za mala do uzyskania tej kwoty (brakuje miejsca na nominały).\n";
        return false;
    }

    vector<vector<int>> prev(num + 1, vector<int>(amountUnits + 1, -1));
    prev[0][0] = -2;

    for (int cnt = 1; cnt <= num; ++cnt) {
        for (int u = 0; u <= amountUnits; ++u) {
            if (prev[cnt][u] != -1) continue; 
            for (size_t j = 0; j < (size_t)m; ++j) {
                int d = denomUnits[j];
                if (u - d < 0) continue;
                if (prev[cnt - 1][u - d] != -1) {
                    prev[cnt][u] = static_cast<int>(j);
                    break;                              
                }
            }
        }
    }

    if (prev[num][amountUnits] == -1) {
        cerr << "Nie znaleziono kombinacji " << number << " banknotow, ktore daja kwote " << amount << " PLN.\n";
        return false;
    }

    vector<long long> toAdd(m, 0);
    int curU = amountUnits;
    int curCnt = num;
    while (curCnt > 0) {
        int j = prev[curCnt][curU];
        if (j < 0 || j >= m) {
            cerr << "Blad odtwarzania rozwiazania.\n";
            return false;
        }
        toAdd[j] += 1;
        curU -= denomUnits[j];
        curCnt -= 1;
    }

    try {
        for (size_t i = 0; i < toAdd.size(); ++i) {
            if (toAdd[i] != 0)
                addNotes(i, toAdd[i]);
        }
    } catch (const exception &e) {
        cerr << "Blad przy aktualizacji stanu: " << e.what() << endl;
        return false;
    }

    if (!saveToFile(dbPath)) {
        cerr << "Blad: nie udalo sie zapisac stanu do bazy (" << dbPath << ").\n";
        return false;
    }

    if (!appendLog(logPath, "WPŁATA", toAdd, amount)) {
        cerr << "Uwaga: nie udalo sie zapisac logu.\n";
    }

    cout << "Przyjeto depozyt: " << amount << " PLN w " << number << " banknotach\n";
    return true;
}

bool readLongLong(long long &out)
{
    string s;
    if (!(cin >> s)) return false;
    try {
        size_t pos;
        long long val = stoll(s, &pos);
        if (pos != s.size()) return false;
        out = val;
        return true;
    } catch (...) {
        return false;
    }
}
