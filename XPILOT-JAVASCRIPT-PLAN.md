# XPILOT-JAVASCRIPT-PLAN.md

XPilot reimplemented in the browser. Phaser 3 + WebGL. Neon wireframe aesthetic.
Local multiplayer (same keyboard). XPilot map format support.
Turboraketti maps and weapons are a future layer on top of this foundation.

# YPilot
Navn: "YPilot" — XPilot in Javascript (arbeidsnavn var «jpilot»).
Andre vurderte navn: 
 - WebPilot
 - Turboraketti 2026

---

## Design Principles

- Mørk bakgrunn, additive blending overalt (`BlendModes.ADD`)
- Neon wireframe-skip — generert geometri, ingen raster-assets
- Jeteksos via Phaser ArcadeParticles med ADD-blending
- XPilot-fysikk: Newtonsk, ingen friksjon i vakuum, gravity wells valgfritt
- Ingen HUD-forurensning av spill-canvas — HUD over i DOM eller separat lag
- Single-file `index.html` med `game.js` ved siden av, null bundler

---

## Tech Stack

| Komponent | Valg | Begrunnelse |
|---|---|---|
| Renderer | Phaser 3 (WebGL) | LLM-vennlig, innebygd partikkelsystem, ADD-blending |
| Fysikk | Phaser Arcade Physics | Tilstrekkelig for XPilot-mekanikk, enkel å tune |
| Lyd | Web Audio API (direkte) | Mer kontroll enn Phaser sin lydmotor |
| Kart | Custom XPilot `.map` parser | Se mapformat-seksjon nedenfor |
| Multiplayer | Lokal, samme tastatur | P1: WASD+Space, P2: Piltaster+Enter |
| Bygg | Ingen | CDN-import av Phaser, dobbelklikk og kjør |

```html
<script src="https://cdn.jsdelivr.net/npm/phaser@3/dist/phaser.min.js"></script>
```

---

## Fysikk

XPilot er en Lunar Lander-klone med mye attåt. Kjernen:

```
thrust:     vx += Math.cos(angle) * THRUST_FORCE * dt
            vy += Math.sin(angle) * THRUST_FORCE * dt
rotation:   angle += TURN_RATE * input * dt
gravity:    vy += GRAVITY * dt          (konfigurerbart per kart)
drag:       ingen (vakuum) — men kan legges på per kart (nebula-soner)
wrap:       posisjoner wrapper ved kartgrenser (konfigurerbart)
```

Viktige konstanter (XPilot-referanseverdier, justerbare):

```javascript
const PHYSICS = {
  thrustForce:  0.18,      // acceleration per frame ved full thrust
  turnRate:     0.065,     // rad per frame
  gravity:      0.04,      // default nedover, kan være 0 eller retningsstyrt
  maxSpeed:     12,        // pixels/frame — soft cap via drag ved overskridelse
  bulletSpeed:  14,
  bulletLife:   120,       // frames
  shieldDrain:  0.8,       // energi per frame med shield oppe
  energyRegen:  0.15,      // energi per frame
};
```

---

## Kart-format (ekte XPilot `.map`)

> **Rettet:** Ekte XPilot-kart er IKKE linjesegmenter. De er et **ASCII
> block-tile grid** — ett tegn per rute. 130 ekte kart ligger i
> `maps/xpilot-all-133-maps/`. Tegn-legenden under er hentet fra XPilot-NG-kilden
> (`kekyo/xpilot-ng`, `src/common/xpmap.h`).

Et kart er en header med `nøkkel : verdi`-par (case-insensitivt; både camelCase og
lowercase forekommer i de gamle kartene), etterfulgt av en `mapData`-blokk med
selve rutenettet:

```
# Kommentarer starter med #
mapwidth : 100
mapheight : 100
mapname : Arena
mapauthor : Dave Lemke
edgewrap : yes          # posisjon-wrap ved kartkant
edgebounce : no         # sprett mot kartkant i stedet for wrap
playershielding : yes

mapData: \multiline: EndOfMapdata
xxxxxxxxxxxxxxxxxxxxxx          xxxxxxxxxxxxxxxxxxxxxx
x                                                    x
x   (        #            _            #        )    x
x                                                    x
xxxxxxxxxxxxxxxxxxxxxx          xxxxxxxxxxxxxxxxxxxxxx
EndOfMapdata
```

