# jpilot

XPilot reimagined in the browser. Phaser 4 + WebGL, neon-wireframe aesthetic,
local same-keyboard multiplayer, and support for the original XPilot `.map`
format. A homage to the XPilot crew from the University of Tromsø and to the
Finnish author of Turboraketti.

No build step, no dependencies — open `index.html` and play.

---

> Resten av prosjektet er på norsk (se `CLAUDE.md` for hvorfor).

## Kjør

Åpne `index.html` i en nettleser (dobbeltklikk holder). Eventuelt server mappa
over HTTP for å unngå caching:

```
python -m http.server 8000
```

…og åpne `http://localhost:8000`.

## Kontroller (Fase 1 — lokal 2-spiller)

| Handling | Spiller 1 | Spiller 2 |
|---|---|---|
| Gass     | `W`       | `↑`       |
| Rotér venstre | `A`  | `←`       |
| Rotér høyre   | `D`  | `→`       |
| Fyr      | `Mellomrom` | `Enter` |
| Skjold (Fase 2) | `S` | `↓`    |

## Status

- **Fase 1 (MVP):** to skip, newtonsk fysikk, wrap, skyting, kollisjoner,
  jeteksos, score. ✅
- **Fase 2:** skjold/energi, fuel-pods, gravitasjon, radar, lyd, `.map`-lasting.
- **Fase 3:** Turboraketti-lag (kart og våpen).

## Stack

Phaser 4 (CDN, ingen bundler), ren JavaScript i én `game.js`. Se `CLAUDE.md` for
arkitektur og `XPILOT-JAVASCRIPT-PLAN.md` for full plan.
