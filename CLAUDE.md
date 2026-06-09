# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

> **Språk:** Dette prosjektet kjøres på norsk. All dokumentasjon og alle
> kodekommentarer skrives på norsk. Eneste unntak: første del av en fremtidig
> `README.md` skal være på engelsk. (Denne ene engelske linjen øverst er en fast
> Claude Code-konvensjon og beholdes.)

## Prosjektstatus

**Fase 1 ✅ + mye av Fase 2 ✅.** `index.html` + `game.js` har: newtonsk fysikk
(manuell integrasjon + drag/terminal-fart), wrap, grid-vegg-kollisjon, skyting med
HP (skip tåler flere treff), jeteksos, ekte `.map`-lasting (`parseMap` + kart-velger,
areal-skalert skip-antall), justerbar gravitasjon (slider, localStorage), drivstoff
(én ressurs: gass+skyting forbruker, auto-fylling ved `#`-stasjon), landing på flate
topper, **bot-multiplayer free-for-all med takeover** (menneske=Starfighter, bot=TIE;
dø → overta dårligste bot, morf + frys + nedtelling), 3 liv → eliminert → siste vinner,
**skjold** (drenerer drivstoff, absorberer, spretter av vegg), blast-push, AI med
nødbrems, **stor-kart single-player** (scrolling-kamera som følger spilleren ved zoom 1
+ minimap nederst til høyre som viser hele banen og alle skip; små kart / fler-menneske
beholder fit-til-skjerm). Alle justerbare verdier er samlet i `PHYSICS`/`AI`/`GAME`
(TUNING-blokk øverst i `game.js`). Publisert via GitHub Pages (repo `aweussom/ypilot`).

Kart kan svartelistes i `EXCLUDED_MAPS` (kjent ødelagte i YPilot) — `dog1776.map` er
ute. Takeover skjer kun når ≥2 skip står igjen (ellers vinner siste skip).

**NESTE OPPGAVE: lyd** — gjenbruk Solstice-lydmotoren («soundbed» + SFX), se memory
`audio-fra-solstice`. Deretter: radar, så «prettification» (Solstice-grafikk).
`XPILOT-JAVASCRIPT-PLAN.md` har fasene; memory-filene har alle design-beslutninger og
framtids-notater.

> **Stor-kart-begrensning (kjent):** scroll-kameraet sentrerer momentant på skipet, så
> på wrap-kart vises tomrom utenfor kartkanten (ingen toroidal dobbel-rendering ennå).
> De fleste store kart har solid kant-vegg, så skipet når sjelden den ekte kanten.
> Toroidal rendering er en framtidig forbedring.

**YPilot** (arbeidskatalog/repo `jpilot`; localStorage-nøkler `jpilot.*` beholdes) er
XPilot reimplementert i nettleseren: et newtonsk romkamp-spill
(Lunar Lander-slekt) med neon-wireframe-estetikk og lokal multiplayer på samme
tastatur. Turboraketti-kart/-våpen er et bevisst utsatt lag (Fase 3).

Prosjektet er en homage til XPilot-folka fra Universitetet i Tromsø og til
**Heikki Kosola**, den finske programmereren bak TurboRaketti II (utgitt 1992) —
derav norsk som gjennomgående språk.

## Bygg, kjør, test

Det finnes **ingen byggesteg og ingen avhengigheter å installere.** Phaser 4
lastes fra et CDN via en `<script>`-tag i `index.html`:

```html
<script src="https://cdn.jsdelivr.net/npm/phaser@4.1.0/dist/phaser.min.js"></script>
```

- **Kjør:** åpne `index.html` i en nettleser (dobbeltklikk), eller server mappa
  over HTTP hvis `.map`-filer lastes via `fetch` (noen nettlesere blokkerer
  `file://`-XHR — `python -m http.server` eller hvilken som helst statisk server
  funker).
- **Test / lint:** ingenting er satt opp. Ikke innfør bundler, npm-scripts eller
  testrammeverk uten eksplisitt beskjed — "ingen bygg" er en bevisst
  designbeskrankning, ikke en forglemmelse.
