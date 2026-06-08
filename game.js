/*
 * jpilot — XPilot reimplementert i nettleseren. Fase 1 MVP.
 * Phaser 4. All spillogikk i én fil (se CLAUDE.md). Norsk i kommentarer.
 *
 * Fase 1: to skip, full newtonsk fysikk, wrap, skyting, kollisjoner,
 * jeteksos og score. Skjold/energi/fuel/gravitasjon/lyd/.map-lasting
 * kommer i Fase 2.
 */

'use strict';

/* ----------------------------------------------------------------------------
 * Konstanter
 * Verdiene er XPilot-referanse, uttrykt per frame @60fps. Integrasjonen
 * skalerer med dtScale = delta / (1000/60) så det blir frame-rate-uavhengig.
 * Dette (manuell integrasjon + manuell vegg-kollisjon) er de to bevisste
 * justeringsknappene; alt annet lener seg på Phaser. Se CLAUDE.md.
 * ------------------------------------------------------------------------- */
const PHYSICS = {
  thrustForce:  0.18,   // akselerasjon per frame ved full gass
  turnRate:     0.065,  // rad per frame
  maxSpeed:     12,     // px/frame — mykt tak (clamp i MVP)
  bulletSpeed:  14,     // px/frame
  bulletLife:   120,    // frames (~2s)
  fireCooldown: 13,     // frames mellom skudd
  respawnDelay: 180,    // frames (~3s)
};

const COLORS = {
  p1:   0x00ffff,  // cyan
  p2:   0xff00ff,  // magenta
  wall: 0x4488ff,  // kjølig neon-blå
};

const BLOCK = 32;  // piksler per kartrute

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

  // Spawn-punkter i åpne hjørner; angle = retning skipet peker (rad).
  const spawnCells = [
    { col: 3,        row: 3,        angle: 0 },
    { col: cols - 4, row: 3,        angle: Math.PI },
    { col: 3,        row: rows - 4, angle: 0 },
    { col: cols - 4, row: rows - 4, angle: Math.PI },
  ];
  for (const s of spawnCells) tiles[s.row][s.col] = '_';

  return {
    cols, rows, blockSize: BLOCK,
    widthPx: cols * BLOCK, heightPx: rows * BLOCK,
    tiles, edgewrap: true,
    spawns: spawnCells.map(s => ({
      x: (s.col + 0.5) * BLOCK,
      y: (s.row + 0.5) * BLOCK,
      angle: s.angle,
    })),
  };
}

const MAP = buildTestMap();

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

/* ----------------------------------------------------------------------------
 * Teksturer — genereres én gang fra Graphics (neon-wireframe, ingen assets).
 * Skipet tegnes hvitt og tintes per spiller.
 * ------------------------------------------------------------------------- */
function makeTextures(scene) {
  const g = scene.make.graphics({ x: 0, y: 0, add: false });

  // Skip-wireframe (peker mot +x; rotasjon = vinkel direkte).
  g.lineStyle(2, 0xffffff, 1);
  g.beginPath();
  g.moveTo(26, 16);   // nese
  g.lineTo(11, 6);    // hekk topp
  g.lineTo(15, 16);   // cockpit-innhakk
  g.lineTo(11, 26);   // hekk bunn
  g.lineTo(26, 16);
  g.strokePath();
  g.generateTexture('ship', 32, 32);
  g.clear();

  // Bullet — liten lysende prikk (ADD gir glød).
  g.fillStyle(0xffffff, 0.35); g.fillCircle(8, 8, 7);
  g.fillStyle(0xffffff, 0.7);  g.fillCircle(8, 8, 4);
  g.fillStyle(0xffffff, 1.0);  g.fillCircle(8, 8, 2);
  g.generateTexture('bullet', 16, 16);
  g.clear();

  // Partikkel-prikk (eksos/eksplosjon).
  g.fillStyle(0xffffff, 0.5); g.fillCircle(4, 4, 4);
  g.fillStyle(0xffffff, 1.0); g.fillCircle(4, 4, 2);
  g.generateTexture('spark', 8, 8);

  g.destroy();
}

