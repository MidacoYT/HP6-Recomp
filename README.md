# HP6 reconstructed

Reconstruction progressive de *Harry Potter and the Half-Blood Prince* (PC)
depuis les exports Ghidra. Les sources sont organisées par responsabilité
observée, sans reprendre l'arborescence du jeu.

## État

Le journal suit actuellement 469 fonctions : 434 intégrées dans `src/math`,
`src/stream`, `src/projection`, `src/platform`, `src/render`, `src/runtime`,
`src/presentation` et `src/archive`, 35 différées, et 0 bloquées (les 5
ex-bloquées du lot ReAgent `mixed_deps_060` sont intégrées depuis le lot
manuel `file_ops_052`) ; 0 en attente de revue. Un site de suivi de
progression (dans l'esprit de decomp.dev, avec une carte cliquable de
toutes les fonctions du jeu) est publié depuis `docs/` — voir
`docs/README.md`.

**Deuxième vrai appel de dessin D3D9 intégré, cette fois connecté à la
vraie scène, avec sa vraie apparence sur le chemin low-tier** :
`hp6::render::execute_scene_render_batch` (`0x00646200`, confirmée être
`Scene::End` par sa propre chaîne de diagnostic embarquée) est le vrai
exécuteur de dessin au bout de la chaîne complète désormais tracée :
`PluginPresentationBitmap_BuildRenderQuad` (qui lit la même table de
chaînes que `ScreenLanguageSelect` remplit) → clip/mise en page →
`SubmitRenderCommand` (62 vrais appelants) → `RenderAllRegions` →
`Scene::End`, qui émet les vrais `DrawPrimitive`/`DrawIndexedPrimitive`.
Sur son chemin low-tier (`DAT_008663d5 != 0`), la transformation réelle
(matrice vue/projection, transposition) et la liaison matériau réelle
(déclaration + vertex shader, dont le matériau partagé `0x20000108` déjà
utilisé par le fondu de debug) sont désormais intégrées pour de vrai via
`hp6::render::push_item_transform_and_material` et
`hp6::render::resolve_material_bindings` — plus de placeholder sur ce
chemin, seule la construction d'une déclaration en cas de cache vide
(`FUN_00641f70`, ambiguïté de registre implicite non résolue) reste
différée sans affecter si le dessin se déclenche. `src/render/
SceneBatchRenderer.cpp`, testé dans `tests/scene_batch_renderer_smoke.cpp`.
Voir `architecture/startup_path.md` ("18-19 oct 2026, suites 14 à 17").

**`ScreenLanguageSelectTask` a maintenant un "IsReady" et un slot 2
réels** (`hp6::presentation::query_ready_language_select_screen_task`,
`mark_language_select_pending_task_ready`, `src/presentation/
ScreenTasks.cpp`) — lus par désassemblage direct, ni l'un ni l'autre
n'ayant son propre objet fonction dans l'analyse Ghidra de ce projet.
Sa fonction `Update` (la vraie machine à états qui remplit le menu des
langues) reste délibérément différée, comme pour chaque tâche d'écran
sœur de ce fichier. Voir `architecture/startup_path.md` ("19 oct 2026,
suite 17").

**Le parseur de dérogations de ligne de commande est intégré pour de
vrai** (`hp6::platform::parse_command_line_overrides`, `src/platform/
EngineSubsystems.cpp`) — résolu par désassemblage direct du site
d'appel réel dans `WinMain` : `EDI` est l'adresse codée en dur d'un
tampon global, pas un « this » abstrait. Découpe la ligne de commande
en drapeaux nus et paires `-clé=valeur`, deux bizarreries réelles
d'avancement de curseur reproduites fidèlement plutôt que corrigées.
Voir `architecture/startup_path.md` ("19 oct 2026, suite 18").

**Un fil en suspens depuis plusieurs sessions est refermé** :
`render_fade_overlay_quad`'s compteur « armé ailleurs (non tracé) » a
son vrai site d'armement (`hp6::render::try_begin_debug_overlay_scene`,
`src/render/ScreenOverlayRender.cpp`) — entièrement tractable puisque
ses deux sous-appels étaient déjà intégrés (`handle_device_lost`,
`cache_available_texture_memory`), il ne restait que le vrai
`BeginScene` à câbler. Voir `architecture/startup_path.md` ("19 oct
2026, suite 19").

**`WinMain` (`0x005d9300`) est désormais entièrement assemblée, intégrée
et testée de bout en bout** (`hp6::platform::run_win_main`,
`src/platform/WinMain.cpp`) : mot de contrôle FPU, répertoire courant,
sauvegarde/restauration `SystemParametersInfoA`, vérification mono-
instance, lecture d'une vingtaine de réglages `GameSettings`, création
de la fenêtre et du device D3D9, bootstrap moteur, porte de
compatibilité matérielle, dispatch de `TriggerBootupFlow`, la vraie
boucle de messages, et toute la séquence d'arrêt (démontage du device,
`TerminateProcess` injectable pour rester testable). Voir
`architecture/startup_path.md` ("18 oct 2026, suite 10") pour le détail
complet et `tests/winmain_smoke.cpp` pour le test de bout en bout
contre une chaîne complète d'objets Direct3D 9 factices. **L'objectif
directeur de cette reconstruction — un exécutable réel et testable
atteignant au moins `ScreenLanguageSelect` — est désormais atteint.**