- **Phaser 4-referanse (lokalt):** Phaser-kilden er shallow-klonet til
  `vendor/phaser/` (gitignorert — ikke i repoet). Verifiser v4-API mot ekte kode
  der, særlig `vendor/phaser/skills/*/SKILL.md` (LLM-optimaliserte) og
  `skills/v3-to-v4-migration/SKILL.md`. v4-fallgruver: `setTintFill` fjernet (bruk
  `setTint`+`setTintMode`), partikler bruker `emitting`/`setParticleSpeed`,
  verdens-Y er fortsatt Y-ned (GL-flippen gjelder kun teksturer/shaders).

## Førende prinsipp: len deg på Phaser

Bruk Phaser sine innebygde systemer (rendering, `Graphics`, partikkel-emittere,
scener, Arcade-kollisjon for skip/bullet) i stedet for å reimplementere dem.
Tiden skal gå til å finjustere spill-*følelse*, ikke til å bygge motor-infrastruktur
på nytt. De få håndlagde bitene under er ikke unntak fra dette — de finnes
**fordi de er justeringsknappene**, der Phasers standarder ville motarbeidet
spillet vi vil ha. Før du håndlager noe Phaser allerede gjør bra, foretrekk
Phaser-API-et.

## Arkitektur

All spillogikk bor i **én enkelt `game.js`** som lastes etter Phaser-CDN-scriptet.
Ikke splitt den i moduler uten god grunn. Forventet intern struktur
(se planen §Fil-struktur):

- `PHYSICS` — justerbare konstanter (thrust, turn rate, gravitasjon, maxfart,
  bullet-levetid, energi-/skjold-rater). Dette er XPilot-referanseverdier; juster
  her, ikke inline.
- `parseMap()` — parser XPilot `.map`-tekst. **Viktig:** ekte XPilot-kart er et
  ASCII **block-tile grid**, ikke linjesegmenter (se «Kart-format» under). Output
  bør være `{ width, height, tiles, name, header, ...avledede lister }` der `tiles`
  er rutenettet og resten utledes fra det.
- `renderMap()` — tegner vegger **én gang** til et statisk `Graphics`-objekt
  (ikke tegnet på nytt per frame). NB: dette er Fase 1–2-tilnærmingen — se
  «Fremtidig retning» om levende polygoner før du baker veggene hardt inn i en
  statisk tekstur.
- `Ship`, `Bullet` — entitetsklasser som holder både spilltilstand og sine
  Phaser-visningsobjekter.
- `AudioEngine` — Web Audio-wrapper (se under).
- `GameScene` / `MenuScene` / `HUDScene` — Phaser-scener. HUD bor i sin egen
  scene/DOM-lag; **hold score/energi/radar-overlays unna spill-canvaset.**

### Fysikk og kollisjon — de to bevisste justeringsknappene

Dette er de *eneste* tingene som håndlages i stedet for å overlates til Phaser,
og det er med vilje — det er her spill-følelsen finjusteres:

- Bevegelses-integrasjon (thrust/rotasjon/gravitasjon/wrap) er **håndskrevet.**
  Arcade Physics' drag/friksjon-modell motarbeider XPilots friksjonsløse
  vakuum-følelse, så hastighet integreres direkte. En **lett, justerbar drag**
  (`PHYSICS.drag`) gir terminal-fart så fall ikke akselererer i det uendelige
  (sett `1` for rent vakuum); `maxSpeed` er et hardt sikkerhetstak.
- **Vegg-kollisjon er manuell grid-basert** (rute-oppslag `tileAt`, liten ~8px
  skip-hitbox), *ikke* Matter.js og ikke sirkel-mot-vilkårlig-linje — kartet er et
  block-tile grid, og dette gir kontroll over refleksjon-med-skjold vs. instant
  death-oppførsel. (Fase 2: trekant-test for 45°-skråninger.)

Alt annet bruker Phaser: **Arcade Physics for skip-mot-skip og bullet-mot-skip**,
Graphics for wireframes, partikkel-emittere for eksos, scener for spill/meny/HUD.

### Gravitasjon (Fase 1)

