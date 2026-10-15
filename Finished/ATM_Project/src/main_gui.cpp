#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QMessageBox>
#include <QInputDialog>
#include <QFrame>
#include <QDebug>
#include <QRegularExpressionValidator>
#include <string>

#include "Transaction.h"
#include "PathResolver.h"

using namespace std;

// --- STYLE CSS ---
const QString STYLESHEET = R"(
    QWidget#MainWindow {
        background-color: #003366; 
    }
    
    /* Przyciski boczne */
    QPushButton[class="sideButton"] {
        background-color: #3c3c3c;
        color: white;
        border: 2px solid #1a1a1a;
        border-radius: 5px;
        font-weight: bold;
        font-size: 14px;
        min-height: 60px;
        min-width: 140px; /* Szersze przyciski */
        margin: 5px;
        text-align: center;
    }
    QPushButton[class="sideButton"]:hover { background-color: #555555; }
    QPushButton[class="sideButton"]:pressed { background-color: #2a2a2a; }
    QPushButton[class="sideButton"]:disabled { color: #777777; background-color: #222222; border-color: #222222; }

    /* Ekran */
    QFrame#ScreenFrame {
        background-color: #004a99;
        border: 4px solid #002244;
        border-radius: 8px;
    }
    QLabel { color: black; font-family: 'Roboto Mono', monospace; }
    
    /* Pola tekstowe na ekranie */
    QLineEdit {
        font-size: 18px; 
        padding: 5px; 
        background-color: white; 
        color: black;
        border: 2px solid #002244;
        border-radius: 4px;
    }

    /* Klawiatura PIN */
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
    QPushButton#enterBtn { background-color: #4CAF50; color: white; }
    QPushButton#clearBtn { background-color: #f44336; color: white; }

    QLineEdit#pinDisplay {
        background-color: #003366;
        color: yellow;
        border: 2px solid #0055aa;
        font-size: 24px;
        font-family: monospace;
        padding: 5px;
    }
)";

class BankWindow : public QWidget
{
    Q_OBJECT

private:
    // UI Components
    QStackedWidget *stackedWidget;
    QFrame *screenFrame;

    // Hardware buttons
    QPushButton *btnL1, *btnL2, *btnL3, *btnL4;
    QPushButton *btnR1, *btnR2, *btnR3, *btnR4;

    // --- PAGE 0: Login ---
    QWidget *loginPage;
    QLineEdit *cardInput;
    QLineEdit *pinDisplay;
    QString currentPinBuffer;

    // --- PAGE 1: Menu ---
    QWidget *menuPage;
    QLabel *welcomeLabel;
    QLabel *balanceLabel;

    // --- PAGE 2: Registration ---
    QWidget *registerPage;
    QLineEdit *regCardInput;
    QLineEdit *regPinInput;
    QLineEdit *regBalanceInput;

    // Logic
    BankDatabaseHandler dbHandler;
    Transaction transaction;
    string currentCardNumber;

public:
    BankWindow(QWidget *parent = nullptr) : QWidget(parent)
    {
        setObjectName("MainWindow");
        setStyleSheet(STYLESHEET);

        if (!dbHandler.loadData(resolvePath("DataBase/BankDatabase.json")))
        {
            showMessage("Krytyczny błąd", "Nie można załadować bazy danych!");
        }

        setupHardwareLayout();

        // Setup Screens
        setupLoginPage();
        setupMenuPage();
        setupRegisterPage();

        // Add pages to stack
        stackedWidget->addWidget(loginPage);    // Index 0
        stackedWidget->addWidget(menuPage);     // Index 1
        stackedWidget->addWidget(registerPage); // Index 2

        // Start state
        stackedWidget->setCurrentIndex(0);
        updateSideButtonsState(0);
    }

