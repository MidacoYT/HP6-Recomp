# Rapport de reconstruction HP6 — 22 octobre 2026

## Provenance

La reconstruction utilise deux images vérifiées par SHA-256 :

| Image locale | SHA-256 | Usage |
| --- | --- | --- |
| `D:\HP\hp6.exe` | `4653e4301ed6dfc271409da2443fd817a65b6514704a3487e86895337e487954` | Original protégé par SecuROM ; données et tables |
| `D:\HP\hp6ss\build\sdk-lab\pc\hp6.exe` | `41cbb7ad4b08f6edeb5c2a06aea037d1c0634e9b71df093cc9a26b2287916e10` | Code désassemblable sans SecuROM |

Les corps reconstruits sont justifiés par les instructions de l'image sans
SecuROM. Leur identité octet pour octet avec la section de code protégée n'est
pas démontrée. Aucun de ces exécutables n'est inclus dans le dépôt.

## État du démarrage natif

Le vrai point d'entrée et une partie du chemin de démarrage, des services
mémoire, des tâches, de la sélection de langue et du rendu ont été intégrés.
Les neuf entrées de la vtable du service à `0x0083E55C` sont relocalisées ; les
trois méthodes d'allocation appellent les fonctions internes encore absentes
au lieu d'en simuler le résultat. Le registre des noms de contexte à
`0x005DB380` et le callback de croissance à `0x005DB640` proviennent des
instructions vérifiées. Sept handlers courts de commandes sont reconstruits,
dont `OnLanguageMenuSelectItem` à `0x0048C950`.

Le programme `hp6.exe` **ne se lie pas encore**. L'édition de liens du
22 octobre 2026 signale ces sept références externes :

| Symbole | Travail requis |
| --- | --- |
| `service_heap_allocate_core_005dbc50` | Allocation nommée, contexte et alignement |
| `service_heap_allocate_core_005dde30` | Allocation dans les listes de blocs du tas |
| `service_secondary_heap_expand_005dbbb0` | Agrandissement du tas principal |
| `service_heap_allocate_00ff9850` | Ajout et initialisation d'une région mémoire |
| `render_overlay_005dd020` | Rendu appelé par la porte du moteur |
| `PTR_PTR_00876ab0` | Table de 93 commandes, 86 handlers distincts |
| `PTR_PTR_00870948` | Table de 26 tâches d'écran, 24 activations distinctes |

Le nombre sept est un nombre de **symboles de liaison** ; il ne mesure pas le
nombre de fonctions nécessaires derrière les deux tables. Les tables ne sont
pas remplacées par des pointeurs factices. Le menu de langue du véritable
exécutable reconstruit n'a donc pas encore été affiché.

## Vérifications exécutées

- `python tools/audit_remaining_symbols.py` : mots des vtables et tables
  identiques dans les deux PE vérifiés.
- `python tools/audit_native_dispatch_tables.py D:\HP\hp6.exe D:\HP\hp6ss\build\sdk-lab\pc\hp6.exe` : 93 commandes et 26 tâches vérifiées.
- `cmake --build build --config Debug --target ALL_BUILD` : bibliothèque et
  programmes de test compilés.
- `ctest --test-dir build -C Debug --output-on-failure --timeout 20` : **74/74**
  tests réussis, dont le registre de contexte et le callback de croissance.
- `cmake --build build --config Debug --target hp6` : échec attendu avec les
  sept références externes listées ci-dessus.

Les tests isolés emploient parfois des doubles explicites pour un callee
encore absent. Leur succès ne prouve pas que le démarrage ou le rendu complet
fonctionne.

## Suite de la reconstruction

1. Reconstituer `0x005DBBB0` et suivre ses appels réels vers les allocateurs
   du tas, avec les conventions d'appel et les données globales vérifiées.
2. Terminer les trois autres fonctions mémoire et vérifier des allocations
   réelles dans une exécution native.
3. Reconstituer les handlers et activations des deux tables, puis le rendu
   `0x005DD020` ; conserver les dépendances inconnues comme symboles explicites.
4. Lier et lancer `hp6.exe`, puis comparer l'écran de sélection avec la capture
   du jeu original, y compris le curseur lumineux.

Le suivi détaillé et les adresses se trouvent dans `native_executable.md`,
`presentation_commands.csv` et `screen_task_table.csv`.