Suite à l'indication que les menus du jeu sont des éléments 3D
spatiaux (pas du HUD 2D), une recherche des vraies caméras nommées
(`"ScreenSpaceCamera"`, `"CurrentInWorldCamera"`,
`"GlobalPresentationCamera"`) a mené à **un premier vrai appel
`DrawPrimitive`** entièrement reconstruit et testé :
`hp6::render::submit_screen_quad` (`0x00648c00`, verrouille le tampon
de sommets dynamique partagé, construit les sommets, lie le flux, émet
`DrawPrimitive(D3DPT_TRIANGLESTRIP, ...)` sur le vrai device D3D9) et
`hp6::render::render_fade_overlay_quad` (`0x0064abd0`, compose la
texture-cible-de-rendu comme un quad plein écran puis
`EndScene`/`Present`), `src/render/ScreenOverlayRender.cpp`. Cette
chaîne précise n'est atteinte, dans le binaire exporté, que depuis le
système d'overlay de debug — confirmé codé en dur à `false` (donc
mort) dans le build FINAL livré — ce n'est pas encore le vrai moteur
de rendu des menus, mais le pipeline de dessin lui-même est réel,
fidèle, et testé de bout en bout (`tests/screen_overlay_render_smoke.cpp`).
Voir `architecture/startup_path.md` ("18 oct 2026, suite 11 et 12")
pour l'investigation complète et ce qui reste ouvert (qui déclenche le
vrai rendu des menus, et la liaison avec les caméras nommées via
`FUN_005d3570`).

