# Multiplayer for Browser-Based Turn-Based Card Game

## Requirements

- Existing game runs entirely in JavaScript in the browser.
- Intended for friends, not competitive esports.
- Minimize operating costs.
- Existing free Azure VM available.
- Simplicity and maintainability preferred over architectural purity.
- Turn-based gameplay (latency requirements are very low).

---

# Option 1: Simple HTTP API + SQLite

## Architecture

```text
Browser A
    |
 HTTP
    |
Flask/FastAPI
    |
 SQLite
    |
 HTTP
    |
Browser B
```

## Client Flow

```javascript
// Every 2-5 seconds
fetch("/game/123");
```

Player move:

```javascript
fetch("/game/123/move", {
    method: "POST",
    body: JSON.stringify(move)
});
```

## Pros

- Extremely simple.
- Easy to debug.
- No WebSocket complexity.
- SQLite is sufficient for thousands of games.
- Can be implemented in a single evening.
- Easy backup and persistence.

## Cons

- Not real-time.
- Small delay before players see updates.

## Best For

Turn-based games where players spend several seconds (or minutes) thinking.

## Recommendation Score

⭐⭐⭐⭐⭐

---

# Option 2: WebSocket Server

## Architecture

```text
Browser A
    |
WebSocket
    |
Node.js / Python
    |
WebSocket
    |
Browser B
```

## Flow

```javascript
socket.send({
    action: "play_card",
    card: "fireball"
});
```

Server validates and broadcasts new state.

## Pros

- Instant updates.
- Standard multiplayer architecture.
- Scales well.

## Cons

- More code.
- More connection management.
- Harder debugging.

## Best For

Games where responsiveness matters.

## Recommendation Score

⭐⭐⭐⭐

---

# Option 3: MQTT + Mosquitto

## Architecture

```text
Browser A
    |
 MQTT/WebSocket
    |
 Mosquitto
    |
 MQTT/WebSocket
    |
Browser B
```

## Topics

```text
game/ABCD/state
game/ABCD/move
game/ABCD/chat
```

## Pros

- Familiar technology.
- Very lightweight.
- Fast.
- Excellent pub/sub model.
- Easy to inspect traffic.

## Cons

- Must build game state handling yourself.
- Not specifically designed for games.
- Retained messages and session handling require thought.

## Best For

Developers already comfortable with MQTT.

## Recommendation Score

⭐⭐⭐⭐⭐

---

# Option 4: Firebase Realtime Database

## Architecture

```text
Browser A
    |
Firebase
    |
Browser B
```

## Pros

- No server administration.
- Real-time synchronization.
- Fast development.

## Cons

- Vendor lock-in.
- Less control.
- Eventually incurs costs.
- Another platform to learn.

## Best For

Rapid prototyping.

## Recommendation Score

⭐⭐⭐

---

# Option 5: Supabase Realtime

## Architecture

```text
Browser A
    |
Supabase
    |
Browser B
```

## Pros

- Open-source friendly.
- PostgreSQL underneath.
- Real-time support built in.

## Cons

- Additional service dependency.
- More complexity than needed.

## Best For

Applications expected to grow beyond hobby scale.

## Recommendation Score

⭐⭐⭐

---

# Option 6: WebRTC Peer-to-Peer

## Architecture

```text
Browser A <----> Browser B
```

## Pros

- No game server required.
- Very low latency.

## Cons

- NAT traversal.
- STUN/TURN servers.
- Considerably more complex.
- Debugging can be painful.

## Best For

Real-time games requiring direct peer communication.

## Recommendation Score

⭐⭐

---

# Option 7: Cloudflare Durable Objects

## Architecture

```text
Game #123
    |
Durable Object
    |
State
```

Each game room owns its own state.

## Pros

- Elegant model.
- Little infrastructure management.
- Scales automatically.

## Cons

- Cloudflare-specific architecture.
- More complex than required.
- New platform to learn.

## Best For

Modern web-native multiplayer systems.

## Recommendation Score

⭐⭐⭐

---

# Pragmatic Recommendation

For Tommy-style development:

## First Prototype

```text
Javascript Frontend
        +
Python Flask/FastAPI
        +
SQLite
```

API:

```text
GET  /game/<id>
POST /game/<id>/move
```

Polling:

```javascript
setInterval(refreshGame, 3000);
```

This is probably sufficient forever for a turn-based card game.

---

## If You Want Something More Interesting

```text
Javascript Frontend
        +
Mosquitto
        +
MQTT over WebSockets
```

Topics:

```text
game/<id>/state
game/<id>/move
game/<id>/chat
```

This leverages existing MQTT experience and keeps infrastructure tiny.

---

# Final Ranking

1. Flask/FastAPI + SQLite + Polling
2. Mosquitto + MQTT/WebSockets
3. WebSocket Server
4. Supabase
5. Firebase
6. Cloudflare Durable Objects
7. WebRTC

For a hobby turn-based card game running on a free Azure VM, Option 1 is the lowest-risk path and Option 3 is the most fun path.
