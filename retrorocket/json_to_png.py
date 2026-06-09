# Previewer for YPilot-kart-JSON → PNG. QA-verktøy for konverterte kart (TRII + XPilot).
# Tegner: solide celler, spawns (farge per spiller), fuel, gravitasjons-soner (m/ kraft-
# vektor-pil) og væske-soner. Neon-aktig, men funksjonell — dette er for inspeksjon.
#
# Bruk:  python json_to_png.py <kart.json> <ut.png> [px_per_celle=6]
import sys, json
from PIL import Image, ImageDraw

WALL   = (60, 110, 200)     # kjølig neon-blå (som COLORS.wall)
BG     = (10, 15, 30)
FUEL   = (51, 255, 153)
LIQUID = (40, 200, 90, 90)  # halvtransp. grønn (slim)
GRAVZ  = (180, 120, 255)    # lilla sone-ramme
PLAYER = [(0,255,255), (255,0,255), (255,204,51), (102,255,102)]  # p0..p3

def main():
    src, out = sys.argv[1], sys.argv[2]
    s = int(sys.argv[3]) if len(sys.argv) > 3 else 6
    m = json.load(open(src, encoding='utf-8'))
    cols, rows = m['cols'], m['rows']
    img = Image.new('RGB', (cols*s, rows*s), BG)
    d = ImageDraw.Draw(img, 'RGBA')

    # Solide celler
    tiles = m['tiles']
    for r in range(rows):
        row = tiles[r]
        for c in range(cols):
            if row[c]:
                d.rectangle([c*s, r*s, c*s+s-1, r*s+s-1], fill=WALL)

    # Væske-soner (under markørene)
    for z in m.get('liquidZones', []):
        d.rectangle([z['col']*s, z['row']*s, (z['col']+z['cols'])*s, (z['row']+z['rows'])*s], fill=LIQUID)

    # Gravitasjons-soner: ramme + kraftvektor-pil fra senter
    for z in m.get('gravityZones', []):
        x0, y0 = z['col']*s, z['row']*s
        x1, y1 = (z['col']+z['cols'])*s, (z['row']+z['rows'])*s
        d.rectangle([x0, y0, x1, y1], outline=GRAVZ, width=2)
        cx, cy = (x0+x1)//2, (y0+y1)//2
        # fx/fy er piksel/frame-kraft; skalér opp for synlighet
        ex, ey = cx + z.get('fx',0)*1200, cy + z.get('fy',0)*1200
        d.line([cx, cy, ex, ey], fill=GRAVZ, width=2)
        d.ellipse([ex-3, ey-3, ex+3, ey+3], fill=GRAVZ)

    # Fuel
    for f in m.get('fuelStations', []):
        cx, cy = f['col']*s+s//2, f['row']*s+s//2
        d.ellipse([cx-s, cy-s, cx+s, cy+s], outline=FUEL, width=2)

    # Spawns (farge per spiller, m/ nummer)
    for sp in m.get('spawns', []):
        cx, cy = sp['col']*s+s//2, sp['row']*s+s//2
        col = PLAYER[sp.get('player', 0) % len(PLAYER)]
        d.ellipse([cx-s, cy-s, cx+s, cy+s], fill=col)
        d.text((cx+s, cy-s), str(sp.get('player','?')), fill=col)

    img.save(out)
    print("OK -> %s  (%dx%d px, %d×%d celler)" % (out, img.width, img.height, cols, rows))

if __name__ == "__main__":
    main()