/* ----------------------------------------------------------------------------
 * Kart-renderer — neon-vegger. Tegner kun kanter der en fylt rute møter en
 * tom (kontur), ikke fylte blokker. Statisk, tegnet én gang.
 * ------------------------------------------------------------------------- */
function renderMap(scene, map) {
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
    s.body.setCircle(4, 4, 4);
    s.body.setAllowGravity(false);
    // Arcade flytter bullet'en (px/sek). Konstant fart → ingen manuell integrasjon.
    s.body.setVelocity(vxFrame * 60, vyFrame * 60);
    s.setData('bullet', this);

    this.sprite = s;
  }

  update(dtScale) {
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

/* ----------------------------------------------------------------------------
 * Ship
 * ------------------------------------------------------------------------- */
class Ship {
  constructor(scene, opts) {
    this.scene = scene;
    this.color = opts.color;
    this.keys = opts.keys;
    this.spawn = opts.spawn;
    this.kills = 0;
    this.deaths = 0;

    this.vx = 0; this.vy = 0;
    this.angle = this.spawn.angle;
    this.alive = true;
    this.fireTimer = 0;
    this.respawnTimer = 0;

    const s = scene.physics.add.image(this.spawn.x, this.spawn.y, 'ship');
    s.setTint(this.color);
    s.setBlendMode(Phaser.BlendModes.ADD);
    s.setDepth(5);
    s.body.setCircle(8, 8, 8);
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

    this.applySpawn();
  }

  get x() { return this.sprite.x; }
  get y() { return this.sprite.y; }

  applySpawn() {
    this.sprite.setPosition(this.spawn.x, this.spawn.y);
    this.sprite.setRotation(this.angle);
    this.sprite.setVisible(true);
    this.sprite.body.enable = true;
    this.vx = 0; this.vy = 0;
    this.alive = true;
  }

  update(dtScale) {
    if (!this.alive) {
      this.respawnTimer -= dtScale;
      if (this.respawnTimer <= 0) {
        this.angle = this.spawn.angle;
        this.applySpawn();
      }
      return;
    }

    const k = this.keys;

    // Rotasjon
    if (k.left.isDown)  this.angle -= PHYSICS.turnRate * dtScale;
    if (k.right.isDown) this.angle += PHYSICS.turnRate * dtScale;

    // Gass — newtonsk akselerasjon langs nesen, ingen friksjon.
    const thrusting = k.thrust.isDown;
    if (thrusting) {
      this.vx += Math.cos(this.angle) * PHYSICS.thrustForce * dtScale;
      this.vy += Math.sin(this.angle) * PHYSICS.thrustForce * dtScale;
    }

    // Mykt fartstak
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
      const ex = this.x + Math.cos(dir) * 12;
      const ey = this.y + Math.sin(dir) * 12;
      const spd = 90;
      const j = (Math.random() - 0.5) * 40;             // litt spredning
      this.thrust.setPosition(ex, ey);
      this.thrust.setParticleSpeed(
        Math.cos(dir) * spd - Math.sin(dir) * j,
        Math.sin(dir) * spd + Math.cos(dir) * j
      );
      this.thrust.emitting = true;
    } else {
      this.thrust.emitting = false;
    }

    // Skyting
    this.fireTimer -= dtScale;
    if (k.fire.isDown && this.fireTimer <= 0) {
      this.fire();
      this.fireTimer = PHYSICS.fireCooldown;
    }

    // Vegg-død (Fase 1: ingen skjold ennå → instant død)
    if (tileAt(this.scene.map, this.x, this.y) === 'x') {
      this.die();
    }
  }

  fire() {
    const nx = this.x + Math.cos(this.angle) * 14;
    const ny = this.y + Math.sin(this.angle) * 14;
    const bvx = this.vx + Math.cos(this.angle) * PHYSICS.bulletSpeed;
    const bvy = this.vy + Math.sin(this.angle) * PHYSICS.bulletSpeed;
    this.scene.bullets.push(new Bullet(this.scene, this, nx, ny, bvx, bvy));
  }

  die() {
    if (!this.alive) return;
    this.alive = false;
    this.deaths++;
    this.thrust.emitting = false;
    this.sprite.setVisible(false);
    this.sprite.body.enable = false;
    this.respawnTimer = PHYSICS.respawnDelay;
    this.scene.explosion.explode(28, this.x, this.y);
  }
}

