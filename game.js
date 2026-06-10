/*
 * YPilot — XPilot reimplementert i nettleseren («because Y comes after X»).
 * Phaser 4. All spillogikk i én fil (se CLAUDE.md). Norsk i kommentarer.
 * (Den lokale arbeidskatalogen heter fortsatt «jpilot», men repo og localStorage-
 * nøkler bruker «ypilot».)
 */

'use strict';

/* ═══════════════════════════════════════════════════════════════════════════
 * TUNING — alle justerbare verdier samlet her (også de uten GUI). Endre fritt.
 * Fysikk-verdier er per frame @60fps; integrasjonen skalerer med
 * dtScale = delta/(1000/60) → frame-rate-uavhengig. Se CLAUDE.md.
 * ═══════════════════════════════════════════════════════════════════════════ */
const PHYSICS = {
  // — Bevegelse —
  thrustForce:  0.40,   // akselerasjon per frame ved full gass (rikelig over gravitasjons-taket
                        //   0.10 så skip klatrer lett; verdien styrer mest aksel-følelsen)
  turnRate:     0.065,  // rad per frame (grunn-sving)
  turnBoostLow: 0.9,    // ekstra sving-rate ved stillstand (TurboRaketti-skarp pivot i lav fart)
  turnBoostSpeed: 4,    // px/frame — over denne farten: ren grunn-sving (stabilt i høy fart)
  maxSpeed:     8,      // px/frame — hardt sikkerhetstak (clamp) ved FULL tank
  drag:         0.99,   // hastighet beholdt per frame (<1 = drag → terminal-fart; 1 = vakuum)
  // Drivstoff-boost («drivstoff-trykk-kompensasjon»): toppfarten skalerer MED drivstoff —
  // full tank = maxSpeed, tom tank = maxSpeed·(1−fuelBoost). Snudd fra ekte fysikk (lett=rask)
  // med vilje: et nyspawnet skip har full tank → høyest toppfart → kommer seg unna spawn-campere
  // (som har brent drivstoff og er tregere). 0 = av. [tunbar, Bevegelse]
  fuelBoost:    0.4,
  // — Skyting —
  bulletSpeed:  14,     // px/frame
  bulletLife:   120,    // frames (~2s)
  fireCooldown: 13,     // frames mellom skudd
  noseOffset:   14,     // px — der kula/eksosen starter foran/bak senteret
  bulletRadius: 3,      // px — bullet-hitbox
  shotDamage:   1,      // skade per treff
  // — Skip / liv / respawn —
  shipRadius:   8,      // px — skip-hitbox
  shipHP:       5,      // treff skipet tåler før det dør (Turboraketti-stil)
  respawnDelay: 180,    // frames (~3s)
  spawnInvuln:  120,    // frames (~2s) free shield etter spawn/takeover (anti spawn-camp) [tunbar, Kamp]
  winSurviveTime: 150,  // frames (~2.5s) vinneren må OVERLEVE etter at siste motstander dør —
                        // kuler i lufta kan fortsatt drepe → uavgjort (TurboRaketti II). 0 = vinn
                        // straks (gammel oppførsel). [tunbar, Kamp]
  startLives:   3,      // liv før game over
  takeoverPause: 60,    // frames (~1s) skipet «fryser» m/ nedtelling når du tar over en bot
  // — Eksplosjon / blast-push —
  explodeCount: 28,     // partikler i eksplosjonen
  blastRadius:  128,    // px — eksplosjonens dytte-rekkevidde (~4 blokker) [tunbar, Kamp]
  blastForce:   8,      // px/frame impuls ved episenter — nok til å slenge nærliggende skip inn i
                        // vegger (→ død/fuel-dren via vegg-mekanikken). [tunbar, Kamp]
  // — Jeteksos —
  exhaustSpeed:  90,    // partikkel-fart bakover
  exhaustSpread: 40,    // sideveis spredning
  exhaustOffset: 12,    // px bak senteret
  // — Drivstoff —
  fuelMax:      100,    // full tank
  fuelThrust:   0.18,   // drivstoff/frame ved gass (skaleres NED på store kart, se fuelMapRef)
  fuelMapRef:   48,     // tiles — kart ≤ denne dim. bruker full fuelThrust; større kart skalerer
                        //   drivstoff-bruk fra rakett lineært ned (mer å fly over → billigere gass)
  fuelMapMin:   0.20,   // gulv for skaleringen (svært store kart: ned mot 20 % bruk per thrust)
  fuelShot:     2,      // drivstoff per skudd
  fuelRefill:   0.8,    // drivstoff/frame ved fylling nær stasjon
  // — Skjold —
  shieldDrain:  0.5,    // drivstoff/frame med skjold oppe
  shieldBounce: 0.7,    // fart beholdt ved skjold-sprett mot vegg
  bounceKick:   1.1,    // ekstra utdytt-faktor ved sprett ut av fast underlag (skjold/invuln); var 1.5 [tunbar]
  wallLethalSpeed: 5.5, // px/frame — vegg-treff RASKERE enn dette dreper tross skjold (kun grunne/
                        // sakte treff spretter). invuln (spawn-grace) er unntatt. [tunbar]
  // — Landing —
  groundFriction: 0.92, // vx beholdt per frame ved bakke-kontakt (høyere = glir lengre)
};

// AI-oppførsel (heuristisk). Vinkler i rad; fireRange/lookahead/keepDistance i blokker.
const AI = {
  turnDeadzone: 0.06,   // ikke rotér innenfor dette
  aimedCone:    0.4,    // «omtrent siktet» → gass
  fireCone:     0.12,   // «godt siktet» → skyt
  fireRange:    30,     // blokker — maks skyte-avstand
  lookahead:    2,      // blokker — vegg-føler fram
  sensorAngle:  0.6,    // rad — venstre/høyre-føler
  avoidTurn:    0.8,    // rad — vri bort fra vegg
  keepDistance: 3,      // blokker — hold avstand til mål
  // — Increment 2.5: prediktiv sikting, skjold-refleks, gravitasjons-kompensasjon —
  // («target practice»-nivå: overlev og skyt fornuftig, ikke jakt aggressivt.)
  leadMax:      75,     // frames — maks ledetid for prediktiv sikting (ellers sikt på nåposisjon)
  gravComp:     1.0,    // 1 = full hovre-kompensasjon mot tyngdekraft (0 = ignorer gravitasjon)
  shieldSpeed:  1.4,    // px/frame — skjold-refleks først når vi er på vei inn i vegg over denne farten
  shieldFuelMin: 12,    // ikke skjold under dette drivstoff-nivået (spar til manøvrering)
  // — Anti-selvmord: nødbremsen ser lengre fram jo raskere boten går (stoppdistanse), og et
  //   fart-tak hindrer at den redliner rett inn i en vegg. —
  brakeBase:    2.5,    // blokker — grunn-fremsyn for vegg-deteksjon langs fartsretning
  brakeLead:    2.0,    // blokker per (px/frame) fart — ekstra fremsyn (raskere = bremser tidligere)
  brakeClose:   3.2,    // blokker — innenfor dette = «for sent å bremse», utløs skjold-refleks
  cruiseSpeed:  4.0,    // px/frame — over dette sluttes å gasse når vi alt farer i ønsket retning
  // — «Hender»: menneskelig finger-treghet. Min. tid (ms) mellom hver gang boten kan FLIPPE
  //   sving-retning (a/d) eller toggle gass (w/s). Uten dette twitcher den frame-perfekt og blir
  //   urealistisk presis. Høyere = grovere sikting/hovring → litt mindre god. Snappy ved nødbrems.
  handReact:    90,     // ms — sving-retning (venstre↔høyre)
  handThrust:   120,    // ms — gass på↔av
};

// Bot-multiplayer (free-for-all): mange skip på små baner; menneskene + bots.
const GAME = {
  humans:       2,    // menneske-styrte når AI-toggle er AV (resten bots); toggle PÅ = 1
  minShips:     2,    // færrest skip uansett
  maxShips:     8,    // flest skip
  tilesPerShip: 300,  // kart-areal (ruter) per skip → større kart = flere skip
};

const COLORS = {
  p1:   0x00ffff,  // cyan
  p2:   0xff00ff,  // magenta
  wall: 0x4488ff,  // kjølig neon-blå
};

const BLOCK = 32;  // piksler per kartrute (verdens-enhet — alltid 32, uansett kart)

// Viewport = hele nettleservinduet. Kameraet zoomer for å vise HELE kartet, sentrert
// og så stort som mulig (zoom kan være >1 for små kart → fyller skjermen). Verdens-
// koordinater bruker alltid BLOCK=32 så fysikk/skip-følelse er identisk på alle kart.
function viewW() { return window.innerWidth; }
function viewH() { return window.innerHeight; }
// Kart større enn dette (i ruter) blir for små for lokal spilling: skipet er fast
// 32 verdens-px, så større kart = bittesmå skip. Slike utelates fra lokal kart-velger
// («kun nettverk»). Skjerm-uavhengig (skipets skjerm-andel ≈ FIT/maxdim). Justerbar.
const MAX_LOCAL_DIM = 60;
// Kameraet fyller IKKE skjermen kant-i-kant — la ~20% luft rundt kartet, så man kan
// manøvrere rundt hindringer ved kanten og se wrap. Justerbar.
const FIT = 0.8;
// Drivstoff: én ressurs som driver gass/skyting (skjold senere). Fylles ved å hovre
// sakte nær en fuel-stasjon (#). Tom tank = kan ikke gjøre noe → driver i veggen.
const REFUEL_RANGE = BLOCK * 2;   // hvor nær stasjonen man må være
const REFUEL_SPEED = 3;           // px/frame — må være saktere (hovre)
const FUEL_COLOR = 0x33ff99;      // neon-grønn fuel-stasjon

// Landing (Lunar Lander / Turboraketti): nedstigning på en flat topp → hvile. Tål litt
// hardere landing (overlev opp til LAND_CRASH_SPEED), behold lateral fart (glir), og rett
// opp skipet etterpå. Skrå landing tolereres; nesa vris mot loddrett når den hviler.
const LAND_SPEED = 3;             // px/frame — under dette = «myk» landing (uten skade uansett)
let   LAND_CRASH_SPEED = 7;       // px/frame — flate-treff raskere enn dette = krasj (mellom = hard, men overlev) [tunbar]
const LAND_ANGLE_TOL = 0.25;      // rad (~14°) — (beholdt; brukes ikke lenger til å gate landing)
const LAND_MARGIN = 12;           // px senteret hviler over vegg-toppen
let   UPRIGHT_RATE = 0.06;        // rad/frame — auto-oppretting mot loddrett etter landing [tunbar]

/* ----------------------------------------------------------------------------
 * Tuning-tilstand (justeres live via Tuning-panelet; persisteres i localStorage
 * 'ypilot.tuning' — som jeg/Claude kan lese direkte via Chrome-pluginen).
 * ------------------------------------------------------------------------- */
let ROUNDING = 3;          // «Avrunding»: antall Chaikin-hjørnekutt for New-look-konturer (0–6)
let ORGANIC = 8;           // px — organisk noise-forskyvning av konturen (0 = ren glatt kurve)
let DETAIL = 0.02;         // fBm-frekvens — høyere = tettere/mer ruglete organisk variasjon
let GLOW_STRENGTH = 1.6;   // kamera-Glow outerStrength (New look)
let MYCEL = 0.7;           // mycel-fyll: glødende forgrenet vene-nett inni veggene (New, 0 = av)
let TRI_FILL = 0.6;        // trekant-fyll av solide soner i Trad-look (0 = av)
let STARS = 1.0;           // parallax-starfield + måne i bakgrunnen (0 = av)
let BEVEL = 0.3;           // Trad: 45°-bevel av skarpe konvekse hjørner (andel av blokk, 0 = av)
let GAME_SCENE = null;     // aktiv GameScene (for live tuning av rendering)
function clampRound(v) { v = Math.round(+v); return Number.isNaN(v) ? 3 : Math.min(6, Math.max(0, v)); }
function readRound() { return ROUNDING; }

/* ----------------------------------------------------------------------------
 * Justerbar global gravitasjon (nedover, px/frame²). Settes via DOM-slider og
 * huskes mellom sesjoner i localStorage. Leses i Ship.update. I Fase 2 kan
 * per-kart-gravitasjon fra `.map`-headeren sette default-verdien her.
 * ------------------------------------------------------------------------- */
const GRAVITY_KEY = 'ypilot.gravity';
const GRAVITY_MAX = 0.10;   // absolutt tak — over ~0.10 er spillet uspillbart (faller for fort
                            //   til å manøvrere). Slider og per-kart-header klippes til dette.
const DEFAULT_GRAVITY = 0.07;   // default når ingen lagret/kart-spesifikk verdi (gravitasjons-spill)
let GRAVITY = 0;

// Tuning-invariant (IKKE hard-klampet her): gravitasjonen bør aldri være høyere enn
// at rakettstrålen greit kan stoppe skipet selv i høy fart innen X px. Balanseres
// senere via gravitasjon og/eller strålestyrke. Ved dagens thrustForce (0.18) er det
// praktiske spillbare området derfor lavt; slideren tillater likevel utforsking opp
// til det absolutte taket.
function clampGravity(v) { return Math.min(Math.max(0, v), GRAVITY_MAX); }

function setupGravityControl() {
  let saved = NaN;
  try { saved = parseFloat(localStorage.getItem(GRAVITY_KEY)); } catch (e) { /* privat modus */ }
  if (!Number.isNaN(saved)) GRAVITY = clampGravity(saved);
  else GRAVITY = clampGravity((MAP && MAP.gravity) || DEFAULT_GRAVITY);   // kart-default, ellers 0.07

  const slider = document.getElementById('grav');
  const out = document.getElementById('grav-val');
  if (!slider) return;

  const sync = () => {
    GRAVITY = clampGravity(parseFloat(slider.value) || 0);
    if (out) out.textContent = GRAVITY.toFixed(3);
    try { localStorage.setItem(GRAVITY_KEY, String(GRAVITY)); } catch (e) { /* privat modus */ }
  };
  slider.value = String(GRAVITY);
  slider.addEventListener('input', sync);
  sync();
}

/* ----------------------------------------------------------------------------
 * Testkart — hardkodet som tile-grid (forward-kompatibelt med Fase 2s
 * parseMap). Åpne kanter (wrap), interne vegg-klynger for å teste vegg-død.
 * 'x' = vegg, ' ' = tom, '_' = spawn-markør (åpen rute).
 * ------------------------------------------------------------------------- */
function buildTestMap() {
  const cols = 30, rows = 18;
  const tiles = [];
  for (let r = 0; r < rows; r++) tiles.push(new Array(cols).fill(' '));

  // Interne vegg-klynger: [kol, rad, bredde, høyde]
  const blocks = [
    [6, 4, 4, 2], [20, 4, 4, 2],
    [13, 8, 4, 2],
    [6, 12, 4, 2], [20, 12, 4, 2],
  ];
  for (const [c, r, w, h] of blocks)
    for (let y = r; y < r + h; y++)
      for (let x = c; x < c + w; x++) tiles[y][x] = 'x';

  // Spawn-punkter (hjørner + midt-sider) — nok til mange skip.
  const mid = Math.floor(rows / 2);
  const spawnCells = [
    { col: 3, row: 3 }, { col: cols - 4, row: 3 },
    { col: 3, row: rows - 4 }, { col: cols - 4, row: rows - 4 },
    { col: 3, row: mid }, { col: cols - 4, row: mid },
  ];
  for (const s of spawnCells) tiles[s.row][s.col] = '_';

  // Et par fuel-stasjoner for å teste fylling.
  const fuelCells = [{ col: 15, row: 3 }, { col: 15, row: 14 }];
  for (const f of fuelCells) tiles[f.row][f.col] = '#';

  // Center-of-gravity er NED som standard; skip spawner med snuten OPP (vekk fra CoG).
  return {
    cols, rows, blockSize: BLOCK,
    widthPx: cols * BLOCK, heightPx: rows * BLOCK,
    tiles, edgewrap: true, cog: null,
    spawns: spawnCells.map(s => ({
      x: (s.col + 0.5) * BLOCK, y: (s.row + 0.5) * BLOCK, angle: -Math.PI / 2,
    })),
    fuelStations: fuelCells.map(f => ({ x: (f.col + 0.5) * BLOCK, y: (f.row + 0.5) * BLOCK })),
  };
}

/* ----------------------------------------------------------------------------
 * parseMap — leser ekte XPilot `.map`-tekst til SAMME form som buildTestMap().
 * Header er case-insensitiv; mapData-delimiteren leses fra selve linja (varierer
 * mellom kart). Rader paddes til mapwidth. Baser (`_`/`0`-`9`) → spawns.
 * Se CLAUDE.md §Kart-format og memory xpilot-kartformat.
 * ------------------------------------------------------------------------- */
