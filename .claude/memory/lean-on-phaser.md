---
name: lean-on-phaser
description: "YPilot dev philosophy — lean on Phaser 3 built-ins, only hand-roll what needs tuning"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 9dd7ad4a-f8a4-4ab2-9c5d-447df56f4bae
---

For YPilot, prefer Phaser 3's built-in systems (rendering, particles, scene
management, Arcade collision for ship/bullet) over reimplementing them. The user
would rather spend time fine-tuning game feel/response than re-inventing code
written by far more capable people.

**Why:** Stated explicitly 2026-06-08. It's about effort allocation — game feel
is where their time should go, not infrastructure.

**How to apply:** Default to Phaser APIs. The two deliberately hand-rolled bits
in the plan — Newtonian movement integration and circle-vs-line wall collision —
are NOT contradictions: they exist precisely because they are the tuning knobs
for game feel (Arcade's drag/friction model fights XPilot's no-friction vacuum
physics). Keep those manual; lean on Phaser for everything else. Confirm before
hand-rolling anything Phaser already provides well.
