# Konverterer RetroRocket/TurboRaketti II-kart til YPilot-JSON.
#
# Kilde: en `*.game`-fil (tekst) + collision-tile-binær (`*_col_*.bin`).
#   - collision er en per-piksel solid-maske lagret som 8x8-tiles (u16/celle):
#     tile-fyll 64 = solid, 0 = åpen, delvis = myk grotte-kant.
#   - `platform x y width h N|g|a|b|r` → spawns (hjem for spiller N) + fuel (g).
#   - `zone x y w h fx fy g|l` → gravitasjons-felt (kraftvektor) / væske-hazard.
#
# Vi rekonstruerer full 640x640-maske (håndterer h/v-flip), block-reduserer til en
# valgt celle-størrelse (default 8 px = native), og klassifiserer hver celle solid/åpen
# med flertall. Skråning-forfining er utsatt (motoren har ikke skråning-kollisjon ennå);
# JSON-en beholder likevel nok til å legge det på senere.
#
# Bruk:  python convert_to_json.py <Navn.game> <ut.json> [celle_px=8] [solid_frac=0.5]
import sys, struct, os, json, re

OPEN, SOLID = 0, 1   # tile-koder (skråninger 2..5 reservert til senere)

def read_info(prefix):
    return struct.unpack('<3i', open(prefix + "_Info.bin", 'rb').read()[:12])  # mode,w,h

def tile_pos(mode, w, x, y):
    if mode == 2:
        mega = ((y >> 8) * (w >> 8)) + (x >> 8)
        shift = 4 if w > 256 else 3
        return mega * 1024 + (((y & 0xff) >> 3) * (w >> shift)) + ((x & 0xff) >> 3)
    return ((y >> 3) * (w >> 3)) + (x >> 3)            # mode 4/5: lineær

