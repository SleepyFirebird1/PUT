#include "atmgui.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QApplication>
#include <QMessageBox>

AtmGui::AtmGui(QWidget *parent)
    : QWidget(parent), m_currentState(State::WELCOME), m_tempAmount(0)
{
    // 1. TWORZYMY TIMER NA POCZĄTKU!
    m_sessionTimer = new QTimer(this);
    m_sessionTimer->setSingleShot(true);
    connect(m_sessionTimer, &QTimer::timeout, this, &AtmGui::onSessionTimeout);

    initGui();

    // 2. Wczytaj bazę danych
    bool dbLoaded = m_transaction.loadData(bankdbPath);
    if (!dbLoaded) {
        // Pokazujemy błąd, ale nadal przechodzimy do WELCOME
        m_screen->setText("BŁĄD KRYTYCZNY:\nNie można wczytać bazy danych banku.\nSprawdź plik DataBase/BankDatabase.json");
    }

    // 3. Zawsze ustawiamy stan początkowy (po stworzeniu timera!)
    setState(State::WELCOME);

    // Opcjonalnie: jeśli baza nie załadowana, pokaż tymczasową wiadomość
    if (!dbLoaded) {
        showTemporaryMessage("Baza danych była pusta – utworzono nową.", State::WELCOME);
    }
}

AtmGui::~AtmGui()
{
}

void AtmGui::initGui()
{
    setFixedSize(400, 600);
    setWindowTitle("Bankomat");
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 1. Ekran
    m_screen = new QTextEdit(this);
    m_screen->setReadOnly(true);
    m_screen->setFont(QFont("Monospace", 12));
    m_screen->setMinimumHeight(250);
    
    // 2. Linia wprowadzania
    m_inputLine = new QLineEdit(this);
    m_inputLine->setReadOnly(true);
    m_inputLine->setFont(QFont("Monospace", 14));
    m_inputLine->setAlignment(Qt::AlignRight);
    
    mainLayout->addWidget(m_screen);
    mainLayout->addWidget(m_inputLine);
    
    // 3. Klawiatura
    createKeypad();
    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (mainLayout) {
        mainLayout->addLayout(keypadLayout);
    } else {
        qWarning("Nie można dodać klawiatury – brak mainLayout!");
    }
}

void AtmGui::createKeypad()
{
    QGridLayout* keypadLayout = new QGridLayout();
    
    // Klawisze 1-9
    for (int i = 1; i <= 9; ++i) {
        QPushButton* btn = new QPushButton(QString::number(i), this);
        btn->setFixedSize(80, 60);
        btn->setFont(QFont("Arial", 14, QFont::Bold));
        connect(btn, &QPushButton::clicked, this, [this, i]() { onNumpadClicked(i); });
        keypadLayout->addWidget(btn, (i - 1) / 3, (i - 1) % 3);
    }
    
    // Klawisz 0
    QPushButton* btn0 = new QPushButton("0", this);
    btn0->setFixedSize(80, 60);
    btn0->setFont(QFont("Arial", 14, QFont::Bold));
    connect(btn0, &QPushButton::clicked, this, [this]() { onNumpadClicked(0); });
    keypadLayout->addWidget(btn0, 3, 1);
    
    // Klawisze funkcyjne
    QPushButton* btnCancel = new QPushButton("ANULUJ", this);
    btnCancel->setFixedSize(80, 60);
    btnCancel->setStyleSheet("background-color: red; color: white;");
    connect(btnCancel, &QPushButton::clicked, this, &AtmGui::onCancelClicked);
    
    QPushButton* btnClear = new QPushButton("COFNIJ", this);
    btnClear->setFixedSize(80, 60);
    btnClear->setStyleSheet("background-color: yellow;");
    connect(btnClear, &QPushButton::clicked, this, &AtmGui::onClearClicked);
    
    QPushButton* btnConfirm = new QPushButton("OK", this);
    btnConfirm->setFixedSize(80, 60);
    btnConfirm->setStyleSheet("background-color: green; color: white;");
    connect(btnConfirm, &QPushButton::clicked, this, &AtmGui::onConfirmClicked);
    
    keypadLayout->addWidget(btnCancel, 0, 3);
    keypadLayout->addWidget(btnClear, 1, 3);
    keypadLayout->addWidget(btnConfirm, 2, 3);
    
    // Pusty przycisk dla 0
    keypadLayout->addWidget(new QWidget(this), 3, 0); 
    keypadLayout->addWidget(new QWidget(this), 3, 2);
    keypadLayout->addWidget(new QWidget(this), 3, 3);


    dynamic_cast<QVBoxLayout*>(layout())->addLayout(keypadLayout);
}

