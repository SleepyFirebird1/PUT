QT       += core gui widgets

TARGET = AtmGui
TEMPLATE = app

# Wymagany C++17 (dla std::filesystem)
CONFIG += c++17

SOURCES += \
    main.cpp \
    atmgui.cpp

HEADERS += \
    atmgui.h \
    backend.h

# Należy dodać bibliotekę filesystem (kluczowe!)
# Dla GCC/Clang w systemie Linux/macOS
LIBS += -lstdc++fs
# Dla MSVC (Visual Studio) to zazwyczaj działa automatycznie z C++17

# Poinformuj kompilator, gdzie znaleźć nlohmann/json
# Załóżmy, że plik json.hpp jest w katalogu projektu
INCLUDEPATH += .