#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QMessageBox>
#include <QInputDialog>
#include <QFrame>
#include <QDebug>

// Dołączamy Twoją logikę.
// UWAGA: Upewnij się, że w Account.cpp zakomentowałeś funkcję main()!
#include "Account.cpp"

// --- STYLE CSS (Zaktualizowane) ---
// Używamy selektora [class="nazwa"], ponieważ w C++ Qt nie ma natywnej obsługi klas CSS jak w HTML
const QString STYLESHEET = R"(
    QWidget#MainWindow {
        background-color: #003366; /* Ciemnoniebieskie tło obudowy */
    }
    
    /* Stylizacja bocznych przycisków "sprzętowych" */
    QPushButton[class="sideButton"] {
        background-color: #3c3c3c; /* Ciemnoszare przyciski */
        color: white;
        border: 2px solid #1a1a1a;
        border-radius: 5px;
        font-weight: bold;
        font-size: 14px;
        min-height: 60px;
        min-width: 120px;
        margin: 5px;
    }
    QPushButton[class="sideButton"]:hover { background-color: #555555; }
    QPushButton[class="sideButton"]:pressed { background-color: #2a2a2a; }
    QPushButton[class="sideButton"]:disabled { color: #777777; background-color: #333333; }

    /* Stylizacja "Ekranu" */
    QFrame#ScreenFrame {
        background-color: #004a99; /* Jaśniejszy niebieski ekran */
        border: 4px solid #002244; /* Ciemna ramka wokół ekranu */
        border-radius: 8px;
    }
    QLabel { color: white; }

    /* Stylizacja klawiatury numerycznej */
    QPushButton[class="numpadBtn"] {
        background-color: #e6e6e6;
        color: black;
        border: 2px outset #aaaaaa;
        border-radius: 4px;
        font-size: 18px;
        font-weight: bold;
        min-height: 50px;
        min-width: 50px;
    }
    QPushButton[class="numpadBtn"]:pressed {
        border: 2px inset #aaaaaa;
        background-color: #d4d4d4;
    }
    QPushButton#enterBtn { background-color: #4CAF50; color: white; } /* Zielony Enter */
    QPushButton#clearBtn { background-color: #f44336; color: white; } /* Czerwony Clear */

    QLineEdit#pinDisplay {
        background-color: #003366;
        color: yellow;
        border: 2px solid #0055aa;
        font-size: 24px;
        font-family: monospace;
        padding: 5px;
    }
)";

class BankWindow : public QWidget {
    Q_OBJECT

private:
    // Główne komponenty
    QStackedWidget *stackedWidget;
    QFrame *screenFrame;

    // Boczne przyciski "sprzętowe"
    QPushButton *btnL1, *btnL2, *btnL3, *btnL4;
    QPushButton *btnR1, *btnR2, *btnR3, *btnR4;

    // Ekran logowania i klawiatura
    QWidget *loginPage;
    QLineEdit *cardInput;
    QLineEdit *pinDisplay; // Tylko do wyświetlania gwiazdek
    QString currentPinBuffer; // Przechowuje wpisany PIN wewnętrznie

    // Ekran menu
    QWidget *menuPage;
    QLabel *welcomeLabel;
    QLabel *balanceLabel;
    QLabel *menuInstructionsLabel;

    // Logika biznesowa
    BankDatabaseHandler dbHandler;
    Transaction transaction;
    string currentCardNumber;

public:
    BankWindow(QWidget *parent = nullptr) : QWidget(parent) {
        setObjectName("MainWindow");
        setStyleSheet(STYLESHEET);

        // Inicjalizacja bazy danych
        if (!dbHandler.loadData("DataBase/BankDatabase.json")) {
            QMessageBox::critical(this, "Błąd", "Nie można załadować bazy danych!\nUpewnij się, że folder DataBase istnieje.");
        }

        setupHardwareLayout();
        setupScreenContent();
        
        // Na początku jesteśmy w trybie logowania
        stackedWidget->setCurrentIndex(0);
        updateSideButtonsState(false); // Wyłącz przyciski menu
    }

