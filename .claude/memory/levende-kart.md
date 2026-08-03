---
name: levende-kart
description: "YPilot fremtidsvisjon — last XPilot-maps, men la polygonene bli organiske/levende og kanskje \"gro\" under spill"
metadata: 
  node_type: memory
  type: project
  originSessionId: 9dd7ad4a-f8a4-4ab2-9c5d-447df56f4bae
---

Fremtidig retning for YPilot (uttalt 2026-06-08, markert "vi får sjå" — uavklart,
ikke en committet feature ennå):

- YPilot skal kunne **laste inn ekte XPilot-maps**, inkludert brukerens berømte
  "Calvin & Hobbes Dancing"-map. Kart-lasting via `parseMap()` er allerede i
  planen (Fase 2).
- **På sikt:** vegg-polygonene skal bli mer *levende* / organiske — i samme ånd
  som solstice ([[neon-look-solstice]]). Kanskje de skal "gro" under spillet.
  Solstice gjør tre-vekst frikoblet fra hendelser med kontinuerlig easing mot et
  vekst-mål (ingen diskrete hopp) — relevant referanse for hvordan "gro" kan
  føles.

**Hvordan anvende:** Bygg kart-parser/-renderer i Fase 2 på en måte som ikke
låser veggene til statisk geometri — hold døra åpen for animerte/voksende
polygoner senere. Ikke implementer det levende laget før brukeren bestemmer seg.

**Visuell overhaul (uttalt 2026-06-08 — ETTER at bots er ferdige):** gjør spillet
vakkert med Solstice-type grafikk ([[neon-look-solstice]]): fet bakgrunn, litt neon
her og der fra **supernovaer**, og **vegger rendret som Yggdrasil-treet** — levende,
pulserende, ikke helt rette linjer/kanter/vinkler lenger. **MEN behold de horisontale
landings-plattformene** (flate topper man kan lande på, se [[drivstoff-og-liv]]) —
de må forbli flate selv om resten av veggen blir organisk. Konsekvens: render-laget
kan «forskjønne»/forskyve veggkonturen organisk, men kollisjons-/landings-geometrien
må fortsatt ha flate horisontale topper der landing skal være mulig.

**GJORT (2026-06-09):** organisk vegg-KONTUR er på plass (marching-squares + Chaikin +
fBm-«vekst» + kamera-Glow + puls; «New»-look, justerbar i Tuning-panelet). Konturen er
fortsatt bare en omriss-strek.

**NESTE organiske lag (uttalt 2026-06-09):** **fyll vegg-INNSIDEN** med et organisk,
glødende nettverk — à la Yggdrasil, MEN **uten sentral stamme**. Mer som **mycel**
(sopp-hyfenes forgrenede rot-nettverk): distribuert, forgrenende vene-nett. Idé:
space-colonization/DLA/random-walk-vener seedet fra veggkantene, tynne ADD-glødelinjer,
deterministisk per kart, pre-rendret. Detaljert i `jpilot/TODO.md`. (Henter perf fra
vegg-baking-planen — også i TODO.md.)
