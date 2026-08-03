---
name: kollisjons-folelse
description: YPilot kjerne-følelse — XPilot skjold-bounce (vinkel/fart-avhengig) og Turboraketti rakett-push-off-alt
metadata: 
  node_type: memory
  type: project
  originSessionId: 9dd7ad4a-f8a4-4ab2-9c5d-447df56f4bae
---

To kollisjons-quirks som er kjernen i hvordan YPilot skal føles (uttalt
2026-06-08):

- **XPilot — bounce med skjold på:** med skjold oppe spretter skipet av vegger
  ved visse vinkler/farter (grunne treff) i stedet for å eksplodere. Bratte/harde
  treff koster mer energi eller dreper. Vinkel-/fart-avhengig, ikke binær
  død/bounce — derfor er vegg-kollisjon håndlaget (sirkel-mot-linje), se
  [[lean-on-phaser]].
- **Turboraketti — rakett dytter fra nesten alt:** å dytte/skyte mot flater gir
  fremdrift og føles nesten som et skjold i seg selv. Signatur-følelse fra
  Turboraketti; hører til Fase 3-laget, men kollisjons-koden må designes så den
  ikke stenger for det.

**Hvorfor det betyr noe:** dette er nettopp spill-responsen brukeren vil bruke
tid på å finjustere. Behandle det som kjerne-mekanikk, ikke pynt.

**Konkrete referanseverdier fra XPilot-kilden** (`kekyo/xpilot-ng`,
`lib/defaults.txt`) — bruk som utgangspunkt for justeringsknappene i Fase 2:
- `maxShieldedWallBounceSpeed` / `maxUnshieldedWallBounceSpeed` — fart-terskel for
  om et vegg-treff spretter eller dreper (med/uten skjold).
- `maxShieldedPlayerWallBounceAngle` / `maxUnshieldedPlayerWallBounceAngle` —
  vinkel-terskel for samme.
Bekreftet action-sett i `src/common/keys.h`: KEY_TURN_LEFT/RIGHT, KEY_THRUST,
KEY_SHIELD, KEY_FIRE_SHOT (det femte er shield).

**Rakettstrålen som våpen** (framtid, uttalt 2026-06-08 — kjerne-Turboraketti-følelse):
- Rakettstrålen (eksosen) skal gjøre **skade** på motstanderens skip når den treffer.
- Strålen **dytter** motstanderen (påfører kraft), og gir samtidig **«skyv-fra»** for
  eget skip — MER skyv-fra om motstanderen har **skjold på** (større flate å dytte mot).
- Klassisk taktikk: treff f.eks. «venstre vinge» på motstanderen → motstanderen
  roterer uventet (torque fra off-center treff) → snuten treffer noe hardt → boom.
- Konsekvens for design: strålen må påføre både skade, lineær dytt OG rotasjon
  (torque ut fra treffpunkt vs. massesenter). Henger sammen med Turboraketti
  rakett-push-off-alt over og [[gravitasjon-tuning]] (strålestyrke vs. gravitasjon).

**Skjold (IMPLEMENTERT v1, 2026-06-09):** hold-tast (P1 `S` / P2 `↓`), dreneres av
drivstoff (`PHYSICS.shieldDrain`), krever fuel. Mens oppe: absorberer kuler, gjør deg
uskadelig (`die()` guardes på `shielded`), blokkerer EGEN skyting, og spretter deg ut av
vegger i stedet for å dø (`PHYSICS.shieldBounce`). Blast-push gir ×2 dytt mot skjoldet
(hooket fra før). Quirk-refinement (vinkel/fart-avhengig grunn-sprett vs. hard stopp) +
bullet-REFLEKSJON (i stedet for bare absorpsjon) gjenstår.

**Blast-push (IMPLEMENTERT, grunnversjon — uttalt 2026-06-08):** når et skip
eksploderer dyttes nærliggende levende skip radielt vekk (impuls avtar med avstand,
`PHYSICS.blastRadius`/`blastForce` i game.js). Skjold-multiplikator (MER dytt om
motstander har skjold på i øyeblikket) er lagt inn som hook (`other.shielded ? 2 : 1`)
og aktiveres når skjold-mekanikken kommer.

**Vegger «channel»-er eksplosjoner (FRAMTID — vindtunnel-effekt):** kartveggene skal
forme/lede eksplosjoner fra skip (og framtidige våpen) langs korridorer i stedet for
ren radiell falloff — en slags vindtunnel. Kompleks; krever at blast-utbredelsen tar
hensyn til geometri (raycast/flow langs åpne ruter). Ikke bygd ennå.

**Eksplosjons-biter med fysikk + skade (FRAMTID/polish — uttalt 2026-06-09):** når et skip
eksploderer i biter, skal bitene følge fysikken (fly utover med fart, evt. bounce) OG gjøre
**skade likt 1× kule** om de treffer et motstander-skip. Dvs. ekte debris-fragmenter, ikke
bare partikkel-pynt. Kobler til takeover-cinematic (se [[ai-spiller]]): egne skip-biter kan
føyke ut av skjermen og konvergere på AI-skipet man tar over, ELLER samles til et
heat-seeking missile som «overtar» nærmeste AI-skip.
