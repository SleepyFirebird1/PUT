#include "CashStorage.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <stdexcept>

using namespace std;
namespace fs = filesystem;

CashStorage::CashStorage() : denominations{500, 200, 100, 50}, quantities{0,0,0,0} {}

bool CashStorage::ensureParentDir(const string &path) {
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

bool CashStorage::loadFromFile(const string &path) {
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

bool CashStorage::saveToFile(const string &path) {
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

const vector<int>& CashStorage::getDenominations() const { return denominations; }
const vector<long long>& CashStorage::getQuantities() const { return quantities; }

void CashStorage::addNotes(size_t index, long long count) {
    if (index >= quantities.size()) {
        throw out_of_range("Nieprawidlowy indeks nominalu.");
    }
    if (quantities[index] + count < 0) {
        throw runtime_error("Nie mozna odjac wiecej banknotow niz jest w magazynie.");
    }
    quantities[index] += count;
}

void CashStorage::printStorage() const {
    cout << "Stan magazynu:" << endl;
    for (size_t i = 0; i < denominations.size(); ++i) {
        cout << denominations[i] << " PLN: " << quantities[i] << " szt." << endl;
    }
}
