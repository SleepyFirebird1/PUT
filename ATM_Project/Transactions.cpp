#include "DatabaseHandler.cpp"
#include "CashMachine.cpp"

using namespace std;

const string dbPath = "DataBase/CashStorage.txt";
const string logPath = "DataBase/CashStorage_log.txt";
const string bankdbPath = "DataBase/BankDatabase.json";


class Transaction : public CashDispenser, public BankDatabaseHandler {
//wpłata
public:
    bool deposit(const string &cardNumber, const long long &amount, const long long &number) {
        if (!loadFromFile(dbPath)) {
        cerr << "Blad: nie mozna wczytac bazy. Sprawdz uprawnienia katalogu DataBase.\n";
        // kontynuujemy z zerowym stanem
        }
        if (!loadData(bankdbPath)) {
            return false;
        }
        if (!depositAmount(amount, number, dbPath, logPath)) {
            return false;
        }
        if (!changeBalance(cardNumber, amount)) {
            return false;
        }
        if (!saveData(bankdbPath)) {
            return false;
        }
        return true;
    }
//wypłata
    bool canWithdrawal(const long long &amount, const string &cardNumber) {
        return (canDispense(amount) && getBalance(cardNumber) >= amount);
    }
  
    bool withdrawal(const string &cardNumber, const long long &amount) {
        if (!loadFromFile(dbPath)) {
            cerr << "Blad: nie mozna wczytac bazy. Sprawdz uprawnienia katalogu DataBase.\n";
            // kontynuujemy z zerowym stanem
        }
        if (!loadData(bankdbPath)) {
            return false;
        }
        if (!canWithdrawal(amount, cardNumber)) {
            // Komunikat błędu zostanie wyświetlony wewnątrz canWithdrawal
            return false;
        }
        if (!withdrawAmount(amount, dbPath, logPath)) {
            cerr << "Operacja wypłaty nie powiodła się.\n";
            return false;
        }
        if (!changeBalance(cardNumber, -amount)) {
            return false;
        }
        if (!saveData(bankdbPath)) {
            return false;
        }
        return true;     
    }
//sprawdzenie balansu

};
int main(){
    Transaction transaction;

    string cardNumber;
    if (!transaction.loadData(bankdbPath)) {
            return 1;
    }
    cout << "Podaj numer karty: ";
    cin >> cardNumber;
    if (transaction.existenceOfAccount(cardNumber)) {
        int choice;
        cout << "1: wpłata ; 2: wypłata ; 3: balans\n";
        cin >> choice;
        if (choice == 1) {
            cout << "Podaj kwote do wpłaty (liczba całkowita, bez PLN maks 100 000PLN): ";
            long long amount;
            if (!readLongLong(amount)) {
                cerr << "Niepoprawne dane wejsciowe. Oczekiwano liczby calkowitej.\n";
                return 1;
            }
            if(amount>100000){
                cerr << "Niepoprawne dane wejsciowe. Maksymalna kwota wpłaty to 100 000 PLN.\n";
                return 1;
            }
            cout << "Podaj liczbe banknotów do wpłaty (max 200szt): ";
            long long number;
            if (!readLongLong(number)) {
                cerr << "Niepoprawne dane wejsciowe. Oczekiwano liczby calkowitej.\n";
                return 1;
            }
            
            if (transaction.deposit(cardNumber, amount, number)) {
                cout << "Operacja wpłaty zakończyła się pomyślnie.\n";
            } else {
                cout << "Operacja wpłaty nie powiodła się.\n";
            }
        }
        if (choice == 2) {
            cout << "Podaj kwote do wypłaty (liczba całkowita, bez PLN maks 100 000PLN): ";
            long long amount;
            if (!readLongLong(amount)) {
                cerr << "Niepoprawne dane wejsciowe. Oczekiwano liczby calkowitej.\n";
                return 1;
            }
            if(amount>100000){
                cerr << "Niepoprawne dane wejsciowe. Maksymalna kwota wypłaty to 100 000 PLN.\n";
                return 1;
            }        
            if (transaction.withdrawal(cardNumber, amount)) {
                cout << "Operacja wypłaty zakończyła się pomyślnie.\n";
            } else {
                cout << "Operacja wypłaty nie powiodła się.\n";
            }
        }
        if (choice == 3) {
            if (transaction.loadData(bankdbPath)) {
                long long balance = transaction.getBalance(cardNumber);
                if (balance != -1){ 
                    cout << "Balans: " << balance << "PLN\n";
                }
            } else {
                cerr << "Błąd wczytywania bazy danych\n";
            }
        }
    } else {
        cout << "Nie ma takiego konta\n";
        exit(1);
    }
    return 0;
}