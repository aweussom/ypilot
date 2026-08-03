---
name: ai-spiller
description: "YPilot Fase 2 — AI-spiller via inputProvider; enriching BESLUTTET = runtime Chrome Gemini Nano (on-device, dev.to)"
metadata: 
  node_type: memory
  type: project
  originSessionId: 9dd7ad4a-f8a4-4ab2-9c5d-447df56f4bae
---

Fase 2-gameplay-infra (prioritert):

- **Start-punkter:** gjenbruk kart-baser (`_` + lag-baser `0`–`9`, se
  [[xpilot-kartformat]]) som spawns i `parseMap`; loop skip over `spawns[i % n]`.
- **AI-spiller (veldig fort):** brukeren blir fort lei av å styre to skip alene.
  To trinn: (1) **heuristisk AI raskt** (styr mot/rundt motstander, unngå vegger via
  `tileAt`-lookahead, skyt når siktet) via et **`inputProvider`-sømpunkt** i `Ship`
  (returnerer `{thrust,left,right,fire,shield}` — tastatur og AI deler grensesnitt);
  så (2) **smartere AI** drevet av enriched kartdata.

**BESLUTTET 2026-06-08 — enriching = on-device Chrome Gemini Nano**
(`window.LanguageModel`), IKKE offline subagent-prerender (ombestemt). Enricher hvert
kart ved lasting til AI-nav-metadata (choke-points, sniping-steder, waypoints, soner),
caches i localStorage (nøkkel = navn + innholds-hash), brukes av AIController. Gjenbruk
tabtabtab-mønster `..\tabtabtab\enrich-ondevice.js` (system-prompt, `session.clone()`,
robust JSON-parsing m/ salvage). **Graceful fallback** til heuristikk hvis Nano mangler.
Motivasjon inkl. en **dev.to-artikkel** om lokal Gemini Nano til map-enriching.
Referanse-pipeline: `C:\devel\q-free\geomap-united-nations\sweden\llm-benchmark`.

**Bot-multiplayer + takeover (IMPLEMENTERT 2026-06-08):**
- Free-for-all, `GAME.shipCount` skip (default 6) på små baner. `humanCount` mennesker
  (AI-toggle PÅ = 1, AV = `GAME.humans`=2), resten bots (`makeAIProvider`, sikter på
  NÆRMESTE levende motstander). 3 liv → eliminert. Siste skip i live vinner.
- **Takeover (Counter-Strike-stil):** når et menneske blir eliminert, hopper det inn i
  boten som gjør det DÅRLIGST (færrest kills, så færrest liv). Ingen poeng. Input-sømpunktet
  (`this.humanKeys[hi]`) gjør byttet rent; boten får label = spillerens. `humanShips[]`
  sporer hvilket skip hvert menneske styrer nå; HUD bindes til det.
- **Free shield mot spawn-camping:** `PHYSICS.spawnInvuln` (~1.5s, justerbar) usårbarhet
  ved hver spawn OG ved takeover.
- **Skip-former (IMPLEMENTERT):** menneske = Starfighter (pil, tydelig retning), bot =
  TIE-fighter; mennesker beholder cyan/magenta. Ved takeover MORFER boten til Starfighter +
  spillerens farge, FRYSER ~1s med nedtelling + pulserende highlight (`PHYSICS.takeoverPause`)
  → tydelig hvilket skip som ble deg. Gjenstår (polish, uttalt 2026-06-09): mitt SISTE skip
  eksploderer → bitene føyker UT av skjermen → konvergerer på AI-skipet jeg tar over.
  Alternativ å teste: bitene samles til et **heat-seeking missile** som tar over nærmeste
  AI-skip. Bitene skal følge fysikk + gjøre skade (se [[kollisjons-folelse]] eksplosjons-biter).
- **AI-nødbrems (IMPLEMENTERT):** bot sjekker FARTSRETNINGEN mot vegg og retro-bremser →
  suicider ikke lenger på vegger (1v1 varer nå, ikke <1s). Veggunngåelse kan fortsatt
  forbedres (pathfinding), men grunnproblemet er løst.

Se [[lokal-vs-nettverk-kart]], [[sky-lagring-tabtabtab]], [[kart-navigator]], [[drivstoff-og-liv]].
