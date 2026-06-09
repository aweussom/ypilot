# Konverterer et B&W-bilde (silhuett) til et REVIEWBART ASCII-kart: mørke piksler → vegg ('x'),
# lyse → åpent ('.'), pluss spawns ('S') og fuel ('#') i romslige åpne celler. Brukt til å
# gjenskape Tommys «Calvin & Hobbes Dancing» XPilot-brett (~1992, UiT).
#
# Pipeline (ASCII-først så du kan review/hånd-editere i Notepad):
#   PNG --(dette skriptet)--> .txt  --(ascii_to_json.py)--> .json  --(gen_embedded.py)--> embedded
#
# Bruk: python retrorocket/png_to_map.py <inn.png> <ut.txt> [--width 150] [--thresh 128] [--border 2]
#
# - Nedskalerer bildet til <width> kolonner (høyde bevarer aspect), terskler hver celle på
#   gjennomsnittlig luminans (komposittert over hvit for RGBA), legger en solid ramme, fjerner
#   1-celle-flekker, og stempler spawns/fuel i romslige åpne celler. Tegn-legende i topp av fila.
import json, sys, argparse
from PIL import Image


def luminance_grid(path, width):
    im = Image.open(path).convert('RGBA')
    bg = Image.new('RGBA', im.size, (255, 255, 255, 255))   # komposittér over hvit (transparens → hvit)
    im = Image.alpha_composite(bg, im).convert('L')
    w, h = im.size
    cols = width
    rows = max(1, round(width * h / w))
    im = im.resize((cols, rows), Image.BILINEAR)             # områdesnitt-aktig nedskalering
    px = im.load()
    return cols, rows, [[px[c, r] for c in range(cols)] for r in range(rows)]


def despeckle(tiles, cols, rows):
    # Fjern isolerte 1-celle-vegger (ingen 4-nabo-vegg) og fyll isolerte 1-celle-hull.
    def wall(c, r):
        return 0 <= c < cols and 0 <= r < rows and tiles[r][c] == 1
    out = [row[:] for row in tiles]
    for r in range(rows):
        for c in range(cols):
            n = sum(wall(c + dc, r + dr) for dc, dr in ((1, 0), (-1, 0), (0, 1), (0, -1)))
            if tiles[r][c] == 1 and n == 0:
                out[r][c] = 0                                 # ensom vegg-flekk → bort
            elif tiles[r][c] == 0 and n == 4:
                out[r][c] = 1                                 # ensomt hull → fyll
    return out


def pick_open(tiles, cols, rows, count, margin=2, exclude=None):
    # Romslige åpne celler (alle 8 naboer åpne), jevnt fordelt via rutenett-sampling.
    import math
    exclude = exclude or set()
    def open_at(c, r):
        return 0 <= c < cols and 0 <= r < rows and tiles[r][c] == 0
    roomy = []
    for r in range(margin, rows - margin):
        for c in range(margin, cols - margin):
            if (c, r) not in exclude and all(open_at(c + dc, r + dr) for dc in (-1, 0, 1) for dr in (-1, 0, 1)):
                roomy.append((c, r))
    if not roomy:
        return []
    # Finmasket rutenett (≥ count ruter), ta én kandidat nærmest hvert rute-senter.
    g = max(2, math.ceil(math.sqrt(count)) + 1)
    picks, used = [], set()
    for gy in range(g):
        for gx in range(g):
            tx = (gx + 0.5) / g * cols
            ty = (gy + 0.5) / g * rows
            best = min(roomy, key=lambda p: (p[0] - tx) ** 2 + (p[1] - ty) ** 2)
            if best not in used:
                used.add(best); picks.append(best)
            if len(picks) >= count:
                return picks
    return picks


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('inp'); ap.add_argument('out')
    ap.add_argument('--name', default='Calvin & Hobbes Dancing')
    ap.add_argument('--width', type=int, default=150)
    ap.add_argument('--thresh', type=int, default=128)
    ap.add_argument('--border', type=int, default=0)        # 0 = ingen ramme (wraparound-kart)
    ap.add_argument('--no-wrap', action='store_true')       # sett edgewrap:false (lukket kart)
    a = ap.parse_args()
    edgewrap = not a.no_wrap

    cols, rows, lum = luminance_grid(a.inp, a.width)
    tiles = [[1 if lum[r][c] < a.thresh else 0 for c in range(cols)] for r in range(rows)]
    tiles = despeckle(tiles, cols, rows)

    # Solid ramme kun hvis eksplisitt bedt om (--border > 0). Wraparound-kart har INGEN ramme.
    b = a.border
    if b > 0:
        for r in range(rows):
            for c in range(cols):
                if r < b or r >= rows - b or c < b or c >= cols - b:
                    tiles[r][c] = 1

    spawns = pick_open(tiles, cols, rows, 8)
    fuel = pick_open(tiles, cols, rows, 4, margin=3, exclude=set(spawns))[:4]

    # Skriv REVIEWBAR ASCII: x=vegg, .=åpent, S=spawn, #=fuel. Round-trip via ascii_to_json.py.
    grid = [['x' if tiles[r][c] else '.' for c in range(cols)] for r in range(rows)]
    for (c, r) in fuel:    grid[r][c] = '#'
    for (c, r) in spawns:  grid[r][c] = 'S'
    lines = [
        '# %s' % a.name,
        '# YPilot ASCII-kart. Legende: x=vegg  .=åpent  S=spawn  #=fuel-stasjon.',
        '# Editer fritt i Notepad (behold rektangulært rutenett), så: ascii_to_json.py + gen_embedded.py',
        '# name: %s' % a.name,
        '# source: XPilot',
        '# edgewrap: %s' % ('true' if edgewrap else 'false'),
    ]
    lines += [''.join(row) for row in grid]
    open(a.out, 'w', encoding='utf-8', newline='\n').write('\n'.join(lines) + '\n')
    wallcount = sum(sum(row) for row in tiles)
    print('OK %s: %dx%d, %d vegg-celler (%.0f%%), %d spawns, %d fuel -> %s'
          % (a.name, cols, rows, wallcount, 100 * wallcount / (cols * rows), len(spawns), len(fuel), a.out))


if __name__ == '__main__':
    main()
