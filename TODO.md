# YPilot — ToDo

Bevisst utsatte oppgaver (parkert, ikke glemt).

## Wormholes (`@` / `(` / `)`)

XPilot-wormholes — kjernemekanikk flere kart er avhengige av. `(` = inngang,
`)` = utgang, `@` = begge. Et skip som treffer en inngang teleporteres til en
**tilfeldig** utgang (fart bevart, kort cooldown så man ikke re-trigger ved utgangen).

**Hvorfor:** flere kart har baser i lukkede 1-celle-rør med en `(` på toppen — man
skal akselerere opp i wormholen og teleporteres ut i arenaen. Uten dette blir basene
uunnslippelige feller (f.eks. `arena2` «Arena» 100×100, med `(`×10 `)`×10 `@`×2).
Diagnostisert 2026-06-09.

**Plan:**
- Konverter (`retrorocket/xpilot_tools.py` + `convert_to_json.py`): trekk ut
  `wormholes: [{col,row,type:'in'|'out'|'both'}]`; cellene forblir åpne.
- Motor (`game.js`): i `Ship.update`, treff på inn/`@`-celle → teleportér til en
  tilfeldig ut/`@`-celle, behold fart, sett kort cooldown.
- Konsekvens: kart der baser kun nås via wormhole (arena2) blir spillbare.

## Andre parkerte ting

- **Organisk kart-rendering** — marching-squares-konturer + Chaikin-glatting så
  blokk-trappene blir flytende grottekanter (neste prettifiserings-iterasjon).
- **Edge-bounce** — for lukkede kart med ufullstendig vegg-kant (motoren wrapper
  alltid i dag). Unødvendig for kart med 100 % solid ramme (Ekolos).
