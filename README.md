# YPilot

*Because Y comes after X.*

XPilot reimagined in the browser. Phaser 4 + WebGL, neon-wireframe aesthetic,
local multiplayer with AI bots, shields, fuel, and support for the original
XPilot `.map` format. A homage to the XPilot crew from the University of Tromsø
and to **Heikki Kosola**, the Finnish author of TurboRaketti II (1992).

**Play online:** https://aweussom.github.io/ypilot/

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

## Kontroller

| Handling | Spiller 1 | Spiller 2 |
|---|---|---|
| Gass     | `W`       | `↑`       |
| Rotér venstre | `A`  | `←`       |
| Rotér høyre   | `D`  | `→`       |
| Fyr      | `Mellomrom` | `Enter` |
| Skjold   | `S` | `↓`    |

`R` = ny runde etter game over. Slå «AI P2» av for 2 mennesker; på for solo mot bots.

## Status

- **Fase 1 (MVP):** newtonsk fysikk, wrap, skyting, kollisjoner, jeteksos, score. ✅
- **Fase 2 (i praksis ferdig):** ekte `.map`-lasting, AI-bots + free-for-all m/
  takeover, drivstoff + fylling, liv/game-over, skjold, justerbar gravitasjon (også
  på skudd → buet skuddbane), landing, stor-kart single-player (scroll-kamera +
  minimap), newbie-modus (auto-skjold), bot-pathfinding, dødelig vegg-fart + blast-push,
  organisk veggrendering (mycel/baking) + parallax-starfield. ✅ Gjenstår: lyd, radar,
  wormholes.
- **Fase 3:** TurboRaketti II-lag — kart, ekstra våpen, og «push-off»/fraspark fra flater.

## Stack

Phaser 4 (CDN, ingen bundler), ren JavaScript i én `game.js`. Se `CLAUDE.md` for
arkitektur og `XPILOT-JAVASCRIPT-PLAN.md` for full plan.