/* ----------------------------------------------------------------------------
 * GameScene
 * ------------------------------------------------------------------------- */
class GameScene extends Phaser.Scene {
  constructor() { super('game'); }

  create() {
    this.map = MAP;

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

    // To skip — lokal multiplayer på samme tastatur.
    // P1: W gass / A,D rotér / S skjold (Fase 2) / SPACE fyr.
    // P2: ↑ gass / ←,→ rotér / ↓ skjold (Fase 2) / ENTER fyr.
    this.ship1 = new Ship(this, {
      color: COLORS.p1,
      spawn: this.map.spawns[0],
      keys: { thrust: keys.W, left: keys.A, right: keys.D, fire: keys.SPACE, shield: keys.S },
    });
    this.ship2 = new Ship(this, {
      color: COLORS.p2,
      spawn: this.map.spawns[3],
      keys: { thrust: keys.UP, left: keys.LEFT, right: keys.RIGHT, fire: keys.ENTER, shield: keys.DOWN },
    });
    this.ships = [this.ship1, this.ship2];

    // Kollisjoner — Phaser Arcade for entitet-mot-entitet (vegger er grid-basert).
    this.physics.add.overlap(this.ship1.sprite, this.ship2.sprite, () => {
      this.ship1.die();
      this.ship2.die();
    });
    for (const ship of this.ships) {
      this.physics.add.overlap(this.bulletGroup, ship.sprite, (a, b) => this.onBulletHit(a, b));
    }

    // HUD-referanser (DOM)
    this.hudP1 = document.getElementById('score-p1');
    this.hudP2 = document.getElementById('score-p2');
  }

  onBulletHit(objA, objB) {
    // overlap-callback: rekkefølge kan variere — finn ut hvem som er hva.
    const bulletSprite = objA.getData('bullet') ? objA : objB;
    const shipSprite   = bulletSprite === objA ? objB : objA;
    const bullet = bulletSprite.getData('bullet');
    const ship   = shipSprite.getData('ship');
    if (!bullet || !ship || bullet.dead || !ship.alive) return;
    if (bullet.owner === ship) return;   // egen kule treffer ikke egen eier

    ship.die();
    bullet.owner.kills++;
    bullet.destroy();
  }

  update(time, delta) {
    const dtScale = Math.min(delta / (1000 / 60), 2);  // clamp ved hakking

    for (const ship of this.ships) ship.update(dtScale);

    for (let i = this.bullets.length - 1; i >= 0; i--) {
      const b = this.bullets[i];
      b.update(dtScale);
      if (b.dead) this.bullets.splice(i, 1);
    }

    this.hudP1.textContent = 'P1  ' + this.ship1.kills;
    this.hudP2.textContent = 'P2  ' + this.ship2.kills;
  }
}

/* ----------------------------------------------------------------------------
 * Boot
 * ------------------------------------------------------------------------- */
new Phaser.Game({
  type: Phaser.AUTO,
  parent: 'game',
  width: MAP.widthPx,
  height: MAP.heightPx,
  backgroundColor: '#05060a',
  physics: { default: 'arcade', arcade: { gravity: { x: 0, y: 0 }, debug: false } },
  scene: [GameScene],
});
