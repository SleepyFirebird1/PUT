#ifndef CASHSTORAGE_H
#define CASHSTORAGE_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <stdexcept>

namespace fs = std::filesystem;

class CashStorage {
protected:
    std::vector<int> denominations{500, 200, 100, 50}; // zawsze malejąco
    std::vector<long long> quantities{0,0,0,0};

    // tworzy katalog rodzica pliku, jeśli nie istnieje
    bool ensureParentDir(const std::string &path) {
        try {
            fs::path p(path);
            fs::path parent = p.parent_path();
            if (!parent.empty() && !fs::exists(parent)) {
                fs::create_directories(parent);
            }
            return true;
        } catch (...) {
            return false;
        }
    }

public:
    CashStorage() = default;
    virtual ~CashStorage() = default;

    // Wczytuje plik w formacie:
    // <nominał> <ilość>
    bool loadFromFile(const std::string &path) {
        std::ifstream in(path);
        if (!in.is_open()) {
            std::cerr << "Nie mozna otworzyc pliku: " << path << " (utworzę plik z zerami)." << std::endl;
            if (!ensureParentDir(path) || !saveToFile(path)) {
                std::cerr << "Nie udalo sie utworzyc pliku: " << path << std::endl;
                return false;
            }
            return true;
        }

        std::fill(quantities.begin(), quantities.end(), 0);

        int denom;
        long long qty;
        while (in >> denom >> qty) {
            auto it = std::find(denominations.begin(), denominations.end(), denom);
            if (it != denominations.end()) {
                size_t idx = std::distance(denominations.begin(), it);
                if (qty < 0) {
                    std::cerr << "Ujemna ilosc w pliku dla nominału " << denom << " - ustawiam 0." << std::endl;
                    quantities[idx] = 0;
                } else {
                    quantities[idx] = qty;
                }
            } else {
                std::cerr << "Ostrzeżenie: nieznany nominał w pliku: " << denom << " - pomijam." << std::endl;
            }
        }

        if (in.bad()) {
            std::cerr << "Blad podczas czytania pliku " << path << std::endl;
            return false;
        }

        in.close();
        return true;
    }

    // Zapisuje aktualne stany do pliku (nadpisuje)
    bool saveToFile(const std::string &path) {
        if (!ensureParentDir(path)) {
            std::cerr << "Nie mozna utworzyc katalogu dla pliku: " << path << std::endl;
            return false;
        }
        std::ofstream out(path);
        if (!out.is_open()) {
            std::cerr << "Nie mozna otworzyc pliku do zapisu: " << path << std::endl;
            return false;
        }

        for (size_t i = 0; i < denominations.size(); ++i) {
            out << denominations[i] << ' ' << quantities[i] << '\n';
        }
        out.close();
        return true;
    }

    const std::vector<int>& getDenominations() const { return denominations; }
    const std::vector<long long>& getQuantities() const { return quantities; }

    // Dodaje / odejmuje banknoty (count może być ujemny)
    void addNotes(size_t index, long long count) {
        if (index >= quantities.size()) throw std::out_of_range("Index poza zasiegiem");
        long long newVal = quantities[index] + count;
        if (newVal < 0) {
            throw std::runtime_error("Operacja prowadzi do ujemnej liczby banknotow dla nominalu " + std::to_string(denominations[index]));
        }
        quantities[index] = newVal;
    }

    void printStorage() const {
        std::cout << "Aktualny stan magazynu banknotow:\n";
        for (size_t i = 0; i < denominations.size(); ++i) {
            std::cout << denominations[i] << " : " << quantities[i] << '\n';
        }
    }
};

#endif // CASHSTORAGE_H