function parseMap(text, fallbackName) {
  const lines = text.replace(/\r\n?/g, '\n').split('\n');
  const header = {};
  let delimiter = 'EndOfMapdata', mapDataLine = -1;

  for (let i = 0; i < lines.length; i++) {
    const t = lines[i].trim();
    if (/^mapdata\s*:/i.test(t)) {
      const m = t.match(/multiline\s*:\s*(\S+)/i);
      if (m) delimiter = m[1];
      mapDataLine = i;
      break;
    }
    if (!t || t[0] === '#') continue;                  // kommentar/blank
    const kv = t.match(/^([^:]+?)\s*:\s*(.*)$/);
    if (kv) header[kv[1].trim().toLowerCase()] = kv[2].trim();   // siste vinner
  }

  // Grid-rader til delimiter-linja (case-insensitivt).
  const grid = [];
  for (let j = mapDataLine + 1; j < lines.length; j++) {
    if (lines[j].trim().toLowerCase() === delimiter.toLowerCase()) break;
    grid.push(lines[j]);
  }

  const R = parseInt(header.mapheight, 10) || grid.length;
  let C = parseInt(header.mapwidth, 10) || 0;
  if (!C) for (const row of grid) C = Math.max(C, row.length);

  const tiles = [];
  for (let r = 0; r < R; r++) {
    const src = grid[r] || '';
    const row = new Array(C);
    for (let c = 0; c < C; c++) { const ch = src[c]; row[c] = (ch && ch !== '\t') ? ch : ' '; }
    tiles.push(row);
  }

  // Center-of-gravity: NED som standard (gravitasjonen drar nedover). Skip spawner med
  // snuten VEKK fra CoG → OPP som standard. Kart med `gravitypoint` (tile-koord) gir et
  // punkt-CoG, og da peker snuten vekk fra det punktet.
  let cog = null;
  // `gravitypoint` brukes KUN som CoG når kartet faktisk er en punkt-kilde
  // (`gravitypointsource: yes`). Mange kart (f.eks. The Blue Dragon) har en default
  // `gravitypoint: 0,0` med `gravitypointsource: no` — der er gravitasjonen ensrettet
  // (gravityangle), og 0,0 skal IKKE tolkes som tyngdepunkt (ga rare spawn-vinkler).
  const gpSource = (header.gravitypointsource || '').toLowerCase() === 'yes';
  const gp = gpSource ? (header.gravitypoint || '').match(/(-?\d+)\s*,\s*(-?\d+)/) : null;
  if (gp) cog = { x: (parseInt(gp[1], 10) + 0.5) * BLOCK, y: (parseInt(gp[2], 10) + 0.5) * BLOCK };
  const spawnAngle = (x, y) =>
    cog ? ((x === cog.x && y === cog.y) ? -Math.PI / 2 : Math.atan2(y - cog.y, x - cog.x))
        : -Math.PI / 2;   // CoG ned → snute opp

  const spawns = [];
  const fuelStations = [];
  for (let r = 0; r < R; r++) for (let c = 0; c < C; c++) {
    const ch = tiles[r][c];
    if (ch === '_' || (ch >= '0' && ch <= '9')) {
      const x = (c + 0.5) * BLOCK, y = (r + 0.5) * BLOCK;
      spawns.push({ x, y, angle: spawnAngle(x, y) });
    } else if (ch === '#') {
      fuelStations.push({ x: (c + 0.5) * BLOCK, y: (r + 0.5) * BLOCK });
    }
  }
  if (spawns.length === 0) {            // fallback: kvart-punkter
    for (const [fx, fy] of [[0.25, 0.25], [0.75, 0.75], [0.75, 0.25], [0.25, 0.75]]) {
      const x = fx * C * BLOCK, y = fy * R * BLOCK;
      spawns.push({ x, y, angle: spawnAngle(x, y) });
    }
  }

  // Gravitasjon fra header — kun hvis innenfor vår skala (ellers 0; tunes med slider).
  let gravity = 0;
  const g = parseFloat(header.gravity);
  if (!Number.isNaN(g) && Math.abs(g) <= GRAVITY_MAX) gravity = Math.abs(g);

  const wrap = (header.edgewrap || '').toLowerCase() !== 'no'
            && (header.edgebounce || '').toLowerCase() !== 'yes';

  return {
    cols: C, rows: R, blockSize: BLOCK,
    widthPx: C * BLOCK, heightPx: R * BLOCK,
    tiles, edgewrap: wrap, gravity, cog, fuelStations,
    spawns, name: header.mapname || fallbackName || 'kart', header,
  };
}

// Lokalt spillbart hvis kartet ikke er for stort (større kart → for små skip).
function isLocallyPlayable(wTiles, hTiles) {
  return Math.max(wTiles, hTiles) <= MAX_LOCAL_DIM;
}

// Svartliste: kart som er kjent ødelagte i YPilot (kan fungere i ekte XPilot, men
// vår parser/fysikk takler dem ikke). Utelates fra velgeren og avvises ved boot.
const EXCLUDED_MAPS = new Set([
  'xpilot-all-133-maps/dog1776.map',   // «Dog 1776» — fungerer ikke i YPilot
]);
function isExcludedMap(file) { return EXCLUDED_MAPS.has(file); }

// Laster valgt kart (filnavn relativt til maps/), eller testbanen.
async function loadMap(sel) {
  // Nytt JSON-format (TRII + konverterte XPilot) er embeddet i maps-embedded.js → ingen
  // fetch, laster også ved file://. Sjekkes først.
  if (window.EMBEDDED_MAPS && window.EMBEDDED_MAPS[sel]) return buildMapFromJson(window.EMBEDDED_MAPS[sel]);
  if (!sel || sel === 'test') return buildTestMap();
  try {
    const text = await fetch('maps/' + sel).then(r => { if (!r.ok) throw new Error('HTTP ' + r.status); return r.text(); });
    return parseMap(text, sel);
  } catch (e) {
    console.warn('Kunne ikke laste kart', sel, '— bruker testbane.', e);
    return buildTestMap();
  }
}

// Adapter: nytt JSON-format → samme runtime-form som parseMap/buildTestMap. Heltall-tiles
// (0=åpen, 1=solid; skråninger 2..5 reservert) → tegn-grid ('x'/' '). Spawns/fuel/soner
// fra celle-koords til verdens-px (BLOCK). gravityZones/liquidZones bæres videre (ennå
// ikke brukt av fysikken — venter på sone-gravitasjon/væske).
function buildMapFromJson(j) {
  const bs = BLOCK;
  const tiles = j.tiles.map(row => row.map(v => (v ? 'x' : ' ')));
  const spawns = (j.spawns || []).slice()
    .sort((a, b) => (a.player ?? 0) - (b.player ?? 0))
    .map(s => ({ x: (s.col + 0.5) * bs, y: (s.row + 0.5) * bs, angle: -Math.PI / 2 }));
  const fuelStations = (j.fuelStations || []).map(f => ({ x: (f.col + 0.5) * bs, y: (f.row + 0.5) * bs }));
  if (spawns.length === 0) {            // fallback: kvart-punkter
    for (const [fx, fy] of [[0.25, 0.25], [0.75, 0.75], [0.75, 0.25], [0.25, 0.75]])
      spawns.push({ x: fx * j.cols * bs, y: fy * j.rows * bs, angle: -Math.PI / 2 });
  }
  return {
    cols: j.cols, rows: j.rows, blockSize: bs,
    widthPx: j.cols * bs, heightPx: j.rows * bs,
    tiles, edgewrap: j.edgewrap !== false, gravity: j.gravity || 0, cog: null,
    fuelStations, spawns, name: j.name || 'kart', header: {},
    gravityZones: j.gravityZones || [], liquidZones: j.liquidZones || [], source: j.source,
  };
}

let MAP = buildTestMap();

/* ----------------------------------------------------------------------------
 * Kart-hjelpere
 * ------------------------------------------------------------------------- */
function isWall(map, c, r) {
  if (r < 0 || r >= map.rows || c < 0 || c >= map.cols) return false;
  return map.tiles[r][c] === 'x';
}

// Tegnet «på terrengets» — slår opp ruta under et pikselpunkt.
function tileAt(map, px, py) {
  const c = Math.floor(px / map.blockSize);
  const r = Math.floor(py / map.blockSize);
  if (r < 0 || r >= map.rows || c < 0 || c >= map.cols) return ' ';
  return map.tiles[r][c];
}

// Drivstoff-skalering for rakettbruk: store kart krever mye thrust for å fly over, så
// drivstoff-forbruket per gass skaleres lineært ned med kart-størrelsen (cachet på map).
function fuelMapScale(map) {
  if (map._fuelScale === undefined) {
    const dim = Math.max(map.cols, map.rows);
    map._fuelScale = Math.max(PHYSICS.fuelMapMin, Math.min(1, PHYSICS.fuelMapRef / dim));
  }
  return map._fuelScale;
}

// Wrap et display-objekt rundt kartgrensene.
function wrapObject(map, obj) {
  if (obj.x < 0) obj.x += map.widthPx; else if (obj.x >= map.widthPx) obj.x -= map.widthPx;
  if (obj.y < 0) obj.y += map.heightPx; else if (obj.y >= map.heightPx) obj.y -= map.heightPx;
}

// Er nesen (nesten) opp? Brukes for landing — toleranse fra rett opp (-PI/2).
function uprightOK(angle) {
  let d = angle + Math.PI / 2;
  while (d > Math.PI) d -= 2 * Math.PI;
  while (d < -Math.PI) d += 2 * Math.PI;
  return Math.abs(d) < LAND_ANGLE_TOL;
}

/* ----------------------------------------------------------------------------
 * Teksturer — genereres én gang fra Graphics (neon-wireframe, ingen assets).
 * Skipet tegnes hvitt og tintes per spiller.
 * ------------------------------------------------------------------------- */
function makeTextures(scene) {
  const g = scene.make.graphics({ x: 0, y: 0, add: false });

  // Menneske-skip: Starfighter (pil) — peker mot +x, tydelig retning.
  g.lineStyle(2, 0xffffff, 1);
  g.beginPath();
  g.moveTo(28, 16);   // nese
  g.lineTo(8, 7);     // hekk topp
  g.lineTo(14, 16);   // cockpit-innhakk
  g.lineTo(8, 25);    // hekk bunn
  g.lineTo(28, 16);
  g.strokePath();
  g.generateTexture('ship-human', 32, 32);
  g.clear();

  // Bot-skip: TIE-fighter — sentral pod + to vinge-paneler + liten nese-stubb (retning).
  g.lineStyle(2, 0xffffff, 1);
  g.strokeCircle(16, 16, 5);
  g.beginPath();
  g.moveTo(7, 4);   g.lineTo(7, 28);    // venstre panel
  g.moveTo(25, 4);  g.lineTo(25, 28);   // høyre panel
  g.moveTo(11, 16); g.lineTo(7, 16);    // strut venstre
  g.moveTo(21, 16); g.lineTo(25, 16);   // strut høyre
  g.moveTo(21, 16); g.lineTo(29, 16);   // nese-stubb (hinter retning)
  g.strokePath();
  g.generateTexture('ship-bot', 32, 32);
  g.clear();

  // Bullet — liten lysende prikk (ADD gir glød). Mindre enn skipet.
  g.fillStyle(0xffffff, 0.35); g.fillCircle(5, 5, 4);
  g.fillStyle(0xffffff, 0.7);  g.fillCircle(5, 5, 2.4);
  g.fillStyle(0xffffff, 1.0);  g.fillCircle(5, 5, 1.2);
  g.generateTexture('bullet', 10, 10);
  g.clear();

  // Partikkel-prikk (eksos/eksplosjon).
  g.fillStyle(0xffffff, 0.5); g.fillCircle(4, 4, 4);
  g.fillStyle(0xffffff, 1.0); g.fillCircle(4, 4, 2);
  g.generateTexture('spark', 8, 8);
  g.clear();

  // Skjold-ring (vises rundt skipet når skjold er oppe).
  g.lineStyle(2, 0xffffff, 1);
  g.strokeCircle(22, 22, 19);
  g.generateTexture('shield', 44, 44);

  g.destroy();
}

/* ----------------------------------------------------------------------------
 * Romfølelse: parallax-starfield + glødende «måne» (med diskret Death Star-hint).
 * Tekstur-tiles genereres én gang; vises som skjerm-festede TileSprites med parallax via
 * tilePosition (funker i både fit- og scroll-modus, og fyller tomrommet utenfor kartkanten
 * på scroll-kart). Alt ADD-blendet, bak veggene (depth < 0, men > bg sin -1).
 * ------------------------------------------------------------------------- */
function makeSpaceTextures(scene) {
  if (scene.textures.exists('moon')) return;         // genereres én gang per økt
  const g = scene.make.graphics({ add: false });
  const T = 512;   // stor tile → stjernene spres tynt utover (diskret, «langt unna»)

  // Fjerne stjerner — FÅ, bittesmå, svært dimme pinprikker (distansert romstøv).
  for (let i = 0; i < 26; i++) {
    g.fillStyle(0x9fbce6, 0.10 + Math.random() * 0.16);
    g.fillCircle(Math.random() * T, Math.random() * T, 0.5 + Math.random() * 0.4);
  }
  g.generateTexture('star-far', T, T); g.clear();

  // Nære stjerner — enda færre, litt lysere, av og til varme/blå; sjelden kryss-glimt.
  for (let i = 0; i < 11; i++) {
    const x = Math.random() * T, y = Math.random() * T, big = Math.random() < 0.16;
    const r = big ? 1.1 + Math.random() * 0.9 : 0.6 + Math.random() * 0.5;
    const tint = Math.random() < 0.14 ? 0xfff0c0 : (Math.random() < 0.12 ? 0xbfe0ff : 0xffffff);
    g.fillStyle(tint, 0.28); g.fillCircle(x, y, r * 1.7);
    g.fillStyle(0xffffff, 0.7); g.fillCircle(x, y, r * 0.6);
    if (big) { g.lineStyle(0.7, 0xffffff, 0.32); g.lineBetween(x - r * 2.6, y, x + r * 2.6, y); g.lineBetween(x, y - r * 2.6, x, y + r * 2.6); }
  }
  g.generateTexture('star-near', T, T); g.clear();

  // Måne — neon-glødende kropp med klart Death Star-hint (superlaser-dish + ekvator-grøft).
  // NB: teksturen må romme HELE haloen (ellers klippes glødet til en avrundet firkant av
  // tekstur-kanten). Største halo-radius = R + 8*4 = 90 < M/2 = 100 → ren sirkulær glød.
  const M = 200, cx = M / 2, cy = M / 2, R = 58;
  for (let i = 8; i >= 1; i--) { g.fillStyle(0x88bbff, 0.022); g.fillCircle(cx, cy, R + i * 4); }  // halo (dim)
  g.fillStyle(0x16273f, 0.5); g.fillCircle(cx, cy, R);                     // dim kropp
  g.lineStyle(2, 0xbfe0ff, 0.7); g.strokeCircle(cx, cy, R);               // rim
  g.lineStyle(1.5, 0xbfe0ff, 0.7); g.strokeCircle(cx - R * 0.3, cy - R * 0.34, R * 0.22);  // superlaser-dish
  g.fillStyle(0x9fc0e8, 0.12); g.fillCircle(cx - R * 0.3, cy - R * 0.34, R * 0.2);
  g.lineStyle(1.5, 0x88bbff, 0.6); g.lineBetween(cx - R * 0.93, cy + R * 0.06, cx + R * 0.93, cy - R * 0.02);  // ekvator-grøft
  g.lineStyle(1, 0x6fa0d0, 0.5);
  g.strokeCircle(cx + R * 0.32, cy + R * 0.26, R * 0.15);                 // et par «kratere»
  g.strokeCircle(cx - R * 0.2, cy + R * 0.42, R * 0.1);
  g.generateTexture('moon', M, M); g.clear();

  g.destroy();
}

function applyStarVisibility(scene) {
  const on = STARS > 0;
  for (const o of [scene.starFar, scene.starNear, scene.moon, scene.moonLabel]) if (o) o.setVisible(on);
  if (scene.starFar)  scene.starFar.setAlpha(0.8 * STARS);
  if (scene.starNear) scene.starNear.setAlpha(0.9 * STARS);
  if (scene.moon)     scene.moon.setAlpha(Math.min(1, 0.58 * STARS));
  if (scene.moonLabel) scene.moonLabel.setAlpha(Math.min(1, 0.5 * STARS));
}

function createStarfield(scene) {
  makeSpaceTextures(scene);
  const W = viewW(), H = viewH();
  // Stjernelagene er skjerm-festet (uendelig fjernt) med parallax via tilePosition (i update).
  const mk = (key, depth) => scene.add.tileSprite(0, 0, W, H, key)
    .setOrigin(0, 0).setScrollFactor(0).setDepth(depth).setBlendMode(Phaser.BlendModes.ADD);
  scene.starFar = mk('star-far', -0.92);
  scene.starNear = mk('star-near', -0.9);
  scene.scale.on('resize', () => {
    if (scene.starFar) scene.starFar.setSize(viewW(), viewH());
    if (scene.starNear) scene.starNear.setSize(viewW(), viewH());
  });

  // Månen står på en FAST verdens-posisjon (følger ikke spilleren) — den romsligste åpne flekken
  // på kartet. Skalert etter kartstørrelse (større kart → større måne; C&H ≈ 2x). Bak veggene.
  const map = scene.map;
  const spot = findMoonSpot(map);
  const moonScale = Math.max(0.8, Math.min(2.2, map.widthPx / 1800));
  scene.moon = scene.add.image(spot.x, spot.y, 'moon')
    .setDepth(-0.88).setBlendMode(Phaser.BlendModes.ADD).setScale(moonScale);
  scene.moonLabel = scene.add.text(spot.x, spot.y, 'Ikke en\nmåne!', {
    fontFamily: 'monospace', fontSize: '11px', color: '#bfe0ff', align: 'center',
  }).setOrigin(0.5, 0.5).setDepth(-0.87).setScale(moonScale);

  applyStarVisibility(scene);
}