    void setupHardwareLayout()
    {
        QHBoxLayout *mainHLayout = new QHBoxLayout(this);
        mainHLayout->setSpacing(15);
        mainHLayout->setContentsMargins(20, 20, 20, 20);

        // Left Buttons
        QVBoxLayout *leftBtnLayout = new QVBoxLayout();
        btnL1 = createSideButton("---");
        btnL2 = createSideButton("---");
        btnL3 = createSideButton("---");
        btnL4 = createSideButton("---");
        leftBtnLayout->addWidget(btnL1);
        leftBtnLayout->addWidget(btnL2);
        leftBtnLayout->addWidget(btnL3);
        leftBtnLayout->addWidget(btnL4);
        leftBtnLayout->addStretch();

        // Screen
        screenFrame = new QFrame();
        screenFrame->setObjectName("ScreenFrame");
        screenFrame->setMinimumSize(500, 650);
        QVBoxLayout *screenLayout = new QVBoxLayout(screenFrame);
        stackedWidget = new QStackedWidget(screenFrame);
        screenLayout->addWidget(stackedWidget);

        // Right Buttons
        QVBoxLayout *rightBtnLayout = new QVBoxLayout();
        btnR1 = createSideButton("---");
        btnR2 = createSideButton("---");
        btnR3 = createSideButton("---");
        btnR4 = createSideButton("---");
        rightBtnLayout->addWidget(btnR1);
        rightBtnLayout->addWidget(btnR2);
        rightBtnLayout->addWidget(btnR3);
        rightBtnLayout->addStretch();
        rightBtnLayout->addWidget(btnR4);

        mainHLayout->addLayout(leftBtnLayout);
        mainHLayout->addWidget(screenFrame);
        mainHLayout->addLayout(rightBtnLayout);

        setWindowTitle("System Bankowy ATM");

        // Global Button Connections (Context dependent)
        connect(btnL1, &QPushButton::clicked, this, [this]()
                {
            if(stackedWidget->currentIndex() == 1) handleDeposit(); });
        connect(btnL2, &QPushButton::clicked, this, [this]()
                {
            if(stackedWidget->currentIndex() == 1) handleWithdraw(); });

        connect(btnR1, &QPushButton::clicked, this, [this]()
                {
                    if (stackedWidget->currentIndex() == 1)
                        updateBalance();
                    if (stackedWidget->currentIndex() == 2)
                        handleRegisterSubmit(); // Zatwierdź rejestrację
                });

        connect(btnR2, &QPushButton::clicked, this, [this]()
                {
                    if (stackedWidget->currentIndex() == 0)
                        switchToRegister(); // Idź do rejestracji
                });

        connect(btnR3, &QPushButton::clicked, this, [this]()
                {
                    if (stackedWidget->currentIndex() == 1)
                        handleBlockCard(); // Zablokuj
                });

        connect(btnR4, &QPushButton::clicked, this, [this]()
                {
                    if (stackedWidget->currentIndex() == 1)
                        handleLogout();
                    if (stackedWidget->currentIndex() == 2)
                        handleLogout(); // Anuluj rejestrację
                });
    }

