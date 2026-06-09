# memory/

Delt øyeblikksbilde av Claude Code sine **session-minner** for YPilot-prosjektet.

Disse filene speiler Claudes lokale minne-katalog
(`~/.claude/projects/<dette prosjektet>/memory/`) og er sjekket inn her slik at
**flere Claude-instanser (og maskiner) deler samme kontekst** via `git pull`.

## Innhold

- `MEMORY.md` — indeksen (én linje per minne, lastes inn ved sesjonsstart).
- `*.md` — ett minne per fil, med frontmatter (`name`, `description`, `metadata.type`).
  Body følger formatet beskrevet i prosjektets minne-konvensjon; `[[navn]]` lenker
  mellom minner.

## Synk og forbehold

- Dette er et **øyeblikksbilde**, ikke en levende kobling. Claude på denne maskinen
  re-kopierer hit og committer ved behov når et minne opprettes/oppdateres.
- Utvikling skjer mest på RTX5090-desktopen, som kan ha **flere minner** (f.eks. de
  design-beslutnings-minnene `CLAUDE.md` viser til, som `audio-fra-solstice`) som
  ennå ikke er kopiert hit. Legg dem inn på samme måte når du er på den maskinen.
- Minner gjenspeiler det som var sant da de ble skrevet — verifiser mot koden før
  du handler på dem.
