# YPilot — ToDo

Bevisst utsatte oppgaver (parkert, ikke glemt).

## ✅ FERDIG: Wormholes (`@` / `(` / `)`) — 2026-06-11

XPilot-wormholes. `(` = inngang, `)` = utgang, `@` = begge. Et skip som treffer en inngang
teleporteres til en **tilfeldig** utgang (fart bevart, cooldown `PHYSICS.wormholeCooldown` så
man ikke re-trigger ved utgangen).

**Implementert:** `xpilot_tools.py` + `ascii_to_json.py` trekker ut `wormholes:[{col,row,type}]`
(cellene forblir åpne). `buildMapFromJson` bærer dem celle→px. `Ship.update` teleporterer (per-
skip `wormholeCd`). Neon-ringer i `renderMap` (cyan inn / magenta ut / hvit begge), pulserer.
**Testkart:** `arena2` (konvertert, 22 wormholes — 10 in / 10 out / 2 both). Baser i lukkede rør
er nå nåbare.

## ✅ FERDIG: TR-II-soner — revers-gravitasjon + syre-bad — 2026-06-11

TR-IIs `zone`-konsept (fra `.game`: `zone x y w h fx fy g|l`). `convert_to_json.py` henter
`gravityZones`/`liquidZones` med kraftvektor (liquid beholder nå òg `fx,fy`).
- **Sone-kraft ERSTATTER global gravitasjon** innenfor (`zoneAt` + `Ship.update`); revers/sideveis
  følger av `fx,fy`-fortegn. Tunbar skala `PHYSICS.zoneForce` (fane «Soner»).
- **Syre-bad** (liquid): «spiser» skjoldet (rask drivstoff-dren mens skjold oppe), tærer skroget
  uten skjold (hp-skade → død); oppdrift fra sonens `fy`. Spawn-grace beskytter. Tunbart
  `acidShieldDrain`/`acidHullDamage`.
- Sone-overlays i `renderMap` (fiolett grav m/retningspil, grønn syre) + på radaren.
- **Testkart:** `likvidius4p` (syre-sone `0 -0.016 l`), `ekolos4p` (grav-sone `0 -0.02`).

## ✅ FERDIG: Radar (toggle) — 2026-06-11

Minimap instansieres nå ALLTID (både fit- og scroll-modus), styrt av en live `Radar`-toggle i
bunnlinja (`RADAR`, persistert `ypilot.radar`, default på). Soner + fuel + skip vises i oversikten.

## Fase 3 — Turboraketti-lag (parkert til senere)

- **Push-off / «fraspark» fra flater** — Turboraketti-signaturfølelsen: å skyte/dytte
  rakettstrålen mot en flate gir FREMDRIFT (føles nesten som et skjold i seg selv). Ikke
  laget ennå. Beslektet med — men ikke det samme som — dagens vegg-mekanikk: `wallLethalSpeed`/
  skjold-sprett lar deg sprette/dø mot en vegg; push-off skal i stedet gi deg kraft FRA den.
  Vegg-/kollisjonskoden er bevisst designet så den ikke stenger for dette (se CLAUDE.md
  «Følelses-quirks» + XPILOT-JAVASCRIPT-PLAN.md Fase 3). Mer kraft jo nærmere/mer rett-på flata.
- **Rakettstrålen som våpen** — eksosen gjør skade + dytter motstanderen (mer dytt mot skjold
  = større flate), torque fra off-center treff → uventet spinn → krasj (XPILOT-JAVASCRIPT-PLAN.md:474).

## Andre parkerte ting

- ✅ **Drivstoff-knapphet + lande-begrensning på TR-II-brett — GJORT (2026-06-12).** TR-II-kart
  har nå `closedLanding`: landing KUN på pads (spawn/plattformer), alt annet terreng = Kaboom
  (`onLandingPad`/`Ship.update`). Fuel hentes ved hovring nær **fuel-pods** (XPilot-koden) — pod
  ved hver base + sprinklet over åpne celler (`POD_SPACING` i `convert_to_json`); «fyll på hvilken
  som helst flate» fjernet → ekte drivstoff-press. Jf. memory [[drivstoff-og-liv]].
  - ✅ **Marker lovlige landingsflater med farge — GJORT.** Tykk neon-strek på pad-overflaten:
    hjem = spillerfarge, fuel = grønn, butikk/garasje = oransje (`renderMap`). XPilot-kart UTLEDER
    markører fra spawns som står på flat grunn → samme tydelige «her er basen»-strek.

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
- ✅ **Auto-shield / Newbie-modus** (toggle) — GJORT. Live-toggle «Newbie» i bunnlinja
  (global `NEWBIE`, persistert `ypilot.newbie`). PÅ → menneske-skip auto-skjolder rett før
  vegg-treff (fart-skalert føler langs fartsretning i `Ship.update`), og et skjoldet treff
  spretter ALLTID (dødelig-fart slått av for newbie-mennesker). Bots upåvirket. Se CLAUDE.md.
- ✅ **Gravitasjon på skudd** — GJORT. Kuler integreres nå med `GRAVITY` i `Bullet.update`
  (px/sek-body, samme akselerasjon som skipet: `GRAVITY·gravityScale·dtScale·60` på
  `velocity.y`). Per-kule `gravityScale` (default 1) → framtidige «tunge» skudd buer mer.
  Leser global `GRAVITY`, ikke skytterens tilstand → kula faller selv om skytteren var
  spawn-usårbar. På 0-gravitasjons-kart flyr kulene rett, som før.

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
