---
name: retrorocket-map-format
description: RetroRocket (NDS homebrew) map binary format + Python PNG renderer; source of authentic TurboRaketti II maps
metadata: 
  node_type: memory
  type: reference
  originSessionId: 48e54b8e-67f7-4a88-aa3d-df3078009514
---

`retrorocket/` holder kildekode + data fra RetroRocket (GPL NDS-homebrew, Asbjørn
Djupdal & Morten Hartmann) som inkluderer **alle originale TurboRaketti II-kart**
(likvidius, ekolos, metarola, sitimus, tropulus, +4p-varianter) — autentisk kilde
for å importere ekte TR2-baner til YPilot (jf. «last ekte kart» i CLAUDE.md).

**Kartformat** (NDS bakgrunns-tiles, dekodet fra `src/field.cpp` `getPixel`/`getTile`):
- `*_Info.bin` = 3× int32 LE: `[mode, width_px, height_px]`. mode 2=megatile,
  4=large, 5=infinite.
- `*_Tiles.bin` = 8×8 tiles, **1 byte/piksel** (palett-indeks), 64 B/tile.
- `*_Pal.bin` = RGB555 u16, 5 bit/kanal (R=bit0-4, G=5-9, B=10-14); indeks 0 = transparent.
- `*_Map.bin` = **u32/celle** i desktop-eksporten: tile-indeks i **12 bit (0xfff)**,
  flip-flagg på **bit 29/30 (0x20000000/0x40000000)** (OR av alle celler = 0x60000fff).
  (field.cpp sine konstanter TILE_N=1023/HFLIP=1024/VFLIP=2048 gjelder DS-u16-varianten
  — kilden kaller selv `getTile` «just guessing».)
- Tile-adressering: mode 4/5 → `tilePos=(y>>3)*(w>>3)+(x>>3)` (lineær, stride=w/8);
  mode 2 → megatiles 256×256px (se `getTile`).

**Fallgruve som kostet tid:** for smal tile-indeks-maske wrapper høye tiles (brukt
nederst i kartet) til feil tiles → øvre del riktig, nedre del rotete. Riktig maske
er `&0xfff` (12 bit) for u32-kartene; et 11-bits forsøk (0x7ff) etterlot et rotete
bunn-belte på kart med >2048 tiles (ekolos/metarola/sitimus).

Renderer: `retrorocket/render_maps.py` → PNG i `retrorocket/png_out/`. Velger u32/u16
ut fra filstørrelse vs. `(w/8)*(h/8)` celler. Python+Pillow, ingen kompilator nødvendig.