// Finn den romsligste åpne flekken på kartet (åpen celle lengst fra nærmeste vegg) — der får
// månen «plass». BFS-avstandsfelt fra alle vegg-celler ut i det åpne; velg argmax.
function findMoonSpot(map) {
  const cols = map.cols, rows = map.rows, bs = map.blockSize;
  const dist = new Int16Array(cols * rows).fill(-1);
  let q = [];
  for (let r = 0; r < rows; r++)
    for (let c = 0; c < cols; c++)
      if (map.tiles[r][c] === 'x') { dist[r * cols + c] = 0; q.push(r * cols + c); }
  if (!q.length) return { x: map.widthPx / 2, y: map.heightPx / 2 };  // ingen vegger → midten
  // Hold månen unna kart-kanten (ellers havner den i et hjørne på sparsomme wraparound-kart,
  // halvveis utenfor — motoren dobbel-rendrer ikke ved sømmen).
  const margin = Math.max(6, Math.floor(Math.min(cols, rows) * 0.12));
  let best = -1, bestI = (Math.floor(rows / 2)) * cols + Math.floor(cols / 2);
  while (q.length) {
    const nq = [];
    for (const i of q) {
      const c = i % cols, r = (i - c) / cols;
      for (const [dc, dr] of [[1, 0], [-1, 0], [0, 1], [0, -1]]) {
        const nc = c + dc, nr = r + dr;
        if (nc < 0 || nc >= cols || nr < 0 || nr >= rows) continue;
        const ni = nr * cols + nc;
        if (dist[ni] !== -1) continue;
        dist[ni] = dist[i] + 1; nq.push(ni);
        const interior = nc >= margin && nc < cols - margin && nr >= margin && nr < rows - margin;
        if (interior && dist[ni] > best) { best = dist[ni]; bestI = ni; }
      }
    }
    q = nq;
  }
  const bc = bestI % cols, br = (bestI - bc) / cols;
  return { x: (bc + 0.5) * bs, y: (br + 0.5) * bs };
}

/* ----------------------------------------------------------------------------
 * Vegg-kontur som ORGANISKE løkker (for «New look»): spor grensen mellom solid og tom
 * langs hjørne-gitteret, kjed segmentene til lukkede løkker, og Chaikin-glatt dem →
 * flytende kanter i stedet for blokk-trapper. Statisk, regnes én gang.
 * ------------------------------------------------------------------------- */
function traceWallLoops(map) {
  const solid = (c, r) => isWall(map, c, r);
  const adj = new Map();                          // hjørne-key -> [nabo-keys]
  const key = (x, y) => x + ',' + y;
  const ensure = k => { let v = adj.get(k); if (!v) { v = []; adj.set(k, v); } return v; };
  const link = (ax, ay, bx, by) => { const a = key(ax, ay), b = key(bx, by); ensure(a).push(b); ensure(b).push(a); };
  for (let r = 0; r < map.rows; r++)
    for (let c = 0; c < map.cols; c++) {
      if (!solid(c, r)) continue;
      if (!solid(c, r - 1)) link(c, r, c + 1, r);             // topp
      if (!solid(c, r + 1)) link(c, r + 1, c + 1, r + 1);     // bunn
      if (!solid(c - 1, r)) link(c, r, c, r + 1);             // venstre
      if (!solid(c + 1, r)) link(c + 1, r, c + 1, r + 1);     // høyre
    }
  const eKey = (a, b) => (a < b ? a + '|' + b : b + '|' + a);
  const used = new Set();
  const loops = [];
  for (const [start, nbs] of adj) {
    for (const first of nbs) {
      if (used.has(eKey(start, first))) continue;
      const loop = [start];
      let prev = start, cur = first, guard = 0;
      used.add(eKey(prev, cur));
      while (cur !== start && guard++ < 200000) {
        loop.push(cur);
        let next = null;
        for (const n of adj.get(cur)) if (n !== prev && !used.has(eKey(cur, n))) { next = n; break; }
        if (next === null) break;
        used.add(eKey(cur, next));
        prev = cur; cur = next;
      }
      if (loop.length >= 4) loops.push(loop);
    }
  }
  const bs = map.blockSize;
  return loops.map(l => l.map(k => { const p = k.split(','); return { x: +p[0] * bs, y: +p[1] * bs }; }));
}

// Chaikin corner-cutting på en LUKKET løkke → avrundede hjørner. Flere iterasjoner = mykere.
function chaikin(pts, iters) {
  let p = pts;
  for (let it = 0; it < iters; it++) {
    const out = [], n = p.length;
    for (let i = 0; i < n; i++) {
      const a = p[i], b = p[(i + 1) % n];
      out.push({ x: a.x * 0.75 + b.x * 0.25, y: a.y * 0.75 + b.y * 0.25 });
      out.push({ x: a.x * 0.25 + b.x * 0.75, y: a.y * 0.25 + b.y * 0.75 });
    }
    p = out;
  }
  return p;
}

/* Organisk «vekst» fra grid-et: forskyv konturen med deterministisk fBm-noise. Grid-et er
 * bare en guide; veggene vokser fram med litt tilfeldig, organisk variasjon. Stabilt per
 * posisjon (hash-basert, ingen Math.random) → ingen flimring mellom (re)bygginger. */
function hash2(ix, iy) {
  let h = (ix | 0) * 374761393 + (iy | 0) * 668265263;
  h = Math.imul(h ^ (h >>> 13), 1274126177);
  return ((h ^ (h >>> 16)) >>> 0) / 4294967296;
}
function valueNoise(x, y) {
  const x0 = Math.floor(x), y0 = Math.floor(y), fx = x - x0, fy = y - y0;
  const sx = fx * fx * (3 - 2 * fx), sy = fy * fy * (3 - 2 * fy);   // smoothstep
  const n00 = hash2(x0, y0), n10 = hash2(x0 + 1, y0), n01 = hash2(x0, y0 + 1), n11 = hash2(x0 + 1, y0 + 1);
  const a = n00 + (n10 - n00) * sx, b = n01 + (n11 - n01) * sx;
  return a + (b - a) * sy;                                          // [0,1)
}
function fbm(x, y) {   // 3 oktaver → rikere, fraktal-aktig detalj
  let v = 0, amp = 0.5, f = 1;
  for (let o = 0; o < 3; o++) { v += valueNoise(x * f, y * f) * amp; f *= 2; amp *= 0.5; }
  return v;
}

// Del opp lukket løkke så ingen kant > maxLen (nok punkter til at noise-forskyvning biter).
function subdivideLoop(pts, maxLen) {
  const out = [], n = pts.length;
  for (let i = 0; i < n; i++) {
    const a = pts[i], b = pts[(i + 1) % n];
    out.push(a);
    const segs = Math.floor(Math.hypot(b.x - a.x, b.y - a.y) / maxLen);
    for (let s = 1; s < segs; s++) { const t = s / segs; out.push({ x: a.x + (b.x - a.x) * t, y: a.y + (b.y - a.y) * t }); }
  }
  return out;
}

// Forskyv hvert punkt langs konturens normal med fBm → bølgende, organisk vegg.
function organicDisplace(pts, amount, freq) {
  if (amount <= 0) return pts;
  const n = pts.length, out = new Array(n);
  for (let i = 0; i < n; i++) {
    const a = pts[(i - 1 + n) % n], b = pts[(i + 1) % n], p = pts[i];
    const dx = b.x - a.x, dy = b.y - a.y, len = Math.hypot(dx, dy) || 1;
    const d = (fbm(p.x * freq, p.y * freq) * 2 - 1) * amount;       // [-amount, amount]
    out[i] = { x: p.x + (-dy / len) * d, y: p.y + (dx / len) * d }; // langs normal
  }
  return out;
}

/* ----------------------------------------------------------------------------
 * Mycel-fyll — vegg-INNSIDEN fylles med et glødende, forgrenet vene-nett (sopp-mycel /
 * Yggdrasil fra Solstice, men UTEN sentral stamme). Vener seedes fra vegg-kantene og vokser
 * INNOVER i de solide regionene, styrt av et BFS-dybdefelt (avstand-til-åpent), med hash-
 * basert jitter og forgrening → deterministisk per kart (ingen Math.random → ingen flimmer).
 * Pre-regnes én gang (cachet på scene), bakes inn i vegg-teksturen. Ref: memory levende-kart.
 * ------------------------------------------------------------------------- */
// «Lyn-nedslag»-look: lange rette løp brutt av skarpe knekk (sikksakk-bolter) som forgrener
// seg i vide vinkler. Svak innover-pull + korte bolter → mindre klumping i sentrum enn et
// gradient-følgende mycel ville gitt; boltene radierer fra kantene innover i den solide sonen.
const MYCEL_CFG = {
  seedStride: 4,      // seed hvert N-te kant-tile (mindre = tettere nett)
  stepLen:    0.9,    // andel av blockSize per steg (lange rette løp → kantet lyn-look)
  maxSteps:   18,     // maks steg per hoved-bolt (kort → ingen sentrum-klump)
  branchProb: 0.14,   // hash-sannsynlighet for forgrening per steg
  kinkProb:   0.30,   // sannsynlighet for et SKARPT lyn-knekk per steg
  kinkAmt:    0.7,    // rad — størrelse på lyn-knekk
  jitter:     0.12,   // rad — liten bølge mellom knekkene
  inwardPull: 0.20,   // svak dragning mot dybde-gradienten (innover) — lav = mindre klumping
  maxVeins:   3200,   // hardt tak (store kart) så teksturen ikke eksploderer
};

// BFS-dybdefelt: 0 = åpen celle, ≥1 = solid, der tallet er antall celler inn fra nærmeste åpne.
// Out-of-bounds regnes som åpent (arena-kant), så grense-vegger får dybde 1.
function solidDepthField(map) {
  const cols = map.cols, rows = map.rows;
  const depth = new Int16Array(cols * rows).fill(-1);
  let q = [];
  for (let r = 0; r < rows; r++)
    for (let c = 0; c < cols; c++)
      if (map.tiles[r][c] !== 'x') { depth[r * cols + c] = 0; q.push(r * cols + c); }
  while (q.length) {
    const nq = [];
    for (const i of q) {
      const c = i % cols, r = (i - c) / cols;
      for (const [dc, dr] of [[1, 0], [-1, 0], [0, 1], [0, -1]]) {
        const nc = c + dc, nr = r + dr;
        if (nc < 0 || nc >= cols || nr < 0 || nr >= rows) continue;
        const ni = nr * cols + nc;
        if (depth[ni] !== -1 || map.tiles[nr][nc] !== 'x') continue;
        depth[ni] = depth[i] + 1; nq.push(ni);
      }
    }
    q = nq;
  }
  return depth;
}

function buildMycel(map) {
  const cols = map.cols, rows = map.rows, bs = map.blockSize;
  const depth = solidDepthField(map);
  const depthCell = (c, r) => (c < 0 || c >= cols || r < 0 || r >= rows) ? 0 : Math.max(0, depth[r * cols + c]);
  const solidAt = (px, py) => depthCell(Math.floor(px / bs), Math.floor(py / bs)) >= 1;
  // Innover-retning i et punkt: gradient av dybdefeltet (mot dypere solid).
  const inwardAngle = (px, py) => {
    const c = Math.floor(px / bs), r = Math.floor(py / bs);
    const gx = depthCell(c + 1, r) - depthCell(c - 1, r);
    const gy = depthCell(c, r + 1) - depthCell(c, r - 1);
    return (gx === 0 && gy === 0) ? null : Math.atan2(gy, gx);
  };

  const veins = [];
  const stack = [];
  const grow = (x, y, ang, steps, seed) => {
    const pts = [{ x, y }];
    for (let s = 0; s < steps && veins.length + stack.length < MYCEL_CFG.maxVeins; s++) {
      ang += (hash2(seed * 131 + s, (x | 0) + (y | 0) * 7) * 2 - 1) * MYCEL_CFG.jitter;  // liten bølge
      if (hash2(seed * 257 + s, 3) < MYCEL_CFG.kinkProb)                                  // skarpt lyn-knekk
        ang += (hash2(s, seed) < 0.5 ? -1 : 1) * MYCEL_CFG.kinkAmt;
      const inn = inwardAngle(x, y);                 // svak dragning mot dybde-gradienten (innover)
      if (inn !== null) {
        let da = inn - ang; while (da > Math.PI) da -= 2 * Math.PI; while (da < -Math.PI) da += 2 * Math.PI;
        ang += da * MYCEL_CFG.inwardPull;
      }
      const nx = x + Math.cos(ang) * bs * MYCEL_CFG.stepLen;
      const ny = y + Math.sin(ang) * bs * MYCEL_CFG.stepLen;
      if (!solidAt(nx, ny)) break;                   // nådde kanten → stopp (fyller kun solid)
      x = nx; y = ny; pts.push({ x, y });
      if (s > 2 && hash2(seed * 977 + s, 13) < MYCEL_CFG.branchProb) {
        const off = (hash2(seed + s, 3) < 0.5 ? 1 : -1) * (0.7 + 0.6 * hash2(s, seed));  // vid lyn-forgrening
        stack.push({ x, y, ang: ang + off, steps: Math.floor(steps * 0.55), seed: seed * 7 + s + 1 });
      }
    }
    if (pts.length > 1) veins.push(pts);
  };

  let seedIx = 0;
  for (let r = 0; r < rows && veins.length < MYCEL_CFG.maxVeins; r++)
    for (let c = 0; c < cols; c++) {
      if (depth[r * cols + c] !== 1) continue;        // kant-solid (depth 1) = der vener seedes
      if ((seedIx++ % MYCEL_CFG.seedStride) !== 0) continue;
      const x = (c + 0.5) * bs, y = (r + 0.5) * bs;
      const inn = inwardAngle(x, y);
      const ang = inn !== null ? inn : hash2(c, r) * Math.PI * 2;
      grow(x, y, ang, MYCEL_CFG.maxSteps, c * 73856093 + r * 19349663);
      while (stack.length) { const b = stack.pop(); grow(b.x, b.y, b.ang, b.steps, b.seed); }
    }
  return veins;
}

/* ----------------------------------------------------------------------------
 * Kart-renderer — neon-vegger i to moduser (persistert, se readLook):
 *   «trad» — klassiske blokk-kanter (XPilot-trofast).
 *   «new»  — organiske Chaikin-konturer. «Avrunding» (antall hjørne-kutt) er justerbar
 *            LIVE via slideren: rawLoops caches, rebuildNeonWalls re-glatter dem billig.
 * Vegg-lag: bred halo (pulseres) + midtre + nær-hvit kjerne, alle ADD. Soft bloom via
 * kamera-Glow-filteret i create() (kun new). bg + fuel tegnes én gang.
 * ------------------------------------------------------------------------- */
function renderMap(scene, map) {
  const bs = map.blockSize;
  const bg = scene.add.graphics();
  bg.fillStyle(0x0a0f1e, 1);
  bg.fillRect(0, 0, map.widthPx, map.heightPx);
  bg.lineStyle(2, 0x2a4f8f, 0.9);
  bg.strokeRect(0, 0, map.widthPx, map.heightPx);
  bg.setDepth(-1);

  // Fuel-stasjoner — pulserende neon-markør (statisk; uavhengig av look/avrunding).
  let fuelGfx = null;
  if (map.fuelStations && map.fuelStations.length) {
    fuelGfx = scene.add.graphics().setDepth(1);
    fuelGfx.lineStyle(2, FUEL_COLOR, 0.95);
    const rad = bs * 0.42;
    for (const f of map.fuelStations) {
      fuelGfx.strokeCircle(f.x, f.y, rad);
      fuelGfx.lineBetween(f.x - rad * 0.5, f.y, f.x + rad * 0.5, f.y);
      fuelGfx.lineBetween(f.x, f.y - rad * 0.5, f.x, f.y + rad * 0.5);
    }
    fuelGfx.setBlendMode(Phaser.BlendModes.ADD);
  }
  scene.fuelGfx = fuelGfx;

  // Cache rå konturer for «new» (Chaikin påføres ved (re)bygging → billig avrunding-slider).
  scene.wallMap = map;
  scene.rawLoops = (readLook() !== 'trad') ? traceWallLoops(map) : null;
  // Mycel-vener (deterministisk per kart) — regnes én gang her, bakes inn ved hver (re)bygging.
  scene.mycelVeins = (readLook() !== 'trad') ? buildMycel(map) : null;

  // BAKING (ytelse): bak vegg-lagene til ÉN DynamicTexture og vis dem som én quad,
  // i stedet for å la Phaser re-tessellere titusenvis av linjesegmenter hver frame.
  // Nøkkelen forrige forsøk bommet på: i Phaser 4 KØER `dt.draw(...)` bare kommandoer i
  // et buffer — man MÅ kalle `dt.render()` for å flushe dem til teksturen (se
  // vendor/phaser/src/textures/DynamicTexture.js#render). Gjøres i rebuildNeonWalls.
  //
  // GPU-tekstur-tak: store kart (f.eks. kits ~6400px) kan overstige maks tekstur-størrelse,
  // så vi caper til 4096 og skalerer visnings-image-et tilsvarende opp. Faller tilbake til
  // live graphics hvis DynamicTexture ikke kan lages (Canvas-renderer e.l.).
  const MAX_TEX = 4096;
  scene.wallTexKey = 'wallbake';
  if (scene.wallImage) { scene.wallImage.destroy(); scene.wallImage = null; }
  if (scene.textures.exists(scene.wallTexKey)) scene.textures.remove(scene.wallTexKey);
  const s = Math.min(1, MAX_TEX / map.widthPx, MAX_TEX / map.heightPx);
  scene.wallBakeScale = s;
  let dt = null;
  try {
    dt = scene.textures.addDynamicTexture(scene.wallTexKey, Math.ceil(map.widthPx * s), Math.ceil(map.heightPx * s));
  } catch (e) { console.warn('DynamicTexture-baking utilgjengelig — bruker live graphics.', e); }
  scene.wallDT = dt;
  if (dt) {
    // Bakte vegger vises ADD over bg-en — speiler den forrige live-ADD-stakken.
    scene.wallImage = scene.add.image(0, 0, scene.wallTexKey)
      .setOrigin(0, 0).setDepth(0).setScale(1 / s)
      .setBlendMode(Phaser.BlendModes.ADD);
  }
  rebuildNeonWalls(scene, clampRound(readRound()));
}

