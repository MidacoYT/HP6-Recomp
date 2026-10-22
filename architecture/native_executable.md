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
  5 symboles non résolus après reconstruction des trois méthodes d'allocation
  de la vtable de service : deux fonctions internes du service mémoire,
  le rendu `0x005DD020`, et les tables de commandes et d'écrans. Le nombre
  compte les dépendances nouvellement exposées, pas les fonctions cachées
  derrière ces deux tables.
  Les deux
  vtables `0x007159B0` et
  `0x0071B520` restent à reconstruire, mais leurs constructeurs sont maintenant
  dans `ServiceVtableConstructors.cpp` : le linker ne les charge plus avec
  `service_holder`, qui n'en dépend pas. Les valeurs et objets Direct3D factices
  des tests ne sont pas utilisés par l'exécutable. Certains noms Ghidra
  recouvrent les mêmes octets (par exemple `DAT_00b108a0` et
  `DAT_00b108a4`) ; ces alias partagent maintenant le même stockage natif.
- Quatre feuilles du destructeur de tas `0x005DD6B0` sont reconstruites dans
  `ServiceHeapLeafs.cpp` depuis les instructions à `0x005DB440`, `0x005DB470`,
  `0x005DB480` et `0x005DB490` : décalages de pointeurs `+8`/`+0x30`,
  pointeur relatif et lecture du bit 0 de l'en-tête. La fonction englobante
  appelle encore d'autres routines qui ne sont pas intégrées. Son corps est
  maintenant dans `ServiceHeapTeardown.cpp` : callbacks, verrou, deux listes,
  champs remis à zéro et libération du verrou. `ServiceHeapLock.cpp` conserve
  les appels Win32 et le compteur à `+0x18` ; `ServiceHeapNode.cpp` conserve
  le retrait de la liste et l'appel de destruction. `ServiceHeapCoalesce.cpp`
  reconstruit aussi la fusion `0x005DEC90`. `ServiceHeapFree.cpp` reconstruit
  les trois chemins de libération `0x005DE580`, et
  `ServiceHeapReleaseCheck.cpp` le test de fin de bloc `0x005DDB50`.
  `ServiceHeapInitialize.cpp` reconstruit `0x005DD4C0` : listes, tailles,
  callbacks et branche optionnelle d'allocation. `ServiceHeapEnableLock.cpp`
  reconstruit l'opération 0 de `0x005DD830` avec le verrou embarqué à `+0x4E8`
  et l'initialisation de `0x005DD260`/`0x00FF8F50`. L'allocateur appelé par
  `0x005DD4C0` (`0x00FF9850`) est maintenant reconstruit et testé dans
  `ServiceHeapAllocateRegion.cpp`. Le test isolé de l'initialisation conserve
  un double explicite pour cet appel.
  `ServiceHeapConstruct.cpp` reprend le constructeur `0x005DD2D0` utilisé
  avant l'enregistrement des sous-allocateurs. Il initialise les champs
  observés dans le désassemblage, place les deux rappels à `+0x4B8`/`+0x4C0`
  et appelle `0x005DD4C0`. `ServiceHeapDispatch.cpp` reprend les slots 1 et 2
  du dispatcher `0x005DD830` appelés par `0x00FF8B60` : le premier retourne,
  le second écrit le booléen à `+0x46D`. Les rappels `0x005DDC80` et
  `0x005DD2C0`, ainsi que l'appelant `0x00FF8B60`, ne sont pas encore reliés
  au constructeur. `create_local_allocator_register` dans `RenderHeap.cpp`
  retourne encore un succès fixe ; ce comportement préexistant reste un
  obstacle réel au démarrage fidèle et doit être remplacé par `0x00FF8B60`.
  `ServiceHeapReleasePages.cpp` reconstruit
  la remise des pages `0x00FF9D00` et son auxiliaire `0x005DDC40`. Le chemin
  d'appel depuis `0x005DD6B0` teste d'abord le bit 0 du mot `heap+4`, donc
  son appel à `0x005DEC90` ne peut pas entrer dans la branche du mot nul
  qui appelle `0x005DD4C0`. Ce chemin vérifié est compilé séparément dans
  `ServiceHeapCoalesce.cpp` ; l'entrée générale reste dans
  `ServiceHeapCoalesceGeneral.cpp` jusqu'à la reconstruction de l'allocateur.