// --- Logika Aplikacji (Maszyna Stanów) ---

void AtmGui::onNumpadClicked(int number)
{
    // Ignoruj cyfry w stanach, które ich nie oczekują
    if (m_currentState == State::SHOW_MESSAGE) return;

    m_inputBuffer.append(QString::number(number));
    
    // Maskowanie PINu
    if (m_currentState == State::LOGIN_PIN || m_currentState == State::REGISTER_PIN) {
        m_inputLine->setText(QString(m_inputBuffer.length(), '*'));
    } else {
        m_inputLine->setText(m_inputBuffer);
    }
}

void AtmGui::onClearClicked()
{
    m_inputBuffer.clear();
    m_inputLine->clear();
}

void AtmGui::onCancelClicked()
{
    m_sessionTimer->stop();
    m_currentCardNumber.clear();
    setState(State::WELCOME);
}

void AtmGui::onSessionTimeout()
{
    m_currentCardNumber.clear();
    showTemporaryMessage("Sesja wygasła.\nZostałeś wylogowany.", State::WELCOME);
}

void AtmGui::resetSessionTimer()
{
    m_sessionTimer->start(SESSION_TIMEOUT_MS);
}

void AtmGui::showTemporaryMessage(const QString& message, State nextState)
{
    m_screen->setText(message);
    m_currentState = State::SHOW_MESSAGE; // Stan tymczasowy
    m_inputLine->clear();
    m_inputBuffer.clear();
    
    // Po 3 sekundach przejdź do następnego stanu
    QTimer::singleShot(3000, this, [this, nextState]() {
        setState(nextState);
    });
}

void AtmGui::updateScreenText()
{
    switch (m_currentState) {
        case State::WELCOME:
            m_screen->setText("=== WITAJ W SYSTEMIE BANKOWYM ===\n\n1. Zaloguj się\n2. Zarejestruj nowe konto\n3. Wyjście");
            m_inputLine->setEchoMode(QLineEdit::Normal);
            break;
        case State::LOGIN_CARD:
            m_screen->setText("Podaj numer karty (16 cyfr)\ni naciśnij OK.");
            m_inputLine->setEchoMode(QLineEdit::Normal);
            break;
        case State::LOGIN_PIN:
            m_screen->setText("Podaj kod PIN (4 cyfry)\ni naciśnij OK.");
            m_inputLine->setEchoMode(QLineEdit::Password);
            break;
        case State::REGISTER_CARD:
            m_screen->setText("REJESTRACJA:\nPodaj nowy numer karty (16 cyfr)\ni naciśnij OK.");
            m_inputLine->setEchoMode(QLineEdit::Normal);
            break;
        case State::REGISTER_PIN:
            m_screen->setText("REJESTRACJA:\nPodaj nowy PIN (4 cyfry)\ni naciśnij OK.");
            m_inputLine->setEchoMode(QLineEdit::Password);
            break;
        case State::REGISTER_BALANCE:
            m_screen->setText("REJESTRACJA:\nPodaj saldo początkowe (np. 1000)\ni naciśnij OK.");
            m_inputLine->setEchoMode(QLineEdit::Normal);
            break;
        case State::MAIN_MENU:
            m_screen->setText(QString("Zalogowano jako: %1\n\n--- MENU GŁÓWNE ---\n\n1. Sprawdź saldo\n2. Wpłata\n3. Wypłata\n4. Zablokuj kartę\n5. Usuń konto\n6. Wyloguj")
                              .arg(m_currentCardNumber));
            m_inputLine->setEchoMode(QLineEdit::Normal);
            break;
        case State::BALANCE_CHECK:
            {
                long long balance = m_transaction.getBalance(m_currentCardNumber.toStdString());
                showTemporaryMessage(QString("Twoje saldo wynosi:\n\n%1 PLN").arg(balance), State::MAIN_MENU);
            }
            break;
        case State::DEPOSIT_AMOUNT:
            m_screen->setText("WPŁATA:\nPodaj kwotę do wpłaty (max 100000)\ni naciśnij OK.");
            break;
        case State::DEPOSIT_COUNT:
            m_screen->setText(QString("WPŁATA (Kwota: %1 PLN):\nPodaj liczbę banknotów (max 200)\ni naciśnij OK.").arg(m_tempAmount));
            break;
        case State::WITHDRAW_AMOUNT:
            m_screen->setText("WYPŁATA:\nPodaj kwotę do wypłaty (max 100000)\ni naciśnij OK.");
            break;
        case State::BLOCK_CONFIRM:
            m_screen->setText("Czy na pewno chcesz ZABLOKOWAĆ kartę?\nTej operacji nie można cofnąć z bankomatu.\n\n1. TAK\n2. NIE (Anuluj)");
            break;
        case State::DELETE_CONFIRM:
             m_screen->setText("Czy na pewno chcesz USUNĄĆ konto?\nTo usunie wszystkie środki i dane.\n\nWpisz '1' aby POTWIERDZIĆ.");
            break;
        default:
            break;
    }
}

