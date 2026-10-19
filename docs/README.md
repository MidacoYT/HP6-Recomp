# Progress site (HP6-Recomp)

A small static site (vanilla HTML/CSS/JS, no dependencies) showing
decompilation/reconstruction progress, in the spirit of decomp.dev:
overall percentage, breakdown by module, and the full list of functions
with their status.

A treemap (also decomp.dev-inspired) shows all ~14,200 functions as
squares (size = byte size, colour = status): hovering or tapping a
square identifies it, clicking it highlights the matching row in the
table below. It's drawn on a single `<canvas>` (not one div per
function) to stay smooth at this scale, and respects the same
search/status filters as the table (non-matching squares fade out).

Two distinct percentages are shown: an overall **global** percentage
(integrated functions / ~14,200 functions across the whole binary,
cross-referencing this repo's Ghidra catalogue with the independent IDA
catalogue from the neighbouring `knowledge/hp6` project), and a
percentage across functions **reviewed so far** (the subset this
project has explicitly looked at, currently 466).

## Regenerating the data

The numbers come from `reports/pipeline/state.json` (the same source of
truth as `architecture/address_map.csv`), cross-referenced with
`../knowledge/hp6/functions.json` (the neighbouring project's full IDA
catalogue, not checked into this repo) for the global percentage. After
an integration session, rerun:

```
python tools/export_progress.py
```

This rewrites `docs/data/progress.json`. If the neighbouring IDA
catalogue is missing (e.g. a clean clone without the neighbour
project), the script automatically falls back to the reviewed-subset-only
output (as before). Nothing else needs regenerating.

## Viewing locally

```
cd docs
python -m http.server 8080
```

then open `http://localhost:8080/`.

## Publishing on GitHub Pages

Once the repo is pushed to `origin` (`main`), enable GitHub Pages under
Settings > Pages with source "Deploy from a branch", branch `main`,
folder `/docs`. No build step is needed: the folder is served as-is.