// (Re)bygger de tre neon-vegg-lagene. Kalles ved oppstart OG live når «Avrunding» dras.
// Trad: blokk-kanter (avrunding ignoreres). New: cachede løkker Chaikin-glattet `rounding` ganger
// (0 = skarpe hjørner, høyere = mer trekant-kuttede/avrundede). bg + fuel røres ikke.
function rebuildNeonWalls(scene, rounding) {
  const map = scene.wallMap;
  if (!map) return;

  const isNew = readLook() !== 'trad';
  let drawInto;
  if (isNew && scene.rawLoops) {
    // Chaikin-glatt (avrunding) → del opp → organisk noise-forskyvning → siste glatting.
    // Når vi BAKER (scene.wallDT) tegnes geometrien kun ÉN gang inn i teksturen, så vi kan
    // kjøre full Avrunding og tett oppdeling. Faller vi tilbake til live graphics (tegnes
    // hver frame), holdes punkt-antallet BUNDET (capped Chaikin + grov oppdeling) så det
    // forblir spillbart.
    const baking = !!scene.wallDT;
    const cap = baking ? rounding : Math.min(rounding, 4);
    const subLen = baking ? 4 : 10;
    const loops = scene.rawLoops.map(l => {
      let p = chaikin(l, cap);
      if (ORGANIC > 0) { p = subdivideLoop(p, subLen); p = organicDisplace(p, ORGANIC, DETAIL); p = chaikin(p, 1); }
      return p;
    });
    drawInto = (g, w, color, alpha) => {
      g.lineStyle(w, color, alpha);
      for (const loop of loops) {
        g.beginPath();
        g.moveTo(loop[0].x, loop[0].y);
        for (let i = 1; i < loop.length; i++) g.lineTo(loop[i].x, loop[i].y);
        g.closePath();
        g.strokePath();
      }
    };
  } else {
    const bs = map.blockSize;
    // KONVEKSE hjørner (to åpne sider møtes) kuttes med en 45°-bevel → mindre blokkete, matcher
    // XPilots slope-tiles (s/w/a/q). `b` = kuttlengde (BEVEL·blokk, 0 = skarpe hjørner som før).
    // Kun visuelt — kollisjon er fortsatt rute-basert (firkant), så bevelen holdes konservativ
    // (kuttet ER fortsatt solid for kollisjon; skipet stopper litt før den visuelle skråningen).
    const b = Math.max(0, Math.min(0.5, BEVEL)) * bs;
    drawInto = (g, w, color, alpha) => {
      g.lineStyle(w, color, alpha);
      for (let r = 0; r < map.rows; r++)
        for (let c = 0; c < map.cols; c++) {
          if (map.tiles[r][c] !== 'x') continue;
          const x = c * bs, y = r * bs;
          const oU = !isWall(map, c, r - 1), oD = !isWall(map, c, r + 1),
                oL = !isWall(map, c - 1, r), oR = !isWall(map, c + 1, r);
          // Konveks = begge tilstøtende sider åpne. b=0 → ingen bevel.
          const TL = b > 0 && oU && oL, TR = b > 0 && oU && oR,
                BL = b > 0 && oD && oL, BR = b > 0 && oD && oR;
          if (oU) g.lineBetween(x + (TL ? b : 0), y, x + bs - (TR ? b : 0), y);            // topp
          if (oD) g.lineBetween(x + (BL ? b : 0), y + bs, x + bs - (BR ? b : 0), y + bs);  // bunn
          if (oL) g.lineBetween(x, y + (TL ? b : 0), x, y + bs - (BL ? b : 0));            // venstre
          if (oR) g.lineBetween(x + bs, y + (TR ? b : 0), x + bs, y + bs - (BR ? b : 0));  // høyre
          if (TL) g.lineBetween(x + b, y, x, y + b);                  // bevel-diagonaler (45°)
          if (TR) g.lineBetween(x + bs - b, y, x + bs, y + b);
          if (BL) g.lineBetween(x + b, y + bs, x, y + bs - b);
          if (BR) g.lineBetween(x + bs - b, y + bs, x + bs, y + bs - b);
        }
    };
  }
  // Tre lag: bred halo + midtre + nær-hvit kjerne (alle ADD).
  const layers = [
    { w: isNew ? 18 : 10, c: COLORS.wall, a: isNew ? 0.11 : 0.08, depth: 0 },
    { w: isNew ? 8  : 4,  c: COLORS.wall, a: isNew ? 0.20 : 0.18, depth: 0 },
    { w: 2,               c: 0xdff0ff,    a: 0.95,                depth: 1 },
  ];

  // Mycel-fyll: tegn de cachede venene inn i en graphics med flere ADD-pass — bred halo +
  // glød + kromatisk-aberrasjon (RGB-split) + nær-hvit kjerne (Solstice-aktig neon-frynse).
  // Returnerer null hvis mycel er av/tomt. Tegnes UNDER konturen (ADD → rekkefølge er uansett
  // kommutativ). Brukes både i bake (offscreen) og live-fallback.
  const fillMycel = (g) => {
    const stroke = (w, color, alpha, ox, oy) => {
      g.lineStyle(w, color, alpha);
      for (const v of scene.mycelVeins) {
        g.beginPath(); g.moveTo(v[0].x + ox, v[0].y + oy);
        for (let i = 1; i < v.length; i++) g.lineTo(v[i].x + ox, v[i].y + oy);
        g.strokePath();
      }
    };
    const M = MYCEL;
    stroke(4.0, 0x4488ff,   0.05 * M,  0,    0);   // smal elektrisk halo
    stroke(2.0, 0x88ccff,   0.11 * M,  0,    0);   // glød
    stroke(1.4, 0xff3a6a,   0.17 * M, -0.8,  0);   // kromatisk rød (venstre-skift)
    stroke(1.4, 0x3a6aff,   0.17 * M,  0.8,  0);   // kromatisk blå (høyre-skift)
    stroke(0.8, 0xf0ffff,   0.50 * M,  0,    0);   // skarp hvit-blå lyn-kjerne
  };
  // Trad-fyll: triangulert neon-mesh inni de solide sonene (viser «fast» tydelig, slik Tommy
  // husker XPilot). Per solid tile: delte indre kanter (topp/venstre, hver tegnet én gang) +
  // én uniform anti-diagonal → hver celle blir to trekanter. Dim ADD, under den lyse konturen.
  const fillTrad = (g) => {
    const bs = map.blockSize, T = TRI_FILL;
    g.lineStyle(1, COLORS.wall, 0.16 * T);
    for (let r = 0; r < map.rows; r++)
      for (let c = 0; c < map.cols; c++) {
        if (map.tiles[r][c] !== 'x') continue;
        const x = c * bs, y = r * bs;
        if (isWall(map, c, r - 1)) g.lineBetween(x, y, x + bs, y);   // delt indre topp-kant
        if (isWall(map, c - 1, r)) g.lineBetween(x, y, x, y + bs);   // delt indre venstre-kant
        g.lineBetween(x, y + bs, x + bs, y);                          // uniform anti-diagonal
      }
  };

  // Velg fyll etter look: New → mycel, Trad → trekanter (eller ingen hvis slått av).
  const hasMycel = isNew && scene.mycelVeins && scene.mycelVeins.length && MYCEL > 0;
  const hasTrad = !isNew && TRI_FILL > 0;
  const fillFn = hasMycel ? fillMycel : (hasTrad ? fillTrad : null);

  // Rydd forrige (DT klares under; live-graphics destrueres).
  const old = scene.neon;
  if (old && old.liveGfx) old.liveGfx.forEach(g => g && g.destroy());

  if (scene.wallDT) {
    // BAKE: tegn mycel + de tre ADD-lagene til offscreen-graphics → inn i DynamicTexturen ÉN
    // gang. Alt er ADD-blendet, og DynamicTextureHandler honorerer hvert objekts blendMode
    // (DRAW-kommando) → glødet akkumulerer korrekt inne i teksturen. Skaler ned ved cap.
    const sc = scene.wallBakeScale || 1;
    const offs = layers.map(L => {
      const g = scene.make.graphics({ add: false });
      drawInto(g, L.w, L.c, L.a);
      g.setBlendMode(Phaser.BlendModes.ADD);
      if (sc !== 1) g.setScale(sc);
      return g;
    });
    if (fillFn) {
      const mg = scene.make.graphics({ add: false });
      fillFn(mg);
      mg.setBlendMode(Phaser.BlendModes.ADD);
      if (sc !== 1) mg.setScale(sc);
      offs.unshift(mg);   // under konturen
    }
    scene.wallDT.clear();
    scene.wallDT.draw(offs, 0, 0);
    scene.wallDT.render();   // ← flush av kommandobufferet (det forrige forsøk glemte)
    offs.forEach(g => g.destroy());
    scene.neon = { dt: scene.wallDT, image: scene.wallImage, fuel: scene.fuelGfx };
  } else {
    // Fallback: live graphics (hvis RT ikke kunne lages).
    const live = layers.map(L => {
      const g = scene.add.graphics().setDepth(L.depth);
      drawInto(g, L.w, L.c, L.a);
      g.setBlendMode(Phaser.BlendModes.ADD);
      return g;
    });
    let fillG = null;
    if (fillFn) {
      fillG = scene.add.graphics().setDepth(0);
      fillFn(fillG);
      fillG.setBlendMode(Phaser.BlendModes.ADD);
    }
    scene.neon = { liveGfx: fillG ? [...live, fillG] : live, glowWide: live[0], glowMid: live[1], core: live[2], fuel: scene.fuelGfx };
  }
}

/* ----------------------------------------------------------------------------
 * Minimap — vises kun i scroll-modus (store single-player-kart). Et lite kart i
 * hjørnet som viser HELE banen + alle skip som prikker (spilleren uthevet), pluss
 * en ramme som markerer hva hovedkameraet ser akkurat nå. Tegnet med Graphics
 * (scrollFactor 0 → festet til skjermen). Forutsetter zoom 1 (scroll-modus), så
 * skjerm-px = verdens-px og plasseringen blir presis.
 * ------------------------------------------------------------------------- */
const MINIMAP = {
  maxW: 220, maxH: 220, margin: 12,
  bg: 0x0a0f1e, bgAlpha: 0.55,
  dotPlayer: 5, dotShip: 3, dotBullet: 1.2,
};

class Minimap {
  constructor(scene, map) {
    this.scene = scene;
    this.map = map;

    // Behold kartets bredde/høyde-forhold, pass inn i maks-boksen.
    const aspect = map.widthPx / map.heightPx;
    let w = MINIMAP.maxW, h = MINIMAP.maxW / aspect;
    if (h > MINIMAP.maxH) { h = MINIMAP.maxH; w = MINIMAP.maxH * aspect; }
    this.w = w; this.h = h;
    this.scale = w / map.widthPx;   // verdens-px → minimap-px

    // Tre lag: bakgrunn (normal blend, demper), vegger (ADD-neon, statisk),
    // prikker (ADD, tegnes på nytt hver frame).
    this.frameGfx = scene.add.graphics().setScrollFactor(0).setDepth(20);
    this.wallGfx = scene.add.graphics().setScrollFactor(0).setDepth(21)
      .setBlendMode(Phaser.BlendModes.ADD);
    this.dotGfx = scene.add.graphics().setScrollFactor(0).setDepth(22)
      .setBlendMode(Phaser.BlendModes.ADD);

    this.drawStatic();
    this.reposition();
  }

  // Bakgrunn + vegger + fuel — tegnes én gang (statisk). Vegger slås sammen til
  // horisontale kjøringer per rad for å holde antall draw-kall nede på store kart.
  drawStatic() {
    const s = this.scale, bs = this.map.blockSize;

    this.frameGfx.clear();
    this.frameGfx.fillStyle(MINIMAP.bg, MINIMAP.bgAlpha);
    this.frameGfx.fillRect(0, 0, this.w, this.h);

    const g = this.wallGfx;
    g.clear();
    g.fillStyle(COLORS.wall, 0.85);
    for (let r = 0; r < this.map.rows; r++) {
      let runStart = -1;
      for (let c = 0; c <= this.map.cols; c++) {
        const wall = c < this.map.cols && this.map.tiles[r][c] === 'x';
        if (wall && runStart === -1) runStart = c;
        else if (!wall && runStart !== -1) {
          g.fillRect(runStart * bs * s, r * bs * s, (c - runStart) * bs * s, bs * s);
          runStart = -1;
        }
      }
    }
    // Fuel-stasjoner som små grønne prikker.
    if (this.map.fuelStations) {
      g.fillStyle(FUEL_COLOR, 0.9);
      for (const f of this.map.fuelStations) g.fillCircle(f.x * s, f.y * s, 2);
    }
    // Neon-ramme rundt minimapet.
    g.lineStyle(1, COLORS.wall, 0.9);
    g.strokeRect(0, 0, this.w, this.h);
  }

  // Plasser de tre lagene nederst til høyre (kalles ved oppstart + resize).
  reposition() {
    const ox = viewW() - this.w - MINIMAP.margin;
    const oy = viewH() - this.h - MINIMAP.margin;
    this.frameGfx.setPosition(ox, oy);
    this.wallGfx.setPosition(ox, oy);
    this.dotGfx.setPosition(ox, oy);
  }

  // Dynamisk lag: kamera-utsyn-ramme, kuler, skip-prikker. Tegnes hver frame.
  update() {
    const s = this.scale, g = this.dotGfx;
    g.clear();

    // Hva hovedkameraet ser nå (klampet til minimapet, kan wrappe utenfor).
    const v = this.scene.cameras.main.worldView;
    const vx = Math.max(0, Math.min(this.w, v.x * s));
    const vy = Math.max(0, Math.min(this.h, v.y * s));
    const vr = Math.max(0, Math.min(this.w, (v.x + v.width) * s));
    const vb = Math.max(0, Math.min(this.h, (v.y + v.height) * s));
    g.lineStyle(1, 0xffffff, 0.35);
    g.strokeRect(vx, vy, vr - vx, vb - vy);

    // Kuler — bittesmå prikker i eierens farge.
    for (const b of this.scene.bullets) {
      if (b.dead || !b.sprite) continue;
      g.fillStyle(b.owner.color, 0.7);
      g.fillCircle(b.sprite.x * s, b.sprite.y * s, MINIMAP.dotBullet);
    }

    // Skip — alle levende (eller midt i respawn) tegnes; spilleren uthevet med ring.
    const player = this.scene.humanShips[0];
    for (const ship of this.scene.ships) {
      if (ship.eliminated) continue;
      const px = ship.x * s, py = ship.y * s;
      if (ship === player) {
        g.fillStyle(0xffffff, 1);
        g.fillCircle(px, py, MINIMAP.dotPlayer);
        g.lineStyle(1.5, ship.color, 1);
        g.strokeCircle(px, py, MINIMAP.dotPlayer + 2);
      } else {
        g.fillStyle(ship.color, ship.alive ? 0.95 : 0.4);
        g.fillCircle(px, py, MINIMAP.dotShip);
      }
    }
  }
}

/* ----------------------------------------------------------------------------
 * Bullet
 * ------------------------------------------------------------------------- */
class Bullet {
  constructor(scene, owner, x, y, vxFrame, vyFrame, gravityScale = 1) {
    this.scene = scene;
    this.owner = owner;
    this.life = PHYSICS.bulletLife;
    // Hvor sterkt kula påvirkes av tyngdekraften. 1 = som skipet; framtidige «tunge» skudd
    // setter > 1 (faller/buer mer). Leses fra global GRAVITY, IKKE skytterens tilstand → en
    // kule faller selv om skytteren var spawn-usårbar (gravitasjon av på skipet). Se update().
    this.gravityScale = gravityScale;

    // Arcade-image i bullet-gruppa → overlap-deteksjon mot skip.
    const s = scene.bulletGroup.create(x, y, 'bullet');
    s.setBlendMode(Phaser.BlendModes.ADD);
    s.setTint(owner.color);
    s.setDepth(4);
    s.body.setCircle(PHYSICS.bulletRadius, PHYSICS.bulletRadius, PHYSICS.bulletRadius);
    s.body.setAllowGravity(false);
    // Arcade flytter bullet'en (px/sek). Konstant fart → ingen manuell integrasjon.
    s.body.setVelocity(vxFrame * 60, vyFrame * 60);
    s.setData('bullet', this);

    this.sprite = s;
  }

  update(dtScale) {
    if (this.dead || !this.sprite) return;   // kan ha blitt destruert via overlap samme frame
    this.life -= dtScale;
    // Tyngdekraft på kula: skipet integrerer px/frame (vy += GRAVITY·dtScale); Arcade-body'en
    // er px/sek, så samme akselerasjon = GRAVITY·gravityScale·dtScale·60 lagt til velocity.y.
    // Gjelder uansett skytterens spawn-tilstand (kula eier sin egen bane).
    const g = GRAVITY * this.gravityScale;
    if (g) this.sprite.body.velocity.y += g * dtScale * 60;
    wrapObject(this.scene.map, this.sprite);
    // Bullet stoppes av vegg.
    if (this.life <= 0 || tileAt(this.scene.map, this.sprite.x, this.sprite.y) === 'x') {
      this.destroy();
    }
  }

  destroy() {
    if (this.sprite) { this.sprite.destroy(); this.sprite = null; }
    this.dead = true;
  }
}

// Input-provider fra tastatur: returnerer {thrust,left,right,fire,shield} per kall.
// AI (Fase 2 / Increment 2) leverer samme grensesnitt gjennom samme sømpunkt.
function keyboardInput(keys) {
  return () => ({
    thrust: keys.thrust.isDown,
    left:   keys.left.isDown,
    right:  keys.right.isDown,
    fire:   keys.fire.isDown,
    shield: keys.shield ? keys.shield.isDown : false,
  });
}

