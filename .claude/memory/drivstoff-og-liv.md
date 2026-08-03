---
name: drivstoff-og-liv
description: YPilot Fase 2 — drivstoff/fylling (XPilot hover+tast / landing på flat polygon) og liv/game-over
metadata: 
  node_type: memory
  type: project
  originSessionId: 9dd7ad4a-f8a4-4ab2-9c5d-447df56f4bae
---

Neste Fase 2-mekanikker (uttalt 2026-06-08):

**Drivstoff + fylling (XPilot-signatur):**
- XPilot hadde en spesiell fylle-metode: man måtte **«hoovre» nær fylte/fuel-polygoner**
  og trykke en tast for å fylle. Noen FÅ steder kunne man **lande på en fylt polygon**
  og fylle uten å hoovre.
- Sannsynlig enkel regel for landing: **flatt område på toppen av en polygon → landing
  mulig** (skip i ro, oppreist, på flat topp → fyller).
- `#` = fuel-stasjon i kart-legenden (se [[xpilot-kartformat]]) — må rendres + brukes.
- Implementasjon trenger: fuel-ressurs som brukes (gass forbruker drivstoff), HUD-måler,
  nærhets-fylling (hover + evt. tast eller auto), og landings-deteksjon (i ro på flat
  vegg-topp). Henger sammen med [[gravitasjon-tuning]] (tom for drivstoff → kan ikke
  motvirke gravitasjon → driver i veggen — klassisk XPilot-spenning).

**Liv / game-over:**
- X antall liv før spillet er over (foreslått **3**). Når en spiller er tom for liv →
  motstanderen vinner; vis game-over + restart.

**Besluttet 2026-06-08:**
- **Én ressurs «drivstoff»** som driver alt (gass + skyting, skjold senere). Tom tank =
  kan ikke gjøre noe → driver i veggen.
- **3 liv** før game over (motstander vinner, R for ny runde).
- **Auto-fylling** ved å hovre sakte (< REFUEL_SPEED) nær en `#`-stasjon (REFUEL_RANGE).
  Ingen ekstra tast. Landing på flat polygon-topp = bare en fysisk måte å hovre nær på.
Implementert i game.js: `PHYSICS.fuelMax/fuelThrust/fuelShot/fuelRefill/startLives`,
fuel-bar + liv (♥) i DOM-HUD, game-over-overlay.
