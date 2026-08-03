---
name: vaapen
description: "YPilot framtidige våpen — Turboraketti hadde i praksis 2: mitraljøse (m/ rekyl) + tung \"mine\"-sky"
metadata: 
  node_type: memory
  type: project
  originSessionId: 9dd7ad4a-f8a4-4ab2-9c5d-447df56f4bae
---

Våpen-design (framtidig versjon — uttalt 2026-06-08, må diskuteres nærmere):

- XPilot hadde trolig **pickups/oppgraderinger** av våpen — brukeren er usikker på om
  hen vil ha det i YPilot.
- I **Turboraketti** brukte de i praksis kun **TO** våpen:
  1. **Front-«mitraljøse»:** rapid-fire skudd, IKKE mye skade, men kunne ofte **dytte
     motstanderen i veggen**. Greit utgangspunkt for YPilot.
  2. **Tungt skudd / «mine»:** samme type «kuler» som frontskuddet, men en **mine** som
     gikk av og **spredte seg utover i en sky**. Senteret av mina var **4× skudd** —
     traff DE, var det **Game Over**. (Kraftig, sentrum = instakill.)

- **VIKTIG fysikk-regel:** **rekyl** påvirker skipet som skyter **like mye** som skuddene
  påvirker det som blir truffet. Newtons 3. lov — å fyre mitraljøsen dytter deg bakover.
  Dette er kjernen i kamp-følelsen (kobler til [[kollisjons-folelse]] blast-push og
  rakettstråle-som-våpen). Må implementeres når våpen bygges.

Nåværende YPilot: én enkel bullet-type uten rekyl. Mitraljøse-med-rekyl er det naturlige
grunnlaget; mine-skyet er andre-våpenet. Hører til Fase 3 / våpen-laget.