// Velg de n spawnene som ligger lengst fra hverandre (for n=2: paret med størst avstand).
// Hindrer at skip spawner oppå hverandre på kart med klyngede baser.
function pickSpawns(spawns, n) {
  if (spawns.length <= n) return spawns.slice();
  if (n === 2) {
    let best = [spawns[0], spawns[1]], bestD = -1;
    for (let i = 0; i < spawns.length; i++)
      for (let j = i + 1; j < spawns.length; j++) {
        const d = Math.hypot(spawns[i].x - spawns[j].x, spawns[i].y - spawns[j].y);
        if (d > bestD) { bestD = d; best = [spawns[i], spawns[j]]; }
      }
    return best;
  }
  return spawns.slice(0, n);
}

// Fri siktelinje fra (sx,sy) langs (dx,dy) til avstand dist — ingen vegg mellom.
function lineClear(map, sx, sy, dx, dy, dist) {
  const steps = Math.max(1, Math.ceil(dist / (map.blockSize / 2)));
  for (let i = 1; i <= steps; i++) {
    const t = i / steps;
    if (tileAt(map, sx + dx * t, sy + dy * t) === 'x') return false;
  }
  return true;
}

// Korteste delta fra→til, wrappet rundt kartet hvis edgewrap (toroidal).
function wrappedDelta(map, fromX, fromY, toX, toY) {
  let dx = toX - fromX, dy = toY - fromY;
  if (map.edgewrap) {
    if (Math.abs(dx) > map.widthPx / 2) dx -= Math.sign(dx) * map.widthPx;
    if (Math.abs(dy) > map.heightPx / 2) dy -= Math.sign(dy) * map.heightPx;
  }
  return { dx, dy };
}

/* ----------------------------------------------------------------------------
 * Grid-pathfinding for bots — BFS over map.tiles ('x' = blokkert). INGEN kart-
 * enrichment: bruker rutenettet som allerede finnes. Respekterer edgewrap
 * (toroidale naboer). Returnerer celle-liste [{c,r}, ...] fra start mot mål, eller
 * null hvis ingen vei innen node-cap. Kalleren throttler hvor ofte dette kjøres.
 * BFS gir korteste vei i celler; bot-en flyr den newtonsk via waypoint-styring.
 * ------------------------------------------------------------------------- */
const NAV = {
  recomputeMs: 400,    // hvor ofte en bot regner ny rute (throttle pr. bot)
  nodeCap:     8000,   // maks utforskede celler (sikkerhetstak mot patologiske kart)
  losAhead:    12,     // hvor mange waypoints fram vi «string-puller» mot
};

function findPathCells(map, c0, r0, c1, r1) {
  const cols = map.cols, rows = map.rows, wrap = !!map.edgewrap;
  if (c0 === c1 && r0 === r1) return [{ c: c1, r: r1 }];
  const prev = new Int32Array(cols * rows).fill(-1);
  const seen = new Uint8Array(cols * rows);
  const start = r0 * cols + c0, goal = r1 * cols + c1;
  seen[start] = 1;
  let queue = [start], explored = 0, found = false;
  // Bredde-først lag-for-lag (uniform kost → korteste celle-vei).
  while (queue.length && explored < NAV.nodeCap && !found) {
    const next = [];
    for (const cur of queue) {
      explored++;
      const cc = cur % cols, cr = (cur - cc) / cols;
      const cand = [[cc + 1, cr], [cc - 1, cr], [cc, cr + 1], [cc, cr - 1]];
      for (let k = 0; k < 4; k++) {
        let nc = cand[k][0], nr = cand[k][1];
        if (wrap) { nc = (nc + cols) % cols; nr = (nr + rows) % rows; }
        else if (nc < 0 || nc >= cols || nr < 0 || nr >= rows) continue;
        const ni = nr * cols + nc;
        if (seen[ni]) continue;
        seen[ni] = 1;
        if (map.tiles[nr][nc] === 'x') continue;   // vegg: markert sett, men ikke gåbar
        prev[ni] = cur;
        if (ni === goal) { found = true; break; }
        next.push(ni);
      }
      if (found) break;
    }
    queue = next;
  }
  if (!found) return null;
  const path = [];
  for (let cur = goal; cur !== -1; cur = prev[cur]) {
    const cc = cur % cols;
    path.push({ c: cc, r: (cur - cc) / cols });
  }
  path.reverse();
  return path;
}

// Ønsket BEVEGELSES-retning for en bot: rett mot målet ved fri sikt, ellers langs en
// BFS-rute rundt veggene (string-pullet mot fjerneste synlige waypoint). Sikting/skyting
// bruker fortsatt det EKTE målet — dette styrer bare hvor boten flyr. Cacher ruta på
// self._nav og regner kun på nytt ved throttle / mål-celle-bytte.
function navSteerBearing(self, scene, map, target, dx, dy, dist, fallbackBearing) {
  if (lineClear(map, self.x, self.y, dx, dy, dist)) { self._nav = null; return fallbackBearing; }

  const bs = map.blockSize, now = scene.time.now;
  const sc = Math.floor(self.x / bs), sr = Math.floor(self.y / bs);
  const gc = Math.floor(target.x / bs), gr = Math.floor(target.y / bs);
  let nav = self._nav;
  if (!nav || nav.gc !== gc || nav.gr !== gr || (now - nav.t) > NAV.recomputeMs) {
    nav = self._nav = { path: findPathCells(map, sc, sr, gc, gr), gc, gr, t: now };
  }
  const path = nav.path;
  if (!path || path.length < 2) return fallbackBearing;

  // Finn node nærmest boten (forbi passerte), string-pull så mot fjerneste node med fri sikt.
  let startI = 0, bestD = Infinity;
  const scanN = Math.min(path.length, NAV.losAhead + 4);
  for (let i = 0; i < scanN; i++) {
    const d = wrappedDelta(map, self.x, self.y, (path[i].c + 0.5) * bs, (path[i].r + 0.5) * bs);
    const dd = d.dx * d.dx + d.dy * d.dy;
    if (dd < bestD) { bestD = dd; startI = i; }
  }
  let chosen = Math.min(startI + 1, path.length - 1);
  for (let i = Math.min(startI + NAV.losAhead, path.length - 1); i > startI; i--) {
    const d = wrappedDelta(map, self.x, self.y, (path[i].c + 0.5) * bs, (path[i].r + 0.5) * bs);
    if (lineClear(map, self.x, self.y, d.dx, d.dy, Math.hypot(d.dx, d.dy))) { chosen = i; break; }
  }
  const d = wrappedDelta(map, self.x, self.y, (path[chosen].c + 0.5) * bs, (path[chosen].r + 0.5) * bs);
  return Math.atan2(d.dy, d.dx);
}

// Heuristisk AI (Increment 2): styr mot motstander, unngå vegger, skyt med fri sikt.
// Samme {thrust,left,right,fire,shield}-grensesnitt som tastatur (inputProvider-søm).
// Increment 3 (Gemini Nano-enriching) gir smartere posisjonering oppå dette.
function makeAIProvider(self, scene) {
  return () => {
    const cmd = { thrust: false, left: false, right: false, fire: false, shield: false };
    if (!self.alive) return cmd;
    const map = scene.map;
    const W = map.widthPx, H = map.heightPx;

    // Nærmeste levende motstander (free-for-all) — korteste vei med wrap. Hopp over USÅRBARE
    // (invuln: nyspawnede / takeover-grace) — kuler absorberes uansett, så å jakte dem er
    // ren spawn-camping. Å ignorere dem løser camping ved roten: boten lar nyspawnede være.
    let target = null, dx = 0, dy = 0, best = Infinity;
    for (const o of scene.ships) {
      if (o === self || !o.alive || o.eliminated || o.invuln > 0) continue;
      let ox = o.x - self.x, oy = o.y - self.y;
      if (map.edgewrap) {
        if (Math.abs(ox) > W / 2) ox -= Math.sign(ox) * W;
        if (Math.abs(oy) > H / 2) oy -= Math.sign(oy) * H;
      }
      const d = Math.hypot(ox, oy);
      if (d < best) { best = d; target = o; dx = ox; dy = oy; }
    }
    if (!target) {
      // Ingen lovlig mål (f.eks. bare nyspawnede igjen) → heng i lufta mot tyngdekraften,
      // ikke frys på stedet (som ville vært ufrivillig spawn-camping).
      if (GRAVITY > 0 && self.fuel > 0) {
        let da = (-Math.PI / 2) - self.angle;
        while (da > Math.PI) da -= 2 * Math.PI;
        while (da < -Math.PI) da += 2 * Math.PI;
        if (da > AI.turnDeadzone) cmd.right = true; else if (da < -AI.turnDeadzone) cmd.left = true;
        if (Math.abs(da) < AI.aimedCone && self.vy > -0.5) cmd.thrust = true;
      }
      return cmd;
    }
    const dist = best;
    const bearing = Math.atan2(dy, dx);

    const bs = map.blockSize;
    const speed = Math.hypot(self.vx, self.vy);

    // — Prediktiv sikting: kula arver skip-farten (se Ship.fire), så vi løser for
    //   treffpunktet med RELATIV fart. |P + Vrel·t| = bulletSpeed·t → andregradslikning.
    let leadBearing = bearing;
    {
      const vrx = (target.vx || 0) - self.vx, vry = (target.vy || 0) - self.vy;
      const a = vrx * vrx + vry * vry - PHYSICS.bulletSpeed * PHYSICS.bulletSpeed;
      const b = 2 * (dx * vrx + dy * vry);
      const c = dx * dx + dy * dy;
      let t = -1;
      if (Math.abs(a) < 1e-4) {            // farten ~= kulefart: lineær
        if (Math.abs(b) > 1e-6) t = -c / b;
      } else {
        const disc = b * b - 4 * a * c;
        if (disc >= 0) {
          const sq = Math.sqrt(disc);
          const t1 = (-b - sq) / (2 * a), t2 = (-b + sq) / (2 * a);
          t = Math.min(t1 > 0 ? t1 : Infinity, t2 > 0 ? t2 : Infinity);
        }
      }
      if (t > 0 && t < AI.leadMax) leadBearing = Math.atan2(dy + vry * t, dx + vrx * t);
    }

    // Bevegelses-retning: fri sikt → jag direkte; ellers BFS-rute rundt veggene. Skiller
    // FLYGE-retning (rundt hindringer) fra SIKTE-retning (leadBearing, mot ekte mål).
    const moveBearing = navSteerBearing(self, scene, map, target, dx, dy, dist, leadBearing);
    const following = !!(self._nav && self._nav.path && self._nav.path.length >= 2);

    // Nødbrems: ser FRAMOVER langs fartsretningen, lengre jo raskere vi går (stoppdistanse).
    // Fast [1.5,3,4.5]-blokk-sikt bremset for sent ved høy fart → bots krasjet. Nå skalerer
    // fremsynet med farten, så raske bots begynner å bremse i god tid.
    let danger = false, dangerClose = false;
    if (speed > 0.3) {
      const vang = Math.atan2(self.vy, self.vx);
      const reach = bs * (AI.brakeBase + speed * AI.brakeLead);   // px stopp-margin
      const steps = Math.max(3, Math.ceil(reach / bs));
      for (let i = 1; i <= steps; i++) {
        const d = (i / steps) * reach;
        if (tileAt(map, self.x + Math.cos(vang) * d, self.y + Math.sin(vang) * d) === 'x') {
          danger = true; if (d < bs * AI.brakeClose) dangerClose = true; break;
        }
      }
    }
    // Nær-proximitets-skjold: enhver USKJOLDET vegg-berøring dreper (unntatt myk topp-landing),
    // så selv sakte graze i trange korridorer var dødelig. Skjold ved ENHVER nær fare (uavhengig
    // av fart) når vi er under dødelig-fart og har drivstoff → boten spretter i stedet for å dø.
    if (dangerClose && speed <= PHYSICS.wallLethalSpeed && self.fuel > AI.shieldFuelMin) {
      cmd.shield = true;
    }

    let desired, allowThrust = false, allowFire = false;
    if (danger) {
      // Pek mot fartsretningen og brems (retro-thrust) — unngår vegg-selvmord. Over dødelig-fart
      // er skjold nytteløst (treffet dreper uansett), så da gjelder kun maks brems.
      desired = Math.atan2(-self.vy, -self.vx);
      allowThrust = true;
    } else {
      // Horisontal «vilje»: mot bevegelses-retningen (rute eller direkte). Følger vi en rute
      // rundt vegger, vil vi alltid fram; ellers hold keepDistance til målet.
      const wantApproach = (following || dist > bs * AI.keepDistance) ? 1 : 0;
      let tgtX = Math.cos(moveBearing) * wantApproach;
      let tgtY = Math.sin(moveBearing) * wantApproach;

      // Vegg rett foran nesen → styr unna (overstyrer den horisontale viljen med veer-retning).
      const look = bs * AI.lookahead;
      const wallAhead = tileAt(map, self.x + Math.cos(self.angle) * look, self.y + Math.sin(self.angle) * look) === 'x';
      if (wallAhead) {
        const leftWall = tileAt(map, self.x + Math.cos(self.angle - AI.sensorAngle) * look, self.y + Math.sin(self.angle - AI.sensorAngle) * look) === 'x';
        const veer = self.angle + (leftWall ? AI.avoidTurn : -AI.avoidTurn);
        tgtX = Math.cos(veer); tgtY = Math.sin(veer);
      } else {
        allowFire = true;
      }

      // Ønsket THRUST-retning = horisontal vilje + ALLTID en opp-komponent mot tyngdekraften.
      // Tyngdekraften legges til vy hver frame; for å hovre må thrust-andelen opp være
      // GRAVITY/thrustForce. Slik bobler boten oppe i stedet for å synke ned i gulvet — også
      // mens den svinger unna en vegg. På gravitasjonsløse kart (GRAVITY≈0) faller dette
      // tilbake til «sikt/styr mot målet».
      const accX = tgtX;
      const accY = tgtY - (GRAVITY > 0 ? (GRAVITY / PHYSICS.thrustForce) * AI.gravComp : 0);
      if (accX === 0 && accY === 0) { desired = leadBearing; }   // hold stilling, bare sikt
      else { desired = Math.atan2(accY, accX); allowThrust = true; }
    }

    // Roter korteste vei mot ønsket retning.
    let da = desired - self.angle;
    while (da > Math.PI) da -= 2 * Math.PI;
    while (da < -Math.PI) da += 2 * Math.PI;
    if (da > AI.turnDeadzone) cmd.right = true;
    else if (da < -AI.turnDeadzone) cmd.left = true;

    // Fart-tak: cruiser vi alt raskt i ~ønsket retning, slutt å gasse (la drag dempe). Hindrer
    // at boten redliner inn i vegger den ikke rekker å bremse for. Gjelder ikke ved nødbrems
    // (da ER thrust retro-brems) — der vil vi alltid gasse mot farten.
    let speedOK = true;
    if (!danger && speed > AI.cruiseSpeed) {
      let dv = desired - Math.atan2(self.vy, self.vx);
      while (dv > Math.PI) dv -= 2 * Math.PI;
      while (dv < -Math.PI) dv += 2 * Math.PI;
      if (Math.abs(dv) < AI.aimedCone) speedOK = false;   // farer alt dit vi vil, fort nok
    }
    if (allowThrust && speedOK && Math.abs(da) < AI.aimedCone && self.fuel > 0) cmd.thrust = true;

    // Skyt når NESEN er på linje med LEDE-pekingen (egen vinkel, uavhengig av thrust-retning),
    // i rekkevidde, fri sikt, og ikke mens vi skjolder (skjold blokkerer skyting).
    let df = leadBearing - self.angle;
    while (df > Math.PI) df -= 2 * Math.PI;
    while (df < -Math.PI) df += 2 * Math.PI;
    if (allowFire && !cmd.shield && Math.abs(df) < AI.fireCone && dist < bs * AI.fireRange
        && lineClear(map, self.x, self.y, dx, dy, dist)) {
      cmd.fire = true;
    }

    // «Hender»: begrens hvor ofte rotasjons-retning/gass kan endres (finger-treghet). Snappy
    // ved nødbrems (overlevelse). Ellers tracker boten ønsket retning grovere → mindre presis.
    const now = scene.time.now;
    const hand = self._hand || (self._hand = { rot: 0, rotT: 0, thr: false, thrT: 0 });
    const desRot = cmd.right ? 1 : (cmd.left ? -1 : 0), desThr = !!cmd.thrust;
    if (danger) {                                   // full kontroll: hold hånd-tilstand i sync
      hand.rot = desRot; hand.thr = desThr; hand.rotT = now; hand.thrT = now;
    } else {
      if (desRot !== hand.rot && (now - hand.rotT) >= AI.handReact)  { hand.rot = desRot; hand.rotT = now; }
      if (desThr !== hand.thr && (now - hand.thrT) >= AI.handThrust) { hand.thr = desThr; hand.thrT = now; }
      cmd.left = hand.rot < 0; cmd.right = hand.rot > 0; cmd.thrust = hand.thr;
    }
    return cmd;
  };
}

/* ----------------------------------------------------------------------------
 * Ship
 * ------------------------------------------------------------------------- */
