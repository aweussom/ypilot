/*
 * jpilot — XPilot reimplementert i nettleseren. Fase 1 MVP.
 * Phaser 4. All spillogikk i én fil (se CLAUDE.md). Norsk i kommentarer.
 *
 * Fase 1: to skip, full newtonsk fysikk, wrap, skyting, kollisjoner,
 * jeteksos og score. Skjold/energi/fuel/gravitasjon/lyd/.map-lasting
 * kommer i Fase 2.
 */

'use strict';

/* ═══════════════════════════════════════════════════════════════════════════
 * TUNING — alle justerbare verdier samlet her (også de uten GUI). Endre fritt.
 * Fysikk-verdier er per frame @60fps; integrasjonen skalerer med
 * dtScale = delta/(1000/60) → frame-rate-uavhengig. Se CLAUDE.md.
 * ═══════════════════════════════════════════════════════════════════════════ */
const PHYSICS = {
  // — Bevegelse —
  thrustForce:  0.18,   // akselerasjon per frame ved full gass
  turnRate:     0.065,  // rad per frame
  maxSpeed:     12,     // px/frame — hardt sikkerhetstak (clamp)
  drag:         0.99,   // hastighet beholdt per frame (<1 = drag → terminal-fart; 1 = vakuum)
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
  spawnInvuln:  90,     // frames (~1.5s) free shield etter spawn/takeover (anti spawn-camp)
  startLives:   3,      // liv før game over
  takeoverPause: 60,    // frames (~1s) skipet «fryser» m/ nedtelling når du tar over en bot
  // — Eksplosjon / blast-push —
  explodeCount: 28,     // partikler i eksplosjonen
  blastRadius:  128,    // px — eksplosjonens dytte-rekkevidde (~4 blokker)
  blastForce:   6,      // px/frame impuls ved episenter
  // — Jeteksos —
  exhaustSpeed:  90,    // partikkel-fart bakover
  exhaustSpread: 40,    // sideveis spredning
  exhaustOffset: 12,    // px bak senteret
  // — Drivstoff —
  fuelMax:      100,    // full tank
  fuelThrust:   0.18,   // drivstoff/frame ved gass
  fuelShot:     2,      // drivstoff per skudd
  fuelRefill:   0.8,    // drivstoff/frame ved fylling nær stasjon
  // — Skjold —
  shieldDrain:  0.5,    // drivstoff/frame med skjold oppe
  shieldBounce: 0.7,    // fart beholdt ved skjold-sprett mot vegg
  // — Landing —
  groundFriction: 0.85, // vx beholdt per frame ved bakke-kontakt
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

// Landing (Lunar Lander / Turboraketti): myk + nesten oppreist nedstigning på en flat
// vegg-topp → hvile. Foreløpig enkel regel; for fort eller for skjevt = krasj.
const LAND_SPEED = 3;             // px/frame — må være saktere for å lande
const LAND_ANGLE_TOL = 0.25;      // rad (~14°) — bittelitt skjevt går, ikke mye
const LAND_MARGIN = 12;           // px senteret hviler over vegg-toppen

/* ----------------------------------------------------------------------------
 * Justerbar global gravitasjon (nedover, px/frame²). Settes via DOM-slider og
 * huskes mellom sesjoner i localStorage. Leses i Ship.update. I Fase 2 kan
 * per-kart-gravitasjon fra `.map`-headeren sette default-verdien her.
 * ------------------------------------------------------------------------- */
const GRAVITY_KEY = 'jpilot.gravity';
const GRAVITY_MAX = 0.50;   // absolutt tak — over dette er uaktuelt
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
  else GRAVITY = clampGravity((MAP && MAP.gravity) || 0);   // per-kart default

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
  const gp = (header.gravitypoint || '').match(/(-?\d+)\s*,\s*(-?\d+)/);
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

// Laster valgt kart (filnavn relativt til maps/), eller testbanen.
async function loadMap(sel) {
  if (!sel || sel === 'test') return buildTestMap();
  try {
    const text = await fetch('maps/' + sel).then(r => { if (!r.ok) throw new Error('HTTP ' + r.status); return r.text(); });
    return parseMap(text, sel);
  } catch (e) {
    console.warn('Kunne ikke laste kart', sel, '— bruker testbane.', e);
    return buildTestMap();
  }
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
 * Kart-renderer — neon-vegger. Tegner kun kanter der en fylt rute møter en
 * tom (kontur), ikke fylte blokker. Statisk, tegnet én gang.
 * ------------------------------------------------------------------------- */
function renderMap(scene, map) {
  // Spillefelt-bakgrunn + ramme — viser hvor kartet (og dermed wrap-grensen) går.
  const bg = scene.add.graphics();
  bg.fillStyle(0x0a0f1e, 1);            // litt lysere enn bakgrunnen utenfor (#05060a)
  bg.fillRect(0, 0, map.widthPx, map.heightPx);
  bg.lineStyle(2, 0x2a4f8f, 0.9);       // svak ramme markerer wrap-grensen
  bg.strokeRect(0, 0, map.widthPx, map.heightPx);
  bg.setDepth(-1);

  const g = scene.add.graphics();
  g.lineStyle(1.5, COLORS.wall, 0.9);
  const bs = map.blockSize;
  for (let r = 0; r < map.rows; r++) {
    for (let c = 0; c < map.cols; c++) {
      if (map.tiles[r][c] !== 'x') continue;
      const x = c * bs, y = r * bs;
      if (!isWall(map, c, r - 1)) g.lineBetween(x, y, x + bs, y);
      if (!isWall(map, c, r + 1)) g.lineBetween(x, y + bs, x + bs, y + bs);
      if (!isWall(map, c - 1, r)) g.lineBetween(x, y, x, y + bs);
      if (!isWall(map, c + 1, r)) g.lineBetween(x + bs, y, x + bs, y + bs);
    }
  }
  g.setBlendMode(Phaser.BlendModes.ADD);
  g.setDepth(0);

  // Fuel-stasjoner — neon-markør (sirkel + kryss).
  if (map.fuelStations && map.fuelStations.length) {
    const fg = scene.add.graphics();
    fg.lineStyle(2, FUEL_COLOR, 0.9);
    const rad = map.blockSize * 0.42;
    for (const f of map.fuelStations) {
      fg.strokeCircle(f.x, f.y, rad);
      fg.lineBetween(f.x - rad * 0.5, f.y, f.x + rad * 0.5, f.y);
      fg.lineBetween(f.x, f.y - rad * 0.5, f.x, f.y + rad * 0.5);
    }
    fg.setBlendMode(Phaser.BlendModes.ADD);
    fg.setDepth(0);
  }
}

/* ----------------------------------------------------------------------------
 * Bullet
 * ------------------------------------------------------------------------- */
class Bullet {
  constructor(scene, owner, x, y, vxFrame, vyFrame) {
    this.scene = scene;
    this.owner = owner;
    this.life = PHYSICS.bulletLife;

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

// Heuristisk AI (Increment 2): styr mot motstander, unngå vegger, skyt med fri sikt.
// Samme {thrust,left,right,fire,shield}-grensesnitt som tastatur (inputProvider-søm).
// Increment 3 (Gemini Nano-enriching) gir smartere posisjonering oppå dette.
function makeAIProvider(self, scene) {
  return () => {
    const cmd = { thrust: false, left: false, right: false, fire: false, shield: false };
    if (!self.alive) return cmd;
    const map = scene.map;
    const W = map.widthPx, H = map.heightPx;

    // Nærmeste levende motstander (free-for-all) — korteste vei med wrap.
    let target = null, dx = 0, dy = 0, best = Infinity;
    for (const o of scene.ships) {
      if (o === self || !o.alive || o.eliminated) continue;
      let ox = o.x - self.x, oy = o.y - self.y;
      if (map.edgewrap) {
        if (Math.abs(ox) > W / 2) ox -= Math.sign(ox) * W;
        if (Math.abs(oy) > H / 2) oy -= Math.sign(oy) * H;
      }
      const d = Math.hypot(ox, oy);
      if (d < best) { best = d; target = o; dx = ox; dy = oy; }
    }
    if (!target) return cmd;
    const dist = best;
    const bearing = Math.atan2(dy, dx);

    const bs = map.blockSize;
    const speed = Math.hypot(self.vx, self.vy);

    // Nødbrems: er skipet på vei (FARTSRETNING, ikke bare nese) inn i en vegg snart?
    let danger = false;
    if (speed > 0.6) {
      const vang = Math.atan2(self.vy, self.vx);
      for (const ahead of [1.5, 3, 4.5]) {
        if (tileAt(map, self.x + Math.cos(vang) * bs * ahead, self.y + Math.sin(vang) * bs * ahead) === 'x') { danger = true; break; }
      }
    }

    let desired, allowThrust = false, allowFire = false;
    if (danger) {
      // Pek mot fartsretningen og brems (retro-thrust) — unngår vegg-selvmord.
      desired = Math.atan2(-self.vy, -self.vx);
      allowThrust = true;
    } else {
      desired = bearing;
      // Vri unna hvis vegg rett foran nesen.
      const look = bs * AI.lookahead;
      const wallAhead = tileAt(map, self.x + Math.cos(self.angle) * look, self.y + Math.sin(self.angle) * look) === 'x';
      if (wallAhead) {
        const leftWall = tileAt(map, self.x + Math.cos(self.angle - AI.sensorAngle) * look, self.y + Math.sin(self.angle - AI.sensorAngle) * look) === 'x';
        desired = self.angle + (leftWall ? AI.avoidTurn : -AI.avoidTurn);
      } else {
        allowThrust = dist > bs * AI.keepDistance;
        allowFire = true;
      }
    }

    // Roter korteste vei mot ønsket retning.
    let da = desired - self.angle;
    while (da > Math.PI) da -= 2 * Math.PI;
    while (da < -Math.PI) da += 2 * Math.PI;
    if (da > AI.turnDeadzone) cmd.right = true;
    else if (da < -AI.turnDeadzone) cmd.left = true;

    const aimed = Math.abs(da) < AI.aimedCone;
    if (allowThrust && aimed) cmd.thrust = true;
    if (allowFire && aimed && Math.abs(da) < AI.fireCone && dist < bs * AI.fireRange
        && lineClear(map, self.x, self.y, dx, dy, dist)) {
      cmd.fire = true;
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
    if (this.shielded) this.fuel -= PHYSICS.shieldDrain * dtScale;

    // Rotasjon
    if (input.left)  this.angle -= PHYSICS.turnRate * dtScale;
    if (input.right) this.angle += PHYSICS.turnRate * dtScale;

    // Gass — newtonsk akselerasjon langs nesen. Krever drivstoff.
    const thrusting = input.thrust && this.fuel > 0;
    if (thrusting) {
      this.vx += Math.cos(this.angle) * PHYSICS.thrustForce * dtScale;
      this.vy += Math.sin(this.angle) * PHYSICS.thrustForce * dtScale;
      this.fuel -= PHYSICS.fuelThrust * dtScale;
    }

    // Global gravitasjon (nedover) — AV mens spawn-usårbarheten varer, så man ikke
    // dras rett i en vegg det første sekundet.
    if (this.invuln <= 0) this.vy += GRAVITY * dtScale;

    // Litt drag → terminal-fart, så et fall ikke akselererer i det uendelige.
    // PHYSICS.drag = 1 gir rent friksjonsløst vakuum.
    const dragF = 1 - (1 - PHYSICS.drag) * dtScale;
    this.vx *= dragF; this.vy *= dragF;

    // Hardt sikkerhetstak
    const sp = Math.hypot(this.vx, this.vy);
    if (sp > PHYSICS.maxSpeed) {
      const f = PHYSICS.maxSpeed / sp;
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
    }

    // Fylling — hovre sakte nær en fuel-stasjon.
    if (this.fuel < PHYSICS.fuelMax) {
      const map = this.scene.map;
      if (Math.hypot(this.vx, this.vy) < REFUEL_SPEED && map.fuelStations) {
        for (const f of map.fuelStations) {
          if (Math.hypot(f.x - this.x, f.y - this.y) < REFUEL_RANGE) {
            this.fuel = Math.min(PHYSICS.fuelMax, this.fuel + PHYSICS.fuelRefill * dtScale);
            break;
          }
        }
      }
    }
    if (this.fuel < 0) this.fuel = 0;

    // Vegg-død / landing. Senter i vegg = krasj. Ellers: myk + nesten oppreist
    // nedstigning på en flat vegg-topp rett under → hvil (Lunar Lander-stil).
    const wmap = this.scene.map;
    if (tileAt(wmap, this.x, this.y) === 'x') {
      if (this.shielded || this.invuln > 0) {
        // Skjold / free-shield: sprett tilbake ut av veggen i stedet for å dø.
        this.vx = -this.vx * PHYSICS.shieldBounce;
        this.vy = -this.vy * PHYSICS.shieldBounce;
        this.sprite.x += this.vx * dtScale * 1.5;
        this.sprite.y += this.vy * dtScale * 1.5;
        if (this.shielded) this.fuel -= PHYSICS.shieldDrain * 2 * dtScale;   // ekstra dren ved treff
      } else {
        this.die();
      }
    } else if (this.vy >= 0) {
      const topBelow = (Math.floor(this.y / wmap.blockSize) + 1) * wmap.blockSize;
      if (tileAt(wmap, this.x, topBelow + 1) === 'x'
          && this.y > topBelow - LAND_MARGIN
          && Math.hypot(this.vx, this.vy) < LAND_SPEED
          && uprightOK(this.angle)) {
        this.sprite.y = topBelow - LAND_MARGIN;   // hvil på toppen
        this.vy = 0;
        this.vx *= PHYSICS.groundFriction;         // litt bakke-friksjon
      }
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
    // avstand). MER dytt om motstander har skjold på (Fase 2 — `shielded` finnes ikke ennå).
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
      this.marker.setVisible(true);
      this.countText.setVisible(true);
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

    // Kamera: zoom for å vise HELE kartet, sentrert og så stort som mulig.
    // Følger vindusstørrelsen (RESIZE), så kartet fyller skjermen.
    const fitCamera = () => {
      const cam = this.cameras.main;
      cam.setZoom(FIT * Math.min(cam.width / this.map.widthPx, cam.height / this.map.heightPx));
      cam.centerOn(this.map.widthPx / 2, this.map.heightPx / 2);
    };
    fitCamera();
    this.scale.on('resize', fitCamera);

    makeTextures(this);
    renderMap(this, this.map);

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
    let aiOn = true;
    try { aiOn = localStorage.getItem('jpilot.ai') !== 'off'; } catch (e) { /* privat modus */ }
    // Antall skip skaleres med kartets størrelse (større kart → flere skip).
    const area = this.map.cols * this.map.rows;
    const shipCount = Math.max(GAME.minShips, Math.min(GAME.maxShips, Math.round(area / GAME.tilesPerShip)));
    const humanCount = Math.min(shipCount, aiOn ? 1 : GAME.humans);

    const humanKeyDefs = [
      { thrust: keys.W,  left: keys.A,    right: keys.D,     fire: keys.SPACE, shield: keys.S },
      { thrust: keys.UP, left: keys.LEFT, right: keys.RIGHT, fire: keys.ENTER, shield: keys.DOWN },
    ];
    const palette = [COLORS.p1, COLORS.p2, 0xffcc33, 0x66ff66, 0xff6688, 0xaa88ff, 0xffffff, 0xff8844];

    const spawns = this.map.spawns;
    this.humanKeys = [];   // tastatur-providere (for takeover)
    this.humanCount = humanCount;
    this.ships = [];
    for (let i = 0; i < shipCount; i++) {
      const human = i < humanCount;
      const ship = new Ship(this, {
        color: palette[i % palette.length],
        spawn: spawns[i % spawns.length],
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

    // R for ny runde etter game over.
    this.input.keyboard.on('keydown-R', () => { if (this.over) location.reload(); });
  }

  onEliminated(ship) {
    ship.marker.setVisible(false);
    ship.countText.setVisible(false);

    // Var et menneske eliminert? Ta over boten som gjør det DÅRLIGST (færrest kills,
    // så færrest liv) — Counter-Strike-stil, ingen poeng. Free shield ved overtakelse.
    const hi = this.humanShips.indexOf(ship);
    if (hi !== -1) {
      const bots = this.ships.filter(s => s.isBot && !s.eliminated);
      bots.sort((a, b) => (a.kills - b.kills) || (a.lives - b.lives));
      const bot = bots[0];
      if (bot) {
        bot.isBot = false;
        bot.input = this.humanKeys[hi];
        bot.label = ship.label;
        // Morf til menneske-form + spillerens farge → tydelig hvilket skip som er deg.
        bot.texKey = 'ship-human';
        bot.color = ship.color;
        bot.sprite.setTexture('ship-human').setTint(ship.color);
        bot.takeoverTimer = PHYSICS.takeoverPause;                   // pause i lufta + nedtelling
        bot.invuln = PHYSICS.spawnInvuln + PHYSICS.takeoverPause;    // free shield gjennom overgangen
        this.humanShips[hi] = bot;
      } else {
        this.humanShips[hi] = null;           // ingen bot å overta → ute
      }
    }
    this.checkWin();
  }

  checkWin() {
    if (this.over) return;
    const left = this.ships.filter(s => !s.eliminated);
    if (left.length <= 1) this.gameOver(left[0] || null);
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

    for (const ship of this.ships) ship.update(dtScale);

    for (let i = this.bullets.length - 1; i >= 0; i--) {
      const b = this.bullets[i];
      b.update(dtScale);
      if (b.dead) this.bullets.splice(i, 1);
    }

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
    if (this.remainingEl) this.remainingEl.textContent = 'Skip igjen: ' + this.ships.filter(s => !s.eliminated).length;
  }
}

/* ----------------------------------------------------------------------------
 * Boot — laster valgt kart (async) før Phaser startes. Canvas = fast viewport;
 * kameraet zoomer for å vise hele kartet.
 * ------------------------------------------------------------------------- */
const MAP_KEY = 'jpilot.map';

async function boot() {
  let sel = 'test';
  try { sel = localStorage.getItem(MAP_KEY) || 'test'; } catch (e) { /* privat modus */ }
  MAP = await loadMap(sel);
  // Falt et lagret kart utenfor lokal-filteret (for stort) eller mangler? Bruk testbanen.
  if (sel !== 'test' && !isLocallyPlayable(MAP.cols, MAP.rows)) {
    MAP = buildTestMap(); sel = 'test';
    try { localStorage.setItem(MAP_KEY, 'test'); } catch (e) { /* privat modus */ }
  }

  setupGravityControl();          // bruker MAP.gravity som default hvis ingen lagret verdi
  setupMapSelector(sel);
  setupAIToggle();

  new Phaser.Game({
    type: Phaser.AUTO,
    scale: { mode: Phaser.Scale.RESIZE, parent: 'game', width: viewW(), height: viewH() },
    backgroundColor: '#05060a',
    physics: { default: 'arcade', arcade: { gravity: { x: 0, y: 0 }, debug: false } },
    scene: [GameScene],
  });
}

// Fyller kart-velgeren fra manifestet (kun lokalt spillbare kart), reload ved valg.
async function setupMapSelector(current) {
  const sel = document.getElementById('map-select');
  if (!sel) return;
  const addOpt = (val, label) => {
    const o = document.createElement('option');
    o.value = val; o.textContent = label;
    if (val === current) o.selected = true;
    sel.appendChild(o);
  };
  addOpt('test', 'Testbane');
  try {
    const list = await fetch('maps/index.json').then(r => r.json());
    list
      .filter(m => m.w && m.h && isLocallyPlayable(m.w, m.h))
      .sort((a, b) => (a.name || a.file).localeCompare(b.name || b.file))
      .forEach(m => addOpt(m.file, `${m.name || m.file}  (${m.w}×${m.h})`));
  } catch (e) { console.warn('Fant ikke maps/index.json', e); }
  sel.addEventListener('change', () => {
    try { localStorage.setItem(MAP_KEY, sel.value); } catch (e) { /* privat modus */ }
    location.reload();
  });
}

// AI-motstander-toggle (P2). Reload ved endring (skip opprettes ved boot).
function setupAIToggle() {
  const cb = document.getElementById('ai-toggle');
  if (!cb) return;
  let on = true; try { on = localStorage.getItem('jpilot.ai') !== 'off'; } catch (e) { /* privat modus */ }
  cb.checked = on;
  cb.addEventListener('change', () => {
    try { localStorage.setItem('jpilot.ai', cb.checked ? 'on' : 'off'); } catch (e) { /* privat modus */ }
    location.reload();
  });
}

boot();
