---
name: lokal-vs-nettverk-kart
description: YPilot — lokal 2-spiller kun for små kart som får plass på skjermen; store kart = kun nettverk
metadata: 
  node_type: memory
  type: project
  originSessionId: 9dd7ad4a-f8a4-4ab2-9c5d-447df56f4bae
---

Besluttet 2026-06-08: **lokal 2-spiller støtter kun kart som får plass på skjermen**
(små/medium). Store XPilot-kart (opp mot 600×600 ruter) spilles **kun over nettverk**
(framtidig LAN/Internett-lag) — vi gjør IKKE split-screen eller scrollende kamera for
lokal spilling.

**Konsekvens for kode:** blockSize er ALLTID 32 (verdens-enhet); kameraet zoomer for å
vise hele kartet (FIT=0.8 margin). Lokal-filteret er skjerm-uavhengig: et kart er lokalt
spillbart (fit-hele-kartet-modus) hvis `max(cols,rows) <= MAX_LOCAL_DIM` (=60, justerbar)
— større kart gir for små skip (selv 80×80 ble for smått; 120×120 umulig på 4K).

**REVIDERT 2026-06-09:** store kart skal LIKEVEL kunne spilles lokalt i **single-player
(1 menneske vs AI-bots)** via **scrolling-kamera** som følger menneskets skip + et
**minimap i et hjørne** som viser HELE kartet og ALLE spillere. Altså: fit-hele-kartet +
MAX_LOCAL_DIM gjelder fler-menneske-lokalt; single-player låser opp store kart med
scroll + minimap. Ekte nettverks-multiplayer på store kart er fortsatt framtid
(`jpilot/ACTION-BASED-MULTIPLAYER-OPTIONS.md`).

**IMPLEMENTERT 2026-06-09** i `game.js`: `GameScene.scrollMode = (humanCount===1 &&
!isLocallyPlayable)`. `setupCamera()` velger fit vs scroll (zoom 1); `updateCamera()`
sentrerer momentant på `humanShips[0]` hver frame. `Minimap`-klasse: tre Graphics-lag
(scrollFactor 0) — bakgrunn, statiske vegger (run-length per rad), dynamiske prikker +
kamera-utsyn-ramme. Velgeren viser store kart kun når AI er PÅ (merket «▸stor»).
Verifisert på Boo.map (321×474, 115k vegger) uten feil.

**Kjent begrensning:** momentan sentrering → wrap-kart viser tomrom utenfor kanten
(ingen toroidal dobbel-rendering). De fleste store kart har solid kant-vegg, så det
treffer sjelden. Toroidal rendering = framtidig forbedring.

**Nettverks-multiplayer (framtid):** når nettverkslaget for store kart skal bygges,
les `jpilot/ACTION-BASED-MULTIPLAYER-OPTIONS.md` i repoet (uttalt 2026-06-08) — den
beskriver hvordan action-basert nettverks-multiplayer kan støttes.

Se [[kart-navigator]] (velgeren filtrerer på dette) og [[ai-spiller]].
