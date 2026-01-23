#include "Transaction.h"
#include "PathResolver.h"
#include <iostream>
#include <string>
#include <chrono>
#include <limits>

using namespace std;

const string dbPath() { return resolvePath("DataBase/CashStorage.txt"); }
const string logPath() { return resolvePath("DataBase/CashStorage_log.txt"); }
const string receiptPath() { return "Receipt.txt"; } 
const string bankdbPath() { return resolvePath("DataBase/BankDatabase.json"); }

bool Transaction::deposit(const string &cardNumber, const long long &amount, const long long &number) {
    if (!loadFromFile(dbPath())) {
    cerr << "Blad: nie mozna wczytac bazy. Sprawdz uprawnienia katalogu DataBase.\n";
    }
    if (!loadData(bankdbPath())) {
        return false;
    }
    if (!depositAmount(amount, number, dbPath(), logPath())) {
        return false;
    }
    if (!changeBalance(cardNumber, amount)) {
        return false;
    }
    if (!saveData(bankdbPath())) {
        return false;
    }
    return true;
}

bool Transaction::canWithdrawal(const long long &amount, const string &cardNumber) {
    return (canDispense(amount) && getBalance(cardNumber) >= amount);
}

bool Transaction::withdrawal(const string &cardNumber, const long long &amount) {
    if (!loadFromFile(dbPath())) {
        cerr << "Blad: nie mozna wczytac bazy. Sprawdz uprawnienia katalogu DataBase.\n";
    }
    if (!loadData(bankdbPath())) {
        return false;
    }
    if (!canWithdrawal(amount, cardNumber)) {
        return false;
    }
    if (!withdrawAmount(amount, dbPath(), logPath())) {
        cerr << "Operacja wypłaty nie powiodła się.\n";
        return false;
    }
    if (!changeBalance(cardNumber, -amount)) {
        return false;
    }
    if (!saveData(bankdbPath())) {
        return false;
    }
    return true;     
}

// Funkcja obsługująca menu dla zalogowanego użytkownika
void showLoggedInMenu(string cardNumber) {
    Transaction transaction; 
    BankDatabaseHandler dbHandler;
    
    auto loginTime = chrono::steady_clock::now();
    const int SESSION_TIMEOUT_MINUTES = 10;
    const long long TIMEOUT_SECONDS = SESSION_TIMEOUT_MINUTES * 60;

    int choice;
    while (true) {
        auto currentTime = chrono::steady_clock::now();
        long long elapsedSeconds = chrono::duration_cast<chrono::seconds>(currentTime - loginTime).count();
        
        if (elapsedSeconds >= TIMEOUT_SECONDS) {
            cout << "\n*** SESJA WYGASŁA ***\n";
            return;
        }
        
        long long remainingSeconds = TIMEOUT_SECONDS - elapsedSeconds;
        cout << "\n--- Jesteś zalogowany jako: " << cardNumber << " ---\n";
        cout << "--- Pozostały czas sesji: " << remainingSeconds / 60 << ":" << (remainingSeconds % 60 < 10 ? "0" : "") << (remainingSeconds % 60) << " ---\n";
        cout << "1. Sprawdź saldo\n";
        cout << "2. Wpłata\n";
        cout << "3. Wypłata\n";
        cout << "4. Zablokuj kartę\n";
        cout << "5. Usuń konto\n";
        cout << "6. Wyloguj\n";
        cout << "Wybór: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cerr << "Niepoprawny wybór. Spróbuj ponownie.\n";
            continue;
        }

        if (choice == 1) { 
            if (transaction.loadData(bankdbPath())) {
                long long balance = transaction.getBalance(cardNumber);
                if (balance != -1) {
                    cout << "Aktualny balans: " << balance << " PLN\n";
                }
            } else {
                cerr << "Błąd wczytywania bazy danych\n";
            }
        } 
        else if (choice == 2) { 
            cout << "Podaj kwote do wpłaty (maks 100 000 PLN): ";
            long long amount;
            if (!readLongLong(amount) || amount > 100000 || amount <= 0) {
                cerr << "Niepoprawna kwota.\n"; continue;
            }
            cout << "Podaj liczbe banknotów (max 200szt): ";
            long long number;
            if (!readLongLong(number) || number > 200 || number <= 0) {
                cerr << "Niepoprawna liczba banknotów.\n"; continue;
            }
            
            if (transaction.deposit(cardNumber, amount, number)) {
                cout << "Czy wydrukować potwierdzenie? (T): ";
                string printReceipt;
                cin >> printReceipt;
                if (printReceipt == "T" || printReceipt == "t") {
                        transaction.printReceipt(receiptPath(), "WPŁATA", amount,cardNumber);
                        cout << "Wydrukowano potwierdzenie wpłaty.\n";
                    }
                cout << "Operacja wpłaty zakończyła się pomyślnie.\n";
            } else {
                cout << "Operacja wpłaty nie powiodła się.\n";
            }
        } 
        else if (choice == 3) { 
            cout << "Podaj kwote do wypłaty (maks 100 000 PLN): ";
            long long amount;
            if (!readLongLong(amount) || amount > 100000 || amount <= 0) {
                cerr << "Niepoprawna kwota.\n"; continue;
            }        
            if (transaction.withdrawal(cardNumber, amount)) {
                cout << "Czy wydrukować potwierdzenie? (T): ";
                string printReceipt;
                cin >> printReceipt;
                if (printReceipt == "T" || printReceipt == "t") {
                        transaction.printReceipt(receiptPath(), "WYPŁATA", amount,cardNumber);
                        cout << "Wydrukowano potwierdzenie wpłaty.\n";
                    }
                cout << "Operacja wypłaty zakończyła się pomyślnie.\n";
            } else {
                cout << "Operacja wypłaty nie powiodła się.\n";
            }
        } 
        else if (choice == 4) { 
            if (!dbHandler.loadData(bankdbPath())) {
                cerr << "Błąd bazy danych.\n"; continue;
            }
            if (dbHandler.changeBlockStatus(cardNumber, true) && dbHandler.saveData(bankdbPath())) {
                cout << "Karta została zablokowana. Nastąpi wylogowanie.\n";
                return; 
            } else {
                cerr << "Nie udało się zablokować karty.\n";
            }
        } 
        else if (choice == 5) { 
            cout << "Czy na pewno chcesz usunąć to konto? (wpisz 'TAK' aby potwierdzić): ";
            string confirmation;
            cin >> confirmation;
            if (confirmation == "TAK") {
                if (!dbHandler.loadData(bankdbPath())) {
                    cerr << "Błąd bazy danych.\n"; continue;
                }
                if (dbHandler.deleteData(cardNumber) && dbHandler.saveData(bankdbPath())) {
                    cout << "Konto zostało usunięte. Nastąpi wylogowanie.\n";
                    return; 
                } else {
                    cerr << "Nie udało się usunąć konta.\n";
                }
            } else {
                cout << "Anulowano usuwanie konta.\n";
            }
        } 
        else if (choice == 6) { 
            cout << "Wylogowano.\n";
            return;
        } 
        else {
            cerr << "Niepoprawny wybór.\n";
        }
    }
}