    void setupLoginPage()
    {
        loginPage = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(loginPage);
        layout->setAlignment(Qt::AlignCenter);

        QLabel *logo = new QLabel("WITAJ W BANKU");
        logo->setStyleSheet("font-size: 28px; font-weight: bold; color: #FFD700; margin-bottom: 20px;");
        logo->setAlignment(Qt::AlignCenter);

        cardInput = new QLineEdit();
        cardInput->setPlaceholderText("Numer karty (użyj klawiatury)...");

        pinDisplay = new QLineEdit();
        pinDisplay->setObjectName("pinDisplay");
        pinDisplay->setPlaceholderText("----");
        pinDisplay->setReadOnly(true);
        pinDisplay->setEchoMode(QLineEdit::Password);
        pinDisplay->setAlignment(Qt::AlignCenter);

        // Numpad
        QGridLayout *numpadLayout = new QGridLayout();
        numpadLayout->setSpacing(10);
        int values[3][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
        for (int r = 0; r < 3; ++r)
        {
            for (int c = 0; c < 3; ++c)
            {
                QPushButton *btn = new QPushButton(QString::number(values[r][c]));
                btn->setProperty("digit", values[r][c]);
                btn->setProperty("class", "numpadBtn");
                connect(btn, &QPushButton::clicked, this, &BankWindow::digitClicked);
                numpadLayout->addWidget(btn, r, c);
            }
        }
        QPushButton *clearBtn = new QPushButton("C");
        clearBtn->setObjectName("clearBtn");
        clearBtn->setProperty("class", "numpadBtn");
        connect(clearBtn, &QPushButton::clicked, this, &BankWindow::clearPin);

        QPushButton *zeroBtn = new QPushButton("0");
        zeroBtn->setProperty("digit", 0);
        zeroBtn->setProperty("class", "numpadBtn");
        connect(zeroBtn, &QPushButton::clicked, this, &BankWindow::digitClicked);

        QPushButton *enterBtn = new QPushButton("OK");
        enterBtn->setObjectName("enterBtn");
        enterBtn->setProperty("class", "numpadBtn");
        connect(enterBtn, &QPushButton::clicked, this, &BankWindow::handleLoginAction);

        numpadLayout->addWidget(clearBtn, 3, 0);
        numpadLayout->addWidget(zeroBtn, 3, 1);
        numpadLayout->addWidget(enterBtn, 3, 2);

        QWidget *padWidget = new QWidget();
        padWidget->setLayout(numpadLayout);
        padWidget->setMaximumWidth(320);

        layout->addWidget(logo);
        layout->addWidget(new QLabel("Wprowadź kartę:"));
        layout->addWidget(cardInput);
        layout->addWidget(new QLabel("Wprowadź PIN (na ekranie):"));
        layout->addWidget(pinDisplay);
        layout->addSpacing(10);
        layout->addWidget(padWidget);
        layout->setAlignment(padWidget, Qt::AlignCenter);
    }

    void setupMenuPage()
    {
        menuPage = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(menuPage);
        layout->setAlignment(Qt::AlignCenter);

        welcomeLabel = new QLabel("Witaj!");
        welcomeLabel->setStyleSheet("font-size: 22px; font-weight: bold; margin-top: 20px;");
        welcomeLabel->setAlignment(Qt::AlignCenter);

        balanceLabel = new QLabel("SALDO: --- PLN");
        balanceLabel->setStyleSheet("font-size: 34px; font-weight: bold; color: #4CAF50; margin: 30px 0;");
        balanceLabel->setAlignment(Qt::AlignCenter);

        QLabel *instr = new QLabel("Wybierz operację korzystając z przycisków\npo bokach ekranu.");
        instr->setStyleSheet("font-size: 14px; color: black; font-style: italic;");
        instr->setAlignment(Qt::AlignCenter);

        layout->addStretch();
        layout->addWidget(welcomeLabel);
        layout->addWidget(balanceLabel);
        layout->addWidget(instr);
        layout->addStretch();
    }

    void setupRegisterPage()
    {
        registerPage = new QWidget();
        QVBoxLayout *mainLayout = new QVBoxLayout(registerPage);
        mainLayout->setAlignment(Qt::AlignCenter);

        QLabel *title = new QLabel("REJESTRACJA NOWEGO KONTA");
        title->setStyleSheet("font-size: 20px; font-weight: bold; color: #FFD700; margin-bottom: 20px;");
        title->setAlignment(Qt::AlignCenter);

        QFormLayout *formLayout = new QFormLayout();
        formLayout->setLabelAlignment(Qt::AlignRight);

        regCardInput = new QLineEdit();
        regCardInput->setPlaceholderText("Nowy numer karty");

        regPinInput = new QLineEdit();
        regPinInput->setPlaceholderText("4 cyfry");
        regPinInput->setMaxLength(4);

        regBalanceInput = new QLineEdit();
        regBalanceInput->setPlaceholderText("np. 1000");
        regBalanceInput->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]*"), this));

        // Stylowanie etykiet formularza
        QLabel *l1 = new QLabel("Numer Karty:");
        l1->setStyleSheet("font-size: 16px; font-weight: bold;");
        QLabel *l2 = new QLabel("Nowy PIN:");
        l2->setStyleSheet("font-size: 16px; font-weight: bold;");
        QLabel *l3 = new QLabel("Saldo Startowe:");
        l3->setStyleSheet("font-size: 16px; font-weight: bold;");

        formLayout->addRow(l1, regCardInput);
        formLayout->addRow(l2, regPinInput);
        formLayout->addRow(l3, regBalanceInput);

        QWidget *formContainer = new QWidget();
        formContainer->setLayout(formLayout);
        formContainer->setStyleSheet("background-color: #003366; padding: 10px; border-radius: 5px;");

        mainLayout->addWidget(title);
        mainLayout->addWidget(formContainer);
        mainLayout->addSpacing(20);

        QLabel *info = new QLabel("Naciśnij 'Zatwierdź' (Prawy Górny)\nlub 'Anuluj' (Prawy Dolny)");
        info->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(info);
        mainLayout->addStretch();
    }

