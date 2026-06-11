# Konverterer et REVIEWBART ASCII-kart (fra png_to_map.py eller hånd-editert i Notepad) til
# YPilot JSON. Legende: x=vegg(1)  .=åpent(0)  S=spawn  #=fuel-stasjon  (space = åpent òg).
# Metadata kan settes via «# name:» / «# source:»-linjer i toppen (ellers defaults).
#
# Bruk: python retrorocket/ascii_to_json.py <inn.txt> <ut.json>
# Deretter: python retrorocket/gen_embedded.py   (regenererer maps-embedded.js)
import json, sys


def main():
    src, out = sys.argv[1], sys.argv[2]
    name, source, edgewrap = 'Kart', 'XPilot', True
    rows_txt = []
    for line in open(src, encoding='utf-8'):
        line = line.rstrip('\n').rstrip('\r')
        if line.startswith('#'):                       # kommentar / metadata
            low = line.lower()
            if 'name:' in low:   name = line.split(':', 1)[1].strip()
            elif 'source:' in low: source = line.split(':', 1)[1].strip()
            elif 'edgewrap:' in low: edgewrap = 'true' in low.split('edgewrap:', 1)[1] or low.split('edgewrap:', 1)[1].strip() in ('1', 'yes')
            continue
        if line == '':
            continue
        rows_txt.append(line)

    cols = max(len(r) for r in rows_txt)
    rows = len(rows_txt)
    tiles, spawns, fuel, wormholes = [], [], [], []
    for r, line in enumerate(rows_txt):
        row = []
        for c in range(cols):
            ch = line[c] if c < len(line) else '.'      # kortere linjer → åpent til høyre
            if ch == 'x' or ch == 'X':
                row.append(1)
            else:
                row.append(0)
                if ch == 'S' or ch == 's':
                    spawns.append({'col': c, 'row': r, 'player': len(spawns)})
                elif ch == '#':
                    fuel.append({'col': c, 'row': r})
                elif ch == '(':                          # wormhole inngang/utgang/begge
                    wormholes.append({'col': c, 'row': r, 'type': 'in'})
                elif ch == ')':
                    wormholes.append({'col': c, 'row': r, 'type': 'out'})
                elif ch == '@':
                    wormholes.append({'col': c, 'row': r, 'type': 'both'})
        tiles.append(row)

    data = {
        'name': name, 'source': source, 'cols': cols, 'rows': rows, 'cellPx': 24,
        'tiles': tiles, 'spawns': spawns, 'fuelStations': fuel,
        'edgewrap': edgewrap, 'gravity': 0.0, 'gravityZones': [], 'liquidZones': [],
        'wormholes': wormholes,
    }
    json.dump(data, open(out, 'w', encoding='utf-8'), ensure_ascii=False, separators=(',', ':'))
    wall = sum(sum(r) for r in tiles)
    print('OK %s: %dx%d, %d vegg (%.0f%%), %d spawns, %d fuel -> %s'
          % (name, cols, rows, wall, 100 * wall / (cols * rows), len(spawns), len(fuel), out))


if __name__ == '__main__':
    main()
