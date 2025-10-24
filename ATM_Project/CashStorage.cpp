#ifndef CASHSTORAGE_H
#define CASHSTORAGE_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <stdexcept>

using namespace std;


namespace fs = filesystem;

class CashStorage {
protected:
    vector<int> denominations{500, 200, 100, 50}; // zawsze malejąco
    vector<long long> quantities{0,0,0,0};

    // tworzy katalog rodzica pliku, jeśli nie istnieje
    bool ensureParentDir(const string &path) {
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
    bool loadFromFile(const string &path) {
        ifstream in(path);
        if (!in.is_open()) {
            cerr << "Nie mozna otworzyc pliku: " << path << " (utworzę plik z zerami)." << endl;
            if (!ensureParentDir(path) || !saveToFile(path)) {
                cerr << "Nie udalo sie utworzyc pliku: " << path << endl;
                return false;
            }
            return true;
        }

        fill(quantities.begin(), quantities.end(), 0);

        int denom;
        long long qty;
        while (in >> denom >> qty) {
            auto it = find(denominations.begin(), denominations.end(), denom);
            if (it != denominations.end()) {
                size_t idx = distance(denominations.begin(), it);
                if (qty < 0) {
                    cerr << "Ujemna ilosc w pliku dla nominału " << denom << " - ustawiam 0." << endl;
                    quantities[idx] = 0;
                } else {
                    quantities[idx] = qty;
                }
            } else {
                cerr << "Ostrzeżenie: nieznany nominał w pliku: " << denom << " - pomijam." << endl;
            }
        }

        if (in.bad()) {
            cerr << "Blad podczas czytania pliku " << path << endl;
            return false;
        }

        in.close();
        return true;
    }

    // Zapisuje aktualne stany do pliku (nadpisuje)
    bool saveToFile(const string &path) {
        if (!ensureParentDir(path)) {
            cerr << "Nie mozna utworzyc katalogu dla pliku: " << path << endl;
            return false;
        }
        ofstream out(path);
        if (!out.is_open()) {
            cerr << "Nie mozna otworzyc pliku do zapisu: " << path << endl;
            return false;
        }

        for (size_t i = 0; i < denominations.size(); ++i) {
            out << denominations[i] << ' ' << quantities[i] << '\n';
        }
        out.close();
        return true;
    }

    const vector<int>& getDenominations() const { return denominations; }
    const vector<long long>& getQuantities() const { return quantities; }

    // Dodaje / odejmuje banknoty (count może być ujemny)
    void addNotes(size_t index, long long count) {
        if (index >= quantities.size()) throw out_of_range("Index poza zasiegiem");
        long long newVal = quantities[index] + count;
        if (newVal < 0) {
            throw runtime_error("Operacja prowadzi do ujemnej liczby banknotow dla nominalu " + to_string(denominations[index]));
        }
        quantities[index] = newVal;
    }

    void printStorage() const {
        cout << "Aktualny stan magazynu banknotow:\n";
        for (size_t i = 0; i < denominations.size(); ++i) {
            cout << denominations[i] << " : " << quantities[i] << '\n';
        }
    }
};

#endif // CASHSTORAGE_H