def build_mask(data_dir, col_tiles_name, col_map_name, info):
    """Full w*h solid-maske (1=solid) fra collision-laget."""
    mode, w, h = info
    tiles = open(os.path.join(data_dir, col_tiles_name), 'rb').read()
    mp = open(os.path.join(data_dir, col_map_name), 'rb').read()
    # collision-kartet er u16/celle (mindre enn foreground-u32).
    ncells = (w // 8) * (h // 8)
    if len(mp) >= ncells * 4:
        cells = struct.unpack('<%dI' % (len(mp)//4), mp); TILE_N, HF, VF = 0xfff, 0x20000000, 0x40000000
    else:
        cells = struct.unpack('<%dH' % (len(mp)//2), mp); TILE_N, HF, VF = 0x3ff, 0x400, 0x800
    mask = bytearray(w * h)
    for y in range(h):
        for x in range(w):
            tp = tile_pos(mode, w, x, y)
            if tp >= len(cells): continue
            t = cells[tp]; idx = t & TILE_N
            xx = (7 - (x & 7)) if (t & HF) else (x & 7)
            yy = (7 - (y & 7)) if (t & VF) else (y & 7)
            off = idx * 64 + yy * 8 + xx
            if off < len(tiles) and tiles[off] != 0:
                mask[y * w + x] = 1
    return mask, w, h

def downsample(mask, w, h, cell, solid_frac):
    """Block-reduser maske til grid; celle solid hvis ≥ solid_frac av pikslene er solide."""
    cols, rows = w // cell, h // cell
    thresh = solid_frac * cell * cell
    grid = []
    for r in range(rows):
        row = []
        for c in range(cols):
            s = 0
            for yy in range(r*cell, r*cell+cell):
                base = yy * w + c*cell
                s += sum(mask[base:base+cell])
            row.append(SOLID if s >= thresh else OPEN)
        grid.append(row)
    return grid, cols, rows

def parse_game(path):
    """Hent width/height/collision-filer + platform/zone-linjer fra .game-teksten."""
    g = {'platforms': [], 'zones': [], 'collision': None, 'width': 0, 'height': 0}
    for raw in open(path, encoding='utf-8', errors='replace'):
        line = raw.split('#', 1)[0].strip()
        if not line: continue
        tok = line.split()
        k = tok[0].lower()
        if k == 'width':  g['width'] = int(tok[1])
        elif k == 'height': g['height'] = int(tok[1])
        elif k == 'collision': g['collision'] = (tok[1], tok[2])           # tiles, map
        elif k == 'platform':
            # platform x y width <flags...>   (flags: 'h N' = hjem spiller N, g/a/b/r)
            x, y, wdt = int(tok[1]), int(tok[2]), int(tok[3])
            flags = tok[4:]
            p = {'x': x, 'y': y, 'w': wdt, 'home': None, 'fuel': False}
            i = 0
            while i < len(flags):
                f = flags[i]
                if f == 'h' and i+1 < len(flags): p['home'] = int(flags[i+1]); i += 2; continue
                if f == 'g': p['fuel'] = True
                i += 1
            g['platforms'].append(p)
        elif k == 'zone':
            x, y, zw, zh = map(int, tok[1:5]); fx, fy = float(tok[5]), float(tok[6])
            fl = tok[7:] if len(tok) > 7 else []
            g['zones'].append({'x': x, 'y': y, 'w': zw, 'h': zh, 'fx': fx, 'fy': fy,
                               'gravity': 'g' in fl, 'liquid': 'l' in fl})
    return g

def main():
    game_path, out_path = sys.argv[1], sys.argv[2]
    cell = int(sys.argv[3]) if len(sys.argv) > 3 else 8
    solid_frac = float(sys.argv[4]) if len(sys.argv) > 4 else 0.5
    data_dir = os.path.dirname(game_path)

    g = parse_game(game_path)
    # collision-filnavn i .game er relative til data_dir (f.eks. "ekolos4p/ekolos4p_col_Tiles.bin")
    col_tiles, col_map = g['collision']
    # Info-prefiks utledes fra collision-stien (samme undermappe/prefiks, uten _col_*).
    prefix = os.path.join(data_dir, col_tiles.replace('_col_Tiles.bin', ''))
    info = read_info(prefix)
    mask, w, h = build_mask(data_dir, col_tiles, col_map, info)
    grid, cols, rows = downsample(mask, w, h, cell, solid_frac)

    def to_cell(px): return px // cell
    spawns, fuel = [], []
    for p in g['platforms']:
        # TRII-plattformer er solide utskytnings-pads, men ligger IKKE i terreng-collision-
        # laget. Stempl pad-en som solid (2 celler tykk) og plassér spawn/fuel rett OVER den,
        # så skipet hviler på en flate (gravitasjon holder det) i stedet for å henge i lufta.
        prow = to_cell(p['y'])
        c0 = max(0, to_cell(p['x'] - p['w'] // 2))
        c1 = min(cols - 1, to_cell(p['x'] + p['w'] // 2))
        for rr in (prow, min(rows - 1, prow + 1)):
            for cc in range(c0, c1 + 1):
                grid[rr][cc] = SOLID
        center = (c0 + c1) // 2
        top = max(0, prow - 1)                  # hvile-celle rett over pad-en
        grid[top][center] = OPEN                # sørg for at den er åpen (carve)
        if p['home'] is not None:
            spawns.append({'col': center, 'row': top, 'player': p['home']})
        if p['fuel']:
            fuel.append({'col': center, 'row': top})
    grav_zones, liquid_zones = [], []
    for z in g['zones']:
        rect = {'col': to_cell(z['x']), 'row': to_cell(z['y']),
                'cols': max(1, to_cell(z['w'])), 'rows': max(1, to_cell(z['h']))}
        if z['liquid']: liquid_zones.append(rect)
        if z['gravity']: grav_zones.append({**rect, 'fx': z['fx'], 'fy': z['fy']})

    name = re.sub(r'\.game$', '', os.path.basename(game_path)).replace('_', ' ')
    out = {
        'name': name, 'source': 'TurboRaketti II', 'cols': cols, 'rows': rows,
        'cellPx': cell, 'tiles': grid, 'spawns': spawns, 'fuelStations': fuel,
        'gravityZones': grav_zones, 'liquidZones': liquid_zones,
    }
    json.dump(out, open(out_path, 'w'), separators=(',', ':'))
    solid = sum(r.count(SOLID) for r in grid)
    print("OK %s  %dx%d celler (cellePx=%d)  solid=%d (%.0f%%)  spawns=%d fuel=%d gravZ=%d liqZ=%d"
          % (name, cols, rows, cell, solid, 100*solid/(cols*rows),
             len(spawns), len(fuel), len(grav_zones), len(liquid_zones)))
    # ASCII-preview (nedskalert til ~72 kolonner for terminalen).
    step = max(1, cols // 72)
    print("--- topologi-preview (hver celle = %dpx; '#'=solid) ---" % cell)
    for r in range(0, rows, step):
        print("".join('#' if grid[r][c] else ' ' for c in range(0, cols, step)))

if __name__ == "__main__":
    main()
