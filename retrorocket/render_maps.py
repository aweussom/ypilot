# Renderer for RetroRocket NDS tile-maps -> PNG.
# Reimplementerer Field::getPixel / getTile fra src/field.cpp.
#   _Info.bin  = 3x int32 LE: [mode, width_px, height_px]
#   _Tiles.bin = 8x8 tiles, 1 byte/piksel (palett-indeks), 64 byte/tile
#   _Pal.bin   = RGB555 u16, 5 bit/kanal, indeks 0 = transparent
#   _Map.bin   = u16/tile: bit 0-9 tile-indeks, bit 1024 HFLIP, bit 2048 VFLIP
import sys, struct, os, glob
from PIL import Image

# Eksporterte (desktop) kart bruker u32-celler: bit 0-10 = tile-indeks (11 bit),
# bit 29/30 = flip-flagg. (field.cpp sine 10-bit-konstanter gjelder DS-u16-varianten.)
TILE_N32, HFLIP32, VFLIP32 = 0xfff, 0x20000000, 0x40000000
TILE_N16, HFLIP16, VFLIP16 = 0x3ff, 0x400, 0x800

def rd(p): return open(p, 'rb').read()

def get_tile_pos(info, x, y):
    mode, w, h = info[0], info[1], info[2]
    if mode == 2:  # Normal map (megatiles)
        mega = ((y >> 8) * (w >> 8)) + (x >> 8)
        shift = 4 if w > 256 else 3
        return mega * 1024 + (((y & 0xff) >> 3) * (w >> shift)) + ((x & 0xff) >> 3)
    elif mode in (4, 5):  # Large / Infinite map
        return ((y >> 3) * (w >> 3)) + (x >> 3)
    raise ValueError("ukjent mode %d" % mode)

def render(prefix, layer="", bg=(0, 0, 0)):
    info = struct.unpack('<3i', rd(prefix + "_Info.bin")[:12])
    mode, w, h = info
    tiles = rd(prefix + layer + "_Tiles.bin")
    mp = rd(prefix + layer + "_Map.bin")
    pal_raw = rd(prefix + layer + "_Pal.bin")
    # Kartcellene er lagret som u32 (eksportert desktop-format) eller u16 (DS).
    ncells = (w // 8) * (h // 8)
    if len(mp) >= ncells * 4:
        nmap = len(mp) // 4
        mapv = struct.unpack('<%dI' % nmap, mp[:nmap * 4])
        TILE_N, HFLIP, VFLIP = TILE_N32, HFLIP32, VFLIP32
    else:  # u16
        nmap = len(mp) // 2
        mapv = struct.unpack('<%dH' % nmap, mp[:nmap * 2])
        TILE_N, HFLIP, VFLIP = TILE_N16, HFLIP16, VFLIP16
    npal = len(pal_raw) // 2
    pal16 = struct.unpack('<%dH' % npal, pal_raw[:npal * 2])
    pal = [((c & 0x1f) * 255 // 31, ((c >> 5) & 0x1f) * 255 // 31,
            ((c >> 10) & 0x1f) * 255 // 31) for c in pal16]

    img = Image.new("RGB", (w, h), bg)
    px = img.load()
    for y in range(h):
        for x in range(w):
            tp = get_tile_pos(info, x, y)
            if tp >= nmap:
                continue
            tile = mapv[tp]
            idx = tile & TILE_N
            xx = (7 - (x & 7)) if (tile & HFLIP) else (x & 7)
            yy = (7 - (y & 7)) if (tile & VFLIP) else (y & 7)
            off = idx * 64 + yy * 8 + xx
            if off >= len(tiles):
                continue
            pi = tiles[off]
            if pi == 0 or pi >= len(pal):  # 0 = transparent (space)
                continue
            px[x, y] = pal[pi]
    return img

if __name__ == "__main__":
    data = sys.argv[1]
    out = sys.argv[2]
    os.makedirs(out, exist_ok=True)
    for info_path in sorted(glob.glob(os.path.join(data, "*", "*_Info.bin"))):
        prefix = info_path[:-len("_Info.bin")]
        name = os.path.basename(prefix)
        # Behold kun 4p-varianten der den finnes (metarola deler mappe med sin 4p-modus).
        # Hopp over collision-info, ikke-TR2-kart og non-4p-duplikatene.
        SKIP = {"splash", "skull", "ekolos", "likvidius", "sitimus", "tropulus"}
        if name.endswith("_col") or name in SKIP or name.startswith("thrust"):
            continue
        try:
            img = render(prefix)
            dst = os.path.join(out, name + ".png")
            img.save(dst)
            print("OK  %-14s %dx%d -> %s" % (name, img.width, img.height, dst))
        except Exception as e:
            print("ERR %-14s %s" % (name, e))
