# YPilot online multiplayer design

## Mål

Lag en online XPilot-klone i browseren, uten å ødelegge prosjektets nåværende filosofi:

- Phaser 4 + WebGL
- ren JavaScript
- fortsatt mulig å åpne lokalt
- minst mulig backend
- multiplayer først for venner
- lav nok latency til actionspill
- ikke esports-juksesikring i første versjon

---

# Anbefalt modell

## Thin authoritative relay

```text
Browser P1
    |
WebSocket
    |
ypilot-server
    |
WebSocket
    |
Browser P2/P3/P4
```

Serveren er ikke full spillmotor i første versjon.

Serveren gjør:

- romoppretting
- spiller-ID
- input-distribusjon
- enkel clock/tick
- heartbeat/ping
- evt. host-valg
- evt. sanity checks senere

Klientene gjør:

- rendering
- fysikk
- kollisjon
- prediksjon
- interpolering
- lokal spillfølelse

---

# Første fungerende online-versjon

## Ikke send posisjon først

Ikke start med:

```json
{
  "x": 123,
  "y": 456,
  "vx": 1.2,
  "vy": -0.4
}
```

Start med input:

```json
{
  "type": "input",
  "seq": 1842,
  "tick": 91233,
  "playerId": "p1",
  "keys": {
    "thrust": true,
    "left": false,
    "right": true,
    "fire": false,
    "shield": false
  }
}
```

Alle klienter kjører samme simulering.

Serveren videresender input til alle.

Dette passer XPilot bedre enn “send posisjoner hele tiden”, fordi spillet allerede er deterministisk-ish: thrust, rotasjon, bullets, wrap, grid-kollisjon.

---

# Viktig advarsel

JavaScript + Phaser + floating point + ulik frame rate betyr at perfekt lockstep sannsynligvis vil drifte over tid.

Derfor bør dette ikke være ren RTS-lockstep.

Bruk heller:

```text
input sync
+
periodisk state correction
```

---

# Praktisk nettverksmodell

## Fast simulation tick

Skill mellom:

```text
render frame
simulation tick
network tick
```

Forslag:

```text
Render:      requestAnimationFrame
Simulation: 60 Hz
Network:    20 Hz
Snapshot:   5 Hz
```

Klientene simulerer jevnt lokalt.

Serveren sender input fortløpende.

I tillegg sendes periodiske snapshots.

---

# Meldinger

## Client -> Server

### Join

```json
{
  "type": "join",
  "room": "ABCD",
  "name": "Tommy"
}
```

### Input

```json
{
  "type": "input",
  "seq": 1044,
  "tick": 88321,
  "keys": {
    "thrust": true,
    "left": false,
    "right": true,
    "fire": false,
    "shield": false
  }
}
```

### Ping

```json
{
  "type": "ping",
  "clientTime": 1710000000000
}
```

---

## Server -> Client

### Welcome

```json
{
  "type": "welcome",
  "playerId": "p2",
  "room": "ABCD",
  "serverTick": 2201
}
```

### Player joined

```json
{
  "type": "player_joined",
  "playerId": "p3",
  "name": "Rune"
}
```

### Remote input

```json
{
  "type": "remote_input",
  "playerId": "p1",
  "seq": 1044,
  "tick": 88321,
  "keys": {
    "thrust": true,
    "left": false,
    "right": true,
    "fire": false,
    "shield": false
  }
}
```

### Snapshot

```json
{
  "type": "snapshot",
  "tick": 88400,
  "ships": {
    "p1": {
      "x": 122.4,
      "y": 88.1,
      "vx": 1.2,
      "vy": -0.3,
      "angle": 2.81,
      "alive": true,
      "energy": 83
    },
    "p2": {
      "x": 321.0,
      "y": 191.5,
      "vx": -0.1,
      "vy": 0.9,
      "angle": 0.42,
      "alive": true,
      "energy": 71
    }
  },
  "bullets": [
    {
      "id": "b18422",
      "owner": "p1",
      "x": 140,
      "y": 91,
      "vx": 5.2,
      "vy": -0.8,
      "ttl": 73
    }
  ]
}
```

---

# Klientstrategi

## Local player

Lokal spiller må føles direkte.

Derfor:

```text
1. les input
2. bruk input lokalt umiddelbart
3. send input til server
4. korriger senere hvis snapshot avviker
```

Dette heter client-side prediction.

---

## Remote players

Andre spillere skal ikke “teleportere”.

Bruk interpolering:

```text
remote display position = smooth(previous snapshot -> next snapshot)
```

Ikke tegn remote skip direkte på siste mottatte posisjon.

Legg dem typisk 100 ms “bak” server-tid.

---

# Første server

## Node.js + ws

Minste praktiske server:

