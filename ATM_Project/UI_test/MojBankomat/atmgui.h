#ifndef ATMGUI_H
#define ATMGUI_H

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include "backend.h" // Nasza zrefaktoryzowana logika

class AtmGui : public QWidget
{
    Q_OBJECT

public:
    AtmGui(QWidget *parent = nullptr);
    ~AtmGui();

private slots:
    // Słoty dla przycisków
    void onNumpadClicked(int number);
    void onConfirmClicked();
    void onClearClicked();
    void onCancelClicked();
    void onSessionTimeout();

private:
    void initGui();
    void createKeypad();
    
    // Maszyna stanów
    enum class State {
        WELCOME,
        LOGIN_CARD,
        LOGIN_PIN,
        REGISTER_CARD,
        REGISTER_PIN,
        REGISTER_BALANCE,
        MAIN_MENU,
        BALANCE_CHECK,
        DEPOSIT_AMOUNT,
        DEPOSIT_COUNT,
        WITHDRAW_AMOUNT,
        BLOCK_CONFIRM,
        DELETE_CONFIRM,
        SHOW_MESSAGE // Stan do wyświetlania wiadomości przez chwilę
    };

    void setState(State newState);
    void updateScreenText();
    void showTemporaryMessage(const QString& message, State nextState);
    void resetSessionTimer();

    // Elementy GUI
    QTextEdit* m_screen;      // Główny wyświetlacz
    QLineEdit* m_inputLine;   // Linia pokazująca wpisywane cyfry
    
    // Stan wewnętrzny
    State m_currentState;
    QString m_inputBuffer;
    QString m_currentCardNumber;
    QString m_tempPin;
    long long m_tempAmount;

    // Logika backendu
    Transaction m_transaction; // Instancja logiki bankomatu
    
    // Czas sesji
    QTimer* m_sessionTimer;
    const int SESSION_TIMEOUT_MS = 10 * 60 * 1000; // 10 minut
};

#endif // ATMGUI_H