---
name: kart-navigator
description: "YPilot Fase 2 — kart-navigator med terningkast-rating, score-sortering og preview"
metadata: 
  node_type: memory
  type: project
  originSessionId: 9dd7ad4a-f8a4-4ab2-9c5d-447df56f4bae
---

Fase 2-funksjon (når `.map`-lasting er på plass): en **kart-navigator / kart-velger**.
Uttalt 2026-06-08.

- **Terningkast-rating:** spilleren gir hvert kart et terningkast 1–6 (norsk
  anmelder-skala, 6 = best).
- **Sortering etter score:** kart sorteres etter rating. **Hard regel fra bruker:**
  «terningkast 1 kommer UNDER de som ikke har fått terningkast ennå» — dvs.
  rating-1-kart sorteres LAVERE enn ikke-vurderte kart. (Antatt full rekkefølge:
  6→2 synkende, så ikke-vurderte, så 1 nederst — BEKREFT plasseringen av 2–5 vs.
  ikke-vurderte med bruker; den literale regelen fastsetter bare «1 < uvurdert».)
- **Kart-preview:** thumbnail av kartet i velgeren — trivielt å lage ved å rendre
  tile-gridet i miniatyr (samme kontur-tegning som `renderMap`).
- **Persistens:** ratings/favoritter lagres (localStorage nå; Google senere, se
  [[sky-lagring-tabtabtab]]).

Se [[xpilot-kartformat]] (parseMap) og [[levende-kart]].
