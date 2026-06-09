def konwertuj_col_na_txt(nazwa_wejsciowa, nazwa_wyjsciowa):
    try:
        # Odczyt pliku
        with open(nazwa_wejsciowa, "r") as f:
            lines = f.readlines()

        krawedzie = []
        liczba_wierzcholkow = 0

        # Parsowanie
        for line in lines:
            parts = line.strip().split()
            if not parts:
                continue

            # Szukamy linii 'p edge N M'
            if parts[0] == "p" and parts[1] == "edge":
                liczba_wierzcholkow = parts[2]

            # Szukamy krawędzi 'e U V'
            elif parts[0] == "e":
                u, v = parts[1], parts[2]
                krawedzie.append(f"{u} {v}")

        # Zapis do nowego pliku
        with open(nazwa_wyjsciowa, "w") as f:
            # Pierwsza linia: liczba wierzchołków
            f.write(f"{liczba_wierzcholkow}\n")
            # Kolejne linie: krawędzie
            f.write("\n".join(krawedzie))

        print(f"Gotowe! Utworzono plik '{nazwa_wyjsciowa}'")
        print(
            f"Znaleziono {liczba_wierzcholkow} wierzchołków i {len(krawedzie)} krawędzi."
        )

    except FileNotFoundError:
        print(f"Błąd: Nie znaleziono pliku '{nazwa_wejsciowa}' w tym folderze.")


# Uruchomienie funkcji
for instance in ["fpsol2.i.1", "mulsol.i.1", "mulsol.i.2", "mulsol.i.3"]:
    konwertuj_col_na_txt(f"{instance}.col", f"{instance}.txt")
