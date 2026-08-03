---
name: ascii-kart-arbeidsflyt
description: "Når Tommy har editert et ASCII-kart (.txt), er DET sannheten — aldri regenerer fra PNG over editen"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 24aa0ff3-c4ca-4b5c-af23-53ef16e6a0d2
---

For bilde→kart-pipelinen (`retrorocket/png_to_map.py` → `maps-ascii/*.txt` → `ascii_to_json.py`
→ `gen_embedded.py`): når Tommy har hånd-editert `.txt`-fila, er DEN kilden. **Ikke kjør
`png_to_map.py` på nytt** — det overskriver editen hans (skjedde to ganger med Calvin & Hobbes:
hår-join + ramme). Gjør bare ASCII→JSON→embedded.

**Why:** Tommy reviewer/editerer kart direkte i Notepad (derfor ASCII-først). Å re-terskle PNG-en
kaster bort manuelt arbeid og er vanskelig å oppdage.

**How to apply:** Etter første `png_to_map.py`-kjøring: behandle `.txt` som master. Endringer
(fjerne ramme, fikse detaljer, flytte spawns/fuel) gjøres i `.txt` (programmatisk surgical edit
ELLER la Tommy gjøre det), aldri ved å regenerere fra bildet. Mekaniske masse-endringer (f.eks.
strippe en 2-celle-ramme på et 150-bredt kart) kan jeg gjøre programmatisk på `.txt` — men bevar
alle interiør-celler. Wraparound-kart skal ha INGEN ramme + `edgewrap: true`.