- `NativeEngineRenderGateVtable.cpp` relocalise le premier mot à `0x0087EADC`
  vers les trois méthodes vérifiées de la vtable `0x0083E5C8` ; les autres
  champs de l'objet moteur restent à relocaliser. `DAT_00FCAF34` est initialement
  nul dans les deux PE, et le pool d'observateurs à `0x00892990` pointe vers
  les quatre mots nuls à `0x00D63EE0`. La liaison expose maintenant le
  callee de rendu `0x005DD020` au lieu du pointeur moteur lui-même.
- `NativeServiceFinalVtable.cpp` relocalise les quatre entrées de
  `0x008201F8` (destructeur `0x005A2450`, puis trois `_purecall`) ; le mot
  suivant à `0x00820208` ouvre une autre vtable. La liaison native est passée
  de cinq à quatre symboles manquants. `ServiceHandleFree.cpp` reconstruit la
  méthode de libération `0x005DB8D0`, slot 3 de la vtable de service, avec
  son verrou et l'appel de libération du tas.
- `NativeServiceHolderVtable.cpp` relocalise les neuf entrées contiguës de
  `0x0083E55C` et `0x0083E56C`. Les thunks secondaires `0x005E00C0` et
  `0x005E0080` ajustent `this` de quatre octets, et les deux slots
  `0x00407B30` renvoient 1. Les trois méthodes d'allocation
  `0x005DB7A0`, `0x005DB710` et `0x005DB800` sont maintenant reconstruites
  depuis le désassemblage : ordre des arguments, liste d'options et choix
  du tas. Elles dépendent encore du cœur d'allocation et du registre des
  contextes.
- `ServiceContextName.cpp` reconstruit `0x005DB680` : retour direct du nom
  si le préfixe est vide, sinon concaténation `préfixe::nom`. L'initialisation
  du holder `0x005DBA40` est dans `ServiceHolderHeapInitialize.cpp` : elle
  alloue le bloc principal de `0x1FFC0000` octets, construit les trois
  contrôles de tas statiques, applique les opérations du dispatcher et
  installe les callbacks. `ServiceHeapAllocationWrappers.cpp` reconstruit
  le verrou autour de `0x005DDE30` (`0x005DDDB0`) et le calcul de l'offset
  utilisable du bloc (`0x005DEDE0`). Les callbacks de diagnostic
  `0x005DD2C0`/`0x005DDC80` sont relocalisés. Le cœur `0x005DDE30`,
  `0x005DDE30` reste absent. `0x00FF9850` est reconstruit dans
  `ServiceHeapAllocateRegion.cpp`, avec ses deux branches d'ajout de région.
  `ServiceHeapReserveRegion.cpp` et `ServiceHeapInsertRegion.cpp` reprennent
  la réservation des pages et l'insertion en liste depuis `0x005DF3A0` et
  `0x005DDBA0`. Le cœur `0x005DBC50`
  est reconstruit dans `ServiceHeapAllocateNamedCore.cpp` : registre des
  contextes, arrondi, choix du chemin aligné et mot de métadonnées final.
  `ServiceHeapAllocateAlignedLocked.cpp` reprend le verrou autour de
  `0x005DE780` depuis le wrapper `0x005DE700`. Ce cœur aligné reste absent.
  `ServiceHeapAlignmentLeafs.cpp`, `ServiceHeapSplitBlocks.cpp` et
  `ServiceHeapFreeBin.cpp` reconstruisent les classes de tailles et les
  transformations de blocs utilisées par les deux cœurs encore absents.
  `ServiceHeapAllocateDirect.cpp` reprend l'allocation directe par
  `VirtualAlloc` de `0x005DF6D0`, appelée depuis `0x005DDE30`.
  La croissance `0x005DB640`
  est reconstruite dans `ServicePrimaryHeapGrowth.cpp` avec ses quatre
  arguments vérifiés au site d'appel `0x005DE252`. Elle dépend encore de
  l'agrandissement `0x005DBBB0`, maintenant reconstruit dans
  `ServiceSecondaryHeapExpand.cpp` : arrondi du bloc, allocation dans le tas
  secondaire, ajout au tas principal et libération sur échec. Son test couvre
  les trois branches avec des callees explicites. La libération
  `0x005DE540` appelée sur échec est maintenant reconstruite dans
  `ServiceHeapReleaseRegion.cpp` : verrou, libération par `0x005DE580`,
  déverrouillage. Son test couvre les branches avec et sans verrou.
  `ServiceContextRegistry.cpp` reconstruit `0x005DB380` et ses appels
  `0x005E0010`/`0x005DFF60` depuis les corps `0x00FF8820`, `0x0100ED60`
  et `0x0100ED00` : hachage `0x00404620`, recherche exacte, chaîne
  canonique et limites du dictionnaire. Son test vérifie les offsets,
  doublons, variantes de casse et deux débordements.
