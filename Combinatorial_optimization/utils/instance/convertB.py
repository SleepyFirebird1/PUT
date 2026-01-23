import re

def convert_col_b_to_txt(input_file, output_file):
    print(f"Przetwarzanie pliku {input_file}...")
    
    with open(input_file, 'rb') as f:
        # 1. Odczyt długości nagłówka
        line = f.readline()
        if not line:
            print("Błąd: Pusty plik.")
            return
        header_len = int(line.strip())

        # 2. Odczyt i parsowanie nagłówka
        header = f.read(header_len).decode('ascii', errors='ignore')
        match = re.search(r'p edge (\d+) (\d+)', header)
        if not match:
            print("Błąd: Nie znaleziono definicji 'p edge' w nagłówku.")
            return
        
        N = int(match.group(1)) # Liczba wierzchołków
        print(f"Znaleziono {N} wierzchołków. Rozpoczynam dekodowanie bitmapy...")

        # 3. Odczyt danych binarnych
        bitmap = f.read()

    # 4. Zapis do pliku wynikowego
    with open(output_file, 'w') as out:
        # Zapisz liczbę wierzchołków w pierwszej linii
        out.write(f"{N}\n")
        
        bit_idx = 0
        edges_count = 0
        
        # Iteracja po macierzy sąsiedztwa (dolny trójkąt)
        # Format Culbersona: dla każdego wiersza i (od 1 do N), bity dla kolumn 1..i-1
        for i in range(1, N + 1):
            for j in range(1, i):
                byte_pos = bit_idx // 8
                bit_pos = 7 - (bit_idx % 8)

                if byte_pos < len(bitmap):
                    is_set = (bitmap[byte_pos] >> bit_pos) & 1
                    if is_set:
                        # Zapis w formacie: "Mniejszy Większy" (np. 1 2)
                        out.write(f"{j} {i}\n")
                        edges_count += 1
                bit_idx += 1
    
    print(f"Gotowe! Zapisano {edges_count} krawędzi do pliku '{output_file}'.")

# Uruchomienie funkcji
for instance in ["flat300_20_0","flat300_26_0","flat300_28_0","flat1000_60_0","flat1000_76_0"]:
    convert_col_b_to_txt(f"{instance}.col.b", f"{instance}.txt")