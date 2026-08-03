---
name: xpilot-kartformat
description: Ekte XPilot .map-format — ASCII block-tile grid (IKKE linjesegmenter); full tegn-legende fra xpmap.h
metadata: 
  node_type: memory
  type: reference
  originSessionId: 9dd7ad4a-f8a4-4ab2-9c5d-447df56f4bae
---

De ekte XPilot-kartene (130 stk lastet ned i
`jpilot/maps/xpilot-all-133-maps/`, klassisk-format 1993–96) er IKKE
linjesegment-formatet (`Wall: x1,y1 x2,y2`) som ble oppfunnet i
`XPILOT-JAVASCRIPT-PLAN.md`. Det er et **ASCII block-tile grid**:

Struktur:
- Header med `nøkkel : verdi`-par (case-insensitivt; både camelCase og lowercase
  finnes): `mapwidth`, `mapheight`, `mapname`, `mapauthor`, `edgewrap`,
  `edgebounce`, `playershielding`, `allowplayerbounces`, osv.
- Deretter en blokk: `mapData: \multiline: EndOfMapdata` … `EndOfMapdata`, med ett
  tegn per rute (`mapwidth` × `mapheight` ruter, fast blokkstørrelse).

Tegn-legende (autoritativ, fra `kekyo/xpilot-ng` `src/common/xpmap.h`):
- ` ` space, `.` space-alt
- `x` fylt blokk (vegg)
- `s`/`w`/`a`/`q` = fire rettvinklede trekant-skråninger (REC_LU/LD/RU/RD) — 45°
  vegger som fyller halve ruta. `b`/`h`/`y`/`g`/`t` = DEKOR-versjoner (kun visuelt,
  ingen kollisjon)
- `#` fuel-stasjon
- `r`/`d`/`f`/`c` kanon opp/venstre/høyre/ned
- `_` base; `0`–`9` lag-baser; `$` base-attractor
- `@` wormhole normal, `(` wormhole inn, `)` wormhole ut
- `*` treasure, `^` tom treasure, `!` target
- `%` item-concentrator, `&` asteroid-concentrator
- `+`/`-` grav pos/neg, `>`/`<` med/mot klokka, `i`/`m`/`k`/`j` grav opp/ned/høyre/venstre
- `z` friksjons-sone
- `A`–`Z` checkpoints 0–25

**Konsekvens for YPilot:** `parseMap()` må parse header + tile-grid, ikke
linjesegmenter. Plan-dokumentets `parseMap()`-output-form
(`{walls:[{x1,y1,x2,y2}]}`) er feil. For neon-looken: trekk konturen rundt fylte
blokk-regioner → rene neon-linjer (skråningene gir 45°-kanter). Kollisjon blir
grid-basert (rute-oppslag + trekant for skråninger) — enklere og mer robust enn
sirkel-mot-vilkårlig-linje. `edgewrap`/`edgebounce` i headeren styrer direkte
wrap/bounce — se [[kollisjons-folelse]]. Se også [[levende-kart]].

Calvin & Hobbes Dancing-kartet er IKKE i 133-settet; må skaffes separat.
