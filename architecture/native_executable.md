# Exécutable natif `hp6.exe`

L'objectif est que `hp6.exe` entre dans `run_win_main` (reconstruction de
`0x005d9300`), initialise la fenêtre et Direct3D 9, charge `Game.str`, exécute
le flux de démarrage, puis active `ScreenLanguageSelect` par le gestionnaire de
chapitres et le moteur de présentation. Le programme
`hp6_language_select_preview` est uniquement un outil de contrôle des assets ;
il n'est pas le jeu et sa cible est désactivée par défaut. La cible `hp6` est
exclue de la compilation globale tant qu'elle ne peut pas être liée.

## État vérifié

- L'entrée PE réelle de l'original est `0x016B0940` dans `Sitext` et commence
  par du code obfusqué. La chaîne CRT décompilée `0x006B9718` → `0x006B9457`
  appelle la fonction applicative `0x005D9300` avec l'ABI `WinMain`. Le symbole
  `WinMain` du projet est maintenant dans `WinMain.cpp`, associé à cette
  reconstruction ; `Hp6Executable.cpp`, qui ajoutait une entrée sans preuve,
  a été retiré. Le CRT du compilateur remplit le même rôle standard. Le saut du
  stub PE obfusqué vers cette chaîne n'est pas encore prouvé ni reconstruit.
- `run_win_main` crée la fenêtre `HalfBloodPrinceMainWndClass` et appelle la
  fonction reconstruite `create_and_configure_render_device`. Cette dernière
  copie maintenant le HWND dans `DAT_009978e4` avant `CreateDevice`, comme
  l'indique la décompilation de `FUN_005dca10` à l'adresse `0x005dca10`.
- Le premier essai de liaison avait 334 symboles uniques non résolus : 331
  variables du binaire original et trois fonctions (`cleanup_00711e60`,
  `register_timer_completion`, `autosave_timer_callback`).
  `NativeInitialGlobals.cpp` reconstruit 262 scalaires et 26 emplacements de
  pointeurs initialement nuls depuis leurs octets dans le PE original, avec
  son SHA-256 enregistré dans le source. `NativeImageArrays.cpp` conserve les
  valeurs initiales de 23 tableaux, `NativeChapterTables.cpp` les noms et
  lignes de chapitres, et `NativeDriveTable.cpp` les 26 pointeurs relocalisés
  vers leurs enregistrements originaux de 20 octets. `NativeHeapOverrides.cpp`
  relocalise les trois tables d'options du tas de rendu. Les quatre tables de
  handles de buffers suivent le stride `0x14`, les indices sur 32 entrées
  des fonctions `0x00651D60`, `0x00651F80` et `0x01003930`, ainsi que les
  zones nulles et les adresses voisines du PE. La capacité de 64 groupes
  pour les tables statiques reste une déduction du bloc de compteurs
  `0x0098B490` (stride `0x48`) et des limites de stockage voisines. La variante native
  de `DeviceCreate.cpp` et `RenderStateCache.cpp` relie quatre adresses
  intérieures à leur tableau de base réel. La liaison comporte encore
  12 symboles non résolus : dix tables ou pointeurs vers du code et des objets,
  et deux fonctions. Les valeurs et objets Direct3D factices
  des tests ne sont pas utilisés par l'exécutable. Certains noms Ghidra
  recouvrent les mêmes octets (par exemple `DAT_00b108a0` et
  `DAT_00b108a4`) ; ces alias partagent maintenant le même stockage natif.
- La liaison seule ne suffira pas : `create_bootup_service_object` retourne
  encore `nullptr`, le chargement de `Game.str` est différé, le repli sur
  chapitre `Global` ne sélectionne pas `LANGUAGE_SELECT`, et le gestionnaire
  d'écran ainsi que le rendu de présentation natif ne sont pas intégrés.
- `cleanup_00711e60` est un thunk vers `0x005DB9A0`, un destructeur qui
  dépend lui-même d'autres fonctions moteur. `register_timer_completion`,
  `FUN_006D95D0`, son point de départ `0x006D9351` et la branche atteinte
  de réglage de priorité `0x006D94F5` sont reconstruits dans
  `TimerCompletion.cpp`. Les octets sur disque à `LAB_006D9CAE`, adresse du
  callback d'autosauvegarde, sont obfusqués et sa décompilation n'est pas
  disponible dans les exports actuels. Les deux autres fonctions restent absentes
  de l'exécutable natif ; leur donner un corps vide masquerait les chemins
  d'initialisation et de nettoyage encore inconnus.

## Pointeurs encore à relocaliser

Les valeurs ci-dessous sont les mots lus dans le PE original, avant
relocalisation. Les recopier comme adresses absolues dans le nouvel exécutable
serait incorrect ; leurs cibles doivent être reconstruites.

| Symbole | Mot initial du PE | Dépendance |
| --- | --- | --- |
| `PTR_PTR_00876AB0` | `0x0087B0CC` | 93 commandes et leurs handlers |
| `PTR_PTR_00870948` | `0x0087BB8C` | 26 tâches d'écran et leurs vtables |
| `PTR_FUN_007159B0` | `0x006B2E49` | méthode de service |
| `PTR_FUN_00715D0C` | `0x006B4A02` | méthode de service |
| `PTR_FUN_0083E55C` | `0x005DB970` | vtable du service de handles |
| `PTR_FUN_0083E56C` | `0x005DB800` | vtable du service de handles |
| `PTR_LAB_00715DA8` | `0x006B92B0` | méthode de service |
| `PTR_LAB_0071B520` | `0x006DCB59` | méthode de service |
| `PTR_LAB_00828360` | `0x005D2D00` | vtable de la tâche de démarrage |
| `PTR_PTR_0087EADC` | `0x0083E5C8` | objet du moteur et ses méthodes |

## Chemin d'intégration

1. Terminer le stockage global initialisé de la version x86 originale, ses
   alias, ses pointeurs relocalisés et les services alloués au démarrage. Les
   tables de tests ne sont qu'un inventaire des dépendances.
2. Reconstruire les deux fonctions manquantes, puis l'allocation des services
   du moteur et vérifier une fenêtre et un périphérique D3D9 réels.
3. Raccorder le chargement des archives, le graphe de tâches, le choix du
   chapitre initial et l'activation de `ScreenLanguageSelect`.
4. Envoyer les objets du chapitre dans le pipeline de rendu natif et vérifier
   le démarrage à partir des fichiers du jeu, avec la capture de référence.

Un `hp6.exe` lancé aujourd'hui avec les définitions des tests simulerait le
jeu au lieu de le reconstruire. La cible restera non produite jusqu'à ce que
ses dépendances natives soient implémentées et vérifiées.
