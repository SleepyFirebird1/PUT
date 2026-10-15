import json
import struct
import os


def clean_glb(filename):
    path = os.path.join("assets", filename)
    if not os.path.exists(path):
        print(f"[{filename}] Nie znaleziono pliku.")
        return

    with open(path, "rb") as f:
        data = f.read()

    # GLB header to 12 bajtów
    magic, version, length = struct.unpack("<4sII", data[:12])
    # Kolejne 8 bajtów to nagłówek danych JSON
    json_chunk_len, chunk_type = struct.unpack("<II", data[12:20])

    # Wyciągamy nagłówki modelu w formacie JSON
    json_data = data[20 : 20 + json_chunk_len]
    gltf = json.loads(json_data.decode("utf-8"))

    modified = False

    # Krok 1 - Usuwamy uciążliwe rozszerzenia
    bad_extensions = ["KHR_materials_specular", "KHR_materials_ior"]

    for ext_key in ["extensionsUsed", "extensionsRequired"]:
        if ext_key in gltf:
            for bad_ext in bad_extensions:
                if bad_ext in gltf[ext_key]:
                    gltf[ext_key].remove(bad_ext)
                    modified = True

    # Krok 2 - Usuwamy powiązania bad_extensions z materiałów
    if "materials" in gltf:
        for mat in gltf["materials"]:
            if "extensions" in mat:
                for bad_ext in bad_extensions:
                    if bad_ext in mat["extensions"]:
                        del mat["extensions"][bad_ext]
                        modified = True

    # Krok 3 - Usuwamy żądanie dodatkowych map UV z siatki (moderngl_window akceptuje tylko TEXCOORD_0)
    if "meshes" in gltf:
        for mesh in gltf["meshes"]:
            if "primitives" in mesh:
                for prim in mesh["primitives"]:
                    if "attributes" in prim:
                        for bad_uv in ["TEXCOORD_1", "TEXCOORD_2", "TEXCOORD_3"]:
                            if bad_uv in prim["attributes"]:
                                del prim["attributes"][bad_uv]
                                modified = True

    if not modified:
        print(f"[{filename}] Nie znaleziono KHR_materials_specular, plik jest czysty.")
        return

    # Krok 3 - Pakujemy to z powrotem zachowując zasady (wyrównanie do 4 bajtów spacjami)
    new_json = json.dumps(gltf).encode("utf-8")
    padding_length = (4 - (len(new_json) % 4)) % 4
    new_json += b" " * padding_length

    new_chunk_len = len(new_json)

    # Składamy nowy plik GLB do kupy
    new_length = 12 + 8 + new_chunk_len + (length - (20 + json_chunk_len))

    out_data = struct.pack("<4sII", magic, version, new_length)
    out_data += struct.pack("<I4s", new_chunk_len, b"JSON")
    out_data += new_json
    out_data += data[20 + json_chunk_len :]  # reszta pliku bez zmian

    # Zapis
    with open(path, "wb") as f:
        f.write(out_data)

    print(f"[{filename}] Pomyślnie wycięto KHR_materials_specular!")


if __name__ == "__main__":
    clean_glb("Go_tablev2.glb")
    clean_glb("Grass.glb")
    clean_glb("Flowers.glb")