void AtmGui::setState(State newState)
{
    m_currentState = newState;
    m_inputBuffer.clear();
    m_inputLine->clear();
    
    // Jeśli wchodzimy do stanu logowania, zatrzymujemy timer
    if (newState == State::WELCOME || newState == State::LOGIN_CARD || newState == State::LOGIN_PIN) {
        m_sessionTimer->stop();
        m_currentCardNumber.clear();
    } else {
        // Dla każdej innej akcji w menu głównym resetujemy timer
        resetSessionTimer();
    }
    
    updateScreenText();
}

void AtmGui::onConfirmClicked()
{
    // Zabezpieczenie przed wielokrotnym kliknięciem podczas wyświetlania wiadomości
    if (m_currentState == State::SHOW_MESSAGE) return;

    // Pobierz dane z bufora
    string input = m_inputBuffer.toStdString();
    long long inputNum = 0;
    bool isNum = readLongLong(input, inputNum);

    // Główna maszyna stanów
    switch (m_currentState) {
        
        case State::WELCOME:
            if (isNum && inputNum == 1) setState(State::LOGIN_CARD);
            else if (isNum && inputNum == 2) setState(State::REGISTER_CARD);
            else if (isNum && inputNum == 3) QApplication::instance()->quit();
            else onClearClicked();
            break;
            
        case State::LOGIN_CARD:
            if (m_transaction.existenceOfAccount(input)) {
                m_currentCardNumber = QString::fromStdString(input);
                setState(State::LOGIN_PIN);
            } else {
                showTemporaryMessage("Błąd: Nie znaleziono konta o podanym numerze.", State::WELCOME);
            }
            break;
            
        case State::LOGIN_PIN:
            {
                int pinCheck = m_transaction.checkPin(m_currentCardNumber.toStdString(), input);
                if (pinCheck == 1) { // Sukces
                    setState(State::MAIN_MENU);
                } else if (pinCheck == 0) {
                    showTemporaryMessage("Błędny PIN.", State::WELCOME);
                } else if (pinCheck == -2) {
                    showTemporaryMessage("Konto jest zablokowane.\nSkontaktuj się z bankiem.", State::WELCOME);
                } else {
                    showTemporaryMessage("Błąd systemu.", State::WELCOME);
                }
            }
            break;

        case State::MAIN_MENU:
            if (!isNum) { onClearClicked(); break; }
            switch(inputNum) {
                case 1: setState(State::BALANCE_CHECK); break;
                case 2: setState(State::DEPOSIT_AMOUNT); break;
                case 3: setState(State::WITHDRAW_AMOUNT); break;
                case 4: setState(State::BLOCK_CONFIRM); break;
                case 5: setState(State::DELETE_CONFIRM); break;
                case 6: setState(State::WELCOME); break;
                default: onClearClicked();
            }
            break;

        case State::DEPOSIT_AMOUNT:
            if (!isNum || inputNum <= 0 || inputNum > 100000) {
                showTemporaryMessage("Błędna kwota.\n(Musi być > 0 i <= 100000)", State::MAIN_MENU);
            } else {
                m_tempAmount = inputNum;
                setState(State::DEPOSIT_COUNT);
            }
            break;
            
        case State::DEPOSIT_COUNT:
            if (!isNum || inputNum <= 0 || inputNum > 200) {
                showTemporaryMessage("Błędna liczba banknotów.\n(Musi być > 0 i <= 200)", State::MAIN_MENU);
            } else {
                string result = m_transaction.deposit(m_currentCardNumber.toStdString(), m_tempAmount, inputNum);
                if (result == "OK") {
                    showTemporaryMessage("Wpłata zakończona pomyślnie.", State::MAIN_MENU);
                } else {
                    showTemporaryMessage(QString("Błąd wpłaty:\n%1").arg(QString::fromStdString(result)), State::MAIN_MENU);
                }
            }
            break;

        case State::WITHDRAW_AMOUNT:
            if (!isNum || inputNum <= 0 || inputNum > 100000) {
                showTemporaryMessage("Błędna kwota.\n(Musi być > 0 i <= 100000)", State::MAIN_MENU);
            } else {
                string result = m_transaction.withdrawal(m_currentCardNumber.toStdString(), inputNum);
                if (result == "OK") {
                    showTemporaryMessage("Wypłata zakończona pomyślnie.\nPobierz gotówkę.", State::MAIN_MENU);
                } else {
                    showTemporaryMessage(QString("Błąd wypłaty:\n%1").arg(QString::fromStdString(result)), State::MAIN_MENU);
                }
            }
            break;
            
        case State::BLOCK_CONFIRM:
            if (isNum && inputNum == 1) { // Potwierdź
                m_transaction.changeBlockStatus(m_currentCardNumber.toStdString(), true);
                m_transaction.saveData(bankdbPath);
                showTemporaryMessage("Karta została zablokowana.\nZostaniesz wylogowany.", State::WELCOME);
            } else {
                setState(State::MAIN_MENU); // Anuluj
            }
            break;

        case State::DELETE_CONFIRM:
            if (isNum && inputNum == 1) { // Potwierdź
                 m_transaction.deleteData(m_currentCardNumber.toStdString());
                 m_transaction.saveData(bankdbPath);
                 showTemporaryMessage("Konto zostało usunięte.\nZostaniesz wylogowany.", State::WELCOME);
            } else {
                setState(State::MAIN_MENU); // Anuluj
            }
            break;

        case State::REGISTER_CARD:
            if (m_transaction.existenceOfAccount(input)) {
                showTemporaryMessage("Ten numer karty już istnieje.\nSpróbuj inny.", State::WELCOME);
            } else if (input.length() != 16) { // Przykładowa walidacja
                 showTemporaryMessage("Numer karty musi mieć 16 cyfr.", State::REGISTER_CARD);
            } else {
                m_currentCardNumber = QString::fromStdString(input);
                setState(State::REGISTER_PIN);
            }
            break;
            
        case State::REGISTER_PIN:
             if (input.length() != 4 || !all_of(input.begin(), input.end(), ::isdigit)) {
                 showTemporaryMessage("PIN musi mieć 4 cyfry.", State::REGISTER_PIN);
             } else {
                m_tempPin = QString::fromStdString(input);
                setState(State::REGISTER_BALANCE);
             }
            break;

        case State::REGISTER_BALANCE:
            if (!isNum || inputNum < 0) {
                showTemporaryMessage("Saldo musi być liczbą nieujemną.", State::REGISTER_BALANCE);
            } else {
                m_transaction.addData(m_currentCardNumber.toStdString(), m_currentCardNumber.toStdString(), m_tempPin.toStdString(), inputNum);
                m_transaction.saveData(bankdbPath);
                showTemporaryMessage("Rejestracja pomyślna!\nMożesz się teraz zalogować.", State::WELCOME);
            }
            break;

        default:
            break;
    }
}