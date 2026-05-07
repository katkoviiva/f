import os

def extract_prg(d64_path, target_filename):
    with open(d64_path, "rb") as f:
        # Hakemisto alkaa uralta 18, sektorista 1
        # Lasketaan offset: (17 uraa * sektoreiden määrä) * 256 tavua
        # Uralle 18 asti on 45760 tavua. Sektori 1 alkaa kohdasta 45760 + 256.
        dir_track_offset = 45760
        
        # Käydään läpi hakemistosektorit (Ura 18, Sektorit 1-18)
        for sector in range(1, 19):
            f.seek(dir_track_offset + (sector * 256))
            sector_data = f.read(256)
            
            # Jokaisessa sektorissa on 8 tiedostomerkintää (32 tavua kpl)
            for i in range(0, 256, 32):
                entry = sector_data[i:i+32]
                if len(entry) < 32 or entry[2] == 0: continue # Tyhjä merkintä
                
                # Tiedostonimi on tavuissa 5-20 (PETSCII-muodossa)
                raw_name = entry[5:21].split(b"\xa0")[0] # Poistetaan tyhjät välit
                name = "".join([chr(b) if 32 <= b <= 126 else "" for b in raw_name])
                
                if name.upper() == target_filename.upper():
                    start_track = entry[3]
                    start_sector = entry[4]
                    print(f"Löytyi! Alkaa: Ura {start_track}, Sektori {start_sector}")
                    return save_file(f, start_track, start_sector, name)

    print("Tiedostoa ei löytynyt.")

def get_offset(track, sector):
    # Yksinkertaistettu lasku standardille D64-levylle (Track 1-35)
    track_offsets = [0, 0, 5376, 10752, 16128, 21504, 26880, 32256, 37632, 43008, 48384, 
                     53760, 59136, 64512, 69888, 75264, 80640, 86016, 91392, 96768, 102144, 
                     107520, 112896, 118272, 123648, 129024, 134400, 139776, 145152, 150528, 
                     154112, 157696, 161280, 164864, 168448, 172032]
    return track_offsets[track] + (sector * 256)

def save_file(f, track, sector, name):
    prg_data = bytearray()
    while track != 0:
        f.seek(get_offset(track, sector))
        data = f.read(256)
        next_track = data[0]
        next_sector = data[1]
        
        if next_track == 0: # Viimeinen sektori
            prg_data.extend(data[2:next_sector + 1])
        else:
            prg_data.extend(data[2:])
            
        track, sector = next_track, next_sector

    with open(f"{name}.prg", "wb") as out:
        out.write(prg_data)
    print(f"Tallennettu: {name}.prg ({len(prg_data)} tavua)")

# Käyttö:
extract_prg("8BB AI 10 Prompt Game.d64", "PELINNIMI")
