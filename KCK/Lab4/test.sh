#!/bin/bash

cd "$(dirname "$0")" || exit 1

PYTHON="python3"
if [ -x "../venv/bin/python3" ]; then
    PYTHON="../venv/bin/python3"
fi

PLIK_WYNIKOW="wyniki.txt"
> "$PLIK_WYNIKOW"

for plik in trainall/*.wav; do
    nazwa=$(basename "$plik")
    wynik=$("$PYTHON" jakubowski_torzewicz.py "$plik")
    echo "$nazwa $wynik" >> "$PLIK_WYNIKOW"
done