    // Tworzy "fizyczną" obudowę bankomatu (przyciski boczne i ramkę ekranu)
    void setupHardwareLayout() {
        QHBoxLayout *mainHLayout = new QHBoxLayout(this);
        mainHLayout->setSpacing(15);
        mainHLayout->setContentsMargins(20, 20, 20, 20);

        // --- Lewa kolumna przycisków ---
        QVBoxLayout *leftBtnLayout = new QVBoxLayout();
        btnL1 = createSideButton("Wpłata");
        btnL2 = createSideButton("Wypłata");
        btnL3 = createSideButton("---"); // Nieużywany
        btnL4 = createSideButton("---"); // Nieużywany
        leftBtnLayout->addWidget(btnL1);
        leftBtnLayout->addWidget(btnL2);
        leftBtnLayout->addWidget(btnL3);
        leftBtnLayout->addWidget(btnL4);
        leftBtnLayout->addStretch();

        // --- Środkowy Ekran ---
        screenFrame = new QFrame();
        screenFrame->setObjectName("ScreenFrame");
        screenFrame->setMinimumSize(500, 600);
        QVBoxLayout *screenLayout = new QVBoxLayout(screenFrame);
        stackedWidget = new QStackedWidget(screenFrame);
        screenLayout->addWidget(stackedWidget);

        // --- Prawa kolumna przycisków ---
        QVBoxLayout *rightBtnLayout = new QVBoxLayout();
        btnR1 = createSideButton("Odśwież");
        btnR2 = createSideButton("Zarejestruj"); // Widoczny przy logowaniu
        btnR3 = createSideButton("---");
        btnR4 = createSideButton("Wyloguj");
        rightBtnLayout->addWidget(btnR1);
        rightBtnLayout->addWidget(btnR2);
        rightBtnLayout->addWidget(btnR3);
        rightBtnLayout->addStretch();
        rightBtnLayout->addWidget(btnR4);

        // Złożenie całości
        mainHLayout->addLayout(leftBtnLayout);
        mainHLayout->addWidget(screenFrame);
        mainHLayout->addLayout(rightBtnLayout);

        setWindowTitle("Symulator Bankomatu");
        
        // Podłączenie bocznych przycisków do akcji
        connect(btnL1, &QPushButton::clicked, this, &BankWindow::handleDeposit);
        connect(btnL2, &QPushButton::clicked, this, &BankWindow::handleWithdraw);
        connect(btnR1, &QPushButton::clicked, this, &BankWindow::updateBalance);
        connect(btnR2, &QPushButton::clicked, this, &BankWindow::handleRegister);
        connect(btnR4, &QPushButton::clicked, this, &BankWindow::handleLogout);
    }

