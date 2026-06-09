# Genererer maps-embedded.js fra maps-json/*.json → window.EMBEDDED_MAPS = { nøkkel: {...} }.
# Embeddet (ingen fetch) så de nye JSON-kartene laster også ved file:// dobbeltklikk.
# Auto-generert; ikke rediger maps-embedded.js for hånd — kjør dette på nytt.
import json, glob, os, sys

src_dir = sys.argv[1] if len(sys.argv) > 1 else 'maps-json'
out = sys.argv[2] if len(sys.argv) > 2 else 'maps-embedded.js'
maps = {}
for f in sorted(glob.glob(os.path.join(src_dir, '*.json'))):
    key = os.path.splitext(os.path.basename(f))[0]
    maps[key] = json.load(open(f, encoding='utf-8'))

with open(out, 'w', encoding='utf-8') as fh:
    fh.write("// Auto-generert av retrorocket/gen_embedded.py — ikke rediger for hånd.\n")
    fh.write("// Embeddede YPilot-kart i nytt JSON-format (TRII + konverterte XPilot).\n")
    fh.write("window.EMBEDDED_MAPS = ")
    json.dump(maps, fh, ensure_ascii=False, separators=(',', ':'))
    fh.write(";\n")
print("OK %d kart -> %s (%d bytes): %s"
      % (len(maps), out, os.path.getsize(out), ", ".join(maps.keys())))