```text
server/
  server.js
```

Bruk:

```text
npm install ws
```

Serveransvar:

```text
rooms = {
  "ABCD": {
    players: {},
    lastInputs: {},
    hostId: "p1",
    tick: 0
  }
}
```

I første versjon kan serveren være ren relay.

---

# Hosting

## Azure VM

Bruk din gratis Azure VM.

Kjør:

```text
nginx
  /ypilot/        -> statiske filer
  /ypilot-ws/     -> WebSocket proxy til node server
```

Prosess:

```text
systemd -> node server.js
```

TLS:

```text
Caddy eller nginx + certbot
```

WebSocket krever HTTPS/WSS hvis siden lastes over HTTPS.

---

# Viktig designvalg

## Host-authoritative v1

For venner er dette den beste kompromissmodellen.

```text
Host client kjører fasit-simulering
Server videresender
Andre klienter predikerer/interpolerer
```

Fordeler:

- mye enklere enn ekte autoritativ server
- ingen komplett spillmotor på server
- godt nok for venner
- lett å debugge
- billig

Ulemper:

- host kan jukse
- host disconnect er plagsomt
- litt drift må korrigeres

---

# Senere designvalg

## Server-authoritative v2

Når spillet er stabilt:

```text
Client sender input
Server kjører fysikk
Server sender snapshots
Client renderer
```

Dette er riktig modell for offentlig multiplayer.

Men ikke start her.

Det gjør prosjektet til et nettverksmotor-prosjekt med et lite spill inni.

---

# Integrasjon i eksisterende game.js

Ikke riv opp alt.

Legg til et lite nettverkslag:

```javascript
const NET = {
    mode: "local", // "local", "client", "host"
    socket: null,
    playerId: null,
    room: null,
    inputSeq: 0,
    remoteInputs: {},
    snapshots: []
};
```

Så endrer du input-kilden:

```text
før:
  Ship.update(keys)

etter:
  Ship.update(inputState)
```

Der `inputState` kan komme fra:

```text
local keyboard
remote websocket input
recorded replay
AI bot
```

Det gir også gratis replay/test senere.

---

# Minimal trinnvis plan

## Trinn 1: Skill input fra keyboard

Lag en struktur:

```javascript
{
  thrust: false,
  left: false,
  right: false,
  fire: false,
  shield: false
}
```

Skip skal ikke lese tastatur direkte.

Skip skal bare få input-state.

---

## Trinn 2: Lag lokal “network simulator”

Før WebSocket:

```javascript
sendInput(input) {
    setTimeout(() => receiveRemoteInput(input), 80);
}
```

Simuler latency lokalt.

Hvis spillet tåler dette, tåler det ekte nett bedre.

---

## Trinn 3: WebSocket relay

Lag server som bare gjør:

```text
on message from player:
    broadcast to all other players in same room
```

Ingen fysikk.

Ingen database.

Ingen auth.

---

## Trinn 4: Romkode

URL:

```text
https://example.com/ypilot/?room=ABCD
```

Hvis room mangler:

```text
lag tilfeldig firebokstavs kode
```

---

## Trinn 5: Snapshots

Host sender snapshot 5 ganger i sekundet.

Andre klienter bruker snapshot til å rette opp drift.

---

## Trinn 6: Interpolering

Remote skip tegnes interpolert.

Ikke direkte på rå nettverksdata.

---

## Trinn 7: Reconnect / pause

Hvis en spiller mister kontakt:

```text
ship becomes ghost
or
round pauses
```

For venner er pause enklest.

---

# Ikke gjør dette først

Ikke start med:

- WebRTC
- Firebase
- full server-authoritative physics
- anti-cheat
- database
- accounts/login
- matchmaking
- rollback netcode
- binary protocol
- ECS rewrite
- TypeScript-convertering
- bundler

Dette er veien til myra.

Fin myr, sikkert. Men myr.

---

# Anbefalt MVP

## Online Duel

```text
2 spillere
ett kart
ingen reconnect
ingen persistent score
host-authoritative
WebSocket relay
periodiske snapshots
```

Det er nok til å avgjøre om YPilot faktisk er morsomt online.

---

# Endelig anbefaling

For YPilot:

```text
Phase Online-1:
    Node.js ws relay på Azure VM
    browser-klient beholder all fysikk
    host sender snapshots
    clients sender inputs
    remote players interpoleres

Phase Online-2:
    server room state
    reconnect
    spectator
    2-4 spillere

Phase Online-3:
    vurder server-authoritative physics
    bare hvis spillet faktisk brukes av folk utenfor vennekretsen
```

Dette holder prosjektet nært nåværende filosofi:

```text
enkelt
billig
browser-first
ingen unødvendig plattform
mest mulig spill, minst mulig enterprise
```