Global gravitasjon (nedover) er en **justerbar DOM-slider** (`#grav`), område
0–0.10, persistert i localStorage (`jpilot.gravity`; `clampGravity()` i `game.js`),
anvendt i `Ship.update`. **Invariant:** gravitasjonen må aldri overstige det
rakettstrålen kan bremse skipet med. Praktisk tak er senket til **0.10** — over det
faller skipet for fort til å manøvrere (uspillbart), selv om `thrustForce` (0.40)
fysisk overgår det. `thrustForce` styrer derfor mest aksel-følelsen. Per-kart-gravitasjon
(Fase 2) setter default. Innstillinger kan senere synkes til Google (gjenbruk `..\tabtabtab\`).

### Lyd — Web Audio API direkte (Fase 2)

Lyd bruker rå Web Audio API (`AudioContext`, oscillatorer, filtre,
gain-envelopes), **ikke** Phasers lydmotor, for finere kontroll. All lyd er
generert — det finnes ingen lyd-assets. Ramp gain med
`linearRampToValueAtTime` for å unngå klikk.

**Plan:** gjenbruk lydmotoren («soundbed» + SFX) fra solstice-prosjektet
(`C:\devel\aweussom\javascript\solstice\`, se `audio-spike/audio-system.js`) i
stedet for å skrive ny. Nye lyder som trengs: skudd (flere varianter), shield
on/off, rakett-motor (thrust-loop), eksplosjon, fuel-pickup.

## Visuelle konvensjoner (urokkelig estetikk)

- Mørk bakgrunn; **additiv blending (`Phaser.BlendModes.ADD`)** overalt der det
  gir mening.
- **Kun neon-wireframe-geometri** — genererte `Graphics`-former, ingen
  sprite-/raster-assets.
- Jeteksos: varm gradient (oransje→hvit), ADD-blendede partikler, emitter-posisjon/
  -retning oppdatert hver frame (`setParticleSpeed`), `emitting` styrt av thrust.
- Vegger: kjølig blå/cyan, ADD. Skip P1: cyan `#00ffff`; skip P2: magenta `#ff00ff`.

### Neon-referanse: solstice

Neon-looken vi er på jakt etter er demonstrert i
`C:\devel\aweussom\javascript\solstice\` (Colour The Solstice). Teknikkene der,
oversatt til vårt Phaser-oppsett:

- **Additiv blending** overalt: solstice bruker Canvas 2D `globalCompositeOperation
  = 'lighter'`; vår ekvivalent er `Phaser.BlendModes.ADD`.
- **Glød/halo:** solstice bruker `shadowBlur` + `shadowColor` (hsla) og
  `createRadialGradient` rundt lyspunkter. I Phaser/WebGL: tegn additive
  radial-gradient-glød-teksturer rundt skip/bullets, eller bruk en bloom post-FX
  pipeline.
- **Farger:** HSL med høy metning (100%) og ~58–60% lyshet gir den vivide neonen.
- **Signatur-effekt:** en WebGL feedback ping-pong-loop som warper/fader/
  hue-shifter forrige frame (MilkDrop-stil) gir trails og bloom. Vurder tilsvarende
  for eksos/bullets hvis vi vil ha samme "smelt".

## Spillregler som skal bevares (XPilot-semantikk)

- Energi driver **både** skyting og skjold.
- Bullets dreper uansett fart.
- Fuel-pods gjenoppretter energi (ikke en separat drivstoff-ressurs), regenererer
  etter ~30s.
- Skjold reflekterer bullets og vegg-treff mot en energikostnad; uten skjold +
  vegg-treff = eksplosjon. Respawn ved base etter ~3s.

### Følelses-quirks som skal gjenskapes (kjernen i spillet)

Dette er hvorfor vegg-kollisjon håndlages — oppførselen er vinkel-/fart-avhengig,
ikke en enkel "død eller bounce":

- **XPilot — bounce med skjold på:** med skjold oppe spretter skipet av vegger
  ved visse vinkler/farter (grunne treff) i stedet for å eksplodere. Bratte/harde
  treff koster mer energi (eller dreper hvis energien ikke holder). Refleksjonen
  skal føles fysisk, ikke som en hard stopp.
- **Turboraketti — raketten kan dytte fra nesten hva som helst:** å skyte/dytte
  mot flater gir fremdrift og føles nesten som et skjold i seg selv. Dette
  "push-off-alt" er en signatur-følelse fra Turboraketti og hører hjemme i
  Fase 3-laget, men design vegg-/kollisjons-koden så den ikke stenger for det.

## Kart-format (ekte XPilot)

130 ekte klassisk-kart ligger i `maps/xpilot-all-133-maps/` (rene tekstfiler —
ingen `.exe` nødvendig). Det ekte formatet skiller seg fra det forenklede
linjesegment-formatet som er skissert i `XPILOT-JAVASCRIPT-PLAN.md` — planen må
revideres for å matche dette:

- **Header:** `nøkkel : verdi`-par (case-insensitivt; både camelCase og lowercase
  forekommer) — `mapwidth`, `mapheight`, `mapname`, `edgewrap`, `edgebounce`,
  `playershielding`, `allowplayerbounces`, m.fl. `edgewrap`/`edgebounce` styrer
  direkte wrap-/bounce-følelsen.
- **Grid:** en blokk `mapData: \multiline: EndOfMapdata` … `EndOfMapdata`, ett
  tegn per rute (`mapwidth` × `mapheight`), fast blokkstørrelse.
- **Tegn-legende** (fra `kekyo/xpilot-ng` `src/common/xpmap.h`): ` ` tom, `x` fylt
  vegg-blokk, `s`/`w`/`a`/`q` fire 45°-skråninger, `b`/`h`/`y`/`g`/`t` dekor
  (kun visuelt), `#` fuel, `r`/`d`/`f`/`c` kanon opp/venstre/høyre/ned, `_` base
  + `0`–`9` lag-baser, `@`/`(`/`)` wormholes, `*`/`^` treasure, `!` target,
  `%`/`&` concentrators, `+`/`-`/`>`/`<`/`i`/`m`/`k`/`j` gravitasjon, `z`
  friksjons-sone, `A`–`Z` checkpoints.

Konsekvenser: **render** neon-vegger ved å trekke konturen rundt fylte
blokk-regioner (skråningene gir 45°-kanter), ikke ved å tegne hver blokk.
**Kollisjon** blir grid-basert (rute-oppslag + trekant for skråninger) — enklere
og mer robust enn sirkel-mot-vilkårlig-linje, og det er den ekte XPilot-modellen.

**Kanonisk eksempel-kart:** `maps/xpilot-all-133-maps/arena2.map` — les denne for
et ekte format-eksempel (header + `mapData`-grid med vegger, skråninger, fuel og
wormholes). Ikke bruk tokens på å lete etter et kart; start her.

## Fremtidig retning (uavklart — «vi får sjå»)

Ikke implementer dette ennå, men hold arkitekturen åpen for det:

- **Last ekte XPilot-maps**, inkludert den berømte «Calvin & Hobbes Dancing».
  `parseMap()` (Fase 2) dekker innlasting.
- **Levende/organiske polygoner:** på sikt skal veggene bli mer levende, i samme
  ånd som solstice — kanskje de «gror» under spillet. Konsekvens for design:
  ikke lås veggene til statisk geometri/bakt tekstur på en måte som stenger for
  animerte eller voksende polygoner senere. Solstice gjør vekst frikoblet med
  kontinuerlig easing mot et mål (ingen diskrete hopp) — god referanse for hvordan
  «gro» kan føles.

## Referanser

- Phaser 4-kilde + LLM-skills (lokalt, gitignorert): `vendor/phaser/` — særlig
  `skills/*/SKILL.md` og `skills/v3-to-v4-migration/SKILL.md`
- Phaser docs: https://docs.phaser.io/
- Ekte XPilot-kart: `maps/xpilot-all-133-maps/` (eksempel: `arena2.map`)
- XPilot kildekode (kartformat/tegn-legende/bounce-konstanter): `kekyo/xpilot-ng`
- Neon-estetikk + lydmotor: `C:\devel\aweussom\javascript\solstice\`
