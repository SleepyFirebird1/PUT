TEMPLATE = app
TARGET = BankApp
QT += core gui widgets

# Wymuszamy standard C++17 (dla filesystem)
CONFIG += c++17

SOURCES += main_gui.cpp \
           # Nie dodawaj tutaj Account.cpp, jeśli zaincludowałeś go w main_gui.cpp
           # Najlepiej jednak rozdzielić to na .h i .cpp w przyszłości.

# Jeśli Twoje pliki .cpp są w tym samym folderze:
INCLUDEPATH += .