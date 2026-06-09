# XPilot-fakta (kildemateriale for YPilot)

Samleside for XPilot-fakta vi vil huske mens vi bygger YPilot. Norsk gjennomgående
(prosjekt-konvensjon); den originale engelske beskrivelsen er sitert under som referanse.

## Hva vi alt har gjenskapt

- **«Real physics» — blås noen inn i en vegg = drep dem.** XPilot-beskrivelsen sier
  eksplisitt at eksos og sjokkbølger fra eksplosjoner påvirker deg, og at man kan drepe
  noen ved å blåse dem inn i en vegg. Vi har dette nå: blast-push i `die()` + dødelig
  vegg-fart (`wallLethalSpeed`). Se CLAUDE.md.
- **Drivstoff / docking ved fuel-stasjon.** `#`-celler → `fuelStations`, fylling ved
  hovring/landing nær dem.
- **Cannons / hevngjerrige robot-fightere gir deg en hard tid.** Vi har bots (FFA,
  takeover). Cannons (se under) er IKKE gjort ennå.

## Kanoner — i kart-legenden, men IKKE implementert ennå ⚠️

Kanonene jeg (Tommy) hadde glemt: de **er** markert på kartene. Fra tegn-legenden
(`kekyo/xpilot-ng` `src/common/xpmap.h`, gjengitt i CLAUDE.md):

| Tegn | Betydning            |
|------|----------------------|
| `r`  | kanon som skyter OPP |
| `d`  | kanon som skyter VENSTRE |
| `f`  | kanon som skyter HØYRE |
| `c`  | kanon som skyter NED |

I YPilot i dag: `parseMap`/`buildMapFromJson` henter KUN ut `fuelStations`. Kanon-celler
(og wormholes, treasure, targets, concentrators, gravitasjons-tiles, friksjons-soner,
checkpoints) blir bare vanlige tiles — ingen logikk. **Kanoner er en god neste mekanikk:**
stasjonære vegg-tårn som auto-skyter i fast retning mot skip i sikte. Passer rett inn i
`Bullet`-systemet og gir kartene tenner (XPilot: «Defend your home base»).

Andre uimplementerte spesial-tiles (parkert): wormholes (`@`/`(`/`)` — egen TODO-seksjon),
treasure (`*`/`^`), target (`!`), concentrators (`%`/`&`), gravitasjon (`+`/`-`/`>`/`<`/
`i`/`m`/`k`/`j`), friksjons-sone (`z`), checkpoints (`A`–`Z`).

## XPilot-våpen/forsvar (15+) — referanse for Fase 3-laget

Afterburners, cloaking devices, sensors, transporters, ekstra kanoner, miner og bomber,
rockets (smart/torpedo/nuclear), ECM, laser, ekstra tanker, autopilot. Jf. memory
`vaapen` + TurboRaketti II-ideene (tunge bakoverskudd, heatseekers).

## Spillmodi (XPilot) — mulig retning

- **Klassisk dogfight** — kun kanon, manøvrering + taktikk. (Nærmest dagens YPilot.)
- **Team** — samarbeid, stjel andre lags treasures (fly rundt med en ball i snor, à la
  Thrust), spreng målene deres.
- **All-out nuclear war** — velg blant 20+ våpen/forsvar.
- **Race** — gjennom løypa før motstanderne.

## Original beskrivelse (engelsk, sitert)

> **General Description**
> For historical interest, this is more or less the original description of XPilot.
> XPilot is a multi-player 2D space game. Some features are borrowed from classics like
> the Atari coin-ups Asteroids and Gravitar, and the home-computer games Thrust
> (Commodore 64) and Gravity Force (Commodore Amiga), but XPilot has many new aspects too.
> Highlights include:
>
> - True client/server based game; optimal speed for every player.
> - Meta server with up to date information about servers hosting games around the world.
> - A web of world-wide rating servers; compare your skills with pilots from all around the
>   world, and climb the ladder of the world-wide rating list.
> - 'Real physics'; particles of explosions and sparks from your engines all affect you if
>   you're hit by them. This makes it possible to kill someone by blowing them into a wall
>   with engine thrust or shock waves from explosions.
> - Specialized editors for editing ship-shapes and maps.
> - Game objective and gameplay adjustable through a number of options, specified on the
>   commandline, in special option files, or in the map files. Examples of modes of the game:
>   - classical dogfight; equipped with only your gun, you have to rely on your maneuvering
>     and tactical skills
>   - team; fight together, steal other teams's treasures (involves flying around with a ball
>     in a string, much like in Thrust) and blow up their targets (which are, no doubt,
>     heavily guarded)
>   - all out nuclear war; chose carefully between more than twenty weapon and defense systems
>     to stay alive and annihilate your enemies
>   - race; make it through the deadly course before your opponents
> - Adjustable gravity; adjustable by putting special attractors or deflectors in the world,
>   or by adjusting the global gravity in various ways.
> - Cannons and personalized and vengeful robot fighters give you a hard time.
> - Watch your energy, and remember to 'dock' with a fuel station to refuel before it's too
>   late.
> - Defend your home base, or terrorize and steal someone else's.
> - Equip your ship with the 15+ defense and weapon systems: afterburners, cloaking devices,
>   sensors, transporters, extra cannons, mines and bombs, rockets (smarts, torpedos and
>   nuclear), ECM, laser, extra tanks, autopilot etc.
> - To start playing, you need to connect to a server by using a client program called
>   xpilot. There are always servers running if you check with the meta server, but if you
>   for some reason do not want to join them, you'll have to start a server of your own
>   (see man-page xpilots(6)).