    QPushButton *createSideButton(const QString &text)
    {
        QPushButton *btn = new QPushButton(text);
        btn->setProperty("class", "sideButton");
        return btn;
    }

    // --- LOGIC ---

    // 0 = Login, 1 = Menu, 2 = Register
    void updateSideButtonsState(int pageIndex)
    {
        // Reset tekstów
        btnL1->setText("---");
        btnL2->setText("---");
        btnL3->setText("---");
        btnL4->setText("---");
        btnR1->setText("---");
        btnR2->setText("---");
        btnR3->setText("---");
        btnR4->setText("---");

        // Reset aktywności
        btnL1->setEnabled(false);
        btnL2->setEnabled(false);
        btnL3->setEnabled(false);
        btnL4->setEnabled(false);
        btnR1->setEnabled(false);
        btnR2->setEnabled(false);
        btnR3->setEnabled(false);
        btnR4->setEnabled(false);

        if (pageIndex == 0)
        { // Login
            btnR2->setText("< Załóż Konto");
            btnR2->setEnabled(true);
        }
        else if (pageIndex == 1)
        { // Menu
            btnL1->setText("Wpłata >");
            btnL1->setEnabled(true);
            btnL2->setText("Wypłata >");
            btnL2->setEnabled(true);

            btnR1->setText("< Odśwież");
            btnR1->setEnabled(true);
            btnR3->setText("< ZABLOKUJ");
            btnR3->setStyleSheet("color: red; border-color: red;");
            btnR3->setEnabled(true);
            btnR4->setText("< Wyloguj");
            btnR4->setEnabled(true);
        }
        else if (pageIndex == 2)
        { // Register
            btnR1->setText("< Zatwierdź");
            btnR1->setEnabled(true);
            btnR4->setText("< Anuluj");
            btnR4->setEnabled(true);
        }
    }

    void showMessage(QString title, QString content, bool isError = true)
    {
        QMessageBox msg;
        msg.setWindowTitle(title);
        msg.setText(content);
        msg.setIcon(isError ? QMessageBox::Warning : QMessageBox::Information);
        msg.setStyleSheet("background-color: white; color: black;");
        msg.exec();
    }

private slots:
    // --- PIN PAD ---
    void digitClicked()
    {
        QPushButton *btn = qobject_cast<QPushButton *>(sender());
        if (btn && currentPinBuffer.length() < 4)
        {
            int digit = btn->property("digit").toInt();
            currentPinBuffer.append(QString::number(digit));
            pinDisplay->setText(currentPinBuffer);
        }
    }
    void clearPin()
    {
        currentPinBuffer.clear();
        pinDisplay->clear();
    }