class Ship {
  constructor(scene, opts) {
    this.scene = scene;
    this.color = opts.color;
    // Input-sømpunkt: tastatur (default) eller AI-provider leverer samme signaler.
    this.input = opts.inputProvider || keyboardInput(opts.keys);
    this.spawn = opts.spawn;
    this.kills = 0;
    this.deaths = 0;
    this.fuel = PHYSICS.fuelMax;
    this.lives = PHYSICS.startLives;
    this.isBot = !!opts.isBot;
    this.label = opts.label || 'P?';
    this.eliminated = false;
    this.hp = PHYSICS.shipHP;

    this.vx = 0; this.vy = 0;
    this.angle = this.spawn.angle;
    this.alive = true;
    this.invuln = 0;
    this.fireTimer = 0;
    this.respawnTimer = 0;
    this.takeoverTimer = 0;
    this.shielded = false;
    this.grounded = false;

    // Form: menneske = Starfighter, bot = TIE-fighter (byttes ved takeover).
    this.texKey = this.isBot ? 'ship-bot' : 'ship-human';
    const s = scene.physics.add.image(this.spawn.x, this.spawn.y, this.texKey);
    s.setTint(this.color);
    s.setBlendMode(Phaser.BlendModes.ADD);
    s.setDepth(5);
    s.body.setCircle(PHYSICS.shipRadius, PHYSICS.shipRadius, PHYSICS.shipRadius);
    s.body.setAllowGravity(false);
    s.setData('ship', this);
    this.sprite = s;

    // Jeteksos: varm gradient, ADD, retning settes per frame med setParticleSpeed.
    this.thrust = scene.add.particles(0, 0, 'spark', {
      lifespan: { min: 200, max: 380 },
      scale: { start: 0.75, end: 0 },
      alpha: { start: 0.9, end: 0 },
      color: [0xffffff, 0xffcc00, 0xff9900, 0xff5500],
      blendMode: 'ADD',
      frequency: 14,
      quantity: 2,
      emitting: false,
    });
    this.thrust.setDepth(3);

    // Respawn-markør + nedtelling — vises der skipet kommer tilbake.
    this.marker = scene.add.image(this.spawn.x, this.spawn.y, this.texKey)
      .setTint(this.color).setBlendMode(Phaser.BlendModes.ADD)
      .setRotation(this.spawn.angle).setAlpha(0.3).setDepth(2).setVisible(false);
    this.countText = scene.add.text(this.spawn.x, this.spawn.y - BLOCK, '', {
      fontFamily: 'monospace', fontSize: '28px',
      color: '#' + this.color.toString(16).padStart(6, '0'),
    }).setOrigin(0.5).setDepth(7).setVisible(false);

    // Skjold-ring rundt skipet (vises mens skjold er oppe).
    this.shieldGfx = scene.add.image(this.spawn.x, this.spawn.y, 'shield')
      .setTint(0x66ccff).setBlendMode(Phaser.BlendModes.ADD).setDepth(6).setVisible(false);

    this.applySpawn();
  }

  get x() { return this.sprite.x; }
  get y() { return this.sprite.y; }

  applySpawn() {
    this.sprite.setPosition(this.spawn.x, this.spawn.y);
    this.sprite.setRotation(this.angle);
    this.sprite.setVisible(true);
    this.sprite.setAlpha(0.5);            // halvtransparent mens usårbar
    this.sprite.body.enable = true;
    this.vx = 0; this.vy = 0;
    this.alive = true;
    this.invuln = PHYSICS.spawnInvuln;
    // Spawn-«flyt»: gravitasjonen er AV til spilleren faktisk gjør noe — ellers dras et skip
    // som spawner «ute i ingenting» rett i en vegg det første sekundet. MEN skilt fra
    // skade-immuniteten (invuln): så snart man gir gass slår gravitasjonen inn (se update),
    // så et skip som spawner på solid grunn i trangt rom flyr med forventet thrust−gravitasjon
    // i stedet for full netto-thrust (som gir «for mye fart» → krasj). Skyting fjerner invuln
    // helt (offensiv handling), og da er flyt-flagget uansett uvirksomt.
    this.spawnFloat = true;
    this.fuel = PHYSICS.fuelMax;
    this.hp = PHYSICS.shipHP;
    this.shielded = false;
    this.marker.setVisible(false);
    this.countText.setVisible(false);
    this.shieldGfx.setVisible(false);
  }

  update(dtScale) {
    if (this.eliminated) return;   // ute av banen — ingen respawn
    if (!this.alive) {
      this.respawnTimer -= dtScale;
      // Nedtelling som viser hvor skipet kommer tilbake.
      this.countText.setText(String(Math.max(0, Math.ceil(this.respawnTimer / 60))));
      this.countText.setScale(1 / this.scene.cameras.main.zoom);   // konstant skjermstørrelse
      if (this.respawnTimer <= 0) {
        this.angle = this.spawn.angle;
        this.applySpawn();
      }
      return;
    }

    // Takeover-overgang: skipet «fryser» i lufta med nedtelling + pulserende highlight
    // når et menneske hopper inn i denne boten — så det er tydelig hvilket skip du nå styrer.
    if (this.takeoverTimer > 0) {
      this.takeoverTimer -= dtScale;
      this.vx = 0; this.vy = 0;
      const pulse = 0.35 + 0.45 * Math.abs(Math.sin(this.takeoverTimer * 0.18));
      this.marker.setTexture(this.texKey).setPosition(this.x, this.y)
        .setRotation(this.angle).setAlpha(pulse).setVisible(true);
      this.countText.setPosition(this.x, this.y - BLOCK).setVisible(true)
        .setText(String(Math.max(1, Math.ceil(this.takeoverTimer / 60))))
        .setScale(1 / this.scene.cameras.main.zoom);
      if (this.takeoverTimer <= 0) { this.marker.setVisible(false); this.countText.setVisible(false); }
      return;
    }

    // Respawn-usårbarhet (blink), så tette spawns ikke gir krasj-loop.
    if (this.invuln > 0) {
      this.invuln -= dtScale;
      if (this.invuln <= 0) this.sprite.setAlpha(1);
    }

    const input = this.input();

    // Skjold: hold-tast, krever drivstoff. Dreneres mens oppe; blokkerer egen skyting,
    // absorberer kuler, gjør deg uskadelig (men spretter av vegger). Se die()/onBulletHit.
    this.shielded = !!input.shield && this.fuel > 0;

    // Newbie-modus: auto-skjold rett før et vegg-treff (menneske-skip). Føler langs
    // FARTSRETNINGEN, kort fram (kun når treffet er nært nok til å ikke kunne unngås), så
    // det ikke sløser drivstoff. Engasjerer uansett fart — i newbie spretter et skjoldet
    // vegg-treff ALLTID (se vegg-kollisjonen), så de overlever der de ellers ville dødd.
    if (NEWBIE && !this.isBot && this.alive && this.invuln <= 0 && this.fuel > PHYSICS.shieldDrain) {
      const m = this.scene.map, bs = m.blockSize, sp = Math.hypot(this.vx, this.vy);
      if (sp > 0.6) {
        const vang = Math.atan2(this.vy, this.vx);
        const reach = bs * (1.0 + sp * 0.35);
        const steps = Math.max(2, Math.ceil(reach / bs));
        for (let i = 1; i <= steps; i++) {
          const d = (i / steps) * reach;
          if (tileAt(m, this.x + Math.cos(vang) * d, this.y + Math.sin(vang) * d) === 'x') { this.shielded = true; break; }
        }
      }
    }
    if (this.shielded) this.fuel -= PHYSICS.shieldDrain * dtScale;

    // Rotasjon — fart-avhengig: skarpere sving ved lav fart (TurboRaketti-pivot), normal
    // i høy fart (stabilt). Boost avtar lineært fra stillstand til turnBoostSpeed.
    const sp0 = Math.hypot(this.vx, this.vy);
    const turn = PHYSICS.turnRate
      * (1 + PHYSICS.turnBoostLow * Math.max(0, 1 - sp0 / PHYSICS.turnBoostSpeed));
    if (input.left)  this.angle -= turn * dtScale;
    if (input.right) this.angle += turn * dtScale;

    // Gass — newtonsk akselerasjon langs nesen. Krever drivstoff.
    const thrusting = input.thrust && this.fuel > 0;
    if (thrusting) {
      this.vx += Math.cos(this.angle) * PHYSICS.thrustForce * dtScale;
      this.vy += Math.sin(this.angle) * PHYSICS.thrustForce * dtScale;
      this.fuel -= PHYSICS.fuelThrust * fuelMapScale(this.scene.map) * dtScale;
      this.spawnFloat = false;   // gass = man er i kontroll → gravitasjonen på umiddelbart
    }

    // Global gravitasjon (nedover) — AV kun mens spawn-flyten varer (nyspawnet OG ikke gjort
    // noe ennå), så et skip som spawner «ute i ingenting» ikke dras rett i en vegg. Så snart
    // man gir gass (eller skyter → invuln=0) slår den inn, så flyturen fra spawn matcher
    // resten av kartet (thrust−gravitasjon, ikke full netto-thrust).
    if (!(this.invuln > 0 && this.spawnFloat)) this.vy += GRAVITY * dtScale;

    // Litt drag → terminal-fart, så et fall ikke akselererer i det uendelige.
    // PHYSICS.drag = 1 gir rent friksjonsløst vakuum.
    const dragF = 1 - (1 - PHYSICS.drag) * dtScale;
    this.vx *= dragF; this.vy *= dragF;

    // Hardt sikkerhetstak — skalert av drivstoff (drivstoff-boost): full tank = maxSpeed,
    // tom tank = maxSpeed·(1−fuelBoost). Gir nyspawnede (full tank) en sjanse mot campere.
    const sp = Math.hypot(this.vx, this.vy);
    const effMax = PHYSICS.maxSpeed * (1 - PHYSICS.fuelBoost * (1 - this.fuel / PHYSICS.fuelMax));
    if (sp > effMax) {
      const f = effMax / sp;
      this.vx *= f; this.vy *= f;
    }

    // Integrer posisjon manuelt; Arcade-body følger gameobjektet (for overlap).
    this.sprite.x += this.vx * dtScale;
    this.sprite.y += this.vy * dtScale;
    wrapObject(this.scene.map, this.sprite);
    this.sprite.setRotation(this.angle);

    // Jeteksos
    if (thrusting) {
      const dir = this.angle + Math.PI;                 // bakover
      const ex = this.x + Math.cos(dir) * PHYSICS.exhaustOffset;
      const ey = this.y + Math.sin(dir) * PHYSICS.exhaustOffset;
      const spd = PHYSICS.exhaustSpeed;
      const j = (Math.random() - 0.5) * PHYSICS.exhaustSpread;   // litt spredning
      this.thrust.setPosition(ex, ey);
      this.thrust.setParticleSpeed(
        Math.cos(dir) * spd - Math.sin(dir) * j,
        Math.sin(dir) * spd + Math.cos(dir) * j
      );
      this.thrust.emitting = true;
    } else {
      this.thrust.emitting = false;
    }

    // Skjold-ring (pulserende).
    if (this.shielded) {
      const t = this.scene.time.now;
      this.shieldGfx.setPosition(this.x, this.y).setVisible(true).setAlpha(0.55 + 0.3 * Math.sin(t * 0.012));
    } else {
      this.shieldGfx.setVisible(false);
    }

    // Skyting — krever drivstoff, og kan ikke skyte med skjold oppe.
    this.fireTimer -= dtScale;
    if (input.fire && !this.shielded && this.fireTimer <= 0 && this.fuel >= PHYSICS.fuelShot) {
      this.fire();
      this.fuel -= PHYSICS.fuelShot;
      this.fireTimer = PHYSICS.fireCooldown;
      // Å skyte er en offensiv handling → man forspiller spawn-skjoldet. Hindrer «park på
      // spawn, henge usårbar (tyngdekraft av) og pepre andre mens botene hopper over deg som
      // mål». Free-shieldet blir dermed rent defensivt. Tyngdekraften slår inn igjen neste
      // frame (invuln<=0), alpha → 1. Gjelder alle skip; takeover-frysen nås ikke (return over).
      if (this.invuln > 0) { this.invuln = 0; this.sprite.setAlpha(1); }
    }

    // Fylling — hvile på bakken lader sakte (TurboRaketti: tank opp på/ved base; hindrer
    // også soft-lock når man er tom og landet på et kart uten stasjoner), ELLER hovre sakte
    // nær en fuel-stasjon (#). `this.grounded` settes i landings-koden (forrige frame).
    if (this.fuel < PHYSICS.fuelMax) {
      let refueling = this.grounded;
      if (!refueling && Math.hypot(this.vx, this.vy) < REFUEL_SPEED && this.scene.map.fuelStations) {
        for (const f of this.scene.map.fuelStations) {
          if (Math.hypot(f.x - this.x, f.y - this.y) < REFUEL_RANGE) { refueling = true; break; }
        }
      }
      if (refueling) this.fuel = Math.min(PHYSICS.fuelMax, this.fuel + PHYSICS.fuelRefill * dtScale);
    }
    if (this.fuel < 0) this.fuel = 0;

    // Vegg-død / landing. Senter i vegg = treff. Et FLATE-treff ovenfra (åpent rett over)
    // er en landing: tål opptil LAND_CRASH_SPEED, behold lateral fart (skipet glir), bremses.
    // Side-/tak-treff eller for hardt = krasj. Skrå landing tolereres; nesa rettes opp etterpå.
    const wmap = this.scene.map;
    const bs = wmap.blockSize;
    const speed = Math.hypot(this.vx, this.vy);
    this.grounded = false;
    if (tileAt(wmap, this.x, this.y) === 'x') {
      if (this.invuln > 0) {
        // Spawn-/takeover-grace: sprett alltid harmløst ut (ingen død rett etter respawn).
        this.vx = -this.vx * PHYSICS.shieldBounce;
        this.vy = -this.vy * PHYSICS.shieldBounce;
        this.sprite.x += this.vx * dtScale * PHYSICS.bounceKick;
        this.sprite.y += this.vy * dtScale * PHYSICS.bounceKick;
      } else if (this.shielded) {
        // Newbie-mennesker spretter alltid (dødelig-fart slått av) → modusen redder dem faktisk.
        const newbieSafe = NEWBIE && !this.isBot;
        if (speed > PHYSICS.wallLethalSpeed && !newbieSafe) {
          this.die();   // for hardt treff: skjoldet holder ikke (XPilot-semantikk) → død
        } else {
          // Grunt/sakte treff: skjoldet spretter skipet ut, mot en ekstra drivstoff-kostnad.
          this.vx = -this.vx * PHYSICS.shieldBounce;
          this.vy = -this.vy * PHYSICS.shieldBounce;
          this.sprite.x += this.vx * dtScale * PHYSICS.bounceKick;
          this.sprite.y += this.vy * dtScale * PHYSICS.bounceKick;
          this.fuel -= PHYSICS.shieldDrain * 2 * dtScale;
        }
      } else {
        const cellTop = Math.floor(this.y / bs) * bs;
        const fromAbove = this.vy >= 0 && tileAt(wmap, this.x, cellTop - 1) !== 'x';
        if (fromAbove && speed <= LAND_CRASH_SPEED) {
          this.sprite.y = cellTop - LAND_MARGIN;     // skyv opp på flata
          this.vy = 0;
          this.vx *= PHYSICS.groundFriction;          // glir lateralt, bremses
          this.grounded = true;
        } else {
          this.die();                                 // side-/tak-treff eller for hardt
        }
      }
    } else if (this.vy >= 0) {
      // Pre-emptiv landing rett før flaten nås (mykt, ingen penetrasjon).
      const topBelow = (Math.floor(this.y / bs) + 1) * bs;
      if (tileAt(wmap, this.x, topBelow + 1) === 'x'
          && this.y > topBelow - LAND_MARGIN && speed < LAND_CRASH_SPEED) {
        this.sprite.y = topBelow - LAND_MARGIN;
        this.vy = 0;
        this.vx *= PHYSICS.groundFriction;
        this.grounded = true;
      }
    }

    // Auto-oppretting: når skipet hviler på bakken og spilleren ikke selv roterer, vri
    // nesa mykt mot loddrett (-PI/2) så det ser korrekt ut etter en skrå landing.
    if (this.grounded && !input.left && !input.right) {
      let d = (-Math.PI / 2) - this.angle;
      while (d > Math.PI) d -= 2 * Math.PI;
      while (d < -Math.PI) d += 2 * Math.PI;
      const step = UPRIGHT_RATE * dtScale;
      this.angle += Math.abs(d) <= step ? d : Math.sign(d) * step;
      this.sprite.setRotation(this.angle);
    }
  }

  fire() {
    const nx = this.x + Math.cos(this.angle) * PHYSICS.noseOffset;
    const ny = this.y + Math.sin(this.angle) * PHYSICS.noseOffset;
    const bvx = this.vx + Math.cos(this.angle) * PHYSICS.bulletSpeed;
    const bvy = this.vy + Math.sin(this.angle) * PHYSICS.bulletSpeed;
    this.scene.bullets.push(new Bullet(this.scene, this, nx, ny, bvx, bvy));
  }

