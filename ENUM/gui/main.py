import sys
import subprocess
import os
from PyQt5.QtWidgets import (
    QApplication,
    QMainWindow,
    QGridLayout,
    QWidget,
    QPushButton,
    QLineEdit,
    QTextEdit,
    QLabel,
    QDesktopWidget,
    QSpacerItem,
    QSizePolicy,
    QComboBox,
)
from PyQt5.QtCore import Qt


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        # Ustawienie okna głównego
        self.setWindowTitle(
            "Obliczanie Równania Nieliniowego Metodą Newtona Raphsona Drugiego Rzędu"
        )
        self.setGeometry(100, 100, 1400, 240)
        self.center_window()

        # Główny widget i układ siatki
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        grid_layout = QGridLayout()
        central_widget.setLayout(grid_layout)

        # Podpis
        self.label = QLabel("Wybierz Rodzaj Arytmetyki Zmiennopozycyjnej")
        grid_layout.addWidget(self.label, 0, 0)

        self.label1 = QLabel("Wynik")
        grid_layout.addWidget(self.label1, 3, 2)
        grid_layout.setAlignment(self.label1, Qt.AlignHCenter)

        # Zmienna do przechowywania aktualnego trybu
        self.current_mode = None

        # Przyciski wyboru trybu
        self.button = QPushButton("Rzeczywista")
        self.button.clicked.connect(lambda: self.set_mode("rzeczywista"))
        grid_layout.addWidget(self.button, 1, 0)
        self.button.setStyleSheet(
            """
            QPushButton {
                background-color: lightgrey;
                color: black;
                border: 0.3px solid black;
                border-radius: 10px;
                font-size: 13px;
                padding: 5px;
            }
            QPushButton:pressed {
                background-color: gray;  /* Kolor podczas kliknięcia */
            }
            """)

        self.button1 = QPushButton("Przedziałowa")
        self.button1.clicked.connect(lambda: self.set_mode("przedzialowa"))
        grid_layout.addWidget(self.button1, 2, 0)
        self.button1.setStyleSheet(
            """
            QPushButton {
                background-color: lightgrey;
                color: black;
                border: 0.3px solid black;
                border-radius: 10px;
                font-size: 13px;
                padding: 5px;
            }
            QPushButton:pressed {
                background-color: gray;  /* Kolor podczas kliknięcia */
            }
            """)

        self.button2 = QPushButton("Przedziałowa z danymi rzeczywistymi")
        self.button2.clicked.connect(lambda: self.set_mode("przedzialowa_rzeczywista"))
        grid_layout.addWidget(self.button2, 3, 0)
        self.button2.setStyleSheet(
            """
            QPushButton {
                background-color: lightgrey;
                color: black;
                border: 0.3px solid black;
                border-radius: 10px;
                font-size: 13px;
                padding: 5px;
            }
            QPushButton:pressed {
                background-color: gray;  /* Kolor podczas kliknięcia */
            }
            """)

        # Dodanie pustej przestrzeni, aby wyrównać przycisk
        spacer = QSpacerItem(20, 20, QSizePolicy.Minimum, QSizePolicy.Expanding)
        grid_layout.addItem(spacer, 4, 0)
        # Dodanie przycisku oblicz
        self.button3 = QPushButton("Oblicz")
        self.button3.setFixedSize(100, 25)
        self.button3.setStyleSheet(
            """
            QPushButton {
                background-color: cornflowerblue;
                color: white;
                border: 0.3px solid black;
                border-radius: 10px;
                font-size: 13px;
                padding: 5px;
            }
            QPushButton:pressed {
                background-color: gray;  /* Kolor podczas kliknięcia */
            }
            """
        )
        grid_layout.addWidget(self.button3, 2, 2)
        grid_layout.setAlignment(self.button3, Qt.AlignHCenter)
        self.button3.clicked.connect(self.count)

        # Dodanie rozwijanej listy wyboru dla równania
        self.equationDropdown = QComboBox()
        self.equationDropdown.setStyleSheet(
            """
            QComboBox {
                background-color: white;
                color: black;
                border: 0.3px solid black;
                border-radius: 10px; /* Zaokrąglenie rogów */
                font-size: 13px;
                padding: 5px;
            }
            """
        )
        grid_layout.addWidget(self.equationDropdown, 1, 2)

        # Załaduj pliki z folderu "biblioteki"
        self.load_equation_files()

        # Pole tekstowe: Wprowadź Punkt Początkowy
        self.initialGuess = QLineEdit()
        self.initialGuess.setPlaceholderText("Wprowadź Punkt Początkowy...")
        self.initialGuess.setStyleSheet(
            """
                color: black;
                border: 0.3px solid black;
                border-radius: 10px;
                font-size: 13px;
                padding: 5px;
            """
        )
        self.initialGuess.textChanged.connect(
            lambda: self.clear_placeholder(self.initialGuess)
        )
        grid_layout.addWidget(self.initialGuess, 0, 1)

        # Pole tekstowe: Wprowadź Tolerancje
        self.tolerance = QLineEdit()
        self.tolerance.setPlaceholderText("Wprowadź Tolerancje...")
        self.tolerance.setStyleSheet(
            """
                color: black;
                border: 0.3px solid black;
                border-radius: 10px;
                font-size: 13px;
                padding: 5px;
            """
        )
        self.tolerance.textChanged.connect(
            lambda: self.clear_placeholder(self.tolerance)
        )
        grid_layout.addWidget(self.tolerance, 0, 2)

        # Pole tekstowe: Wprowadź Maksymalna Liczbę Iteracji
        self.maxIterations = QLineEdit()
        self.maxIterations.setPlaceholderText("Wprowadź Maksymalna Liczbę Iteracji...")
        self.maxIterations.setStyleSheet(
            """
                color: black;
                border: 0.3px solid black;
                border-radius: 10px;
                font-size: 13px;
                padding: 5px;
            """
        )
        self.maxIterations.textChanged.connect(
            lambda: self.clear_placeholder(self.maxIterations)
        )
        grid_layout.addWidget(self.maxIterations, 0, 3)

        # Pole tekstowe: Wyświetlanie wyniku
        self.resultOutput = QTextEdit()
        self.resultOutput.setReadOnly(True)  # Ustawienie pola jako tylko do odczytu
        self.resultOutput.setStyleSheet(
            """
                color: black;
                background-color: lightgray;
                border: 0.3px solid black;
                border-radius: 10px;
                font-size: 13px;
                padding: 5px;
            """
        )
        grid_layout.addWidget(
            self.resultOutput, 4, 1, 1, 3
        )  # Pole zajmuje cały wiersz poniżej

    def center_window(self):
        # Pobierz geometrię okna
        qr = self.frameGeometry()
        # Pobierz środek ekranu
        cp = QDesktopWidget().availableGeometry().center()
        # Ustaw środek okna na środek ekranu
        qr.moveCenter(cp)
        self.move(qr.topLeft())

    def clear_placeholder(self, line_edit):
        if line_edit.text() == line_edit.placeholderText():
            line_edit.clear()

    # Metoda do ustawiania trybu
    def set_mode(self, mode):
        # Resetuj wszystkie przyciski i pola tekstowe
        self.reset_inputs()
        self.current_mode = mode

        # Zmień styl w zależności od wybranego trybu
        self.button.setStyleSheet(self.get_button_style(mode == "rzeczywista"))
        self.button1.setStyleSheet(self.get_button_style(mode == "przedzialowa"))
        self.button2.setStyleSheet(self.get_button_style(mode == "przedzialowa_rzeczywista"))

        # Załaduj odpowiednie pliki do rozwijanej listy
        self.load_equation_files()

    # Metoda do resetowania pól tekstowych i plików
    def reset_inputs(self):
        self.initialGuess.clear()
        self.tolerance.clear()
        self.maxIterations.clear()
        self.resultOutput.clear()

        # Resetuj pliki
        with open("input.txt", "w") as file:
            file.write("")
        with open("output.txt", "w") as file:
            file.write("")

    # Metoda do pobierania stylu przycisku
    def get_button_style(self, is_active):
        if is_active:
            return """
                QPushButton {
                    background-color: cornflowerblue;
                    color: white;
                    border: 0.3px solid black;
                    border-radius: 10px;
                    font-size: 13px;
                    padding: 5px;
                }
            """
        else:
            return """
                QPushButton {
                    background-color: lightgrey;
                    color: black;
                    border: 0.3px solid black;
                    border-radius: 10px;
                    font-size: 13px;
                    padding: 5px;
                }
            """
        
    def keyPressEvent(self, event):
        """Obsługa klawisza Enter do uruchamiania obliczeń."""
        if event.key() == Qt.Key_Return or event.key() == Qt.Key_Enter:
            self.count()

    def load_equation_files(self):
        """Załaduj pliki z folderu 'biblioteki' do rozwijanej listy."""
        library_path = "../biblioteki"
        if os.path.exists(library_path):
            files = [f for f in os.listdir(library_path) if f.endswith(".so")]

            # Filtrowanie plików w zależności od trybu
            if self.current_mode == "rzeczywista":
                filtered_files = [f for f in files if not f.endswith("_p.so")]
            elif self.current_mode == "przedzialowa" or self.current_mode == "przedzialowa_rzeczywista":
                filtered_files = [f for f in files if f.endswith("_p.so")]
            else:
                filtered_files = []

            # Wyczyść listę i załaduj przefiltrowane pliki
            self.equationDropdown.clear()
            self.equationDropdown.addItems(filtered_files)
        else:
            self.resultOutput.setPlainText("Folder 'biblioteki' nie istnieje.")

    def count(self):
        if not self.current_mode:
            self.resultOutput.setPlainText("Wybierz tryb przed rozpoczęciem obliczeń.")
            return

        # Pobierz dane z pól tekstowych
        equation = self.equationDropdown.currentText()  # Pobierz wybrane równanie
        initial_guess = self.initialGuess.text()
        tolerance = self.tolerance.text()
        max_iterations = self.maxIterations.text()

        # Weryfikacja danych
        if self.current_mode in ["rzeczywista", "przedzialowa_rzeczywista"]:
        # Sprawdź, czy initial_guess jest liczbą zmiennoprzecinkową
            try:
                float(initial_guess)
            except ValueError:
                self.resultOutput.setPlainText(
                    "Błąd: W trybie 'rzeczywista' lub 'przedziałowa rzeczywista' punkt początkowy musi być liczbą zmiennoprzecinkową."
                )
                return

        elif self.current_mode == "przedzialowa":
            # Sprawdź, czy initial_guess jest przedziałem w formacie [a, b]
            if not (initial_guess.startswith("[") and initial_guess.endswith("]") and "," in initial_guess):
                self.resultOutput.setPlainText(
                    "Błąd: W trybie 'przedziałowa' punkt początkowy musi być przedziałem w formacie [a, b]."
                )
                return
            
        # Sprawdź, czy tolerance jest liczbą zmiennoprzecinkową
        try:
            if tolerance.startswith("[") and tolerance.endswith("]") and "," in tolerance:
                raise ValueError("Tolerancja nie może być przedziałem.")
            float(tolerance)
        except ValueError:
            self.resultOutput.setPlainText(
                "Błąd: Tolerancja musi być liczbą zmiennoprzecinkową, a nie przedziałem."
            )
            return
        # Sprawdź, czy max_iterations jest liczbą całkowitą
        try:
            max_iterations = int(max_iterations)
            if max_iterations <= 0:
                raise ValueError
        except ValueError:
            self.resultOutput.setPlainText(
                "Błąd: Liczba iteracji musi być dodatnią liczbą całkowitą."
            )
            return
        

        # Ścieżka do pliku
        input_file_path = (
            "../build/input.txt"
        )
        output_file_path = (
            "../build/output.txt"
        )

        # Zapisz dane do pliku
        with open(input_file_path, "w") as file:
            file.write(f"{equation}\n")
            file.write(f"{initial_guess}\n")
            file.write(f"{tolerance}\n")
            file.write(f"{max_iterations}\n")
        # Wyczyść pola tekstowe
        self.initialGuess.clear()
        self.tolerance.clear()
        self.maxIterations.clear()

        # Wybierz odpowiedni plik wykonywalny
        if self.current_mode == "rzeczywista":
            executable_path = "../build/newton_raphson_arytmetyka_zwykla"
        elif self.current_mode == "przedzialowa":
            executable_path = "../build/newton_raphson_arytmetyka_przedziałowa_p"
        elif self.current_mode == "przedzialowa_rzeczywista":
            executable_path = "../build/newton_raphson_arytmetyka_przedziałowa"
        else:
            self.resultOutput.setPlainText("Nieznany tryb obliczeń.")
            return

        try:
            subprocess.run([executable_path], text=True)

            # Odczyt pliku
            with open(output_file_path, "r") as output_file:
                result = output_file.read()

            # Wyświetl wynik w polu tekstowym
            self.resultOutput.setPlainText(result)

            # Wyczyść pliki input.txt i output.txt
            with open(input_file_path, "w") as file:
                file.write("")
            with open(output_file_path, "w") as file:
                file.write("")
        except FileNotFoundError:
            self.resultOutput.setPlainText(
                "Nie znaleziono pliku wykonywalnego newton_raphson."
            )
        except Exception as e:
            self.resultOutput.setPlainText(f"Wystąpił błąd: {e}")


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec_())