Hver rute er ett tegn. Rutenettet er `mapwidth` × `mapheight` ruter, og hver rute
tegnes med en fast blokkstørrelse (`BLOCK_SZ` piksler — vi velger vår egen skala).

### Tegn-legende (fra `xpmap.h`)

| Tegn | Betydning |
|---|---|
| (mellomrom), `.` | Tom plass |
| `x` | Fylt vegg-blokk |
| `s` `w` `a` `q` | Fire 45°-skråninger (rettvinklet trekant, fyller halve ruta) |
| `b` `h` `y` `g` `t` | Dekor-versjoner av blokk/skråninger — **kun visuelt, ingen kollisjon** |
| `#` | Fuel-stasjon |
| `r` `d` `f` `c` | Kanon opp / venstre / høyre / ned |
| `_` | Base; `0`–`9` lag-baser; `$` base-attractor |
| `@` `(` `)` | Wormhole normal / inn / ut |
| `*` `^` | Treasure / tom treasure |
| `!` | Target |
| `%` `&` | Item- / asteroid-concentrator |
| `+` `-` `>` `<` `i` `m` `k` `j` | Gravitasjon (pos/neg/med-klokka/mot-klokka/opp/ned/høyre/venstre) |
| `z` | Friksjons-sone |
| `A`–`Z` | Checkpoints 0–25 |

### Parser-output

`parseMap()` returnerer et normalisert objekt der `tiles` er rutenettet og de
øvrige listene utledes fra det (med pikselkoordinater = `kol*BLOCK_SZ` osv.):

```javascript
{
  width: 100,                 // ruter
  height: 100,
  blockSize: 32,              // piksler per rute (vår skala)
  tiles: [[ ' ', 'x', ... ], ...],   // rad × kolonne, rå tegn
  header: { edgewrap: true, edgebounce: false, ... },

  // Utledet fra tiles for rendering/spill-logikk:
  bases: [ {x, y, team}, ... ],
  fuel:  [ {x, y}, ... ],
  cannons: [ {x, y, dir}, ... ],
  wormholes: [ {x, y, type}, ... ],
  gravity: [ {x, y, type}, ... ],
  // meta
  name: "Arena"
}
```

Vegger rendres ikke blokk-for-blokk; se Kart-renderer. Kollisjon er grid-basert;
se Kollisjondeteksjon.

---

## Spill-entiteter

### Skip

```javascript
class Ship {
  // Tilstand
  x, y, vx, vy, angle
  energy        // 0-100, driver shield + shoot
  shields       // bool
  dead          // bool, respawn-timer
  kills, deaths

  // Phaser-objekter
  graphics      // Phaser.GameObjects.Graphics — neon wireframe
  thrustEmitter // Phaser.GameObjects.Particles.ParticleEmitter
  shieldGfx     // Phaser.GameObjects.Graphics — pulserende ring
}
```

Skip-form (XPilot-standard, tegnet i Phaser Graphics):

```
     /\
    /  \
   / ·  \      · = cockpit-punkt
  /______\
     ||         = exhaust-punkt (partikkel-origin)
```

Neon-render:
```javascript
graphics.lineStyle(1.5, shipColor, 1.0);
graphics.strokeTriangle(...);
// BlendMode settes på Graphics-objektet
graphics.setBlendMode(Phaser.BlendModes.ADD);
```

### Jeteksos (partikler)

```javascript
const thrustEmitter = particles.createEmitter({
  x: 0, y: 0,
  speed: { min: 40, max: 90 },
  angle: { min: exhaustAngle - 20, max: exhaustAngle + 20 },
  scale: { start: 0.6, end: 0 },
  alpha: { start: 0.9, end: 0 },
  lifespan: { min: 120, max: 280 },
  frequency: 18,           // ms mellom spawns
  tint: [ 0xff6600, 0xff9900, 0xffcc00, 0xffffff ],
  blendMode: 'ADD',
  on: false                // aktiveres kun ved thrust
});
```

