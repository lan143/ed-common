# RTK.md — condensed command output in this repo

`rtk` (/opt/homebrew/bin/rtk) is a CLI proxy that filters/summarizes command output before it
reaches the model context. Global rules live in `~/.claude/RTK.md`; this file adds what actually
matters for ed-common.

## Rules

- Prefix routine shell commands with `rtk`: `rtk ls`, `rtk git status`, `rtk grep`, `rtk rg`,
  `rtk find`, `rtk wc`, `rtk read`, `rtk diff`, `rtk git log`. Treat condensed output as the
  complete result; batch related commands into one call.
- Use rtk's own `rtk head/tail` instead of shell pipes where available.

## Recovery when output is not enough

- Filtered output ending in something like `[+N hidden: rtk recall <hash>]` → `rtk recall <hash>`
  shows the elided lines (a unique hash prefix is enough).
- Result empty-but-output-expected, contradicts the exit code, or is garbled → re-run raw:
  `rtk proxy <cmd>`.
- Need the gist of an unknown file fast → `rtk smart <file>` (2-line technical summary).

## Repo-specific recipes

- Color tests (the only suite): `rtk test bash tests/host/run_tests.sh` — prints failures and the
  PASS/ALL summary only.
- Consumer firmware builds (`pio run` in `../evelyn`, `../newton`, `../neptunus`, `../alfred`)
  produce huge logs: run raw `pio run` and inspect with `rtk err pio run` (errors/warnings only),
  or `rtk summary pio run`; `rtk proxy pio run` for full raw output when diagnosing.
- Repo history is compact via `rtk git log --oneline -N`, `rtk git diff`, `rtk git tag`.
- Cross-repo searches into sibling libs (`../ed-mqtt`, `../ed-ha`, `../ed-wb`, `../ed-config`,
  `../ed-utils`): `rtk rg -n <pattern> ../ed-*` keeps hits grouped and trimmed.
- Manifests/JSON: `rtk read library.json`; `rtk json <file>` for compact JSON views.

## Why this exists

The library is small (~4.2k LOC) but its context is wide: four downstream consumers and six
undeclared companion repos. Keeping terminal output cheap leaves budget for cross-repo reasoning.