  die() {
    if (!this.alive || this.invuln > 0 || this.shielded) return;
    this.alive = false;
    this.deaths++;
    this.lives -= 1;
    this.thrust.emitting = false;
    this.sprite.setVisible(false);
    this.sprite.body.enable = false;
    this.scene.explosion.explode(PHYSICS.explodeCount, this.x, this.y);

    // Blast-push: eksplosjonen dytter nærliggende levende skip radielt vekk (avtar med
    // avstand) — ofte rett inn i en vegg, der vegg-mekanikken tar fuel/skjold/liv. DOBBELT
    // dytt om motstander har skjold på (skjoldet absorberer ikke impulsen, bare vegg-treffet).
    const map = this.scene.map;
    for (const other of this.scene.ships) {
      if (other === this || !other.alive) continue;
      let dx = other.x - this.x, dy = other.y - this.y;
      if (map.edgewrap) {
        if (Math.abs(dx) > map.widthPx / 2)  dx -= Math.sign(dx) * map.widthPx;
        if (Math.abs(dy) > map.heightPx / 2) dy -= Math.sign(dy) * map.heightPx;
      }
      const d = Math.hypot(dx, dy);
      if (d > 0 && d < PHYSICS.blastRadius) {
        const imp = PHYSICS.blastForce * (1 - d / PHYSICS.blastRadius) * (other.shielded ? 2 : 1);
        other.vx += (dx / d) * imp;
        other.vy += (dy / d) * imp;
      }
    }

    // Liv: respawn hvis liv igjen, ellers eliminert (ute av banen).
    if (this.lives > 0) {
      this.respawnTimer = PHYSICS.respawnDelay;
      // Plassér respawn-ghost + nedtelling på SKIPETS spawn (ikke der det døde). Viktig
      // for overtatte bots: takeover flyttet markøren til takeover-stedet, og uten denne
      // re-plasseringen viste nedtellingen feil sted (særlig første død etter takeover).
      this.marker.setTexture(this.texKey).setPosition(this.spawn.x, this.spawn.y)
        .setRotation(this.spawn.angle).setAlpha(0.3).setVisible(true);
      this.countText.setPosition(this.spawn.x, this.spawn.y - BLOCK).setVisible(true);
    } else {
      this.eliminated = true;
      this.scene.onEliminated(this);
    }
  }
}

/* ----------------------------------------------------------------------------
 * GameScene
 * ------------------------------------------------------------------------- */
class GameScene extends Phaser.Scene {
  constructor() { super('game'); }

  create() {
    this.map = MAP;
    GAME_SCENE = this;            // for live tuning (rebuild av vegger, glød-styrke)

    makeTextures(this);
    createStarfield(this);        // parallax-starfield + måne (bak veggene)
    renderMap(this, this.map);

    // New look: ekte soft bloom via kamera-Glow-filter (Phaser 4 filters) — WebGL-ekvivalenten
    // til Solstice sin shadowBlur. Glør hele scenen (vegger, skip, kuler) for «smeltende» neon.
    // Filter-kontrolleren lagres så «Glød»-slideren kan justere outerStrength live.
    // Pakket i try/catch så et evt. API-avvik ikke knekker spillet (faller tilbake til lag-glød).
    if (readLook() !== 'trad') {
      try {
        const cam = this.cameras.main;
        if (cam.filters) this.glowFilter = cam.filters.internal.addGlow(0x88ccff, GLOW_STRENGTH, 0, 1, false, 6, 14);
      } catch (e) { console.warn('Glow-filter utilgjengelig — bruker kun lag-glød.', e); }
    }

    // Felles eksplosjons-emitter (radial burst, gjenbrukes).
    this.explosion = this.add.particles(0, 0, 'spark', {
      lifespan: { min: 300, max: 650 },
      speed: { min: 60, max: 280 },
      scale: { start: 1.1, end: 0 },
      alpha: { start: 1, end: 0 },
      color: [0xffffff, 0xffdd66, 0xff7722, 0xaa1100],
      blendMode: 'ADD',
      emitting: false,
    });
    this.explosion.setDepth(10);

    // Bullets
    this.bullets = [];
    this.bulletGroup = this.physics.add.group();

    // Input — poll hver frame (ikke events), så begge spillere kan holde taster.
    const kb = this.input.keyboard;
    kb.addCapture('SPACE,UP,DOWN,LEFT,RIGHT');
    const keys = kb.addKeys('W,A,S,D,UP,DOWN,LEFT,RIGHT,SPACE,ENTER');

    // Free-for-all: GAME.shipCount skip. Første `humanCount` er menneske-styrt
    // (tastatur), resten bots. AI-toggle PÅ = bare P1 (mot bots).
    // P1: W gass / A,D rotér / S skjold (Fase 2) / SPACE fyr.
    // P2: ↑ gass / ←,→ rotér / ↓ skjold (Fase 2) / ENTER fyr.
    const aiOn = readAIOn();
    // Antall skip skaleres med kartets størrelse (større kart → flere skip), MEN aldri
    // flere enn distinkte spawn-punkter — ellers stables skip på samme base og dør
    // momentant av kollisjon (f.eks. TRII N-spiller-kart med få baser: Ekolos_4p = 4 baser).
    const area = this.map.cols * this.map.rows;
    const spawnPts = this.map.spawns.length;
    const shipCount = Math.min(spawnPts,
      Math.max(GAME.minShips, Math.min(GAME.maxShips, Math.round(area / GAME.tilesPerShip))));
    const humanCount = Math.min(shipCount, aiOn ? 1 : GAME.humans);

    const humanKeyDefs = [
      { thrust: keys.W,  left: keys.A,    right: keys.D,     fire: keys.SPACE, shield: keys.S },
      { thrust: keys.UP, left: keys.LEFT, right: keys.RIGHT, fire: keys.ENTER, shield: keys.DOWN },
    ];
    const palette = [COLORS.p1, COLORS.p2, 0xffcc33, 0x66ff66, 0xff6688, 0xaa88ff, 0xffffff, 0xff8844];

    // Distinkte, spredte spawns — én base per skip (ingen stabling).
    const chosenSpawns = pickSpawns(this.map.spawns, shipCount);
    this.humanKeys = [];   // tastatur-providere (for takeover)
    this.humanCount = humanCount;
    this.ships = [];
    for (let i = 0; i < shipCount; i++) {
      const human = i < humanCount;
      const ship = new Ship(this, {
        color: palette[i % palette.length],
        spawn: chosenSpawns[i],
        isBot: !human,
        label: human ? 'P' + (i + 1) : 'Bot ' + (i - humanCount + 1),
        keys: human ? humanKeyDefs[i] : null,
      });
      if (human) this.humanKeys[i] = keyboardInput(humanKeyDefs[i]);
      else ship.input = makeAIProvider(ship, this);
      this.ships.push(ship);
    }
    // Skipet hvert menneske styrer akkurat nå (endres ved takeover).
    this.humanShips = this.ships.slice(0, humanCount);

    // Kamera-modus. Fler-menneske-lokalt OG små single-player-kart: fit-hele-kartet
    // (zoom så hele banen vises). Single-player (1 menneske) på STORT kart: scrolling-
    // kamera (zoom 1) som følger menneskets skip + minimap. Se memory lokal-vs-nettverk-kart.
    this.scrollMode = humanCount === 1 && !isLocallyPlayable(this.map.cols, this.map.rows);
    this.setupCamera();

    // Kollisjoner. Skip-mot-skip: alle par (begge dør). Bullet-mot-skip: per skip.
    for (let i = 0; i < this.ships.length; i++)
      for (let j = i + 1; j < this.ships.length; j++)
        this.physics.add.overlap(this.ships[i].sprite, this.ships[j].sprite, (a, b) => {
          const sa = a.getData('ship'), sb = b.getData('ship');
          if (sa && sb) { sa.die(); sb.die(); }
        });
    for (const ship of this.ships) {
      this.physics.add.overlap(this.bulletGroup, ship.sprite, (a, b) => this.onBulletHit(a, b));
    }

    // HUD-referanser (DOM)
    this.hudP1 = document.getElementById('score-p1');
    this.hudP2 = document.getElementById('score-p2');
    this.fuelP1 = document.getElementById('fuel-p1');
    this.fuelP2 = document.getElementById('fuel-p2');
    this.livesP1 = document.getElementById('lives-p1');
    this.livesP2 = document.getElementById('lives-p2');
    this.remainingEl = document.getElementById('remaining');
    this.over = false;
    this.winTimer = 0;            // >0 = overlevelses-vindu pågår (siste skip må overleve)
    this.pendingWinner = null;

    // R for ny runde etter game over.
    this.input.keyboard.on('keydown-R', () => { if (this.over) location.reload(); });
  }

  // Stiller kameraet etter modus. Fit: zoom for å vise hele kartet, sentrert.
  // Scroll: zoom 1 (skip i normal størrelse), kameraet sentreres på spilleren hver
  // frame i update() + et minimap i hjørnet viser hele kartet og alle skip.
  setupCamera() {
    const cam = this.cameras.main;
    if (this.scrollMode) {
      cam.setZoom(1);
      const t = this.humanShips[0];
      if (t) cam.centerOn(t.x, t.y);
      this.minimap = new Minimap(this, this.map);
      this.scale.on('resize', () => this.minimap && this.minimap.reposition());
    } else {
      const fit = () => {
        cam.setZoom(FIT * Math.min(cam.width / this.map.widthPx, cam.height / this.map.heightPx));
        cam.centerOn(this.map.widthPx / 2, this.map.heightPx / 2);
      };
      fit();
      this.scale.on('resize', fit);
    }
  }

  // Sentrer scroll-kameraet på menneskets nåværende skip (endres ved takeover). Sentrering
  // er momentan så et wrap ser riktig ut (verden wrappet → utsynet wrapper også).
  updateCamera() {
    if (!this.scrollMode) return;
    const t = this.humanShips[0];
    if (t) this.cameras.main.centerOn(t.x, t.y);
    if (this.minimap) this.minimap.update();
  }

  onEliminated(ship) {
    ship.marker.setVisible(false);
    ship.countText.setVisible(false);

    // Var et menneske eliminert? Ta over boten som gjør det DÅRLIGST (færrest kills,
    // så færrest liv) — Counter-Strike-stil, ingen poeng. Free shield ved overtakelse.
    //
    // MEN takeover skjer kun hvis det fortsatt er en reell kamp igjen, dvs. ≥2 skip
    // på banen etter at dette døde. Er det bare ÉTT skip igjen, har det skipet vunnet —
    // å overta det ville feilaktig gi spilleren seieren (rapportert quirk: 1 AI + meg →
    // jeg dør → takeover av siste AI → «jeg vant»). `ship` er allerede eliminert her.
    const survivors = this.ships.filter(s => !s.eliminated);
    const hi = this.humanShips.indexOf(ship);
    if (hi !== -1 && survivors.length >= 2) {
      const bots = this.ships.filter(s => s.isBot && !s.eliminated);
      bots.sort((a, b) => (a.kills - b.kills) || (a.lives - b.lives));
      const bot = bots[0];
      if (bot) {
        bot.isBot = false;
        bot.input = this.humanKeys[hi];
        // Behold botens opprinnelige navn + hvem som styrer den nå → ærlig seier-tekst:
        // «Bot 3 (P1) vant!» i stedet for å late som P1s eget skip vant. Bruk stabil spiller-tag
        // «P{n}» (ikke det døende skipets navn) så gjentatte takeovers ikke nøster seg.
        bot.label = bot.label + ' (P' + (hi + 1) + ')';
        // Morf til menneske-form + spillerens farge → tydelig hvilket skip som er deg.
        bot.texKey = 'ship-human';
        bot.color = ship.color;
        bot.sprite.setTexture('ship-human').setTint(ship.color);
        bot.takeoverTimer = PHYSICS.takeoverPause;                   // pause i lufta + nedtelling
        bot.invuln = PHYSICS.spawnInvuln + PHYSICS.takeoverPause;    // free shield gjennom overgangen
        bot.spawnFloat = true;                                       // flyt-grace til spilleren gir gass/skyter
        this.humanShips[hi] = bot;
      } else {
        this.humanShips[hi] = null;           // ingen bot å overta → ute
      }
    } else if (hi !== -1) {
      this.humanShips[hi] = null;             // ingen reell kamp igjen → siste skip vinner
    }
    this.checkWin();
  }

  checkWin() {
    if (this.over || this.winTimer > 0) return;
    const left = this.ships.filter(s => !s.eliminated);
    if (left.length === 0) { this.gameOver(null); return; }   // alle ute samtidig → uavgjort
    if (left.length === 1) {
      // Siste motstander eliminert — men i TurboRaketti II måtte vinneren OVERLEVE etterskjelvet:
      // kuler i lufta kunne fortsatt drepe → uavgjort. Start et overlevelses-vindu i stedet for å
      // vinne på flekken. Avgjøres i update() (vinn ved utløp, uavgjort om vinneren dør i vinduet).
      if (PHYSICS.winSurviveTime <= 0) { this.gameOver(left[0]); return; }
      this.pendingWinner = left[0];
      this.winTimer = PHYSICS.winSurviveTime;
    }
  }

  gameOver(winner) {
    if (this.over) return;
    this.over = true;
    const txt = document.getElementById('gameover-text');
    const el = document.getElementById('gameover');
    if (txt) txt.textContent = winner ? (winner.label + ' vant!') : 'Uavgjort!';
    if (el) el.style.display = 'flex';
  }

  onBulletHit(objA, objB) {
    // overlap-callback: rekkefølge kan variere — finn ut hvem som er hva.
    const bulletSprite = objA.getData('bullet') ? objA : objB;
    const shipSprite   = bulletSprite === objA ? objB : objA;
    const bullet = bulletSprite.getData('bullet');
    const ship   = shipSprite.getData('ship');
    if (!bullet || !ship || bullet.dead || !ship.alive) return;
    if (bullet.owner === ship) return;             // egen kule treffer ikke egen eier
    if (ship.invuln > 0 || ship.shielded) { bullet.destroy(); return; }   // skjold/free-shield absorberer

    bullet.destroy();
    ship.hp -= PHYSICS.shotDamage;                 // skip tåler flere treff (Turboraketti-stil)
    if (ship.hp <= 0) {
      bullet.owner.kills++;
      ship.die();
    } else {
      this.explosion.explode(4, ship.x, ship.y);   // liten treff-funk
    }
  }

  update(time, delta) {
    if (this.over) return;
    const dtScale = Math.min(delta / (1000 / 60), 2);  // clamp ved hakking

    // Pulserende neon: vegg-glødet «puster». Baket DT → animér hele image-ets alpha
    // (billig, én quad). Fallback (live graphics) → pust halo-lagene som før.
    // «Glød» styres av kamera-Glow-filteret, som blomstrer de bakte, lyse veggene.
    if (this.neon) {
      const t = time * 0.001;
      const breathe = 0.5 + 0.5 * Math.sin(t * 1.4);
      if (this.neon.image) {
        this.neon.image.setAlpha(0.82 + 0.18 * breathe);
      } else if (this.neon.glowWide) {
        const gm = GLOW_STRENGTH / 1.6;
        this.neon.glowWide.setAlpha((0.45 + 0.55 * breathe) * gm);
        this.neon.glowMid.setAlpha((0.7 + 0.3 * (1 - breathe)) * gm);
      }
      if (this.neon.fuel) this.neon.fuel.setAlpha(0.55 + 0.45 * (0.5 + 0.5 * Math.sin(t * 3)));
    }

    // Parallax-starfield: stjernelagene (uendelig fjernt) drifter saktere enn verden (dybde).
    // Månen er verdens-festet (egen world-posisjon) → kamera håndterer den, ingen per-frame her.
    if (this.starFar) {
      const cam = this.cameras.main;
      this.starFar.tilePositionX = cam.scrollX * 0.15;  this.starFar.tilePositionY = cam.scrollY * 0.15;
      this.starNear.tilePositionX = cam.scrollX * 0.40; this.starNear.tilePositionY = cam.scrollY * 0.40;
    }

    for (const ship of this.ships) ship.update(dtScale);

    for (let i = this.bullets.length - 1; i >= 0; i--) {
      const b = this.bullets[i];
      b.update(dtScale);
      if (b.dead) this.bullets.splice(i, 1);
    }

    // Overlevelses-vindu (TurboRaketti II): siste skip vinner først om det lever vinduet ut.
    // Dør det av en kule i lufta i mellomtiden → uavgjort.
    if (this.winTimer > 0) {
      if (!this.pendingWinner.alive || this.pendingWinner.eliminated) {
        this.winTimer = 0; this.gameOver(null);          // truffet i etterskjelvet → uavgjort
      } else {
        this.winTimer -= dtScale;
        if (this.winTimer <= 0) this.gameOver(this.pendingWinner);
      }
    }

    this.updateCamera();

    // HUD bundet til hvert menneskes NÅVÆRENDE skip (endres ved takeover).
    for (let i = 0; i < 2; i++) {
      const score = i === 0 ? this.hudP1 : this.hudP2;
      const fuel  = i === 0 ? this.fuelP1 : this.fuelP2;
      const lives = i === 0 ? this.livesP1 : this.livesP2;
      if (i >= this.humanCount) { score.textContent = ''; fuel.style.width = '0%'; lives.textContent = ''; continue; }
      const s = this.humanShips[i];
      if (s && !s.eliminated) {
        score.textContent = 'P' + (i + 1) + '  ' + s.kills;
        fuel.style.width = Math.max(0, s.fuel) + '%';
        lives.textContent = '♥'.repeat(Math.max(0, s.lives));
      } else {
        score.textContent = 'P' + (i + 1) + '  ute';
        fuel.style.width = '0%';
        lives.textContent = '';
      }
    }
    if (this.remainingEl) {
      this.remainingEl.textContent = this.winTimer > 0
        ? this.pendingWinner.label + ' — OVERLEV! ' + (this.winTimer / 60).toFixed(1) + 's'
        : 'Skip igjen: ' + this.ships.filter(s => !s.eliminated).length;
    }
  }
}

/* ----------------------------------------------------------------------------
 * Boot — laster valgt kart (async) før Phaser startes. Canvas = fast viewport;
 * kameraet zoomer for å vise hele kartet.
 * ------------------------------------------------------------------------- */
const MAP_KEY = 'ypilot.map';