    // --- ACTIONS ---
    void handleLoginAction()
    {
        string card = cardInput->text().toStdString();
        string pin = currentPinBuffer.toStdString();

        if (card.empty())
        {
            showMessage("Błąd", "Wprowadź numer karty.");
            return;
        }
        if (pin.length() != 4)
        {
            showMessage("Błąd", "PIN musi mieć 4 cyfry.");
            return;
        }

        int result = dbHandler.checkPin(card, pin);

        if (result == 1)
        {
            currentCardNumber = card;
            welcomeLabel->setText(QString::fromStdString("Konto: " + card));
            updateBalance();

            cardInput->clear();
            clearPin();

            stackedWidget->setCurrentIndex(1);
            updateSideButtonsState(1);
        }
        else if (result == 0)
        {
            showMessage("Błąd", "Niepoprawny PIN!", true);
            clearPin();
        }
        else if (result == -1)
        {
            showMessage("Błąd", "Nie znaleziono konta.", true);
        }
        else if (result == -2)
        {
            showMessage("Blokada", "KARTA ZABLOKOWANA.\nSkontaktuj się z bankiem.", true);
        }
    }

    void handleLogout()
    {
        currentCardNumber = "";
        stackedWidget->setCurrentIndex(0);
        updateSideButtonsState(0);

        // Wyczyść formularz rejestracji przy wyjściu
        regCardInput->clear();
        regPinInput->clear();
        regBalanceInput->clear();
        btnR3->setStyleSheet(""); // Reset stylu przycisku blokady
    }

    void updateBalance()
    {
        transaction.loadData(resolvePath("DataBase/BankDatabase.json"));
        long long balance = transaction.getBalance(currentCardNumber);
        balanceLabel->setText("SALDO: " + QString::number(balance) + " PLN");
    }

    void switchToRegister()
    {
        stackedWidget->setCurrentIndex(2);
        updateSideButtonsState(2);
    }

    void handleRegisterSubmit()
    {
        string newCard = regCardInput->text().toStdString();
        string newPin = regPinInput->text().toStdString();
        string balStr = regBalanceInput->text().toStdString();

        if (newCard.empty())
        {
            showMessage("Błąd", "Podaj numer karty.");
            return;
        }
        if (newPin.length() != 4)
        {
            showMessage("Błąd", "PIN musi składać się z 4 cyfr.");
            return;
        }

        long long initialBalance = 0;
        try
        {
            if (!balStr.empty())
                initialBalance = stoll(balStr);
        }
        catch (...)
        {
            showMessage("Błąd", "Niepoprawny format salda.");
            return;
        }

        if (dbHandler.existenceOfAccount(newCard))
        {
            showMessage("Błąd", "Konto o tym numerze już istnieje!");
            return;
        }

        // Dodanie do bazy
        dbHandler.addData(newCard, newCard, newPin, initialBalance);
        if (dbHandler.saveData(resolvePath("DataBase/BankDatabase.json")))
        {
            showMessage("Sukces", "Konto utworzone pomyślnie.\nMożesz się zalogować.", false);
            handleLogout(); // Wróć do logowania
        }
        else
        {
            showMessage("Błąd", "Błąd zapisu bazy danych.");
        }
    }

    void handleBlockCard()
    {
        QMessageBox msg;
        msg.setWindowTitle("Blokada Karty");
        msg.setText("Czy na pewno chcesz ZABLOKOWAĆ tę kartę?\nOperacja jest nieodwracalna w bankomacie.");
        msg.setIcon(QMessageBox::Critical);
        msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msg.setStyleSheet("background-color: white; color: black;"); // Fix dla ciemnego motywu

        if (msg.exec() == QMessageBox::Yes)
        {
            // Ponowne załadowanie dla pewności
            dbHandler.loadData(resolvePath("DataBase/BankDatabase.json"));

            if (dbHandler.changeBlockStatus(currentCardNumber, true) &&
                dbHandler.saveData(resolvePath("DataBase/BankDatabase.json")))
            {

                showMessage("Zablokowano", "Karta została zablokowana.\nNastąpi wylogowanie.", false);
                handleLogout();
            }
            else
            {
                showMessage("Błąd", "Nie udało się zablokować karty.");
            }
        }
    }

