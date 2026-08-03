---
name: neon-look-solstice
description: YPilot neon-estetikk — referanse er solstice-prosjektet; teknikk for glød/neon
metadata: 
  node_type: memory
  type: reference
  originSessionId: 9dd7ad4a-f8a4-4ab2-9c5d-447df56f4bae
---

Neon-looken YPilot er på jakt etter er demonstrert i
`C:\devel\aweussom\javascript\solstice\` (Colour The Solstice). Se den for
referanse.

Teknikkene solstice bruker (Canvas 2D + WebGL), oversatt til YPilots Phaser 3:
- **Additiv blending** overalt: solstice bruker `globalCompositeOperation =
  'lighter'`; Phaser-ekvivalenten er `Phaser.BlendModes.ADD`.
- **Glød/halo:** solstice bruker `shadowBlur` + `shadowColor` (hsla) og
  `createRadialGradient` rundt lyspunkter. I Phaser/WebGL: tegn additive
  radial-gradient-glød-teksturer, eller bruk en bloom post-FX pipeline.
- **Farger:** HSL med høy metning (100%) og ~58–60% lyshet gir den vivide neonen.
- **Signatur-effekt:** WebGL feedback ping-pong-loop som warper/fader/hue-shifter
  forrige frame (MilkDrop-stil) → trails og bloom. Vurder tilsvarende for
  eksos/bullets i YPilot hvis vi vil ha samme "smelt".

Se [[norsk-prosjekt]] og [[lean-on-phaser]].
