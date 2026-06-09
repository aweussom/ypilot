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

- ✅ **«Fyll» veggene med organisk mycel-nettverk** — GJORT. `buildMycel` + `solidDepthField`
  i game.js: vener seedes fra vegg-kantene (BFS-dybde 1) og vokser INNOVER i solid (styrt av
  dybde-gradient + hash-jitter + forgrening), tegnes som ADD-glødelinjer med kromatisk-
  aberrasjon (RGB-split, Yggdrasil-frynse) bakt inn i vegg-teksturen. Deterministisk per kart,
  stamme-løst. Tunbar «Mycel»-slider (Visuelt, 0 = av). Framtid: la det «gro» over tid
  (jf. levende-kart) — i dag er det statisk bakt.
  - ⚠️ **RE-THINK vegg-fyll-utseendet (framtidig).** Tommy er ikke fornøyd med mycel/lyn-
    looken — den duger FORELØPIG (viser tydelig hva som er fast vs. ikke-fast), men skal
    tenkes på nytt. Observasjon: XPilot-screenshots viser at FASTE soner var HEL-FYLT, men
    Tommy husker det ikke slik — han husker faste områder fylt med **TREKANTER**. Vurder
    derfor en trekant-tessellering / trekant-mønster-fyll av solide soner (evt. hel-fyll med
    subtil tekstur) i stedet for vene-nettet. Behold «viser fast vs. tomt»-funksjonen.
- **Edge-bounce** — for lukkede kart med ufullstendig vegg-kant (motoren wrapper
  alltid i dag). Unødvendig for kart med 100 % solid ramme (Ekolos).
- **Auto-shield / Newbie-modus** (toggle) — hjelp for de som krasjer i vegger hele
  tiden (tastatur-treghet / rust). Auto-skjold rett før vegg-treff: gjenbruk AI-ens
  skjold-refleks (`dangerClose && speed > shieldSpeed && fuel > shieldFuelMin`) på
  menneske-skip når modus er på. Vurder også mykere kollisjon i denne modusen.
- **Gravitasjon på skudd** (justerbar on/off) — la kuler påvirkes av gravitasjonen
  (buet skuddbane). I dag flyr kuler rett (Arcade-velocity, ingen gravitasjon).
  Krever at bullets integreres med GRAVITY (evt. egen `bulletGravity`-faktor).

## ✅ FERDIG: Bake vegger til tekstur (ytelse) — branch `perf/dynamic-texture-bake`

**Løst (2026-06-09).** Vegg-lagene bakes til ÉN `DynamicTexture` og vises som én quad
(`scene.wallImage`) i stedet for å re-tessellere titusenvis av linjesegmenter hver frame.
Geometri-capet er fjernet når baking er aktiv → full Avrunding/Organisk/Detalj.

**Hva forrige forsøk bommet på:** i Phaser 4 KØER `dt.draw(...)` bare kommandoer i et
buffer — man MÅ kalle **`dt.render()`** etterpå for å flushe dem til teksturen (se
`vendor/phaser/src/textures/DynamicTexture.js#render` + `DynamicTextureHandler`). Uten
`render()` ser man ingenting (akkurat det forrige RenderTexture-forsøk opplevde).

**Implementasjon (`game.js`):**
- `renderMap`: `scene.textures.addDynamicTexture('wallbake', w·s, h·s)` (cap 4096 → skalér
  `wallImage` opp med `1/s`), vis via `scene.add.image(...).setBlendMode(ADD)`. Fjern gammel
  tekstur+image ved kart-bytte.
- `rebuildNeonWalls`: tegn de tre ADD-lagene til offscreen-graphics → `dt.clear()` →
  `dt.draw(offs,0,0)` → **`dt.render()`**. DynamicTextureHandler honorerer hvert objekts
  blendMode (DRAW-kommando), så ADD-lagene akkumulerer korrekt inne i teksturen. Re-bakes
  kun ved slider-endring, ikke per frame. Geometri-cap kun i live-fallback.
- `GameScene.update`: pulsen animerer `image`-alpha (billig).

**Verifisert (chrome-devtools):** Testbane (small) + Kits 200×200 (treffer cap, skaleres) →
vegger vises i begge looker; **60 FPS** ved Avrunding=6/Organisk=24/Detalj=0.060 på stor-kart
(scenarioet som hakket før); ingen konsoll-feil; live re-bake ved slider-endring OK.

**Fallback:** hvis `addDynamicTexture` feiler (Canvas-renderer e.l.) → live graphics som før
(geometri-cap beholdt). Kamera-Glow-filteret blomstrer fortsatt de bakte veggene.
