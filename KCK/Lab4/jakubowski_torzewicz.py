import sys
import numpy as np
import librosa

SR = 16000
FMIN = 65.0
FMAX = 300.0
PROG_F0 = 180.0
MAKS_CZAS_SEK = 3.0
DOMYSLNA_PLEC = "K"


def wczytaj_audio(sciezka):
    y, sr = librosa.load(sciezka, sr=SR, mono=True)

    if len(y) == 0:
        return y, sr

    y_przyciete, indeksy = librosa.effects.trim(y, top_db=25)
    if len(y_przyciete) >= 2048:
        y = y_przyciete

    # Wycięcie środkowego fragmentu dla długich nagrań ze względu na limit 1.0 s
    maks_probek = int(MAKS_CZAS_SEK * sr)
    if len(y) > maks_probek:
        start = (len(y) - maks_probek) // 2
        koniec = start + maks_probek
        y = y[start:koniec]

    return y, sr


def wyznacz_f0(y, sr):
    if len(y) < 512:
        return None

    f0_ramki = librosa.yin(y, fmin=FMIN, fmax=FMAX, sr=sr)

    # Odrzucenie ewentualnych wartości NaN oraz błędów spoza zakresu głosu
    poprawne_f0 = []
    for wartosc in f0_ramki:
        if not np.isnan(wartosc):
            if wartosc >= FMIN and wartosc <= FMAX:
                poprawne_f0.append(wartosc)

    if len(poprawne_f0) == 0:
        return None

    srednia_f0 = float(np.mean(poprawne_f0))
    return srednia_f0


def klasyfikuj_plec(f0):
    if f0 is None:
        return DOMYSLNA_PLEC

    if f0 < PROG_F0:
        return "M"
    else:
        return "K"


if __name__ == "__main__":
    try:
        if len(sys.argv) < 2:
            print(DOMYSLNA_PLEC)
        else:
            sciezka_pliku = sys.argv[1]
            sygnal, probkowanie = wczytaj_audio(sciezka_pliku)
            czestotliwosc_f0 = wyznacz_f0(sygnal, probkowanie)
            wynik = klasyfikuj_plec(czestotliwosc_f0)
            print(wynik)
    except Exception:
        print(DOMYSLNA_PLEC)