- Les appels au slot secondaire `0x005DB800` depuis `TaskSystem.cpp`,
  `RenderHeap.cpp` et `TextLayout.cpp` passent désormais l'adresse de
  `DAT_00FCDD78` dans ECX. Les trois sites ont été vérifiés dans le
  désassemblage du PE sans SecuROM (`0x005DC750`, `0x006194A0`,
  `0x005E27E0`) ; la méthode termine avec `ret 8`.
- Les trois méthodes communes de chaque enregistrement d'écran sont
  reconstruites dans `NativeScreenRecordMethods.cpp` : annulation du travail
  en attente (`0x0050BB50`), effacement du pointeur (`0x0050C0C0`) et
  comparaison (`0x0050C140`). Les 24 méthodes d'activation distinctes et les
  vtables de leurs objets de tâche restent à raccorder.
- `NativeCommandLeafs.cpp` reconstruit sept cibles vérifiées dans le PE
  sans SecuROM : le retour `AL=1` partagé par huit commandes
  (`0x0045DF40`), la lecture de l'état du dialogue de stockage
  (`0x0048AD30`), le basculement du découpage de texte (`0x0048B020`),
  la sortie et la reprise du menu de pause (`0x0048B410`/`0x0048B430`),
  `PostQuitMessage(0)` (`0x0048C130`) et la conversion `atoi` du choix de
  langue (`0x0048C950`). Les deux derniers imports sont identifiés par la
  table d'import du PE sans SecuROM.
  Elles ne constituent pas encore la table de 93 commandes.
- La liaison seule ne suffira pas : `create_bootup_service_object` retourne
  encore `nullptr`, le chargement de `Game.str` est différé, le repli sur
  chapitre `Global` ne sélectionne pas `LANGUAGE_SELECT`, et le gestionnaire
  d'écran ainsi que le rendu de présentation natif ne sont pas intégrés.
- Le repli de `FUN_00576f30("Global")` a été suivi jusqu'aux vtables du
  binaire sans SecuROM : le singleton construit à `0x00FDF6B0` contient à
  `+4` un `EventSlotGroupManager` (`0x008283D8`). Son slot `+0x2C` pointe
  vers `0x0048F0F0`, qui appelle `0x00654C00` puis le slot `+0x2C` du
  gestionnaire secondaire à `+0x14` (`0x00843CF8`, méthode `0x006546A0`).
  Ces méthodes parcourent et réinitialisent des slots d'événements. Elles
  ne choisissent aucun chapitre ; `DAT_00D472DC` reste inchangé dans cette
  branche. Il faut donc tracer séparément son affectation initiale.
- Ce site initial est maintenant identifié dans la table des commandes :
  entrée 71, `LanguageSelection` → `0x0048C8F0`. Le binaire sans SecuROM
  compare la distance entre les pointeurs du vecteur de langues du service
  (`+0xB8/+0xBC`) à quatre octets. Avec plusieurs langues et sans la
  compatibilité TransGaming MacOSX (`0x005DA610`), il appelle
  `FUN_00576F30("LanguageSelect")`, ce qui met l'index de chapitre à 9.
  Avec une seule langue, il choisit l'index 0 et confirme directement la
  langue via `0x00429CB0`/`0x00FD7920`, qui mène à `FrontEnd`. Son appelant
  `0x00505670` installe `DS_HIGHLIGHTTEXT` vide, actualise deux drapeaux
  puis envoie `LanguageSelection` si le vecteur n'est pas vide. Ces trois
  fonctions sont reconstruites dans `ScreenTasks.cpp` et
  `LanguageSelectionStart.cpp`, puis testées avec les
  chemins à zéro, une et plusieurs langues. La vtable `0x00828360` de
  `Task_BootupFlowControl` contient `0x00505670` à son slot `+0x0C` : le
  task créé par `0x0048C970` appelle donc directement cette décision.
  `BootupFlow.cpp` conserve maintenant l'adresse de cette vtable dans
  l'objet, conformément à l'instruction `mov [eax], 0x00828360`.
  Les quatre slots initiaux lus dans le PE sont `0x005D2D00` (destructeur),
  `0x0045DF40` (retourne 1), `0x00405A30` (RET) et `0x00505670`.
  Le destructeur remet la vtable générique `0x0083D174` avant une
  éventuelle libération via le service de handles. Les deux vtables et les
  quatre méthodes initiales sont maintenant relocalisées dans
  `NativeBootupTaskVtable.cpp`. `0x00505670` se trouve dans
  `LanguageSelectionStart.cpp`, séparé du reste de `ScreenTasks.cpp` pour
  conserver la dépendance native exacte de cette tâche. Le service de
  langues et la table de commandes restent à raccorder.