async function boot() {
  let sel = 'test';
  try { sel = localStorage.getItem(MAP_KEY) || 'test'; } catch (e) { /* privat modus */ }
  const aiOn = readAIOn();
  MAP = await loadMap(sel);
  // Lagret kart ekskludert (kjent ødelagt), eller for stort for valgt modus? → testbane.
  // Store kart er OK i single-player (AI på = 1 menneske → scroll-kamera + minimap),
  // men ikke i fler-menneske-lokalt (2 spillere = fit-til-skjerm, krever lite kart).
  const tooBig = !isLocallyPlayable(MAP.cols, MAP.rows) && !aiOn;
  if (sel !== 'test' && (isExcludedMap(sel) || tooBig)) {
    MAP = buildTestMap(); sel = 'test';
    try { localStorage.setItem(MAP_KEY, 'test'); } catch (e) { /* privat modus */ }
  }

  loadTuning();                   // anvend lagrede tuning-verdier FØR Phaser starter
  setupGravityControl();          // bruker MAP.gravity som default hvis ingen lagret verdi
  setupMapSelector(sel);
  setupAIToggle();
  setupNewbieToggle();
  setupLookControl();
  buildTuningPanel();

  new Phaser.Game({
    type: Phaser.AUTO,
    scale: { mode: Phaser.Scale.RESIZE, parent: 'game', width: viewW(), height: viewH() },
    backgroundColor: '#05060a',
    physics: { default: 'arcade', arcade: { gravity: { x: 0, y: 0 }, debug: false } },
    scene: [GameScene],
  });
}

// Leser AI-toggle (single-player mot bots). PÅ = 1 menneske → store kart låses opp.
function readAIOn() {
  try { return localStorage.getItem('ypilot.ai') !== 'off'; } catch (e) { return true; }
}

// Newbie-modus (auto-shield): live-toggle, persistert. Når PÅ skjolder menneske-skip
// automatisk rett før vegg-treff, og et skjoldet treff spretter alltid (ingen dødelig-fart).
const NEWBIE_KEY = 'ypilot.newbie';
let NEWBIE = (() => { try { return localStorage.getItem(NEWBIE_KEY) === 'on'; } catch (e) { return false; } })();

// Look-modus: 'new' (organisk neon-glød, default) eller 'trad' (klassiske blokk-kanter).
const LOOK_KEY = 'ypilot.look';
function readLook() {
  try { return localStorage.getItem(LOOK_KEY) || 'new'; } catch (e) { return 'new'; }
}

// Trad/New-look-velger. Reload ved bytte (rendringen bygges ved scene-create).
function setupLookControl() {
  const sel = document.getElementById('look-select');
  if (!sel) return;
  sel.value = readLook();
  sel.addEventListener('change', () => {
    try { localStorage.setItem(LOOK_KEY, sel.value); } catch (e) { /* privat modus */ }
    location.reload();
  });
}

/* ----------------------------------------------------------------------------
 * Tuning-panel — alle live-justerbare knotter, gruppert i faner. Verdiene anvendes
 * umiddelbart (get/set peker rett på PHYSICS/AI/render-tilstand) og persisteres i
 * localStorage 'ypilot.tuning'. «Eksporter» viser en JSON-popup; alternativt leser
 * Claude localStorage direkte via Chrome-pluginen. Legg til nye knotter i TUNING.
 * ------------------------------------------------------------------------- */
const TUNING_KEY = 'ypilot.tuning';
const TUNING = [
  // — Bevegelse —
  { g: 'Bevegelse', key: 'thrustForce',   label: 'Thrust',        min: 0.1,  max: 0.8,  step: 0.01,  get: () => PHYSICS.thrustForce,  set: v => PHYSICS.thrustForce = +v },
  { g: 'Bevegelse', key: 'turnRate',      label: 'Sving',         min: 0.02, max: 0.15, step: 0.005, get: () => PHYSICS.turnRate,     set: v => PHYSICS.turnRate = +v },
  { g: 'Bevegelse', key: 'turnBoostLow',  label: 'Sving-boost',   min: 0,    max: 2,    step: 0.1,   get: () => PHYSICS.turnBoostLow, set: v => PHYSICS.turnBoostLow = +v },
  { g: 'Bevegelse', key: 'turnBoostSpeed',label: 'Boost-fart',    min: 1,    max: 8,    step: 0.5,   get: () => PHYSICS.turnBoostSpeed, set: v => PHYSICS.turnBoostSpeed = +v },
  { g: 'Bevegelse', key: 'maxSpeed',      label: 'Maks fart',     min: 4,    max: 16,   step: 1,     get: () => PHYSICS.maxSpeed,     set: v => PHYSICS.maxSpeed = +v },
  { g: 'Bevegelse', key: 'drag',          label: 'Drag',          min: 0.95, max: 1,    step: 0.002, get: () => PHYSICS.drag,         set: v => PHYSICS.drag = +v },
  { g: 'Bevegelse', key: 'fuelBoost',     label: 'Drivstoff-boost',min: 0,   max: 0.7,  step: 0.05,  get: () => PHYSICS.fuelBoost,    set: v => PHYSICS.fuelBoost = +v },
  // — Landing —
  { g: 'Landing',   key: 'groundFriction',label: 'Friksjon',      min: 0.7,  max: 1,    step: 0.01,  get: () => PHYSICS.groundFriction, set: v => PHYSICS.groundFriction = +v },
  { g: 'Landing',   key: 'landCrash',     label: 'Krasj-fart',    min: 3,    max: 12,   step: 0.5,   get: () => LAND_CRASH_SPEED,     set: v => LAND_CRASH_SPEED = +v },
  { g: 'Landing',   key: 'uprightRate',   label: 'Opprett-rate',  min: 0.01, max: 0.2,  step: 0.01,  get: () => UPRIGHT_RATE,         set: v => UPRIGHT_RATE = +v },
  // — Drivstoff —
  { g: 'Drivstoff', key: 'fuelThrust',    label: 'Gass-bruk',     min: 0,    max: 0.5,  step: 0.01,  get: () => PHYSICS.fuelThrust,   set: v => PHYSICS.fuelThrust = +v },
  { g: 'Drivstoff', key: 'fuelShot',      label: 'Skudd-bruk',    min: 0,    max: 6,    step: 0.5,   get: () => PHYSICS.fuelShot,     set: v => PHYSICS.fuelShot = +v },
  { g: 'Drivstoff', key: 'fuelRefill',    label: 'Fyll-rate',     min: 0.1,  max: 2,    step: 0.1,   get: () => PHYSICS.fuelRefill,   set: v => PHYSICS.fuelRefill = +v },
  // — Skjold / våpen —
  { g: 'Kamp',      key: 'shieldDrain',   label: 'Skjold-dren',   min: 0,    max: 2,    step: 0.1,   get: () => PHYSICS.shieldDrain,  set: v => PHYSICS.shieldDrain = +v },
  { g: 'Kamp',      key: 'shieldBounce',  label: 'Skjold-sprett', min: 0.2,  max: 1,    step: 0.05,  get: () => PHYSICS.shieldBounce, set: v => PHYSICS.shieldBounce = +v },
  { g: 'Kamp',      key: 'wallLethal',    label: 'Dødelig fart',  min: 2,    max: 16,   step: 0.5,   get: () => PHYSICS.wallLethalSpeed, set: v => PHYSICS.wallLethalSpeed = +v },
  { g: 'Kamp',      key: 'spawnInvuln',   label: 'Spawn-skjold',  min: 30,   max: 240,  step: 15,    get: () => PHYSICS.spawnInvuln,  set: v => PHYSICS.spawnInvuln = +v },
  { g: 'Kamp',      key: 'winSurvive',    label: 'Overlev-tid',   min: 0,    max: 360,  step: 30,    get: () => PHYSICS.winSurviveTime, set: v => PHYSICS.winSurviveTime = +v },
  { g: 'Kamp',      key: 'handReact',     label: 'Bot sving-react', min: 0,  max: 300,  step: 10,    get: () => AI.handReact,         set: v => AI.handReact = +v },
  { g: 'Kamp',      key: 'handThrust',    label: 'Bot gass-react',  min: 0,  max: 300,  step: 10,    get: () => AI.handThrust,        set: v => AI.handThrust = +v },
  { g: 'Kamp',      key: 'bounceKick',    label: 'Sprett-boost',  min: 1,    max: 2.5,  step: 0.05,  get: () => PHYSICS.bounceKick,   set: v => PHYSICS.bounceKick = +v },
  { g: 'Kamp',      key: 'blastForce',    label: 'Blast-dytt',    min: 0,    max: 12,   step: 0.5,   get: () => PHYSICS.blastForce,   set: v => PHYSICS.blastForce = +v },
  { g: 'Kamp',      key: 'blastRadius',   label: 'Blast-vidde',   min: 32,   max: 320,  step: 16,    get: () => PHYSICS.blastRadius,  set: v => PHYSICS.blastRadius = +v },
  { g: 'Kamp',      key: 'bulletSpeed',   label: 'Kulefart',      min: 6,    max: 24,   step: 1,     get: () => PHYSICS.bulletSpeed,  set: v => PHYSICS.bulletSpeed = +v },
  { g: 'Kamp',      key: 'fireCooldown',  label: 'Skyte-pause',   min: 4,    max: 30,   step: 1,     get: () => PHYSICS.fireCooldown, set: v => PHYSICS.fireCooldown = +v },
  // — Visuelt —
  { g: 'Visuelt',   key: 'round',         label: 'Avrunding',     min: 0,    max: 6,    step: 1,     get: () => ROUNDING,             set: v => { ROUNDING = clampRound(v); if (GAME_SCENE) rebuildNeonWalls(GAME_SCENE, ROUNDING); } },
  { g: 'Visuelt',   key: 'organic',       label: 'Organisk',      min: 0,    max: 24,   step: 1,     get: () => ORGANIC,              set: v => { ORGANIC = +v; if (GAME_SCENE) rebuildNeonWalls(GAME_SCENE, ROUNDING); } },
  { g: 'Visuelt',   key: 'detail',        label: 'Detalj',        min: 0.005,max: 0.06, step: 0.005, get: () => DETAIL,               set: v => { DETAIL = +v; if (GAME_SCENE) rebuildNeonWalls(GAME_SCENE, ROUNDING); } },
  { g: 'Visuelt',   key: 'glow',          label: 'Glød',          min: 0,    max: 4,    step: 0.1,   get: () => GLOW_STRENGTH,        set: v => { GLOW_STRENGTH = +v; if (GAME_SCENE && GAME_SCENE.glowFilter) GAME_SCENE.glowFilter.outerStrength = +v; } },
  { g: 'Visuelt',   key: 'mycel',         label: 'Mycel',         min: 0,    max: 2,    step: 0.1,   get: () => MYCEL,                set: v => { MYCEL = +v; if (GAME_SCENE) rebuildNeonWalls(GAME_SCENE, ROUNDING); } },
  { g: 'Visuelt',   key: 'triFill',       label: 'Trekanter',     min: 0,    max: 2,    step: 0.1,   get: () => TRI_FILL,             set: v => { TRI_FILL = +v; if (GAME_SCENE) rebuildNeonWalls(GAME_SCENE, ROUNDING); } },
  { g: 'Visuelt',   key: 'bevel',         label: 'Bevel',         min: 0,    max: 0.5,  step: 0.05,  get: () => BEVEL,                set: v => { BEVEL = +v; if (GAME_SCENE) rebuildNeonWalls(GAME_SCENE, ROUNDING); } },
  { g: 'Visuelt',   key: 'stars',         label: 'Stjerner',      min: 0,    max: 2,    step: 0.1,   get: () => STARS,                set: v => { STARS = +v; if (GAME_SCENE) applyStarVisibility(GAME_SCENE); } },
];

// Anvend lagrede tuning-verdier (kalles i boot FØR Phaser, så PHYSICS m.m. starter tunet).
function loadTuning() {
  let saved = {};
  try { saved = JSON.parse(localStorage.getItem(TUNING_KEY) || '{}'); } catch (e) { /* privat modus */ }
  for (const t of TUNING) if (saved[t.key] !== undefined && !Number.isNaN(+saved[t.key])) t.set(saved[t.key]);
}
function persistTuning() {
  const snap = {};
  for (const t of TUNING) snap[t.key] = t.get();
  try { localStorage.setItem(TUNING_KEY, JSON.stringify(snap)); } catch (e) { /* privat modus */ }
}
// Komplett øyeblikksbilde for eksport (inkl. gravitasjon + look som bor utenfor TUNING).
function exportTuning() {
  const snap = { look: readLook(), gravity: +(+GRAVITY).toFixed(4) };
  for (const t of TUNING) snap[t.key] = +(+t.get()).toFixed(4);
  return JSON.stringify(snap, null, 2);
}

function buildTuningPanel() {
  const root = document.getElementById('tuning');
  if (!root) return;
  const groups = [...new Set(TUNING.map(t => t.g))];
  let activeTab = groups[0];
  try { activeTab = localStorage.getItem('ypilot.tuningTab') || groups[0]; } catch (e) { /* */ }
  if (!groups.includes(activeTab)) activeTab = groups[0];

  root.innerHTML = '';
  const title = document.createElement('div'); title.className = 't-title'; title.textContent = 'Tuning';
  root.appendChild(title);

  const tabs = document.createElement('div'); tabs.className = 't-tabs';
  const body = document.createElement('div'); body.className = 't-body';
  root.appendChild(tabs); root.appendChild(body);

  const dec = step => (step < 1 ? (step < 0.01 ? 3 : (step < 0.1 ? 3 : 2)) : 0);
  const renderBody = () => {
    body.innerHTML = '';
    for (const t of TUNING.filter(t => t.g === activeTab)) {
      const row = document.createElement('label'); row.className = 't-row';
      const name = document.createElement('span'); name.className = 't-name'; name.textContent = t.label;
      const val = document.createElement('span'); val.className = 't-val';
      const sl = document.createElement('input');
      sl.type = 'range'; sl.min = t.min; sl.max = t.max; sl.step = t.step; sl.value = t.get();
      val.textContent = (+t.get()).toFixed(dec(t.step));
      sl.addEventListener('input', () => { t.set(sl.value); val.textContent = (+t.get()).toFixed(dec(t.step)); persistTuning(); });
      row.appendChild(name); row.appendChild(sl); row.appendChild(val);
      body.appendChild(row);
    }
  };
  for (const grp of groups) {
    const b = document.createElement('button'); b.className = 't-tab'; b.textContent = grp;
    if (grp === activeTab) b.classList.add('active');
    b.addEventListener('click', () => {
      activeTab = grp;
      try { localStorage.setItem('ypilot.tuningTab', grp); } catch (e) { /* */ }
      tabs.querySelectorAll('.t-tab').forEach(x => x.classList.toggle('active', x.textContent === grp));
      renderBody();
    });
    tabs.appendChild(b);
  }
  renderBody();

  const exp = document.createElement('button'); exp.className = 't-export'; exp.textContent = 'Eksporter';
  exp.addEventListener('click', showExportModal);
  root.appendChild(exp);
}

function showExportModal() {
  let m = document.getElementById('export-modal');
  if (!m) {
    m = document.createElement('div'); m.id = 'export-modal';
    m.innerHTML = '<div class="em-box"><div class="em-title">Lim disse til Claude (eller jeg leser localStorage <code>ypilot.tuning</code> direkte):</div><textarea class="em-text" readonly></textarea><button class="em-close">Lukk</button></div>';
    document.body.appendChild(m);
    m.addEventListener('click', e => { if (e.target === m || (e.target.className || '').includes('em-close')) m.style.display = 'none'; });
  }
  m.querySelector('.em-text').value = exportTuning();
  m.style.display = 'flex';
  const ta = m.querySelector('.em-text'); ta.focus(); ta.select();
}

// Fyller kart-velgeren med de kuraterte, embeddede kartene (Ekolos_4p + utvalgte XPilot).
// De rå 130 XPilot-.map-ene er bevisst utelatt fra UI-et — mest støy (men ligger fortsatt
// i maps/ + maps/playable-candidates.json for konvertering/testing). Alle embeddede er
// store → single-player (scroll + minimap); skjules i fler-menneske-lokalt.
function setupMapSelector(current) {
  const sel = document.getElementById('map-select');
  if (!sel) return;
  const aiOn = readAIOn();
  const addOpt = (val, label) => {
    const o = document.createElement('option');
    o.value = val; o.textContent = label;
    if (val === current) o.selected = true;
    sel.appendChild(o);
  };
  addOpt('test', 'Testbane');
  if (window.EMBEDDED_MAPS) {
    Object.keys(window.EMBEDDED_MAPS).sort().forEach(k => {
      const m = window.EMBEDDED_MAPS[k];
      if (!aiOn && !isLocallyPlayable(m.cols, m.rows)) return;
      addOpt(k, `${m.name || k}  [${m.source || 'JSON'} ${m.cols}×${m.rows}]`);
    });
  }
  sel.addEventListener('change', () => {
    try { localStorage.setItem(MAP_KEY, sel.value); } catch (e) { /* privat modus */ }
    location.reload();
  });
}

// AI-motstander-toggle (P2). Reload ved endring (skip opprettes ved boot).
function setupAIToggle() {
  const cb = document.getElementById('ai-toggle');
  if (!cb) return;
  cb.checked = readAIOn();
  cb.addEventListener('change', () => {
    try { localStorage.setItem('ypilot.ai', cb.checked ? 'on' : 'off'); } catch (e) { /* privat modus */ }
    location.reload();
  });
}

// Newbie-modus-toggle (auto-shield). LIVE — ingen reload (leser global NEWBIE hver frame).
function setupNewbieToggle() {
  const cb = document.getElementById('newbie-toggle');
  if (!cb) return;
  cb.checked = NEWBIE;
  cb.addEventListener('change', () => {
    NEWBIE = cb.checked;
    try { localStorage.setItem(NEWBIE_KEY, NEWBIE ? 'on' : 'off'); } catch (e) { /* privat modus */ }
  });
}

boot();