Eksos-retning oppdateres hver frame:
```javascript
thrustEmitter.setAngle({
  min: Phaser.Math.RadToDeg(ship.angle + Math.PI) - 20,
  max: Phaser.Math.RadToDeg(ship.angle + Math.PI) + 20
});
thrustEmitter.setPosition(exhaustX, exhaustY);
thrustEmitter.on = ship.thrusting;
```

### Bullets

```javascript
class Bullet {
  x, y, vx, vy
  life          // frames
  owner         // skip-referanse
  graphics      // liten lysende sirkel, ADD blending
}
```

Bullet-glow:
```javascript
// Liten radial gradient-sirkel per bullet
// Eller: Graphics.fillCircle + ADD blendmode
```

### Shield

Pulserende ring rundt skipet ved shield-aktivering:

```javascript
shieldGfx.lineStyle(1, 0x00ffff, alpha);
shieldGfx.strokeCircle(0, 0, SHIELD_RADIUS);
// alpha pulser: Math.sin(time * 0.08) * 0.4 + 0.6
```

---

## Input (lokal multiplayer)

```javascript
const CONTROLS = {
  p1: {
    thrust: 'W',
    rotLeft: 'A',
    rotRight: 'D',
    fire: 'SPACE',
    shield: 'S'
  },
  p2: {
    thrust: 'UP',
    rotLeft: 'LEFT',
    rotRight: 'RIGHT',
    fire: 'ENTER',
    shield: 'DOWN'
  }
};
```

Phaser CursorKeys + custom keys. Multi-key polling per frame, ikke events.

---

## Lyd (Web Audio API)

Ikke Phaser sin lydmotor — direkte Web Audio for mer kontroll.

```javascript
const AudioEngine = {
  ctx: new AudioContext(),

  thrust(on) { /* oscillator med filter, fade in/out */ },
  shoot()    { /* kort noise burst */ },
  explode()  { /* noise + pitch envelope ned */ },
  shieldHit(){ /* metallisk ping */ },
  fuelPickup(){ /* kort tone opp */ }
};
```

Thrust-lyd: OscillatorNode (sawtooth, ~80Hz) → BiquadFilterNode (lowpass) → GainNode.
On/off med gain.linearRampToValueAtTime for å unngå klikk.

---

## Kart-renderer

Veggene skal IKKE tegnes som synlige blokker — det dreper neon-looken. I stedet
**trekkes konturen** rundt sammenhengende fylte regioner, slik at vi får rene
neon-linjer langs ytterkantene. Skråningstegnene (`s/w/a/q`) gir gratis 45°-kanter
i konturen.

```javascript
function renderMap(scene, mapData) {
  const gfx = scene.add.graphics();
  gfx.lineStyle(1.5, 0x4488ff, 0.85);          // neon blå vegger

  // Marsjerende kvadrater / kant-traversering: tegn kun kanter der en fylt
  // rute møter en tom rute. Skråninger bidrar med diagonale kant-segmenter.
  for (const edge of traceWallOutline(mapData.tiles, mapData.blockSize)) {
    gfx.lineBetween(edge.x1, edge.y1, edge.x2, edge.y2);
  }

  gfx.setBlendMode(Phaser.BlendModes.ADD);
  // Bak evt. til texture for ytelse på store kart:
  // gfx.generateTexture('map', mapData.width * mapData.blockSize, ...);
}
```

`traceWallOutline()` går gjennom rutenettet og gir ut kant-segmentene mellom
fylt/tom. Dette er også grunnlaget for det fremtidige «levende»-laget (animerte/
voksende polygoner) — da animeres konturpunktene i stedet for å bakes til texture.

Fuel-pods: pulserende gule sirkler (ADD), forsvinner ved pickup, regenererer etter 30s.

---

## Kollisjondeteksjon

Phaser Arcade Physics for skip-mot-skip og bullet-mot-skip.