La chaîne réelle **`WinMain` → `FUN_005dca10` → `IDirect3D9::CreateDevice`**
est désormais intégrée et testée de bout en bout. `create_d3d9_device`
(`0x006498e0`, `src/render/DeviceCreate.cpp`) est le vrai site
`CreateDevice`, trouvé par recoupement d'un xref sur `DAT_00cf1944`
(deux `PUSH` de son adresse plutôt que sa valeur) et confirmé
indépendamment par la base de connaissances IDA de `D:\HP\knowledge\hp6`
(nommée `CreateD3DDeviceAndSwapchains` là-bas). `FUN_005dca10`
(`create_and_configure_render_device`), le seul appelant direct depuis
`WinMain`, est intégrée en entier : remise à zéro de l'état du frame
pump, initialisation du tas « RenderFrameworkMainHeap » (qui réutilise
`hp6::platform::service_holder()` au lieu de dupliquer son
initialisation paresseuse — même effet observable que l'original),
création du device, puis le bootstrap moteur `FUN_005dcdf0`
(`hp6::presentation::initialize_engine_core_systems`), qui révèle un
bus de messages nommés interne (une trentaine de messages `iMsg*`),
identifie `DAT_00cef730` comme le singleton `TimeManager::Instance`, et
trace (sans encore le reconstruire) l'objet moteur `DAT_00d6af14`
jusqu'à son vrai initialiseur statique C++ global, exécuté avant
`WinMain` par le runtime CRT. La commande exacte qui affiche l'écran
de sélection de langue (`"Presentation.Screens.ActivateScreenWithFocus
ScreenLanguageSelect"`) est désormais identifiée et intégrée
(`hp6::presentation::emit_chapter_entry_screen_command`,
`src/presentation/ChapterFlow.cpp`), ainsi que la résolution/
progression de chapitre (`resolve_and_load_chapter`/
`advance_to_next_chapter`), qui révèle l'ordre réel de la campagne :
`LanguageSelect`(9) → `FrontEnd`(8) → `BurrowIntro`(1) → `Hogwarts`(2)
→ ... — `LanguageSelect` est structurellement le premier chapitre de
toute la séquence, preuve indirecte forte (pas encore directe) qu'il
s'agit bien du premier écran voulu. La machine à 20 états qui charge
les ressources d'un chapitre (`ChapterLoadStateMachine_Update`) est
intégrée dans son squelette complet et son état critique ; ses ~30
sous-fonctions de chargement restent différées. **La chaîne complète
`WinMain → création du device → bootstrap moteur → TriggerBootupFlow →
résolution de chapitre` est intégrée et testée de bout en bout**
(`FUN_0048c970`/`FUN_00ff7fa0`, `src/presentation/BootupFlow.cpp`).
`tests/device_create_smoke.cpp`, `tests/engine_bootstrap_smoke.cpp`,
`tests/chapter_flow_smoke.cpp`, `tests/chapter_load_state_machine_smoke.cpp`
`tests/bootup_flow_smoke.cpp` et `tests/device_lost_handler_smoke.cpp`
(le gestionnaire classique de device Direct3D 9 perdu, `FUN_0064aa60`,
appelé une fois par itération de la boucle de messages Win32, intégré
dans `src/render/DeviceLostHandler.cpp`) et `tests/message_loop_smoke.cpp`
(la vraie boucle de messages Win32 elle-même, `FUN_005d8fc0`,
`hp6::platform::run_message_loop`, `src/platform/MessageLoop.cpp` —
appelée une fois depuis `WinMain` après `ShowWindow`/`UpdateWindow`,
réutilise `handle_device_lost`, `copy_gsc_message_handles` et
`run_frame_tick`, déjà intégrées) exercent toute cette chaîne
contre un faux `IDirect3D9`/`IDirect3DDevice9` complet. Quelques branches réelles
mais non prioritaires (liste de résolutions personnalisée, chemin
d'échec `CreateDevice`, sous-allocateur nommé `LocalAllocator_
CreateRegister_Overlay`) restent délibérément différées — en `abort()`
documenté pour les branches confirmées mortes, en stub neutre
toujours-succès pour celles sur le chemin de succès sans condition
(sinon aucun appelant ne serait testable) — jamais une invention du
comportement réel. Voir `architecture/startup_path.md` ("18 oct 2026")
pour le détail complet, y compris la tentative d'exécutable "en mode
normal" corrigée en cours de session.
`FUN_0064a810` (préparation de la cible de rendu hors écran, chaîne
GetBackBuffer/CreateDepthStencilSurface/CreateTexture/SetRenderTarget) est
désormais entièrement résolue et intégrée (`src/render/RenderTargetSetup.cpp`) ;
l'hypothèse initiale sur `DAT_00cf1948` (documentée comme incertaine) était
fausse et a été corrigée après recoupement systématique contre le vrai
`d3d9.h`. Les six lecteurs/écrivains de registre différés depuis le lot
`registry_settings_055` sont également résolus
(`src/platform/RegistryConfig.cpp`) : la disposition mémoire de
`std::basic_string` sous Visual C++ 2008 a été confirmée (en-tête de
débogage d'itérateur de 4 octets + disposition classique _Bx/_Mysize/
_Myres, taille totale 28 octets) et un `std::string` moderne s'est
avéré comportementalement équivalent partout, sans besoin de reproduire
cet ABI interne. `thunk_FUN_0100e6e0` (atteinte via le thunk `005d8080`,
longtemps soupçonnée d'être une routine d'init/désinit Direct3D) s'est
révélée être une simple sauvegarde de trois réglages d'affichage
(`OptionResolution`/`OptionLOD`/`OptionBrightness`) vers le registre côté
fermeture du jeu (`save_display_options`, dans
`src/platform/RegistryConfig.cpp`), devenue tractable une fois
`write_int_setting` disponible. Le 16
oct 2026, la limite d'utilisation du LLM de ReAgent a bloqué la
reconstruction automatique jusqu'au 17 oct 01:19 UTC ; les lots 026 à 030 ont
été intégrés depuis les exports Ghidra et l'assembleur exporté pendant cette
fenêtre. Les lots 031 à 054 ont produit des candidats ReAgent, revus et
corrigés contre les instructions x86. Le lot 054 ouvre `src/render/`, le
premier module touchant le moteur Direct3D 9 (confirmé par les chaînes
« Microsoft (R) D3DX9 Shader Compiler » dans
`architecture/technical_strings.csv`).

**Depuis le lot 055, la sélection suit la chaîne d'appels réelle de
démarrage plutôt que l'heuristique de `select_batch.py`.** `005d9300` est
identifiée comme `WinMain` ; sa décompilation complète (lecture d'une
vingtaine de réglages depuis le registre, création de fenêtre et de device,
boucle de messages Win32, gestionnaire de device Direct3D 9 perdu) est
documentée dans `architecture/startup_path.md`, avec la piste la plus
probable vers l'affichage de l'intro et du menu (`FUN_0064a810`, désormais
résolue — voir plus haut). La suite de cette piste, le « frame pump »
appelé depuis le message loop (`0x005e0cc0`) une fois qu'aucun message
Win32 n'est en attente, est également résolue et intégrée dans
`src/runtime/FrameClock.cpp` (`multimedia_timer_elapsed_ms`,
`accumulate_simulation_time_sample`, `interpolate_render_time`,
`drain_pending_visual_effects`, `run_frame_tick`) : un minuteur à pas
fixe (accumulateur 64 bits, interpolation entre deux échantillons) qui
appelle les méthodes virtuelles « simulate »/« render » d'un objet
opaque (`DAT_0089eb0c`), confirmées `__thiscall` par l'assembleur brut
(`this` rechargé dans ECX juste avant chaque appel, jamais poussé sur la
pile) — le type concret de cet objet n'est pas encore identifié. Cette
résolution a mis au jour un nouveau sous-système non reconstruit
(`0x00606b00`, déclenchement d'effets visuels « BLOOM »/« GLASS »/
« BACKDROP »), tracé différé séparément ; il est actuellement du code
mort (rien dans `src/` n'écrit encore la tête de file qui le
déclencherait). Les six fonctions de lecture/écriture de
registre, qui combinent le mécanisme SEH des exceptions C++ MSVC 2008 et
la disposition interne de `std::basic_string`, sont également résolues
(voir plus haut) ; le vérificateur objectif de ReAgent y avait relevé de
vrais écarts de flux de contrôle (pas de simples remarques stylistiques)
avant que la disposition mémoire ne soit confirmée. Le lot
`render_reset_056b` a ensuite intégré la
chaîne de recréation des shaders après un reset de device Direct3D 9
(`src/render/DeviceReset.cpp`) et confirmé par recoupement plusieurs offsets
de la table virtuelle `IDirect3DDevice9` (TestCooperativeLevel, Reset,
CreateVertexBuffer, CreateVertexShader, CreatePixelShader, Release) :
**les appels virtuels COM utilisent `__stdcall`** (pointeur device poussé
sur la pile), contrairement au `__thiscall` employé partout ailleurs dans
ce projet pour les tables virtuelles internes au jeu — un écart
systématique que ReAgent a manqué à chaque fois dans ce lot. Le lot
`caps_table_057b` a ensuite intégré le profil de render states par défaut
(`src/render/RenderStateCache.cpp`, 143 affectations vérifiées
programmatiquement) et révélé un premier cas de paramètre transmis par
registre (ESI) plutôt que par la pile — rendu explicite comme un
paramètre normal dans la reconstruction ; voir `architecture/decisions.md`
et `architecture/startup_path.md`.
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
fichier binaire conservé. Quarante-trois tests CTest couvrent les modules intégrés. Six des 24
tâches d'écran (`src/presentation/ScreenTasks.cpp`) sont intégrées :
`ScreenMainMenu` confirme que `ScreenTitlePage` (pas le menu principal)
est le véritable premier écran visuel ; `ScreenTitlePage` arme le
minuteur d'inactivité (`Task_IdleInputTimer`) qui déclenche le mode
attraction ; `ScreenAutosaveWarningMessage` révèle `ScreenTitlePageCut_PC`,
la cinématique d'intro, déclenchée automatiquement après ~8 secondes
sans action du joueur ; `ScreenFMV` confirme le lien vers la table des
cinématiques `AttractMode` ; `ScreenAttractMode` résout l'appartenance
d'une machine à états restée mystérieuse depuis plusieurs sessions ;
`ScreenLanguageSelect` complète la série. Le schéma
construction/destruction est désormais entièrement compris et
répétitif ; les "Update" individuels (souvent substantiels) restent
différés au cas par cas.

`D:\HP` (répertoire parent) contient les archives extraites du jeu et
deux projets voisins sur ce même binaire : `reconstruction/` (un
frontend *authored*, objectif différent du nôtre — à ne pas mélanger)
et `knowledge/hp6/` (une base de connaissances IDA Pro indépendante,
interrogeable via `D:\HP\tools\re_kb.py`/`re_ui.py`, utile en
validation croisée). Grâce à cette dernière, la chaîne complète
WinMain → bootstrap → `TriggerBootupFlow` → tâches → résolution d'écran
par nom (`Presentation.Screens.ActivateScreenWithFocus`) →
`ScreenTask_Screen<Nom>_Enqueue` → `PresentationTaskHandler_RegisterTask`
est désormais confirmée de bout en bout ; voir
`architecture/startup_path.md` pour le détail. Le résolveur par nom
(`activate_mapped_screen_task`) et cinq fonctions utilitaires
(`src/platform/StorageInit.cpp` : énumération des lecteurs, minuteur
d'autosauvegarde) sont intégrés et testés depuis cette découverte.

Exploration menée directement sur le projet Ghidra existant via
`pyghidra`, en lecture seule, pour cartographier le chemin vers
l'intro/menu au-delà du frame pump : le moteur repose sur un framework
interne EA ("GOF", EASTL confirmée) dont le bootstrap (`WinMain` →
`FUN_005dcdf0` → `GameServicesCreate`) alloue une vingtaine de
singletons (`FMVDataManager`, `PresentationTaskHandler`,
`UIComponentManager`, `GameFlowStateController`, ...). Les commandes de
script du jeu ("Presentation.*", "Storage.*", etc.) passent par un
interpréteur central (table de 93 entrées, sauvegardée dans
`architecture/presentation_commands.csv`) ; la tâche `Storage.Bootup`
(machine à 9 états, entièrement cartographiée) mène à l'écran
d'avertissement de sauvegarde, pas encore au menu. Détail complet,
adresses et prochaines pistes (`FMVPlayback`, `Presentation.LeaveTitlePage`,
`OnActivateFrontEndChapter`) dans `architecture/startup_path.md`. Les
quatre primitives génériques mises au jour par cette exploration (le
service d'allocation nommée, la file de tâches d'un
`PresentationTaskHandler`, et le dispatcher de commandes lui-même) sont
intégrées et testées dans le nouveau module `src/presentation/`.

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
