---
name: phaser-versjon
description: YPilot bygges på Phaser 4.x (nyeste); lokal gitignorert checkout av Phaser-kilden brukes som referanse
metadata: 
  node_type: memory
  type: project
  originSessionId: 9dd7ad4a-f8a4-4ab2-9c5d-447df56f4bae
---

YPilot bygges på **Phaser 4.x** (CDN `phaser@4.1.0`), besluttet 2026-06-08.

**Beslutnings-historikk:** Vurderte først 3.x (gløden vi vil ha — Bloom/Glow/Shine,
postFX/preFX — finnes allerede i 3.60+, så 4 hadde ingen *must-have*). Brukeren
valgte likevel nyeste 4.x: prosjektet er ikke banebrytende, så v4-API fikses med
noen ekstra docs-oppslag, og de kjører Claude Code Max nettopp for å kunne det.

**Lokal referanse:** Phaser-kilden er shallow-klonet til `vendor/phaser/`
(gitignorert — kun for oppslag). Bruk den til å verifisere v4-API direkte mot ekte
kode, inkl. `skills/v3-to-v4-migration/SKILL.md`. IKKE commit `vendor/`.

**v4-fallgruver å verifisere mot kilden før bruk** (Phaser 4.1.0 ble sluppet
30. april 2026, etter modellens kunnskaps-cutoff):
- Koordinatsystem: påstått «GL-orientering, Y=0 nederst» — sjekk om dette gjelder
  verdens-/spill-koordinater eller kun renderer/tekstur internt FØR fysikken skrives.
- `setTintFill` fjernet → `setTint` + `setTintMode`.
- `roundPixels` default `false` i v4.
- `DynamicTexture`/`RenderTexture` krever nå `.render()`; sjekk om `generateTexture`
  er påvirket.
- Bloom/Shine/Circle FX er nå Actions på target; Gradient er eget GameObject.
Se [[lean-on-phaser]].