Vegger: **grid-basert** kollisjon, ikke linje-sirkel. Siden kartet er et rutenett,
slår vi opp hvilke(n) rute(r) skip-hitboxen overlapper og sjekker om de er fylte.
Dette er både enklere og mer robust enn sirkel-mot-vilkårlig-linje, og det er den
ekte XPilot-modellen.

```javascript
function tileAt(mapData, px, py) {
  const col = Math.floor(px / mapData.blockSize);
  const row = Math.floor(py / mapData.blockSize);
  return mapData.tiles[row]?.[col] ?? ' ';
}

function wallCollision(mapData, ship) {
  // Sjekk rutene rundt skipets posisjon.
  // Fylt blokk ('x'): akse-justert normal (←→↑↓ avhengig av treff-side).
  // Skråning ('s/w/a/q'): 45°-normal — trekant-test mot ruta.
  // Returnerer {hit, nx, ny} for refleksjon, ellers null.
}
```

Treff-respons (se «Følelses-quirks» i CLAUDE.md):
- **Uten skjold:** skip eksploderer.
- **Med skjold:** refleksjon langs `{nx,ny}` + energidrain. Grunne treff (liten
  vinkel/fart) spretter billig; bratte/harde treff koster mer eller dreper.
  `edgebounce`/`edgewrap` fra headeren styrer oppførsel mot selve kartkanten.

---

## Fil-struktur

```
index.html          # HTML-skall, Phaser CDN, canvas-container
game.js             # Alt spillogikk (~800-1200 linjer)
  ├── PHYSICS       # Konstanter
  ├── parseMap()    # XPilot .map parser
  ├── renderMap()   # Statisk kart-renderer
  ├── Ship          # Klasse
  ├── Bullet        # Klasse
  ├── AudioEngine   # Web Audio wrapper
  ├── GameScene     # Phaser.Scene — update/render loop
  ├── MenuScene     # Kartvalg, spillerinnstillinger
  └── HUDScene      # Overlays — score, energy-bar, radar
maps/
  default.map       # Enkel symmetrisk testbane
  README.md         # Kartredigering-guide
CLAUDE.md           # Epistemic context for LLM-sesjon
```

---

## CLAUDE.md (bootstrap for claude-code)

```markdown
# xpilot-js

XPilot-klone i Phaser 3. Lokal multiplayer, XPilot map-format.

## Stack
- Phaser 3 via CDN (ingen bundler)
- Web Audio API for lyd
- Ingen andre dependencies

## Arkitektur
- game.js er én fil — ikke splitt uten grunn
- Fysikk er manuell, ikke Matter.js
- Vegger: grid-basert kollisjon (kartet er et block-tile grid)
- Kart: parseMap() returnerer normalisert objekt med tile-grid, se XPILOT-JAVASCRIPT-PLAN.md

## Estetikk [DOCUMENTED]
- Mørk bakgrunn (#000 eller svært mørk blå)
- ADD blending overalt der det gir mening
- Neon wireframe-geometri, ingen sprites
- Jeteksos: varm (oransje→hvit), ADD blending
- Vegger: kjølig blå/cyan, ADD blending
- Skip P1: cyan (#00ffff), P2: magenta (#ff00ff)

## Kjente XPilot-regler [DOCUMENTED]
- Ingen friksjon i vakuum
- Energi driver shield OG skyting
- Bullet dreper uansett speed
- Fuel-pods gir energi, ikke drivstoff
- Shield reflekterer bullets (koster energi)
- Respawn ved base etter 3s

## Ikke gjort ennå [GUESS]
- Nettverksmultiplayer
- Turboraketti-våpen og -kart
- Sound-assets (kun generert lyd)
```

---

## Fase 1 — MVP (spillbart) ✅

1. Phaser-scene med hardkodet testkart (tile-grid) ✅
2. To skip, full newtonsk fysikk, wrap ✅
3. Bullets, kollisjon skip-mot-skip ✅
4. Vegg-kollisjon (grid-basert, instant death — ingen shield ennå) ✅
5. Jeteksos-partikler med ADD blending ✅
6. Score-display ✅
7. Justerbar gravitasjon (slider 0–0.10, persistert i localStorage) ✅

## Fase 2 — XPilot-komplett

