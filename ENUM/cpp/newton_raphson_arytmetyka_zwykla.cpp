#include <quadmath.h>
#include <iostream>
#include <fstream>
#include <dlfcn.h>
#include <stdexcept>

// Typy funkcji dla równania i pochodnych
typedef __float128 (*EquationFunc)(__float128);

struct NewtonResult {
    __float128 root;
    int iteration;
    __float128 last_f_value;
};

NewtonResult newtonRaphsonSecondOrder(__float128 x0, __float128 tolerance, int maxIterations,
                                    EquationFunc equation, EquationFunc first_derivative, EquationFunc second_derivative) {
    __float128 num_x = x0;
    __float128 f_value;

    for (int i = 0; i < maxIterations; ++i) {
        __float128 f_value = equation(num_x);
        __float128 f_prime = first_derivative(num_x);
        __float128 f_double_prime = second_derivative(num_x);

        if (f_value == 0.0Q) {
            return {num_x, i + 1, f_value};
        }

        if (f_prime == 0.0Q) {
            throw std::runtime_error("Dzielenie przez zero: pierwsza pochodna wynosi 0.");
        }

        if (f_double_prime == 0.0Q) {
            throw std::runtime_error("Dzielenie przez zero: druga pochodna wynosi 0.");
        }

        __float128 discriminant = (f_prime * f_prime) - (2 * f_value * f_double_prime);
        if (discriminant < 0.0Q) {
            num_x += tolerance;
            continue;
        }
        __float128 sqrt_discriminant = sqrtq(discriminant);

        __float128 numerator = f_prime + sqrt_discriminant;
        __float128 x_next = num_x - (numerator / f_double_prime);

        if (fabsq(x_next - num_x) >= tolerance) {
            numerator = f_prime - sqrt_discriminant;
            x_next = num_x - (numerator / f_double_prime);
        }

        if (fabsq(x_next - num_x) < tolerance) {
            return {x_next, i + 1, f_value};
        }

        num_x = x_next;
    }

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

    std::string libraryName;
    std::string initialGuessStr;
    std::string toleranceStr;
    std::string maxIterationsStr;
    std::getline(file, libraryName);
    std::getline(file, initialGuessStr);
    std::getline(file, toleranceStr);
    std::getline(file, maxIterationsStr);

    file.close();

    // Konwersja wartości na odpowiednie typy
    __float128 initialGuess = strtoflt128(initialGuessStr.c_str(), nullptr);
    __float128 tolerance = strtoflt128(toleranceStr.c_str(), nullptr);
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

        // Wyświetlenie wyniku w konsoli
        char num[128];
        char iteration_r[128];
        char value[128];
        quadmath_snprintf(num, sizeof(num), "%.14Qe", result.root);
        snprintf(iteration_r, sizeof(iteration_r), "%d", result.iteration);
        quadmath_snprintf(value, sizeof(value), "%.14Qe", result.last_f_value);
        std::cout << "Rozwiązanie: " << num << std::endl;

        // Zapisz wynik do pliku output.txt
        std::ofstream outputFile("../build/output.txt");
        if (!outputFile.is_open()) {
            std::cerr << "Nie można otworzyć pliku output.txt do zapisu. Sprawdź ścieżkę i uprawnienia." << std::endl;
            dlclose(handle);
            return 1;
        }
        outputFile << "Rozwiązanie: " << num << std::endl;
        outputFile << "Iteracja w której osiągnięto wynik : " << iteration_r << std::endl;
        outputFile << "Wartość funkcji : " << value << std::endl;
        outputFile.close();

        std::cout << "Rozwiązanie zapisane do pliku output.txt" << std::endl;
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