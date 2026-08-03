---
name: audio-fra-solstice
description: "YPilot Fase 2-lyd — gjenbruk Solstice sin lydmotor (\"soundbed\" + SFX), lag noen nye lyder"
metadata: 
  node_type: memory
  type: project
  originSessionId: 9dd7ad4a-f8a4-4ab2-9c5d-447df56f4bae
---

For YPilot-lyd (Fase 2) skal vi **gjenbruke lydmotoren fra Solstice-prosjektet**
i stedet for å skrive en ny fra bunnen. Solstice har en «soundbed» + lydeffekt-
system (Web Audio) som passer dette spillet også. Engine ligger i
`C:\devel\aweussom\javascript\solstice\` (se `audio-spike/audio-system.js` og
`index.html`). Uttalt 2026-06-08.

Nye lyder som må lages oppå soundbed-en:
- X antall skudd-lyder (varierte)
- shield on / shield off
- rakett-motor (thrust) — sannsynligvis loop med fade in/out
- (sannsynligvis også: eksplosjon, fuel-pickup, vegg-bounce — bekreft med bruker)

Dette forfiner planens §Lyd (som sa «Web Audio API direkte, alt generert»):
fortsatt Web Audio, men gjenbruk Solstice-motoren. Bygges i Fase 2, ikke Fase 1.
Se [[neon-look-solstice]].