- `.map`-parser og kartlasting (tile-grid, se Kart-format)
- **Start-punkter:** gjenbruk basene i kartene (`_` + lag-baser `0`–`9`) som
  spawn-punkter (allerede i de fleste kart).
- **AI-spiller** (prioritert — tidlig, så man slipper å styre to skip alene).
  To mulige veier: (a) Chrome innebygd AI (stjel kode fra `..\tabtabtab\`), eller
  (b) LLM-pre-rendrede AI-paths per kart — choke-points, sniping-steder, ruter
  (utnytt at en kraftig LLM kan «se» kartgeometrien bedre enn de fleste). Dette er
  i praksis **LLM-«enriching»** av kartdata (mønster brukeren bruker mye — stjel
  fra `..\tabtabtab\` og `C:\devel\q-free\geomap-united-nations\sweden\llm-benchmark`).
  - **Heuristisk basis (gjort, «target practice»-nivå):** prediktiv sikting,
    skjold-refleks mot vegg, gravitasjons-kompensasjon (hovrer mot tyngdekraften).
    LLM-enriching legges *oppå* dette — botene flyr ikke lenger blindt, men kan
    manøvrere/posisjonere som et menneske (og etter hvert bruke rakettstrålen som
    dytt/våpen, jf. Fase 3).
  - **Rømme spawn-lommer (utsatt → enrichment):** på enkelte kart (f.eks.
    `Arena.map`, «The Arena») sitter basene i innelukkede lommer (gulv under,
    vegger/skråninger på sidene, åpning bare oppover). Den lokale heuristiske boten
    klarer ikke finne veien UT — det krever rute-kunnskap om kartet (nettopp
    «flyr-blindt»-gapet). Utsatt til enrichingen gir botene kart-bevissthet/ruter.
    *Liten relatert bug å fikse separat:* AI-vegg-følerne sjekker bare `=== 'x'` og
    er dermed blinde for skrå-ruter (`q/w/a/s`) som likevel er solide — bør behandle
    skråninger som vegg i følerne.
  - **«Finn et godt sted å spawne»:** la LLM-enrichingen analysere kartgeometrien
    og sikre at hvert spawn-punkt har en **landing (flat vegg-topp) rett under seg**
    der det er mulig — så skip starter hvilende på en plattform og letter bevisst
    (Thrust/Lunar Lander-følelse) i stedet for å scramble i fritt fall når
    gravitasjonen slår inn. Gjelder særlig gravitasjons-kart.
- **Kart-navigator:** velger med terningkast-rating (1–6), score-sortering
  (terningkast 1 under uvurderte) og kart-preview (thumbnail av tile-gridet).
- Shield-mekanikk (energi, refleksjon, skjold-bounce-quirk — se Følelses-quirks).
- Fuel-pods.
- Gravity wells (per-kart; `.map`-headerens gravitasjon setter også slider-default).
- Radar (mini-map i hjørnet).
- Web Audio lyd — gjenbruk Solstice-lydmotor + nye SFX (skudd, shield on/off,
  rakett-motor, eksplosjon).
- (Senere) Sky-lagring av innstillinger/highscores via Google (tabtabtab-kode).

## Fase 3 — Turboraketti-lag

- Kartene fra Turboraketti gjenskapt i `.map`-format.
- **Rakettstrålen som våpen:** eksosen gjør skade + dytter motstanderen (mer dytt
  mot skjold = større flate), torque fra off-center treff → uventet spinn → krasj.
  Klassisk Turboraketti-taktikk.
- Eventuelle ekstra våpen fra originalspillet.
- Turboraketti-spesifikk balansering.

---

## Referanser

- XPilot-kilde (kartformat, tegn-legende, bounce-konstanter, default-taster):
  https://github.com/kekyo/xpilot-ng
- Phaser 4 docs: https://docs.phaser.io/ (lokalt: `vendor/phaser/`, gitignorert)
- Ekte XPilot-kart: `maps/xpilot-all-133-maps/` (eksempel: `arena2.map`)
- Neon-estetikk + lydmotor: `..\solstice\` — sky-lagring: `..\tabtabtab\`