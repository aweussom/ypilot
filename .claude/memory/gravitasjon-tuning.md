---
name: gravitasjon-tuning
description: "YPilot gravitasjon — justerbar slider (0–0.50, persistert), med tuning-invariant ift. rakettstrålen"
metadata: 
  node_type: memory
  type: project
  originSessionId: 9dd7ad4a-f8a4-4ab2-9c5d-447df56f4bae
---

Gravitasjon i YPilot (implementert i Fase 1):
- Justerbar **DOM-slider**, område **0–0.50**, steg 0.005, persistert i
  localStorage-nøkkel `ypilot.gravity` (huskes mellom sesjoner). `clampGravity()`
  i `game.js`, anvendt i `Ship.update` (`vy += GRAVITY * dtScale`).
- **Absolutt tak 0.50** — over dette er uaktuelt (uttalt 2026-06-08).
- **Tuning-invariant (ikke hard-klampet):** gravitasjonen må ALDRI være høyere enn
  at rakettstrålen greit kan stoppe skipet, selv i høy fart, innen X px. Balanseres
  senere via gravitasjon OG/ELLER rakettstrålens styrke. Ved dagens `thrustForce`
  (0.18) er det praktiske spillbare området derfor lavt; slideren tillater likevel
  utforsking opp til taket.

**Svart hull / attractors + gravity-assist (FRAMTID — uttalt 2026-06-08):** lokale
attractors (svarte hull) som påvirker gravitasjonen i et BEGRENSET område. Flyr man et
spesifikt mønster rundt dem kan man få **gravity-assist / slingshot** → uventet stor
fart. Mest aktuelt på STORE baner. Effekten trenger ikke være kjempestor. Bygger på
XPilot gravity-tiles (`+`/`-`/`>`/`<`/`i`/`m`/`k`/`j`, se [[xpilot-kartformat]]) og
`gravitypoint`-CoG — punkt-gravitasjon med inverse-square/avstandsavhengig kraft i en
radius (Phaser-partiklenes `GravityWell` har samme inverse-square-mønster å stjele fra).

Henger sammen med [[kollisjons-folelse]] (rakettstrålen som dytt/våpen) og
[[sky-lagring-tabtabtab]] (senere sky-lagring av innstillinger).
