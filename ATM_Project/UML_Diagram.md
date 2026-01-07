# UML Class Diagram for ATM_Project

```mermaid
classDiagram
    class CashStorage {
        #vector~int~ denominations
        #vector~long long~ quantities
        +CashStorage()
        +~CashStorage()
        +loadFromFile(path: string) bool
        +saveToFile(path: string) bool
        +getDenominations() vector~int~
        +getQuantities() vector~long long~
        +addNotes(index: size_t, count: long long) void
        +printStorage() void
        #ensureParentDir(path: string) bool
    }

    class CashDispenser {
        +CashDispenser()
        +depositAmount(amount: long long, number: long long, dbPath: string, logPath: string) bool
        +withdrawAmount(amount: long long, dbPath: string, logPath: string) bool
        +canDispense(amount: long long) bool
        +printReceipt(filePath: string, operation: string, totalAmount: long long, accountId: string) bool
        -appendLog(logPath: string, operation: string, changed: vector~long long~, totalAmount: long long) bool
    }

    class BankDatabaseHandler {
        +json db
        +loadData(filename: string) bool
        +saveData(filename: string) bool
        +addData(accountId: string, cardNumber: string, pin: string, balance: long long) bool
        +deleteData(cardNumber: string) bool
        +getBalance(cardNumber: string) long long
        +changeBalance(cardNumber: string, amount: long long) bool
        +changePin(cardNumber: string, newPin: string) bool
        +changeBlockStatus(cardNumber: string, shouldBeBlocked: bool) bool
        +checkPin(cardNumber: string, pinCheck: string) int
        +existenceOfAccount(cardNumber: string) bool
    }

    class Transaction {
        +deposit(cardNumber: string, amount: long long, number: long long) bool
        +canWithdrawal(amount: long long, cardNumber: string) bool
        +withdrawal(cardNumber: string, amount: long long) bool
    }

    class Service {
        +service_collect(amount: long long) bool
        +service_deposit(amount: long long, number: long long) bool
        +unblock_card(cardNumber: string) bool
    }

    class AtmGui {
        -QTextEdit* m_screen
        -QLineEdit* m_inputLine
        -State m_currentState
        -QString m_inputBuffer
        -QString m_currentCardNumber
        -QString m_tempPin
        -long long m_tempAmount
        -Transaction m_transaction
        -QTimer* m_sessionTimer
        -const int SESSION_TIMEOUT_MS
        +AtmGui(parent: QWidget*)
        +~AtmGui()
        -onNumpadClicked(number: int) void
        -onConfirmClicked() void
        -onClearClicked() void
        -onCancelClicked() void
        -onSessionTimeout() void
        -initGui() void
        -createKeypad() void
        -setState(newState: State) void
        -updateScreenText() void
        -showTemporaryMessage(message: QString, nextState: State) void
        -resetSessionTimer() void
    }

    CashStorage <|-- CashDispenser
    CashDispenser <|-- Transaction
    BankDatabaseHandler <|-- Transaction
    CashDispenser <|-- Service
    BankDatabaseHandler <|-- Service
    AtmGui *-- Transaction
```