    void handleWithdraw()
    {
        bool ok;
        double amountD = QInputDialog::getDouble(this, "Wypłata", "Podaj kwotę do wypłaty (wielokrotność 50 PLN):", 0, 0, 100000, 0, &ok);

        if (!ok)
            return; // Użytkownik kliknął Anuluj

        int amount = static_cast<int>(amountD);

        // 1. Walidacja kwoty
        if (amount <= 0)
        {
            showMessage("Błąd", "Kwota musi być większa od zera.");
            return;
        }
        if (amount % 50 != 0)
        {
            showMessage("Błąd", "Bankomat wydaje tylko banknoty o nominałach: 50, 100, 200, 500 PLN.\nKwota musi być podzielna przez 50.");
            return;
        }

        // 2. Sprawdzenie salda PRZED próbą wypłaty
        // long long currentBalance = transaction.getBalance(currentCardNumber);
        if (transaction.withdrawal(currentCardNumber, amount))
        {

            // ---  POLIMORFIZM ---
            // Jeśli kwota duża (> 5000), drukuj RODO, w przeciwnym razie zwykły
            if (amount > 5000)
            {
                PrivacyReceipt privacyStrat;
                transaction.setReceiptStrategy(&privacyStrat);
                transaction.printReceipt("Receipt.txt", "WYPŁATA DUŻA", amount, currentCardNumber);
            }
            else
            {
                StandardReceipt stdStrat;
                transaction.setReceiptStrategy(&stdStrat);
                transaction.printReceipt("Receipt.txt", "WYPŁATA", amount, currentCardNumber);
            }

            QString successMsg = QString("Operacja udana.\nWypłacono: %1 PLN.\nWydrukowano potwierdzenie.").arg(amount);
            showMessage("Sukces", successMsg, false);
            updateBalance();
        }
        else
        {
            showMessage("Błąd techniczny", "Nie udało się wypłacić środków.");
        }
    }

    void handleDeposit()
    {
        bool okAmount;
        double amountD = QInputDialog::getDouble(this, "Wpłata", "Podaj łączną kwotę wpłaty (max 100 000):", 0, 0, 100000, 0, &okAmount);

        if (!okAmount)
            return; // Anuluj

        int amount = static_cast<int>(amountD);
        if (amount <= 0)
        {
            showMessage("Błąd", "Kwota wpłaty musi być dodatnia.");
            return;
        }
        if (amount % 10 != 0)
        {
            showMessage("Błąd", "Nieprawidłowa kwota. Wpłatomat przyjmuje tylko pełne banknoty.");
            return;
        }

        bool okNotes;
        int notes = QInputDialog::getInt(this, "Wpłata", "Podaj liczbę wkładanych banknotów (max 200 szt.):", 1, 1, 200, 1, &okNotes);

        if (!okNotes)
            return; // Anuluj

        showMessage("Przetwarzanie", "Trwa przeliczanie i weryfikacja banknotów...", false);

        if (transaction.deposit(currentCardNumber, amount, notes))
        {
            QString successMsg = QString("Wpłata zakończona sukcesem.\nZaksięgowano: %1 PLN.").arg(amount);
            showMessage("Sukces", successMsg, false);
            updateBalance();
        }
        else
        {
            showMessage("Odmowa wpłaty",
                        "Operacja odrzucona przez system.\n\n"
                        "Możliwe przyczyny:\n"
                        "1. Kwota nie zgadza się z zadeklarowanymi nominałami.\n"
                        "2. Przekroczono limit banknotów (200 szt).\n"
                        "3. Wprowadzono nieobsługiwane nominały.");
        }
    }
};

int main(int argc, char *argv[])
{
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