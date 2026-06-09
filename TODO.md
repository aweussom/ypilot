# YPilot — ToDo

Bevisst utsatte oppgaver (parkert, ikke glemt).

## Wormholes (`@` / `(` / `)`)

XPilot-wormholes — kjernemekanikk flere kart er avhengige av. `(` = inngang,
`)` = utgang, `@` = begge. Et skip som treffer en inngang teleporteres til en
**tilfeldig** utgang (fart bevart, kort cooldown så man ikke re-trigger ved utgangen).

**Hvorfor:** flere kart har baser i lukkede 1-celle-rør med en `(` på toppen — man
skal akselerere opp i wormholen og teleporteres ut i arenaen. Uten dette blir basene
uunnslippelige feller (f.eks. `arena2` «Arena» 100×100, med `(`×10 `)`×10 `@`×2).
Diagnostisert 2026-06-09.

**Plan:**
- Konverter (`retrorocket/xpilot_tools.py` + `convert_to_json.py`): trekk ut
  `wormholes: [{col,row,type:'in'|'out'|'both'}]`; cellene forblir åpne.
- Motor (`game.js`): i `Ship.update`, treff på inn/`@`-celle → teleportér til en
  tilfeldig ut/`@`-celle, behold fart, sett kort cooldown.
- Konsekvens: kart der baser kun nås via wormhole (arena2) blir spillbare.

## Andre parkerte ting

- **Edge-bounce** — for lukkede kart med ufullstendig vegg-kant (motoren wrapper
  alltid i dag). Unødvendig for kart med 100 % solid ramme (Ekolos).
- **Auto-shield / Newbie-modus** (toggle) — hjelp for de som krasjer i vegger hele
  tiden (tastatur-treghet / rust). Auto-skjold rett før vegg-treff: gjenbruk AI-ens
  skjold-refleks (`dangerClose && speed > shieldSpeed && fuel > shieldFuelMin`) på
  menneske-skip når modus er på. Vurder også mykere kollisjon i denne modusen.
- **Gravitasjon på skudd** (justerbar on/off) — la kuler påvirkes av gravitasjonen
  (buet skuddbane). I dag flyr kuler rett (Arcade-velocity, ingen gravitasjon).
  Krever at bullets integreres med GRAVITY (evt. egen `bulletGravity`-faktor).

## PLAN: Bake vegger til tekstur (ytelse) — NESTE ØKT, EGEN BRANCH

**Mål:** bak de organiske vegg-konturene til ÉN tekstur, så per frame tegnes én quad
(ikke titusenvis av linjesegmenter). Da kan geometri-capet droppes og full Avrunding/
Organisk/Detalj kjøre jevnt. I dag: direkte rendering hver frame → hakker ved «alt på fullt»
(akseptabelt ved lav Detalj/Organisk, men ikke målet).

**Hva er prøvd (mislyktes):** `scene.add.renderTexture(0,0,w,h)` + `rt.draw([g1,g2,g3])`
i Phaser 4 → rendret INGENTING synlig (ingen feil i konsoll). Reversert. Koden står nå på
direkte/live graphics: se `scene.wallRT = null` i `renderMap`, og live-grenen i
`rebuildNeonWalls` (+ geometri-cap `Math.min(rounding,4)` og `subdivideLoop(...,10)`).

**Ny tilnærming å prøve (branch `perf/dynamic-texture-bake`):**
1. `const dt = scene.textures.addDynamicTexture(key, map.widthPx, map.heightPx)` (unik key
   per kart). Vis via `scene.add.image(0,0,key).setOrigin(0,0).setDepth(0)` (+ evt. ADD-blend).
2. Tegn lagene inn: prøv `dt.draw(graphics, 0, 0)`; hvis tomt, prøv batch-API:
   `dt.beginDraw(); dt.batchDraw(g, 0, 0); dt.endDraw();` (sjekk vendor
   `src/textures/DynamicTexture.js` for v4-signaturene — draw/beginDraw/batchDraw/stamp).
3. **Isoler først:** bak ÉN enkel fyll-graphics (f.eks. `g.fillStyle(0x00ffff).fillRect`)
   og bekreft at den vises. Da vet vi at dt-pipelinen funker før vi tar de tunge konturene.
4. Fallgruver å sjekke: (a) ADD-blend når man tegner INN i dt → prøv NORMAL inn i dt og
   ADD på `Image`-en ved visning; (b) Y-flip; (c) tekstur-størrelse (kits=6400px kan
   overstige GPU-grense — cap til 4096 og skaler `Image` opp ved behov); (d) premultiplied
   alpha gjør glød nesten usynlig.
5. Når det virker: i `rebuildNeonWalls`, ved slider-endring → `dt.clear()` + tegn på nytt
   (re-bake), IKKE per frame. Fjern geometri-capet (`Math.min(rounding,4)` → `rounding`,
   `subdivideLoop(...,10)` → tettere). Pulsen animerer `Image`-alpha (billig).
6. **Verifiser:** last Ekolos → vegger MÅ vises; scroll ved full Detalj/Organisk → jevnt.
   Behold kamera-Glow-filteret (Glød) — det blomstrer de bakte, lyse veggene.

**Relevante steder i `game.js`:** `renderMap` (lag dt + image), `rebuildNeonWalls`
(tegn/re-bake), `GameScene.update` neon-puls (animer image-alpha), `GameScene.create`
glow-filter. Hard-reload (Ctrl+F5) i nettleser — `game.js` caches.
