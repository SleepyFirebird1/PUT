#ifndef CASHSTORAGE_H
#define CASHSTORAGE_H

#include <vector>
#include <string>

class CashStorage {
protected:
    std::vector<int> denominations; 
    std::vector<long long> quantities;

    // tworzy katalog rodzica pliku, jeśli nie istnieje
    bool ensureParentDir(const std::string &path);

public:
    CashStorage();
    virtual ~CashStorage() = default;

    // Wczytuje plik w formacie:
    // <nominał> <ilość>
    bool loadFromFile(const std::string &path);

    // Zapisuje aktualne stany do pliku (nadpisuje)
    bool saveToFile(const std::string &path);

    const std::vector<int>& getDenominations() const;
    const std::vector<long long>& getQuantities() const;

    // Dodaje (lub odejmuje) banknoty danego typu (wg indeksu w wektorze)
    void addNotes(size_t index, long long count);

    void printStorage() const;
};

#endif // CASHSTORAGE_H
