# XPilot .map-verktøy: skann maps/ for «spillbare» kart, og konverter .map → YPilot-JSON
# (samme schema som TRII-konverteren, så in-engine-loader + previewer dekker begge familier).
#
# Bruk:
#   python xpilot_tools.py scan                      → maps/playable-candidates.json + tabell
#   python xpilot_tools.py convert <inn.map> <ut.json>
import sys, os, re, json

SOLID = set('xqwas')          # 'x' = vegg, q/w/a/s = 45°-skråninger (solide i kollisjon)
EXCLUDE = {'xpilot-all-133-maps/dog1776.map'}   # kjent ødelagt (jf. EXCLUDED_MAPS i game.js)

def _int(s, default=0):
    m = re.match(r'\s*(-?\d+)', s or '')        # robust: ledende heltall (tåler «200;» o.l.)
    return int(m.group(1)) if m else default

def parse_xpilot(text, name=None):
    lines = text.replace('\r\n', '\n').replace('\r', '\n').split('\n')
    header = {}; delim = 'EndOfMapdata'; mapline = -1
    for i, l in enumerate(lines):
        t = l.strip()
        if re.match(r'^mapdata\s*:', t, re.I):
            mm = re.search(r'multiline\s*:\s*(\S+)', t, re.I)
            if mm: delim = mm.group(1)
            mapline = i; break
        if not t or t[0] == '#': continue
        kv = re.match(r'^([^:]+?)\s*:\s*(.*)$', t)
        if kv: header[kv.group(1).strip().lower()] = kv.group(2).strip()
    grid = []
    for j in range(mapline + 1, len(lines)):
        if lines[j].strip().lower() == delim.lower(): break
        grid.append(lines[j])
    R = _int(header.get('mapheight')) or len(grid)
    C = _int(header.get('mapwidth')) or (max((len(r) for r in grid), default=0))

    tiles, spawns, fuel = [], [], []
    for r in range(R):
        src = grid[r] if r < len(grid) else ''
        row = []
        for c in range(C):
            ch = src[c] if (c < len(src) and src[c] != '\t') else ' '
            row.append(1 if ch in SOLID else 0)
            if ch == '_' or ('0' <= ch <= '9'):
                spawns.append({'col': c, 'row': r, 'player': int(ch) if ch.isdigit() else len(spawns)})
            elif ch == '#':
                fuel.append({'col': c, 'row': r})
        tiles.append(row)

    ew = header.get('edgewrap', '').lower() != 'no' and header.get('edgebounce', '').lower() != 'yes'
    grav = 0.0
    try:
        g = abs(float(header.get('gravity', '0')))
        if g <= 0.10: grav = g
    except ValueError:
        pass
    return {'name': header.get('mapname') or name or 'kart', 'source': 'XPilot',
            'cols': C, 'rows': R, 'cellPx': 32, 'tiles': tiles, 'spawns': spawns,
            'fuelStations': fuel, 'edgewrap': ew, 'gravity': grav,
            'gravityZones': [], 'liquidZones': []}

def metrics(m):
    cells = m['cols'] * m['rows']
    solid = sum(sum(r) for r in m['tiles'])
    return {'w': m['cols'], 'h': m['rows'], 'wallPct': round(100 * solid / max(1, cells), 1),
            'spawns': len(m['spawns']), 'fuel': len(m['fuelStations']), 'wrap': m['edgewrap']}

def looks_playable(mt):
    return (mt['spawns'] >= 2 and 5 <= mt['wallPct'] <= 55 and 16 <= max(mt['w'], mt['h']) <= 130)

def scan(maps_dir='maps'):
    index = json.load(open(os.path.join(maps_dir, 'index.json'), encoding='utf-8'))
    cands = []
    for e in index:
        f = e['file']
        if f in EXCLUDE: continue
        try:
            m = parse_xpilot(open(os.path.join(maps_dir, f), encoding='utf-8', errors='replace').read(), f)
            mt = metrics(m)
        except Exception as ex:
            print("ERR", f, ex); continue
        if looks_playable(mt):
            cands.append({'file': f, 'name': m['name'], **mt})
    # Sorter: foretrekk ~25 % vegg, så moderat størrelse.
    cands.sort(key=lambda c: (abs(c['wallPct'] - 25), max(c['w'], c['h'])))
    out = os.path.join(maps_dir, 'playable-candidates.json')
    json.dump(cands, open(out, 'w', encoding='utf-8'), ensure_ascii=False, indent=1)
    print("%d spillbare kandidater -> %s\n" % (len(cands), out))
    print("%-34s %7s %5s %5s %5s %s" % ('fil', 'dims', 'vegg%', 'spawn', 'fuel', 'wrap'))
    for c in cands[:25]:
        print("%-34s %3dx%-3d %5s %5d %5d %s"
              % (c['file'].split('/')[-1], c['w'], c['h'], c['wallPct'], c['spawns'], c['fuel'], c['wrap']))

def main():
    if sys.argv[1] == 'scan':
        scan()
    elif sys.argv[1] == 'convert':
        m = parse_xpilot(open(sys.argv[2], encoding='utf-8', errors='replace').read(), os.path.basename(sys.argv[2]))
        json.dump(m, open(sys.argv[3], 'w', encoding='utf-8'), separators=(',', ':'))
        mt = metrics(m)
        print("OK %s  %dx%d  vegg%%=%s spawns=%d fuel=%d wrap=%s -> %s"
              % (m['name'], mt['w'], mt['h'], mt['wallPct'], mt['spawns'], mt['fuel'], mt['wrap'], sys.argv[3]))

if __name__ == "__main__":
    main()