- Les 26 mots consécutifs de `0x00715DA8` pointent tous vers
  `0x006B92B0`, un thunk du CRT `_purecall`. `NativePurecallVtable.cpp`
  les relocalise vers le même appel CRT ; les constructeurs gardent
  désormais un pointeur de table à 26 entrées.
- `0x00715D0C` contient un seul pointeur de fonction avant les données
  de chaîne voisines. Il mène au destructeur `0x006B4A02`, qui réinstalle
  cette même vtable et libère l'objet par le service lorsque le drapeau
  bit 0 est présent. `NativeSingleDestructorVtable.cpp` le relocalise.
  Les octets des deux vtables de tâche (`0x00828360`, `0x0083D174`), des
  26 pointeurs `_purecall` et de `0x00715D0C` sont identiques dans les
  deux exécutables vérifiés, protégé et sans SecuROM.
- Le slot `+8` de la vtable `0x0083E5C8` appelle `0x005DCDA0`. Son code
  efface `DAT_00866E28` et, si l'argument bas vaut autre chose que zéro,
  réarme `DAT_00CEF71C` à 5. Le slot `+4`, `0x005DCDC0`, décrémente ce
  compteur ; il met le drapeau à 1 lors du passage à zéro et appelle
  `0x005DD020` à l'appel suivant si le drapeau est encore actif. Les deux
  méthodes sont reconstruites dans `EngineRenderGate.cpp` et
  `EngineRenderGateTick.cpp` avec le passage de `this` d'une méthode C++.
  Les deux valeurs initiales viennent des deux PE vérifiés. L'appel natif
  `0x005DD020` et l'objet complet `0x0087EADC` restent à intégrer.
- Le slot simulate `0x005DCD80` charge le pointeur `DAT_00FCAF34`, appelle
  `0x005E03C0`, puis `0x005DCB30`. Son wrapper et la boucle de copie
  `0x005E03C0` sont reconstruits dans `EngineRenderGateSimulate.cpp` et
  `EngineEventSnapshot.cpp`. La boucle appelle les slots `+8` et `+0x18`
  de sa source, remet à zéro `0x480` octets et copie les mots `+4..+0x20`
  des 32 premiers enregistrements de `0x24` octets. Elle continue à drainer
  la source au-delà de 32. `EngineTickDispatch.cpp` reconstruit le thunk
  `0x005DCB30` vers `0x00FF8D50` : trois messages conditionnels, compteur
  incrémenté et éventuel événement de suppression. Les deux callees
  `0x005DB240`/`0x005DB160` sont reconstruits dans
  `EngineMessageDelivery.cpp` : parcours des observateurs, callbacks des
  slots `+4` et `+0`, et drapeaux de notification. Le retrait différé saute
  vers `0x00FF87D0`, reconstruit dans `EngineListenerRecycle.cpp` ; il retire
  les nœuds sans observateur et les remet dans la liste libre du pool.
  Le pool pointé par `0x00892990`, le rendu `0x005DD020` et la relocalisation
  complète de l'objet `0x0087EADC` restent absents.
- Les deux pointeurs de service `0x0083E55C` et `0x0083E56C` sont séparés
  de `0x10` octets dans la même table des deux PE vérifiés. Les mots initiaux
  sont `0x005DB970`, `0x005DB7A0`, `0x005DB710`, `0x005DB8D0`, puis
  `0x005DB800` au second pointeur. Les neuf mots de ces deux interfaces
  sont identiques dans les deux PE. `service_holder` calcule
  maintenant la seconde adresse à partir de la première : ce symbole de
  liaison séparé a disparu. `DAT_00FCDD74` est un bloc contigu de six mots,
  conformément aux accès `+0x0C`, `+0x10` et `+0x14` du destructeur.
  La table et les trois méthodes d'allocation sont relocalisées ; les cœurs
  d'allocation appelés par ces méthodes restent à reconstruire.
- Le premier slot `0x005DB970` est maintenant reconstruit dans
  `ServiceHolderDestructor.cpp` : il appelle d'abord le destructeur de
  stockage `0x005DB9A0`, libère ensuite l'objet par le service seulement si
  le bit 0 du drapeau est présent, et retourne `this`. Le destructeur de
  stockage et renvoie `this`. Les deux interfaces sont maintenant relocalisées,
  mais les cœurs d'allocation dépendants demeurent sans corps.
