#define MPFR_USE_NO_MACRO
#define MPFR_USE_INTMAX_T

#include <iostream>
#include <fstream>
#include <dlfcn.h>
#include <stdexcept>
#include "interval.h"
#include "mpreal.h"

using namespace interval_arithmetic;

// Typy funkcji dla równania i pochodnych
typedef Interval<mpreal> (*EquationFunc)(const Interval<mpreal>&);

struct NewtonResult {
    Interval<mpreal> root;
    int iteration;
    Interval<mpreal> last_f_value;
};

NewtonResult newtonRaphsonSecondOrder(
    Interval<mpreal> x0, mpreal tolerance, int maxIterations,
    EquationFunc equation, EquationFunc first_derivative, EquationFunc second_derivative) {
    
    Interval<mpreal> num_x = x0;
    Interval<mpreal> f_value;

    for (int i = 0; i < maxIterations; ++i) {
        Interval<mpreal> f_value = equation(num_x);
        Interval<mpreal> f_prime = first_derivative(num_x);
        std::cerr << "f_prime: [" << f_prime.a << ", " << f_prime.b << "] dla num_x = [" << num_x.a << ", " << num_x.b << "]" << std::endl;
        Interval<mpreal> f_double_prime = second_derivative(num_x);
        std::cerr << "f_double: [" << f_double_prime.a << ", " << f_double_prime.b << "] dla num_x = [" << num_x.a << ", " << num_x.b << "]" << std::endl;

        // Obsługa zerowej pierwszej pochodnej
        if (f_prime.a <= 0 && f_prime.b >= 0) {
            throw std::runtime_error("Dzielenie przez zero: pierwsza pochodna wynosi 0.");
        }
        // Obsługa zerowej drugiej pochodnej
        if (f_double_prime.a <= 0 && f_double_prime.b >= 0) {
            throw std::runtime_error("Dzielenie przez zero: druga pochodna wynosi 0.");
        }

        // Oblicz wyróżnik
        Interval<mpreal> discriminant = f_prime * f_prime - (Interval<mpreal>(2, 2) * f_value * f_double_prime);
        if (discriminant.a < 0) {
            num_x = num_x + Interval<mpreal>(tolerance, tolerance); // Przesuń x o niewielką wartość
            continue;
        }
        int l;
        // Oblicz pierwiastek z wyróżnika
        Interval<mpreal> sqrt_discriminant = ISqrt(discriminant,l);

        // Próba obu pierwiastków wyróżnika
        Interval<mpreal> x_next1 = num_x - ((f_prime + sqrt_discriminant) / f_double_prime);
        Interval<mpreal> x_next2 = num_x - ((f_prime - sqrt_discriminant) / f_double_prime);
        std::cerr << "Iteracja " << i << ":\n";
        std::cerr << "  x_next1 = [" << x_next1.a << ", " << x_next1.b << "] szerokość = " << IntWidth(x_next1) << std::endl;
        std::cerr << "  x_next2 = [" << x_next2.a << ", " << x_next2.b << "] szerokość = " << IntWidth(x_next2) << std::endl;

        Interval<mpreal> x_next = (IntWidth(x_next1) < IntWidth(x_next2)) ? x_next1 : x_next2;

        
        std::cerr << "abs(x_next.a - num_x.a) = " << abs(x_next.a - num_x.a)
        << ", abs(x_next.b - num_x.b) = " << abs(x_next.b - num_x.b) << std::endl;
        
        // Warunek zbieżności
        if (abs(x_next.a - num_x.a) < tolerance && abs(x_next.b - num_x.b) < tolerance) {
            return {x_next, i+1, f_value};
        }

        // Aktualizacja przybliżenia
        num_x = x_next;
    }

    // Zwróć najlepsze przybliżenie po osiągnięciu maksymalnej liczby iteracji
    std::cerr << "Osiągnięto maksymalną liczbę iteracji. Zwracam najlepsze przybliżenie." << std::endl;
    return {num_x, maxIterations, f_value};
}

int main() {
    const std::string LIBRARY_PATH = "../biblioteki/";

    // Wczytaj nazwę biblioteki z pliku
    std::ifstream file("../build/input.txt");
    if (!file.is_open()) {
        std::cerr << "Nie można otworzyć pliku input.txt" << std::endl;
        return 1;
    }

    //std::string libraryName;
    std::string initialGuessStr;
    std::string toleranceStr;
    std::string maxIterationsStr;
    std::string libraryName;
    std::getline(file, libraryName);
    std::getline(file, initialGuessStr);
    std::getline(file, toleranceStr);
    std::getline(file, maxIterationsStr);

    file.close();
    
    // Konwersja wartości na odpowiednie typy
    Interval<mpreal> initialGuess = ParseInterval<mpreal>(initialGuessStr);
    mpreal tolerance = mpreal(toleranceStr);
    int maxIterations = std::stoi(maxIterationsStr);
    // Załaduj bibliotekę
    std::string libraryPath = LIBRARY_PATH + libraryName;
    void* handle = dlopen(libraryPath.c_str(), RTLD_LAZY);
    if (!handle) {
        std::cerr << "Nie można załadować biblioteki: " << dlerror() << std::endl;
        return 1;
    }

    // Wczytaj funkcje z biblioteki
    EquationFunc equation = (EquationFunc)dlsym(handle, "equation");
    EquationFunc first_derivative = (EquationFunc)dlsym(handle, "first_derivative");
    EquationFunc second_derivative = (EquationFunc)dlsym(handle, "second_derivative");

    if (!equation || !first_derivative || !second_derivative) {
        std::cerr << "Nie można załadować funkcji: " << dlerror() << std::endl;
        dlclose(handle);
        return 1;
    }

    try {
        // Wywołaj algorytm Newtona-Raphsona
        NewtonResult result = newtonRaphsonSecondOrder(initialGuess, tolerance, maxIterations,
                                                           equation, first_derivative, second_derivative);
        mpreal width = IntWidth(result.root);
        std::string width_str = width.toString();
        result.root.b = -result.root.b;

        // Wyświetlenie wyniku w konsoli
        std::string left, right;
        std::string left_f, right_f;
        result.root.IEndsToStrings(left, right);
        result.last_f_value.IEndsToStrings(left_f, right_f);
        int iteration = result.iteration;
        if (!right_f.empty() && right_f[0] == '-') {
            right_f.erase(0, 1);
        }

        // Zapisz wynik do pliku output.txt
        std::ofstream outputFile("../build/output.txt");
        if (!outputFile.is_open()) {
            std::cerr << "Nie można otworzyć pliku output.txt do zapisu. Sprawdź ścieżkę i uprawnienia." << std::endl;
            dlclose(handle);
            return 1;
        }
        outputFile << "Rozwiązanie: [" << left << ", " << right << "]" << std::endl;
        outputFile << "Iteracja w której osiągnięto wynik : " << iteration << std::endl;
        outputFile << "Wartość funkcji : [" << left_f << "," << right_f << "]" << std::endl;
        outputFile << "Szerokość przedziału : " << width_str << std::endl;
        outputFile.close();

    } catch (const std::exception& e) {
        std::cerr << "Błąd: " << e.what() << std::endl;

        // Zapisz błąd do pliku output.txt
        std::ofstream outputFile("../build/output.txt");
        if (outputFile.is_open()) {
            outputFile << "Błąd: " << e.what() << std::endl;
            outputFile.close();
        }
    }

    // Zamknij bibliotekę
    dlclose(handle);
    return 0;
}