    // Tworzy zawartość wyświetlaną na "ekranie"
    void setupScreenContent() {
        // --- Ekran 1: Logowanie z klawiaturą ---
        loginPage = new QWidget();
        QVBoxLayout *loginLayout = new QVBoxLayout(loginPage);
        loginLayout->setAlignment(Qt::AlignCenter);

        QLabel *logo = new QLabel("WITAJ W BANKU");
        logo->setStyleSheet("font-size: 28px; font-weight: bold; color: yellow; margin-bottom: 30px;");
        logo->setAlignment(Qt::AlignCenter);

        QLabel *infoLbl = new QLabel("Wpisz numer karty (klawiatura PC) i PIN (ekran):");
        infoLbl->setAlignment(Qt::AlignCenter);

        cardInput = new QLineEdit();
        cardInput->setPlaceholderText("Numer karty...");
        cardInput->setStyleSheet("font-size: 18px; padding: 5px; background-color: white; color: black;");

        pinDisplay = new QLineEdit();
        pinDisplay->setObjectName("pinDisplay");
        pinDisplay->setPlaceholderText("PIN");
        pinDisplay->setReadOnly(true); // Tylko do odczytu, wpisywanie przez klawiaturę ekranową
        pinDisplay->setEchoMode(QLineEdit::Password);
        pinDisplay->setAlignment(Qt::AlignCenter);

        // Klawiatura numeryczna
        QGridLayout *numpadLayout = new QGridLayout();
        numpadLayout->setSpacing(10);
        int values[3][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
        for (int r = 0; r < 3; ++r) {
            for (int c = 0; c < 3; ++c) {
                QPushButton *btn = new QPushButton(QString::number(values[r][c]));
                btn->setProperty("digit", values[r][c]);
                // POPRAWKA: Używamy setProperty zamiast addClassName
                btn->setProperty("class", "numpadBtn");
                connect(btn, &QPushButton::clicked, this, &BankWindow::digitClicked);
                numpadLayout->addWidget(btn, r, c);
            }
        }
        // Ostatni rząd: Clear, 0, Enter
        QPushButton *clearBtn = new QPushButton("C");
        clearBtn->setObjectName("clearBtn"); 
        // POPRAWKA
        clearBtn->setProperty("class", "numpadBtn");
        connect(clearBtn, &QPushButton::clicked, this, &BankWindow::clearPin);

        QPushButton *zeroBtn = new QPushButton("0");
        zeroBtn->setProperty("digit", 0); 
        // POPRAWKA
        zeroBtn->setProperty("class", "numpadBtn");
        connect(zeroBtn, &QPushButton::clicked, this, &BankWindow::digitClicked);

        QPushButton *enterBtn = new QPushButton("OK");
        enterBtn->setObjectName("enterBtn"); 
        // POPRAWKA
        enterBtn->setProperty("class", "numpadBtn");
        connect(enterBtn, &QPushButton::clicked, this, &BankWindow::handleLoginAction);

        numpadLayout->addWidget(clearBtn, 3, 0);
        numpadLayout->addWidget(zeroBtn, 3, 1);
        numpadLayout->addWidget(enterBtn, 3, 2);

        QWidget *numpadContainer = new QWidget();
        numpadContainer->setLayout(numpadLayout);
        numpadContainer->setMaximumWidth(300);

        loginLayout->addWidget(logo);
        loginLayout->addWidget(infoLbl);
        loginLayout->addWidget(cardInput);
        loginLayout->addWidget(pinDisplay);
        loginLayout->addWidget(numpadContainer);
        loginLayout->setAlignment(numpadContainer, Qt::AlignCenter);


        // --- Ekran 2: Menu Główne ---
        menuPage = new QWidget();
        QVBoxLayout *menuLayout = new QVBoxLayout(menuPage);
        menuLayout->setAlignment(Qt::AlignCenter);

        welcomeLabel = new QLabel("Witaj!");
        welcomeLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: white; margin-top: 20px;");
        welcomeLabel->setAlignment(Qt::AlignCenter);

        balanceLabel = new QLabel("SALDO: --- PLN");
        balanceLabel->setStyleSheet("font-size: 32px; font-weight: bold; color: #4CAF50; margin: 40px 0;");
        balanceLabel->setAlignment(Qt::AlignCenter);
        
        menuInstructionsLabel = new QLabel("Użyj przycisków po bokach ekranu\naby wybrać operację.");
        menuInstructionsLabel->setStyleSheet("font-size: 16px; color: #cccccc; font-style: italic;");
        menuInstructionsLabel->setAlignment(Qt::AlignCenter);

        menuLayout->addStretch();
        menuLayout->addWidget(welcomeLabel);
        menuLayout->addWidget(balanceLabel);
        menuLayout->addWidget(menuInstructionsLabel);
        menuLayout->addStretch();

        stackedWidget->addWidget(loginPage);
        stackedWidget->addWidget(menuPage);
    }

    // Helper do tworzenia bocznych przycisków
    QPushButton* createSideButton(const QString &text) {
        QPushButton *btn = new QPushButton(text);
        // POPRAWKA
        btn->setProperty("class", "sideButton");
        return btn;
    }

    // Zmienia stan przycisków w zależności od tego, czy jesteśmy zalogowani
    void updateSideButtonsState(bool loggedIn) {
        btnL1->setEnabled(loggedIn); // Wpłata
        btnL2->setEnabled(loggedIn); // Wypłata
        btnL3->setEnabled(false);
        btnL4->setEnabled(false);

        btnR1->setEnabled(loggedIn); // Odśwież
        btnR2->setEnabled(!loggedIn); // Rejestracja (tylko gdy wylogowany)
        btnR3->setEnabled(false);
        btnR4->setEnabled(loggedIn); // Wyloguj
        
        if(loggedIn) {
             btnR2->setText("---");
             btnL1->setText("Wpłata >");
             btnL2->setText("Wypłata >");
        } else {
             btnR2->setText("< Zarejestruj");
             btnL1->setText("---");
             btnL2->setText("---");
        }
    }


private slots:
    // Slot dla klawiatury numerycznej
    void digitClicked() {
        QPushButton *btn = qobject_cast<QPushButton*>(sender());
        if (btn && currentPinBuffer.length() < 4) {
            int digit = btn->property("digit").toInt();
            currentPinBuffer.append(QString::number(digit));
            pinDisplay->setText(currentPinBuffer); // Pokaże gwiazdki dzięki EchoMode::Password
        }
    }

    void clearPin() {
        currentPinBuffer.clear();
        pinDisplay->clear();
    }

    void handleLoginAction() {
        string card = cardInput->text().toStdString();
        string pin = currentPinBuffer.toStdString(); // Używamy bufora z klawiatury

        if (card.empty() || pin.length() != 4) {
            QMessageBox::warning(this, "Błąd", "Wprowadź numer karty i 4-cyfrowy PIN.");
            return;
        }

        int result = dbHandler.checkPin(card, pin);

        if (result == 1) {
            currentCardNumber = card;
            welcomeLabel->setText(QString::fromStdString("Witaj, konto: " + card));
            updateBalance();
            
            // Wyczyść pola po udanym logowaniu
            cardInput->clear();
            clearPin();
            
            stackedWidget->setCurrentIndex(1); // Przejdź do menu
            updateSideButtonsState(true); // Aktywuj przyciski menu
        } else if (result == 0) {
            QMessageBox::warning(this, "Błąd", "Niepoprawny PIN.");
            clearPin();
        } else if (result == -1) {
            QMessageBox::warning(this, "Błąd", "Konto nie istnieje.");
        } else if (result == -2) {
            QMessageBox::warning(this, "Blokada", "Konto jest zablokowane.");
        }
    }

    void handleLogout() {
        currentCardNumber = "";
        stackedWidget->setCurrentIndex(0); // Wróć do logowania
        updateSideButtonsState(false);
    }

    void updateBalance() {
        transaction.loadData("DataBase/BankDatabase.json");
        long long balance = transaction.getBalance(currentCardNumber);
        balanceLabel->setText("SALDO: " + QString::number(balance) + " PLN");
    }

    // --- Akcje transakcyjne (używają QInputDialog dla uproszczenia) ---
    void handleWithdraw() {
        bool ok;
        // Używamy "double" żeby pozwolić na większe liczby, ale rzutujemy na int
        double amountD = QInputDialog::getDouble(this, "Wypłata", "Podaj kwotę do wypłaty (wielokrotność 50 PLN):", 0, 0, 100000, 0, &ok);
        int amount = static_cast<int>(amountD);

        if (ok && amount > 0) {
             if (amount % 50 != 0) {
                  QMessageBox::warning(this, "Błąd", "Kwota musi być wielokrotnością 50 PLN.");
                  return;
             }
            if (transaction.withdrawal(currentCardNumber, amount)) {
                QMessageBox::information(this, "Sukces", "Wypłacono środki.\nOdbierz gotówkę.");
                updateBalance();
            } else {
                QMessageBox::warning(this, "Błąd", "Nie udało się wypłacić środków.\nSprawdź saldo lub dostępność banknotów w maszynie.");
            }
        }
    }

    void handleDeposit() {
        bool okAmount;
        double amountD = QInputDialog::getDouble(this, "Wpłata", "Podaj łączną kwotę wpłaty:", 0, 0, 100000, 0, &okAmount);
        int amount = static_cast<int>(amountD);
        if (!okAmount || amount <= 0) return;

        bool okNotes;
        int notes = QInputDialog::getInt(this, "Wpłata", "Podaj liczbę wkładanych banknotów (sztuki):", 1, 1, 200, 1, &okNotes);
        if (!okNotes) return;

        QMessageBox::information(this, "Symulacja", "Trwa przeliczanie banknotów...");

        if (transaction.deposit(currentCardNumber, amount, notes)) {
             QMessageBox::information(this, "Sukces", "Wpłacono środki na konto.");
             updateBalance();
        } else {
             QMessageBox::warning(this, "Błąd", "Wpłata odrzucona.\nKwota nie zgadza się z zadeklarowanymi nominałami lub przekroczono limity.");
        }
    }
    
    void handleRegister() {
        QString newCard = QInputDialog::getText(this, "Rejestracja", "Podaj nowy numer karty:");
        if (newCard.isEmpty()) return;
        
        if (dbHandler.existenceOfAccount(newCard.toStdString())) {
             QMessageBox::warning(this, "Błąd", "Konto o tym numerze już istnieje.");
             return;
        }
        
        QString newPin = QInputDialog::getText(this, "Rejestracja", "Ustaw 4-cyfrowy PIN:");
        if (newPin.length() != 4) {
             QMessageBox::warning(this, "Błąd", "PIN musi mieć dokładnie 4 cyfry.");
             return;
        }

        dbHandler.addData(newCard.toStdString(), newCard.toStdString(), newPin.toStdString(), 0);
        dbHandler.saveData("DataBase/BankDatabase.json");
        QMessageBox::information(this, "Sukces", "Konto utworzone pomyślnie.\nMożesz się teraz zalogować.");
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    QFont font("Roboto Mono");
    font.setStyleHint(QFont::Monospace);
    QApplication::setFont(font);

    BankWindow window;
    window.resize(1024, 768);
    window.show();
    
    return app.exec();
}

#include "main_gui.moc"