- `cleanup_00711e60` est un thunk vers `0x005DB9A0`. Le thunk et le corps de
  ce destructeur sont maintenant reconstruits dans `ServiceHolderCleanup.cpp` :
  les trois appels conditionnels au destructeur de tas, `free` du bloc à
  `+0x0C`, effacement de six pointeurs et restauration des deux vtables de
  fin de vie sont vérifiés par un test isolé. Le callee `0x005DD6B0` et les
  vtables `0x008201E4`/`0x008201F8` ont aussi été reconstruits.
  `register_timer_completion`,
  `FUN_006D95D0`, son point de départ `0x006D9351` et la branche atteinte
  de réglage de priorité `0x006D94F5` sont reconstruits dans
  `TimerCompletion.cpp`. Les octets de l'original à `LAB_006D9CAE` sont
  obfusqués, mais la même adresse dans l'exécutable sans SecuROM contient le
  callback lisible : attente du timer, incrément de deux compteurs, appel des
  huit hooks non nuls, réarmement et arrêt lorsque la date devient nulle.
  `StorageInit.cpp` reconstruit maintenant ce chemin depuis ses instructions
  et les imports `WaitForSingleObject`/`SetWaitableTimer`. L'identité des
  octets de code avec l'original protégé n'est pas démontrable. Le nettoyage
  Le nettoyage reste inutilisable dans l'exécutable natif tant que le
  gestionnaire mémoire `0x005DD6B0` et ses appels ne sont pas raccordés.

## Pointeurs encore à relocaliser

Les valeurs ci-dessous sont les mots lus dans le PE original, avant
relocalisation. Les recopier comme adresses absolues dans le nouvel exécutable
serait incorrect ; leurs cibles doivent être reconstruites.

`tools/audit_native_dispatch_tables.py` vérifie les 93 commandes et les 26
enregistrements de tâches dans les deux PE par SHA-256. Les tables
`presentation_commands.csv` et `screen_task_table.csv` enregistrent les noms,
adresses d'objets, vtables et premiers slots. Les 93 commandes représentent
86 adresses de handlers distinctes ; la table de tâches contient 26 objets
de 12 octets (`vtable`, `name`, état initial nul). Ces inventaires ne sont pas
des implémentations des handlers et ne sont pas liés à `hp6`.
`tools/audit_remaining_symbols.py` vérifie aussi les mots des deux vtables
de service dans les deux PE. La zone utilisée par le holder à `0x0083E55C`
contient neuf entrées : quatre slots primaires et cinq slots secondaires
commençant à `0x0083E56C`. Les mots suivants appartiennent à d'autres
interfaces. La table et ses trois méthodes d'allocation sont relocalisées ;
leurs fonctions internes encore inconnues restent sans stub.

| Symbole | Mot initial du PE | Dépendance |
| --- | --- | --- |
| `PTR_PTR_00876AB0` | `0x0087B0CC` | 93 commandes et leurs handlers |
| `PTR_PTR_00870948` | `0x0087BB8C` | 26 tâches d'écran et leurs vtables |
| `service_heap_allocate_aligned_core_005de780` | `0x005DE780` | cœur d'allocation alignée |
| `service_heap_allocate_core_005dde30` | `0x005DDE30` | allocation dans le tas |
| `render_overlay_005dd020` | `0x005DD020` | rendu appelé par le moteur |
| `PTR_PTR_0087EADC` | `0x0083E5C8` | objet du moteur et ses méthodes |

Les vtables `PTR_FUN_007159B0` (premier mot `0x006B2E49`) et
`PTR_LAB_0071B520` (premier mot `0x006DCB59`) restent absentes pour les
constructeurs correspondants ; elles ne sont plus dans la chaîne de liaison
du démarrage natif actuel.

## Chemin d'intégration

1. Terminer le stockage global initialisé de la version x86 originale, ses
   alias, ses pointeurs relocalisés et les services alloués au démarrage. Les
   tables de tests ne sont qu'un inventaire des dépendances.
2. Reconstruire le nettoyage du service restant, puis l'allocation des services
   du moteur et vérifier une fenêtre et un périphérique D3D9 réels.
3. Raccorder le chargement des archives, le graphe de tâches, le choix du
   chapitre initial et l'activation de `ScreenLanguageSelect`.
4. Envoyer les objets du chapitre dans le pipeline de rendu natif et vérifier
   le démarrage à partir des fichiers du jeu, avec la capture de référence.

Un `hp6.exe` lancé aujourd'hui avec les définitions des tests simulerait le
jeu au lieu de le reconstruire. La cible restera non produite jusqu'à ce que
ses dépendances natives soient implémentées et vérifiées.
