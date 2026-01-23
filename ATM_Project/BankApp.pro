TEMPLATE = app
TARGET = BankApp
QT += core gui widgets

# Wymuszamy standard C++17
CONFIG += c++17

# Dodajemy folder include do ścieżki poszukiwania nagłówków
INCLUDEPATH += $$PWD/include

DISTFILES += \
    DataBase/BankDatabase.json \
    DataBase/CashStorage.txt

# Pliki nagłówkowe (z folderu include)
HEADERS += \
    include/CashStorage.h \
    include/DatabaseHandler.h \
    include/ReceiptStrategy.h \
    include/CashMachine.h \
    include/Transaction.h \
    include/PathResolver.h

# Pliki źródłowe (z folderu src)
SOURCES += \
    src/main_gui.cpp \
    src/CashStorage.cpp \
    src/DatabaseHandler.cpp \
    src/ReceiptStrategy.cpp \
    src/CashMachine.cpp \
    src/Transaction.cpp \
    src/PathResolver.cpp
