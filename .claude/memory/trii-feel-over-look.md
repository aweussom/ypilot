---
name: trii-feel-over-look
description: "TRII-kart-konvertering sikter på FEEL (topologi + tyngdekraft-flyt), ikke pixel-LOOK"
metadata: 
  node_type: memory
  type: project
  originSessionId: 3148be7d-39c2-4d9c-ad9f-2bb1f47873b7
---

Besluttet 2026-06-09: TurboRaketti II-kartene (PNG-er i `trii-maps/`, rendret fra
RetroRocket-homebrewen) kan **aldri** fanges pixel-troverdig i YPilots tile-grid — og
det er ikke poenget. TRII-kart handlet om **FEEL, ikke LOOK**, og feel-en er overførbar.

**TRII-feel = disse pilarene** (alle uttrykkbare i vår motor):
1. Tyngdekraft-grotteflukt (lunar-lander i hule) — gravitasjon ✅ + tunet (tak 0.10).
2. Skyt opp fra plattform/base (hvile → bevisst letting) — «spawn-på-plattform», ikke gjort.
3. Tre nålen: tunneler → kammer → chokepoints — = kart-LAYOUTET, lar seg blokk-konvertere.
4. Lande på hyller, dødelig gulv — vegg=død ✅; hazard-soner (slim/lava) senere.
5. Myk kurve-glid langs hule-vegg — krever **solide 45°-skråninger** (`q/w/a/s`).

**Konsekvens:** konvertering til XPilot-format ER riktig vehikkel — behandle den som
«fang hule-topologi + tyngdekraft-flyt», IKKE «reproduser bildet». Terskle PNG til
solid/åpen på tile-oppløsning, oppdag baser/fuel via fargenøkkel, spawns på plattformer.
Organisk LOOK utsettes til prettification (marching-squares-konturer fra grid-et — jf.
[[levende-kart]]).

**To feel-muliggjørere (ikke bare pynt):** solide skråninger (mykner blokk-huler) og
spawn-på-plattform. «AI-følere blinde for skråninger»-bugen er egentlig en føle-ting.

Se [[kollisjons-folelse]], [[gravitasjon-tuning]], [[levende-kart]].
