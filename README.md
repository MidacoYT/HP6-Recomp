# HP6 reconstructed

Reconstruction progressive de *Harry Potter and the Half-Blood Prince* (PC)
depuis les exports Ghidra. Les sources sont organisées par responsabilité
observée, sans reprendre l'arborescence du jeu.

## État

Le journal suit actuellement 263 fonctions : 255 intégrées dans `src/math`,
`src/stream`, `src/projection`, `src/platform`, `src/runtime` et `src/archive`
(conteneurs BIGF, recherche d'entrée, verrouillage/attente, pool de services,
service de handles, cycle de vie des records, amorcage du moteur de
ressources et roue de minuteries), et 9 différées. Le 16
oct 2026, la limite d'utilisation du LLM de ReAgent a bloqué la reconstruction
automatique jusqu'au 17 oct 01:19 UTC ; les lots 026 à 030 ont été
intégrés depuis les exports Ghidra et l'assembleur exporté pendant cette
fenêtre. Les lots 031, 032b et 033 ont produit des candidats ReAgent, revus et
corrigés contre les instructions x86 ; le lot 038 (service_cleanup), terminé
le 17 oct, clôt la série (retour bool de `006dd00c` confirmé par l'assembleur
exporté) ; voir `architecture/decisions.md`.
`006b9774` et `006b9386` sont suivis comme imports CRT de `tolower` et
`strchr`, utilisés dans `src/archive`.
`006b92c8` est suivi comme fonction CRT MSVC `__onexit`, appelée via
`std::atexit` dans le module de handles.
Un `PASS` ReAgent n'etablit ni la compilation ni l'equivalence binaire.

`005d1580` compile avec une reference externe a `DAT_007db234` ; sa valeur
reste inconnue. Les fonctions retournant `float10` utilisent `long double`
dans le code reconstruit. Sous MSVC, ce type n'a pas la precision x87 80 bits.
L'ABI et l'equivalence numerique restent donc a verifier sur une cible x86
appropriee.

`006b938c` est identifie comme `sprintf` (CRT standard) ; il est declare
comme dependance externe et n'est pas reconstruit.

Le module `src/projection` rassemble la projection de champs, la classification
des codes, l'aggregation de listes chainees par offsets relatifs, le calcul des
tailles, la conversion inverse de plusieurs variantes et les constructeurs
d'enregistrements (appel de callback global, encodage "EAGL%d" via sprintf).
La table de 256 octets a `0x0071b150`, absente de `_globals.json`, a ete lue
dans le projet Ghidra et conservee dans `architecture/projection_format_table.bin`.
Le fichier `src/projection/FormatTable.inc` contient ces octets pour la compilation.
`tools/read_ghidra_bytes.py` relit cette table selon `ghidra-bridge.yaml` ;
`tools/embed_projection_table.py` regenere l'initialiseur C++ a partir du
fichier binaire conservé. Dix-sept tests CTest couvrent les modules intégrés.

## Pipeline

```powershell
.\.venv\Scripts\python.exe tools\build_map.py
.\.venv\Scripts\python.exe tools\orchestrate.py --batch vector_core_001 `
  --address 00401020 --address 005d1580 --address 005d1680 `
  --address 005d16f0 --address 005d1770
cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

`architecture/function_map.jsonl` contient les signatures, appels, globals et
chaines par fonction ; `architecture/call_edges.tsv` contient les dependances.
`reports/pipeline/state.json` distingue la candidate ReAgent, l'integration,
la compilation et l'equivalence. L'orchestrateur ne traite que 5 a 10 adresses
par lot et saute les candidates deja enregistrees lors d'une reprise.
`tools/select_batch.py` propose des groupes partageant un appelé, a examiner
avant de les passer a l'orchestrateur. `tools/record_integration.py` enregistre
les decisions et regenere `architecture/address_map.csv`.

Le test `core_smoke` est un test de comportement local des interfaces C++ ;
il ne compare pas le code au jeu original.
