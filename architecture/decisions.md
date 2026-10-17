# Décisions et limites de reconstruction

## Lot 050 — miettes CRT (crt_crumbs_050), 2026-10-17

- Douze adresses de la bande `006b92xx–006b9bxx` enregistrées comme
  **externes CRT** (motif `volume_registry_044`) — *aucun* code dédié :
  - Thunks d'import (`JMP dword ptr [IAT]`, vérifié dans l'asm exporté) :
    `006b92aa` vsprintf `[0x007131f4]`, `006b92c2` printf `[0x00713210]`,
    `006b93f4` _vsnprintf `[0x0071324c]`, `006b9b36` _CIpow
    `[0x007132ac]`, `006b9b3c` _CIsqrt `[0x007132b0]`, `006b9b48` getenv
    `[0x007132b8]`.
  - Corps d'aides MSVC : `006b9790` __alloca_probe (22 dépendants),
    `006b9be1` __SEH_epilog4 + `006b9b9c` __SEH_prolog4 (9 ch.), le couple
    `_ftol2` `006b9a46`/`006b9a10` (19 + 9 ch., ST0→int64, frère fastcall
    dispatché sur le drapeau SSE2 `DAT_00fd22b8`) et `006b9740` __allmul.
    Notre reconstruction utilise l'arithmétique native, les enregistrements
    servent à la traçabilité et au déblocage du fan-out 006b9x/006bdx.

## Lot 049 — créateur+attente et rappel réservé (pool_waits_049), 2026-10-17

- Wrappers « créer puis attendre » (`src/runtime/PoolWait.{h,cpp}`) —
  chaîne asm-exacte : création (sans mots d'attach) → `006b22c3` (attente)
  → `006b2359` (paire 64 bits) ; `006b17e5`/`006b183a` répondent 1 quand la
  paire est non nulle, `006b175b`/`006b1783` retournent la paire elle-même ;
  `006b23a9` = JMP pur vers `006b2359`. Le créateur répété `006b1783`
  livre chaque argument signé en paire CDQ (valeur, valeur>>31) vers
  +0x28/+0x2c et +0x24/+0x30 — vérifié octet par octet dans l'export.
- `006b2553` → `run_reserved_callback` (`src/runtime/PoolAccess.cpp`) :
  rappel cdecl (valeur de réserve, argument) exécuté sous le verrou
  `frame+0x4c`, mot de réserve visible à `+0x7c` pendant l'appel puis
  restauré, événement `+0x48` signalé ; la trame porteuse est le record ou
  la base de la roue (`root+0x98`) quand le record est marqué à +0x80.
- `006b54b8` → `wrap_true_006b54b8` : `MOV AL,1; RET` (cdecl, 8
  dépendants) rejoint `WrapperStubs`.
- **Couverture de test nouvelle** : le harnais complète maintenant un nœud
  *pendant* l'attente via une entrée à usage unique de la roue
  (`fire_drain`, case 5) — détach du nœud de la liste en attente de la
  trame (dérivée du token à `node+0xc`), vtable réelle d'objet terminé
  (`DoneNodeMethods`, slots 0/2) posée sur le nœud, code de résultat 7 à
  `[+0x1]`, insertion dans la liste terminée `+0x1c`. C'est la première
  couverture qui traverse `wait_pool_node` en boucle (chemin propriétaire :
  pompe de roue + sommeil 1 ms) et `take_completed_node` complet (vtable
  appelée). Assertion piège documentée : lire `[+0x11]` comme *octet* —
  `word(x, 0x11)` lit 4 octets non alignés et échoue.

## Lot 048 — feuilles de wrapper et thunks d'import (wrap_leaves_048), 2026-10-17

- **Preuve par octets** (recette PE corrigée : la table des sections se
  lit à `sections[i]+8`, VSZ à `h+8` — la première tentative lisait le
  nom de section comme VSZ et produisait un faux mappage) :
  - `006b9730` = `JMP dword ptr [0x00713278]`, `006b97bc` =
    `JMP dword ptr [0x007132a4]` — thunks d'import memcpy/memset,
    enregistrés comme externes CRT (119 et 54 dépendants).
  - Famille de wrapper de type DirectDraw : les cibles `0x006b5469`,
    `0x006b6306` des forwarders, et la trame
    `55 8B EC 8B 09 5D E9 rel32` répétée à `0x006b4534/3f/4a/55/60/7b`
    (forwarders **« this-adjust »** : `MOV ECX,[ECX]` re-dérive l'objet
    wrappé avant un saut vers la méthode réelle).
  - No-ops : `006b54af` (RET 8), `006b54b2`/`006b54b5`/`006b54bb`
    (RET 4), `006b54be` (RET) — et `b0 01 c3` (MOV AL,1; RET) à
    0x6b54b8.
- Intégrés comme no-op (`src/runtime/WrapperStubs.{h,cpp}`) :
  `006b454a`/`006b4555`/`006b4560` (forwarders vers no-ops, 1 puis 2
  mots de pile nettoyés), `006b54bb`, `006b54be` (appelé depuis
  `006b6306` avec `this` resté dans ECX). Elles n'écrivent rien.
- Les autres pièces de la famille (`006b4534` -> `006b6306`,
  `006b453f` -> `006b5469`, accesseurs non exportés `006b456c`
  (`MOV EAX,[ECX]; MOV EAX,[EAX+0x14]`), `006b4572`
  (`MOV EAX,[ECX]; MOV AL,[EAX+0x84]; RET`), `006b457b` -> `0x6b5478`)
  forment le lot suivant avec les méthodes réelles du wrapper.

## Complément lot 039 — primitives de file intrusive (pool_leaves2_039), 2026-10-17

- ReAgent (fenêtre LLM retrouvée à 06:53) a réexaminé `006db315` et
  `006db335` — les deux dernières fonctions du lot `pool_leaves2_039` —
  mais ses candidats échouent à sa propre validation ; intégration manuelle
  arbitrée par l'assembleur exporté (`src/runtime/ListQueue.{h,cpp}`) :
  file intrusive { tête +0, queue +4, compteur +8 }, lien du nœud à +0 :

  - `006db315` -> `queue_push_006db315` : thiscall callee-clean (RET 4) ;
    lien du nœud mis à zéro, compteur incrémenté, queue = nœud, puis tête =
    nœud si la file était vide, sinon l'ancienne queue le chainage.
  - `006db335` -> `queue_pop_006db335` : fastcall ECX-seul (RET sans
    ajustement) ; file vide = no-op ; file à un nœud = tête et queue mises
    à zéro ; sinon tête = `next`, puis compteur décrémenté et lien du nœud
    retiré remis à zéro.
- Les appelants (`006b1f02`, `006b1f4b`, `006b268b`, `006b2a23`,
  `006daee1`, `006daf68`, `006db11b`, `006db171`, `006db1a5`) s'étendent
  du pool au voisinage de la roue temporelle : primitive partagée, module
  dédié plutôt que `PoolList` (dont la mécanique `insert_first` diffère :
  lien à +8, adresse brute de nœud).
- Le lot `pool_leaves2_039` est complet : 7 adresses intégrées via les
  lots 043–045, ces 2-ci manuellement.

## Lot 047 — créateurs à volume (pool_names_047), 2026-10-17

- `006b2fbd` -> `construct_bounded_path_node` : attach, vtable
  `PTR_FUN_00715ab0`, attachement de chemin `006b1968`, mots bornés
  (+0x24=0, +0x20=param_2), **ordonnancement en dernier**.
- `006b289e` / `006b2990` -> `create_named_node_a_from_record` /
  `create_bounded_path_node_from_record` : la **moteur des nœuds nommés
  est un enregistrement de volume** : `006b25b8(path)` alimente
  l'attachement (`006b2ef3`/`006b2fbd`), chemin + mode + mots par la pile.
- `006b28d4` -> `create_named_node_b_from_volume` : sans `:` le moteur
  vient du chemin du jeu (**deux** déréférencements de `root+0x88`) ; si
  le chemin n'est pas UNC (`\\` double), le nom du volume est consulté via
  le slot +0x30 **thiscall ECX sans argument pile** (`MOV ECX,[EBX+0x84];
  CALL [EAX+0x30]`), puis un tampon de 260 octets à `[EBP-0x104]` reçoit
  `nom + chemin` : le nom copié avec son NUL, le chemin recopié par-dessus
  le terminateur **avec** son NUL (le strlen asm en ligne compte le NUL ;
  queue `REP MOVSD`) — sémantique exacte strcpy+strcat, sans mise à zéro.
  Le callee du slot nettoie TOUJOURS un mot de pile (RET 4, preuve par
  l'équilibre de `006b25b8`) : l'accès ECX-seul du binaire décale
  silencieusement ESP de +4 dans son cadre EBP — notre reconstruction
  passe un argument fictif pour rester équilibrée (équivalent
  comportemental).
- Rappel des harnais : le slot moteur vivant a besoin de
  `[engine+4]=1` (pas d'amorçage), d'une CS **initialisée à +0x28
  (zone à +4)** et d'un événement réel à +0x48 — sinon interblocage
  (`EnterCriticalSection` sur CS nulle) ou `STATUS_INVALID_HANDLE`
  (`SetEvent(0)`).

## Lot 046 — attachement de chemin et créateurs restants (pool_path_046), 2026-10-17

- `006b1968` -> `attach_node_path` (`src/runtime/PoolPath.{h,cpp}`) :
  argumentpile du chemin (nœud en ECX, chemin sur la pile). Chemin nul →
  chaîne vide ; le littéral `0x007278d3` **vérifié dans le binaire livré**
  (un NUL nu, suivi de « Constant… »). strlen en ligne ; bloc d'erreur à
  12 dwords sur la pile `[EBP-0x30..-0x4]` (type 6, ligne 0x1cc,
  sous-enregistrement `{5, littéral filesys.cpp à 0x715a30}` — contenu
  vérifié dans le binaire —, mots de drapeau 1/1/4/0). L'objet
  `DAT_00851384` (slot +0 de sa vtable, thiscall ECX + deux arguments
  pile nettoyés par l'appelé : longueur puis enregistrement) renvoie un
  tampon de strlen+1, stocké à `node+0x30`, rempli via le dispatch memcpy
  `006b3b04`. Déblocage de la chaîne `006b2ef3`/`006b2f40`/`006b2fbd`.
- `006b2ef3` / `006b2f40` -> `construct_named_node_00715a90` /
  `_00715aa0` : forme d'écriture identique (attach → vtable → chemin →
  ordonnancement), seule la vtable diffère.
- `006b2482` / `006b2527` -> `create_signed_node_from_record` /
  `create_plain_node_from_record` : créateurs cdecl ; entonnoir
  (record, bound_low=p4, bound_high=p2, tag=p3, mode=p5, words=p6,
  record+0xc) et (record, mode, words, record+0xc). **Aucun garde de
  pool vide** : l'asm exporté tombe du branchement nul dans
  `MOV EAX,[EAX+0xc]` et plante, comme le binaire. Cette lecture exonère
  aussi `006b2456`/`006b24b7` — leurs gardes « return 0 » synthétisés du
  lot 043 ont été retirés comme déviations ; leur harnais
  (pool_wait_smoke) n'exerce plus que les chemins peuplés, avec
  ré-alimentation explicite de la liste libre.

## Lot 045 — dispatch CRT de copie + constructeurs (copy_dispatch_045), 2026-10-17

Lot manuel d'attente pendant la fenêtre LLM ; débloquage clé pour l'arbre
`006b1968`.

- **`006b3b04` résolu** (retiré des différées) : thunk CRT
  `PUSH EBP; MOV EBP,ESP; POP EBP; JMP dword ptr [0x008516ac]` —
  pass-through cdecl **sans ajustement de pile** ; `006b1968` nettoie
  `ADD ESP,0xc`, l'appelé reçoit donc exactement 3 arguments cdecl.
  L'identité du slot provient du binaire livré lui-même
  (`D:\HP\hp6ss\build\sdk-lab\pc\hp6.exe`, sha256 `41cbb7…`) : la cible par
  défaut `0x006b3b91` lit `[ESP+4/8/0xc]`, teste l'alignement de l'arg1 et
  enchaîne une boucle de copie 0x20 octets — **memcpy scalaire** ; la
  variante SSE2 `0x006b412e` utilise `MOVEUPS`. Enregistré
  `std::memcpy (CRT external)`.
- `006b3b24` : initialiseur du même dispatch (lu depuis le binaire) : teste
  le drapeau +0xb bit 1 (0x2000000) puis les bits 0x40/0x80 de +0xa et
  remplit en paires `008516a8` (memset : 0x6b423a/0x6b407a/0x6b3e30/0x6b3c8c)
  et `008516ac` (memcpy : 0x6b412e/0x6b3ee4/0x6b3d46/0x6b3b91). Pas de
  reconstruction ad hoc ; la bibliothèque standard fait déjà le dispatch.
- `006b3094` -> `construct_signed_node` : attach, +0x38/+0x3c à zéro,
  registre à +0x14, **paires bornées signées** (+0x28/+0x2c et +0x20/+0x24
  avec mots de signe `>>31`), vtable `PTR_FUN_00715ad0`, +0x30,
  ordonnancement `006b2199`.
- `006b335c` -> `construct_plain_node` : particularité d'ordre — la vtable
  `PTR_FUN_00715b00` est écrite **avant** le mot de registre +0x14
  (contrairement aux autres constructeurs), puis ordonnancement.

## Lot 044 — registre de volumes (volume_registry_044), 2026-10-17

Résolution de la chaîne `006b3c8c` → `006b3b0e` → `006b25b8`.

- `006b3c8c` : **corps CRT statique de `memset`** (décompilé en entier par
  Ghidra : tête non alignée, boucle 32 octets, queue) — enregistré
  `std::memset (CRT external)`, pas de reconstruction ad hoc.
- `006b92bc` / `006b9392` / `006b93a4` : thunks d'import CRT
  `strncpy` / `strncmp` / `_stricmp` (l'avertissement « jumptable » est le
  bruit de modélisation usuel des thunks) ; référencés via `<cstring>`
  / `_stricmp` chez les consommateurs reconstruits.
- `006b3b0e` -> `clear_buffer` : cdecl 2 arguments nettoyés par l'appelant
  (l'asm de `006b25b8` les couvre avec `ADD ESP,0x1c`) ; zéro `bytes`
  octets via le slot memset `PTR_FUN_008516a8` — implémenté comme
  `std::memset`.
- `006b25b8` -> `find_volume_by_name` (
  `src/runtime/VolumeRegistry.{h,cpp}`) : l'assembleur exporté établit la
  **troisième convention** d'appel virtuel : slot +0x30 = thiscall avec
  `ECX = objet à record+0x84` **ET** un argument pile (le tampon du nom)
  nettoyé par l'appelé (`PUSH buf; MOV EAX,[ECX]; CALL [EAX+0x30]`). Le
  slot ne doit pas écraser le tampon, sinon `_stricmp` comparerait le nom
  du record contre lui-même. Extraction du préfixe : exactement **deux**
  déréférencements depuis `root+0x88` (`ECX=[root+0x88]; ECX=[ECX]`) —
  une triple indirection (lecture Ghidra) plante ; tampon local 16 octets
  remis à zéro avant `strncpy` de `len(':')+1` octets, le terminateur
  venant de la zone zérotée. Retour précoce : mot `root+0x98` nu quand
  ni `:` ni `/\\`.

## Lot 043 — roue de minuteries + attente de nœud (pool_wait_043), 2026-10-17

Poursuite du lot manuel pendant la fenêtre LLM ; 6 fonctions de l'arbre
médian du pool de chaînes, toutes les dépendances désormais intégrées.

- `006d971d` -> `pump_timer_slots` (`src/platform/TimerWheel.{h,cpp}`) :
  stdcall sans argument, actif seulement si le thread courant possède la
  racine (`thread_owns(0)`). L'assembleur exporté prouve : roue de **16**
  créneaux de 0x14 octets à `0x00899ad8` (callback, argument, période,
  échéance, occupé — `piVar3` part de `0x899ae4`, sentinelle `0x899c24`) ;
  tirage si callback défini, `now >= due` (comparaison **signée**) et
  `busy == 0` ; appel du callback **cdecl** avec `(argument, now - due)` ;
  `due = now relecture + période` **après** l'appel (le x86 relit
  `[0x00899d88]`), `busy` posé autour de l'appel. Snapshot
  `DAT_00899c1c = DAT_00899d88` en tête.
- `006b22c3` -> `wait_pool_node` (`src/runtime/PoolWait.{h,cpp}`) : -3 si
  jeton nul ou record (+4) absent ; boucle : verrou +0x28, `006b2147`
  (qui remet toujours *source = 0 — donc le drapeau `[EBP+8]` dégénère en
  « trouvé »), puis si trouvé : pompage `006d971d` + `sleep_alertable(1)`
  (thread possesseur) sinon attente/réinitialisation de l'évent manuel à
  +0x78 ; classification `006b2223`. Le candidat ReAgent rejeté surestimait
  le rôle du mot `[EBP+8]` initial.
- `006b3032` -> `construct_timer_node` et `006b318b` ->
  `construct_repeat_node` (`src/runtime/PoolList.cpp`) : constructeurs
  thiscall modélisés en fonctions à récepteur, attach `006b20e8`, vtables
  `PTR_FUN_00715ac0` / `PTR_FUN_00715ae0`, ordre d'écriture fidèle
  (+0x38/+0x3c d'abord, +0x30 en dernier pour le répétitif), ordonnancement
  `006b2199`.
- `006b2456` / `006b24b7` -> `create_node_from_record` /
  `create_repeat_node_from_record` : **l'argument de taille de `006b1d53`
  est poussé par chacun des huit appelants observés (`PUSH 0x40`) mais
  jamais lu** (RET nu, dépilation à la charge de l'appelant cdecl) —
  signature de `detach_first_locked` mise à jour en
  `(std::uint32_t allocation_size)` ignorée, avec harnais existant adapté.
  Retour : mot +0xc du nœud construit, 0 si le pool est vide.

Harnais `pool_wait_smoke` : pompage réel de la roue (tirage, relique d'état
occupé, échéance future, thread étranger), attente complète (-3 déterministe
quand le nœud est introuvable), constructeurs + créateurs avec vérification
de l'insertion ordonnée et du signal d'évent. 20/20 tests du projet.

## Lot 042 — moteur embarqué + lancement de thread (engine_boot_040), 2026-10-17

Pendant la nouvelle fenêtre de limite LLM (jusqu'à 6 h 44), lot manuel de 5
fonctions qui déverrouille l'arbre médian du pool de chaînes.

- `006f363c` -> import CRT `_beginthreadex` (nommé par Ghidra) : suivi comme
  import (comme `006b92c8`/`006b9774`), référencé via `<process.h>` dans la
  reconstruction du lanceur ; pas de reconstruction ad hoc.
- `006d94f5` connu (`set_thread_priority`). `006d95d0` -> `launch_thread` :
  `_beginthreadex` CREATE_SUSPENDED avec trampoline `006d9351`, priorité
  posée, `ResumeThread`, boucle `SleepEx(1,1)` sur le drapeau posé par le
  trampoline (contexte **toujours sur la pile du lanceur**, même quand un
  `storage` est fourni — l'assembleur lit `[EBP-0xc]` et non `ESI+0xc`),
  `CloseHandle` seulement quand `storage == 0`. Retour bool par `AL`
  (`MOV AL,1` / `XOR AL,AL`) — même artefact `CONCAT31` que lot 041.
  `param_4` n'est jamais lu par le binaire.
- `006d9351` -> `thread_trampoline_006d9351` : `ctx[0] = 1` puis
  `ctx[1](ctx[2])`, stdcall (type de proc `_beginthreadex`).
- `006b203a` -> `bootstrap_engine` : l'assembleur prouve que l'appel du slot
  virtuel +4 est **thiscall avec ECX = objet à +0x84** et sans argument pile
  (`MOV ECX,[ESI+0x84]; MOV EAX,[ECX]; CALL/JMP [EAX+4]`) ; la branche
  précoce est un saut de queue modélisé comme appel. Verrou racine
  `DAT_00892b60 + 0xc`, verrous +0x28/+0x4c, events +0x44/+0x78, mot de réveil
  0xff à +0x7c, lancement du corps `006b1e32` (déclaré externe, non
  intégré) via `launch_thread`.
- `006b20e8` -> `attach_engine` : constructeur thiscall, vtable 007159e8,
  moteur amorcé si son drapeau +4 est nul, champs +0x10..+0x30 dont le
  pointeur de table `DAT_006b18de` à +0x1c, jeton via `006b1910` à +0xc.

Harnais `engine_boot_smoke` : threads réels (trampoline + event + verrous),
18 contrôles. 19/19 tests du projet.

## Lot 041 — src/platform (service_cleanup_038), 2026-10-17

Sept fonctions du cycle de vie des records avec table d'entrées
(`src/platform/ServiceLifecycleExtra`) : la limite d'utilisation LLM a bloqué
le lot côté ReAgent (5 candidats rejetés, 2 `PASS` en attente de revue).

- `006dd00c` -> `remove_entry_pointer` : suppression-échange dans la table
  d'entrées à +0x20 (compteur +0xa0), remise des mots auxiliaires dans l'ordre
  fixe 0xac, 0xa8, 0xa4. **Retour bool porté par AL** : l'assembleur exporté
  montre `XOR AL,AL` sur les sorties sans retrait et `MOV AL,1` après retrait ;
  les formes Ghidra `CONCAT31(...)` et `valeur & 0xffffff00` sont des artefacts
  de bits EAX non suivis (le candidat rejeté les avait pris au pied de la
  lettre avec une formule inversée — `.ghidra-exports` garde la preuve).
- `006dd188` / `006dd354` / `006ddb15` -> `finalize_record_0071b560/80/a0` :
  vtable posée (0071b560 / 0071b580 / 0071b5a0), `release_member_0x468`
  (= 006de5b0 déjà intégré), retrait du record de la table du membre à +0x464
  via `remove_entry_pointer` (sans garde nulle, conforme au binaire),
  puis `destroy_record_with_member` (= 006de5e1). La variante 0071b5a0
  ajoute un appel du slot +0x20 du membre à +0x498 ; l'assembleur 006de5b0
  montre `PUSH EAX; CALL [ECX+0x20]` — l'objet passe par la pile avec
  nettoyage callee (fiddles `__stdcall` du projet corrects).
- `006dd2c9` / `006dd894` / `006dddf6` -> `destroy_entry_record_0071b560/80/a0`
  : les candidats ReAgent `PASS` ont été validés ligne à ligne contre le
  décompilé (identiques) puis intégrés ; libération à 0x46c / 0x46c / 0x4a4
  sous le bit 0 du flag.

Le harnais `service_cleanup_smoke` a d'abord gelé sur une invocation des
slots en `cdecl` (dérive de pile) ; des stubs `__stdcall` dédiés corrigent
l'ABI, et un `virtual ~Member()` du harnais décalait les indices de vtable
(supprimé). 18/18 tests du projet passés.

## Architecture

Les fonctions intégrées appartiennent à `hp6::math` selon leurs opérations
observées sur des flottants, vecteurs et matrices. Elles restent des fonctions
libres : les exports ne prouvent pas une classe propriétaire. Le groupe
`004301c0`, `004301e0`, `00430200` partage un appelant (`0042fa90`) et un
schéma d'accès à trois composantes d'un pointeur indirect.

Les signatures d'origine sont conservées dans `function_map.jsonl` et
`address_map.csv`. Les noms C++ reflètent l'opération attestée, sans prétendre
reproduire le nom ou l'ABI de l'original.

Le groupe `006db3f9` et ses cinq appelants est placé dans `hp6::stream`.
`006db3f9` vérifie la signature 32 bits `0x4d525453` (`STRM`) ; les appelants
lisent des mots aux offsets attestés par l'assembleur. Les offsets restent
numériques car aucun nom de champ fiable n'est disponible.
Le lot suivant ajoute six fonctions de ce même groupe : recherche d'entrée,
écriture de descripteur et accès à des mots validés. Le plan a signalé
`006db3f9` comme dépendance hors lot ; l'orchestrateur l'a résolue grâce à
son intégration antérieure consignée dans `state.json`.

## Fonctions différées

| Adresse | Évidence | Décision |
| --- | --- | --- |
| `005d16f0` | Le candidat ReAgent copie `local_14` non initialisé vers la quatrième sortie. | Ne pas intégrer une lecture indéfinie ; inspecter la pile et les instructions avant de définir cette composante. |
| `005e8b50` | Après `UCOMISS`, l'assembleur fait `LAHF`, `TEST AH,0x44`, puis `JP` vers le retour 1. Le candidat `PASS` du second tour inverse les cas d'égalité à zéro. | Différer malgré `PASS` ; trancher par une validation ciblée des drapeaux et de `NaN`. |
| `0068c0a0` | `FCOMPP`, `TEST AH,0x41`, puis `JNZ` vers le retour 0. Le candidat `PASS` du second tour rend 1 dans le cas non ordonné. | Différer ; vérifier le résultat `NaN` contre l'assembleur avant intégration. |

## Lot 030 — src/platform/HandleRegistry (service de handles), 2026-10-16

Huit fonctions autour du service de fermeture de handles Win32
(`src/platform/HandleRegistry`) :

- `006b936d` -> `atexit_register` : thunk CRT `__onexit` (0 succès / -1
  échec) ; la valeur du handler précédent reste documentée pour la parity.
- `005db920` -> `service_holder` : initialisation paresseuse une fois par
  exécution (bit 0 de `DAT_00fd1254`), pose les deux tables de vtables
  (`PTR_FUN_0083e55c` / `0083e56c`), le pointeur de veille `DAT_00cef6bc` et
  enregistre `cleanup_00711e60` via le thunk atexit. Retour toujours
  `&DAT_00fcdd74`.
- `005dc710` -> `release_through_service` : slot virtuel `objet + 0xc`
  (index 3), appel `(objet, 0)`, no-op sur null.
- `006d91f0` -> `close_handle` : hors bloc statique [0x00896498, 0x00899697]
  relâche via 005dc710 ; dans le bloc, zéro une entrée de `DAT_008998c8`
  (LOCK, index = (p - 0x225926)/100) puis `CloseHandle` si non nul, ordre
  de branches conforme au décompilé.
- `006b2e49`/`006b4a02`/`006b7cef`/`006dcef4` -> quatreconstructeurs de style
  distincts qui posent chacun une vtable importée (`007159b0`, `00715d0c`,
  `00715da8`, `0071b520`) et relâchent l'objet neuf si le bit 0 du flag est
  posé. Les tables elles-mêmes restent des déclarations externes : aucune
  classe C++ n'est définie sans preuve.

Le test `handle_registry_smoke` saute proprement sous x64 (layout à pointeurs
4 octets requis pour les lectures +0x0c) et vérifie l'adjacence des globales
du harness avant d'attester le slot. Trois erreurs de mon harness ont été
prises par le test au fil de l'eau (callbacks atexit inobservables avant
main, comptes de slips). 14/14 tests du projet passés.

## Lot 029 — src/runtime + src/platform (feuilles du pool), 2026-10-16

Les feuilles sans appels du sous-arbre `006b17xx/006b2xxx/006b3xxx` (services
de pool appelés par le lot des chaînes, planifié pour ReAgent sous le lot
`pool_leaves_029`) sont intégrées :

- `006d92a1`/`006d92ac` -> `platform::enter_lock` / `leave_lock` :
  `EnterCriticalSection` / `LeaveCriticalSection` à frame+4 (frame en ECX).
- `006d9416` -> `sleep_alertable` : `SleepEx(ms, 1)` (attente alertable).
- `006d944e` -> `thread_owns` : propriétaire 0 compare l'id courant à
  `DAT_00899acc` (externe, valeur inconnue), -1 accorde toujours, sinon le
  DWORD à frame+8.
- `006b18df` -> `runtime::pool_slot_address` : racine `DAT_00892b60` (externe),
  base `*(racine+0x98)`, pas `0x88` après masquage 8 bits, mot propriétaire à
  +0x80, remise à la base terminée uniquement quand le mot est non nul et
  `reset` est vrai.
- `006b1a2e` -> `runtime::read_virtual_pair` : slot de vtable +0x24 (index 9)
  appelé avec la valeur à frame+8, retour 64 bits (EDX:EAX -> uint64) vers
  `out_value`, puis copie des mots +0x18/+0x1c.

`006b3b04` est **différée** : Ghidra ne peut pas récupérer la jumptable à
0x006b3b08 et traite le saut indirect comme un appel de `DAT_008516ac` ;
aucun corps n'est écrit sans preuve de bridge.

Le test local `pool_access_smoke` définit les équivalents test-locaux des
deux globales (`DAT_00892b60`, `DAT_00899acc`), comme pour `DAT_007db234`.
Le premier essai a détecté une erreur de mon harness (CS et frame propriétaire
partageaient le même stockage, +8 écrase l'ABI de CRITICAL_SECTION) ; les
structures sont désormais distinctes. 13/13 tests du projet passés.
Le reste du sous-arbre (`006b175b`, `006b1783`, `006b17e5` et l'arbre
intermédiaire `006b1d53`..`006b31xx`) sera reconstruit par ReAgent à la
reprise d'utilisation.

## Lot 028 — src/archive (fermeture), 2026-10-16

Cinq wrappers ferment le sous-système des entrées d'archive (`src/archive/Entry`) :

- `006dc733` -> `tag_digits_value` : digits de `extract_numeric_tag` seuls.
- `006dc8e4` -> `header_field_32` : 16 bits à +4 pour 0xC0FB, lecture BE à
  +8 pour les types 2..3, 0 sinon.
- `006dc8c2`/`006dc989` -> `entry_pointer_by_name[_twin]` : pointeur
  `bytes + champ32`, résolu par le wrapper correspondant (006dc86f / le
  jumeau 006dc962).
- `006dc9ab` -> `resolve_entry_list` : séparation par `','` avec restauration
  du séparateur in situ (006b9386 = `strchr` CRT, déclaré std::strchr) et
  écriture consécutive des pointeurs.

Le test local vérifie les deux jumeaux et la liste séparée (pointeurs et
restauration de la chaîne). 12/12 avant le lot 029.

## Lot 027 — src/archive (recherche d'entrée), 2026-10-16

Second lot sans ReAgent. Le cur' de recherche `006dc751` et ses cinq wrappers
(`006dc86f`, `006dc896`, `006dc920`, `006dc959`, `006dc962`, `006dc9eb`) plus
la comparaison insensible `006dc5c9` sont intégrés dans `src/archive/Entry` :

- `006dc751` -> `find_entry` : parcours des entrées `[champ big-endian de
  largeur w][chaîne]` à partir du curseur 0x10 (6 pour le marqueur 0xC0FB).
  La largeur vient du 4e octet de marque moins 0x30, bornée à 4 pour le type 3.
  Le pointeur de nom ajoute la largeur DEUX fois : vérifié dans
  l'assembleur exporté (`MOV EAX,[EBP+0x8]; ADD EAX,EBX; LEA EDI,[EAX+ESI]`
  à 0x006dc7c7), ce n'est pas une erreur Ghidra. Deux modes : par nom
  (comparaison insensible, second argument en mode pass-through) et par
  index ; `field_value` reçoit les 32 bits bas du pack, `raw_value` la
  lecture BE stockée juste après le champ.
- `006dc5c9` -> `case_insensitive_compare` : boucle `tolower` du décompilé,
  retour signé. Ghidra laisse la seconde chaîne dans un registre non alloué
  (`in_EAX`) ; l'appelant `006dc751` place la chaîne demandée dans EAX et le
  nom dans ECX avant l'appel. La convention d'appel d'origine reste
  à confirmer (bridge Ghidra) ; la forme C++ à deux chaînes préserve le
  comportement observé.
- `006dc896` -> `entry_pointer_by_index` : retourne `bytes + valeur_32b_kas`.
- `006dc920`/`006dc9eb` -> `copy_entry_string[_twin]` : copie de la chaîne de
  nom en place à distance fixe (source-destination), fidèle au décompilé.
- `006dc959` -> `entry_forward` : passe-plat signature complète.
- `006dc86f`/`006dc962` -> `entry_field_by_name[_twin]` : valeur 32 bits bas
  par nom comparé.

Test `tests/archive_entry_smoke` plus le conteneur minimal type 1 avec deux
entrées : le premier essai a détecté une position erronée du nom (le test
contraint le mécanisme double-largeur à être réellement appliqué) ;
12/12 tests du projet passés. Équivalence binaire toujours inconnue.
Mémoire de repli : à la reprise d'utilisation ReAgent (17 oct 01:19), les
candidats de ces deux lots pourront servir de comparaison, mais
`state.json` reste la référence d'intégration (les fonctions sont déjà
`integrated` ; l'orchestrateur ne les re-soumettra pas).

## Lot 026 — src/archive (conteneur BIGF), 2026-10-16

Le groupe `006dc532`..`006dc6a3` (partagé par le futur appelant `006dc751` et
six wrappers `006dc86f`..`006dc9eb`) forme le sous-système de lecture des
en-têtes d'archives. ReAgent étant indisponible (limite d'utilisation du LLM,
reprise prévue le 17 oct 2026 01:19 UTC), les cinq fonctions ont été intégrées
depuis les exports Ghidra et vérifiées localement par
`tests/archive_header_smoke` (11/11 tests du projet passés) :

- `006dc532` -> `read_partial_uint` : lecture big-endian de 1..4 octets. L'ordre
  big-endian est la seule interprétation cohérente avec les littéraux attestés
  `0x42494746` ("BIGF"), `0x424947xx` ("BIG?" haut 24 bits), `0x536870xx`
  ("Sh??") et le marqueur signé 16 bits `0xC0FB`. Tailles hors 1..4 -> 0.
- `006dc594` -> `pack_bytes_be` : paire EDX:EAX égale à un accumulateur uint64
  décalé de 8 bits par octet (vérifié algébriquement à 5 et 6 octets).
- `006dc5f9` -> `classify_container` : 1 = marqueur `0xC0FB` (comparé en short
  signé), 2 = "BIGF" ou "Sh??", 3 = "BIG" haut 24 bits. Les lectures répétées
  du décompilé sont conservées.
- `006dc664` -> `header_payload_offset` : type 1 -> (16 bits à +2) + 4 ;
  types 2..3 -> lecture 32 bits à +0x0c ; type 0 -> 0.
- `006dc6a3` -> `extract_numeric_tag` : balise `[A-Za-z][0-9][0-9][0-9]`
  à (taille d'en-tête - 8), base calculée avec le wrap 32 bits d'origine ;
  maths char signées avec biais -0x14d0 ; lecture 32 bits à l'octet suivant.

Le premier test a détecté une erreur d'interprétation personnelle des CONCAT
Ghidra (lecteur supposé little-endian) ; corrigée vers big-endian avant
enregistrement de l'état. Aucune classe propriétaire n'est inférée : fichiers
libres dans `hp6::archive`. Les fonctions restent `equivalence: unverified`.
Prochain lot : `006dc751` (2015 octets décompilés, 6 appelés de ce groupe) et
ses wrappers ; à tenter via ReAgent après la reprise d'utilisation.

## Corrections du lot STRM

Le candidat ReAgent `PASS` de `006db3f9` écrivait la valeur de la signature
dans la troisième sortie. `MOV [ECX],EAX` à `0x006db414` prouve que cette
sortie est l'adresse du bloc. Les cinq appelants ont été intégrés depuis leurs
candidats ReAgent initiaux et les instructions exportées. ReAgent a échoué à
valider ces cinq candidats, notamment avec `Candidate must contain exactly one
complete function body` pour ceux qui déclaraient une structure auxiliaire.
Leur `PASS` logique éventuel reste distinct du test de compilation.

Dans le lot suivant, le vérificateur ReAgent a rejeté plusieurs fonctions
parce que les offsets n'avaient pas de noms de champs, alors qu'aucun nom
fiable n'était présent dans les exports. Il a aussi inversé les branches
`JNZ` de `006dbb55` et `006dbdeb`. Les six candidats initiaux sont conservés
dans `reports/pipeline/rejected/` ; l'intégration suit les instructions x86
aux adresses correspondantes et garde les offsets numériques. Les tests
locaux vérifient les bornes et les écritures, sans conclure à l'équivalence
complète avec le binaire.

## Corrections du lot projection_constructors_014

ReAgent a atteint sa limite d'utilisation (Codex Pro) sur tous les candidats de ce lot.
Les fonctions ont ete integree manuellement a partir des decompilations Ghidra
(consignees dans `reports/pipeline/projection_constructors_014.*.reverse.log`)
et de l'analyse x86 :

- `006d6ab6` -> `hp6::projection::invoke_global_callback` : prepare des valeurs
  1..6 et invoque `(*DAT_00895c84)(param_2, &compteur_6)`. Le global externe
  `g_callback_vtable_00895c84` est declare mais non resout (pointeur vers vtable
  inconnue). Retourne `void` car le code assembleur ne positionne pas EAX avant RET.
- `006d793d` -> `hp6::projection::construct_variant_80_record` : appelle
  `initialize_record_header`, pose l'octet 0x69, configure le payload relatif,
  puis `sprintf(payload, "EAGL%d", taille)` (adresse 0x0071b12c).
- `006d7980` -> `hp6::projection::create_clut_element` : appelle le callback
  global, puis `construct_variant_80_record`, puis positionne le bit 10.
- `006d79fa` -> `hp6::projection::create_shp_record_variant_20` : appelle le
  callback global, puis `decode_variant_20`, puis positionne le bit 10.
- `006d7d9c` -> `hp6::projection::create_shp_record_variant_02` : appelle
  `estimate_record_size`, le callback global, puis `decode_variant_02`, puis
  positionne le bit 10.

Les trois create_* retournent `nullptr` car le callback global (DAT_00895c84)
est externe et son comportement/allocation est inconnu.

## Fonctions différées

| Adresse | Évidence | Décision |
| --- | --- | --- |
| `005d16f0` | Le candidat ReAgent copie `local_14` non initialisé vers la quatrième sortie. | Ne pas intégrer une lecture indéfinie ; inspecter la pile et les instructions avant de définir cette composante. |
| `005e8b50` | Après `UCOMISS`, l'assembleur fait `LAHF`, `TEST AH,0x44`, puis `JP` vers le retour 1. Le candidat `PASS` du second tour inverse les cas d'égalité à zéro. | Différer malgré `PASS` ; trancher par une validation ciblée des drapeaux et de `NaN`. |
| `0068c0a0` | `FCOMPP`, `TEST AH,0x41`, puis `JNZ` vers le retour 0. Le candidat `PASS` du second tour rend 1 dans le cas non ordonné. | Différer ; vérifier le résultat `NaN` contre l'assembleur avant intégration. |

## Limites ABI et données

### Réconciliation des constructeurs et des flottants

Des ajouts concurrents dans `RecordDecode.cpp` contenaient trois fonctions de
création qui retournaient toujours `nullptr`, alors que leurs appelants x86
utilisent le pointeur renvoyé par `006d6ab6`. Les versions remplacées sont
conservées dans `reports/reconciliation/`. Le répartiteur actuel encode la
liste de six arguments observée sur la pile et appelle le premier slot virtuel
de l'objet global `DAT_00895c84`. Un service de test vérifie l'ordre des
arguments et les pointeurs retournés. La valeur de l'objet du jeu et le
comportement réel de son allocateur restent inconnus.

Cinq fonctions du lot `float_ops_015` avaient été enregistrées comme intégrées
après une indisponibilité de ReAgent, avec des globales forcées à zéro ou une
logique provisoire. Elles ont été retirées du code et marquées `deferred` :
`00401000`, `00401270`, `004013a0`, `00401da0`, `00403190`. Le test
`005e8b50` a été corrigé après lecture de `UCOMISS/LAHF/TEST AH,0x44/JP` :
il rend 1 pour toute composante non nulle ou non ordonnée, et 0 seulement si
les trois composantes sont nulles.


Le groupe de projection utilise des enregistrements d'octets à offsets connus.
Les exports ne justifient pas encore une classe C++ propriétaire ni des noms
de champs métier. Les candidats ReAgent du lot `record_projection_006` et de
`projection_prereqs_007` ont été corrigés à partir du désassemblage ; leurs
statuts de validation ReAgent sont conservés séparément des compilations.

Pour `projection_dispatch_008`, les 256 octets de `DAT_0071b150` ont été lus
directement dans la mémoire du projet Ghidra `HarryPotter6` avec
`tools/read_ghidra_bytes.py`. La copie binaire est dans
`architecture/projection_format_table.bin`. `006d6c09` retourne des valeurs
entièrement définies entre 0 et `0x80` dans le désassemblage ; la branche
`0x100` de `006d7c07` est présente, mais semble inatteignable depuis ce
répartiteur. Elle reste dans la reconstruction pour préserver la structure.
Les tests vérifient la classification et une liste de deux nœuds ; ils ne
prouvent pas l'équivalence avec le binaire en exécution.

Les setters des lots `projection_flag_setters_009` et
`projection_decode_prereqs_010` ont tous le même motif x86 `DEC/NEG/SBB` :
seule la valeur 1 pose le bit ciblé, toute autre valeur l'efface. Chaque
adresse garde une fonction publique distincte, et l'écriture commune reste
privée à `RecordFlags.cpp`.

`projection_size_011` utilise la table extraite, les alignements et les
niveaux successifs du désassemblage. Le test de taille couvre des entrées
valides précises, sans prétendre résoudre les cas de débordement entier.
`projection_decode_012` encode les pointeurs internes en offsets 32 bits
relatifs, conformément aux instructions `SUB EAX,ECX`. Le prédicat
`006d81a4` n'expose que l'octet bas de retour, seul défini sur toutes les
branches et seul consulté par `006d7cd2`. Le premier test local de décodage
a révélé que `+0x28` sert aussi de limite de niveaux dans le calcul de taille ;
son attente a été corrigée avant validation.

`projection_constructors_014` ajoute cinq constructeurs d'enregistrements qui
s'appuient sur le sous-système projection déjà intégré. `006b938c` est identifié
comme `sprintf` (CRT) et déclaré comme dépendance externe.

- La cible analysée est x86. La construction MSVC `Win32` contrôle la syntaxe et
  les liens employés par le test, mais les fonctions `float10` exposées comme
  `long double` ne reproduisent pas le retour x87 80 bits sous MSVC.
- `DAT_007db234` est lue à l'adresse `0x007db234` par `005d1580`. L'export
  `_globals.json` ne donne pas sa valeur. `VectorWithGlobal.cpp` compile mais
  doit recevoir cette définition avant un lien qui utilise cette fonction.
- Aucun test présent ne compare les résultats au binaire exécuté. Le champ
  `equivalence` de toutes les fonctions reste `unverified`.

`projection_chain_clone_019` int?gre les tailles et copies de cha?nes de
clones (006d88d2, 006d86d5, 006d8816). Un test de deux enregistrements
v?rifie les offsets relatifs et les donn?es copi?es. La cat?gorie `0x100`
appara?t dans le contr?le de flux mais le classifieur 006d6c09 ne la produit
pas dans les branches observ?es ; son comportement reste non v?rifi?.
Les fonctions de remplacement 006d6ec3 et 006d6d39 restent diff?r?es.

## Lots pool_mid_031 et pool_sync_032b (17 octobre 2026)

ReAgent a produit six candidats pour `pool_mid_031`. Aucun n'a passé son
vérificateur ; les fichiers et verdicts restent dans `reports/pipeline/rejected/`
et `state.json`. L'intégration dans `src/runtime/PoolList` suit les exports
Ghidra et l'assembleur x86. La liste contient trois mots de 32 bits (premier,
dernier, compte) ; le lien à `nœud+8` désigne `suivant+8`. Les six adresses
`006b1d53`, `006b1ffc`, `006b2147`, `006b2223`, `006b2cbd` et `006b2d1d`
sont reliées dans `architecture/address_map.csv`. En particulier, `006b2147`
retourne son nœud dans EAX (utilisé à `006b2256`), bien que Ghidra l'annonce
`void`. `006b2d1d` ne garantit que AL à la sortie ; son interface C++ retourne
un booléen. `pool_list_smoke` vérifie la recherche, le détachement, le compteur,
le verrou et les codes d'état.

Pour `pool_sync_032b`, ReAgent a produit sept candidats, dont deux `PASS`.
Cinq fonctions sont intégrées : `006b1910` (jeton compteur/index), `006d928b`
(initialisation de section critique), `006d92b7` (événement auto-reset),
`006d92da` (SetEvent), `006d94f5` (priorité de thread). L'assembleur de
`006d94f5` écrit AL=1 sur tous les chemins, même si l'appel Win32 échoue ; le
retour C++ reflète cet octet défini. `006d95d0` reste différée tant que le
callback `006d9351` et sa durée de vie ne sont pas établis. `006f363c` est un
thunk d'import CRT vers `_beginthreadex` à l'IAT `007131d0`, suivi comme
dépendance externe. `pool_sync_smoke` vérifie le compteur, son bouclage,
l'événement et les appels de priorité. Compilation Win32 et 16 tests CTest
réussis ; aucune équivalence au binaire en exécution n'est encore démontrée.

L'orchestrateur accepte `--audit-integrated` pour demander à ReAgent les
candidats manquants des fonctions déjà intégrées, sans changer leur code,
statut de compilation ou équivalence. Il refuse désormais un candidat issu
d'un CLI en échec et arrête le lot sur limite d'utilisation LLM. Deux tests
Python vérifient ces transitions d'état.

Le rattrapage `--audit-integrated` du lot `stream02_base_026` a ensuite
produit cinq candidats ReAgent `PASS`. Une comparaison statique avec
`src/archive/Header.cpp` et les exports Ghidra n'a révélé aucune différence
matérielle de branche ou de valeur sur les entrées définies. Les cinq entrées
portent `reagent_audit=reviewed_compatible_static` dans `state.json` ; leur
champ `equivalence` reste `unverified` faute de comparaison avec le jeu lancé.

Le rattrapage du lot `archive_entry_027` a rencontré le thunk CRT `006b9774`
(`tolower`, saut indirect IAT `00713288`), désormais suivi comme dépendance
externe. ReAgent a rendu six candidats `PASS`, comparés statiquement à
`Entry.cpp`. Deux candidats sont rejetés : `006dc5c9` emploie ECX et EAX
comme arguments de registre non standards ; le candidat demande lui-même un
wrapper ABI. `006dc751` appelle cette fonction avec ECX = nom stocké et
EAX = nom recherché, confirmé aux instructions `006dc7cd` à `006dc7d2`.
L'intégration C++ existante garde deux arguments explicites et le pointeur
`curseur + 2*largeur` confirmé à `006dc7c7` ; elle n'a pas été remplacée.

Le rattrapage du lot `archive_wrappers_028` a ajouté le thunk CRT `006b9386`
(`strchr`, IAT `00713220`) au suivi des dépendances externes. Les cinq
candidats ReAgent `PASS` ont été comparés statiquement aux wrappers dans
`Entry.cpp`, puis marqués `reviewed_compatible_static`. Aucun source intégré
n'a changé pendant ces audits.

Le rattrapage `pool_leaves_029` a mis en évidence un défaut d'ABI dans
`006b1a2e` : l'assembleur charge ECX depuis frame+0x10 puis pousse le mot
frame+8 avant l'appel du slot virtuel +0x24. `PoolAccess.cpp` transmet
désormais l'objet et l'argument avec `__thiscall` sous MSVC x86 ; le test
utilise une vraie table virtuelle et les 16 tests passent. ReAgent a rejeté
ce candidat ainsi que cinq autres ; `006d9416` est `PASS`. `006b3b04`
reste différée : son `JMP [0x008516ac]` indirect ne livre pas de cible ni de
comportement supplémentaire dans le candidat ReAgent.

Le rattrapage `handle_registry_030` a identifié la fonction de bibliothèque
MSVC `006b92c8` (`__onexit`) comme dépendance CRT externe, abstraite par
`std::atexit`. ReAgent a rendu cinq candidats `PASS` et trois rejetés.
L'assembleur de `005dc710` passe l'adresse du holder en ECX avant l'appel
virtuel avec deux arguments sur la pile ; `HandleRegistry.cpp` passe maintenant
ce pointeur `this`. Dans `006d91f0`, l'index du tableau est calculé depuis
`0x00896498`, et `XCHG [ECX],EDI` impose une opération atomique d'échange ;
le code utilise cette base et `atomic_ref::exchange(seq_cst)`. Le chemin à
adresse fixe a été exercé dans `handle_registry_smoke` sur Win32. Le pointeur
null hors bloc retourne sans lecture ni appel. Compilation Win32 et 16 tests
CTest réussis ; l'équivalence au binaire lancé demeure non vérifiée.

## Lot pool_list_ops_033

Sept candidats ReAgent ont été obtenus et conservés après rejet par son
vérificateur. Les sept adresses `006b2c65`, `006b2ce9`, `006b2199`,
`006b2408`, `006b227b`, `006b2359` et `006b23b2` sont intégrées dans
`src/runtime/PoolList.cpp`. Les deux premières insèrent en tête ou après un
nœud avec un lien encodé vers `suivant+8`. `006b2199` trie sur l'octet +0x12
et les bits 8 à 23 du mot +0xc, puis signale l'événement à frame+0x44.
`006b2408` retire un nœud de la première liste, change cet octet et le remet
en file. Les trois wrappers invoquent respectivement le slot virtuel +0xc,
les slots +8 puis +0 (avec argument 1) et un callback C avec trois arguments.
Pour `006b2359`, les registres EDX:EAX de la première méthode forment le
retour 64 bits avant l'appel du slot +0. `pool_list_smoke` vérifie ces effets
avec une table virtuelle factice x86 et un vrai verrou Win32. Les 16 tests
CTest passent ; l'équivalence au jeu en exécution reste non vérifiée.

## Lot service_lifecycle_034

Cinq fonctions de destruction partagent `006b7d0f` et `006b492b` ; leurs
dépendances étaient entièrement incluses dans le lot ReAgent. Les adresses
`006b492b`, `006b7bcb`, `006b7cca`, `006b7d0f`, `006b7dcb` sont intégrées
dans `src/platform/ServiceLifecycle`. L'assembleur de `006b492b` charge
l'objet `DAT_00892df8` en ECX, sa table virtuelle en EAX, puis appelle le
slot +4. Le candidat ReAgent `PASS` avait omis ce niveau d'indirection ; la
version intégrée et le test le vérifient. `006b7d0f` appelle successivement
les slots +8 des pointeurs aux offsets +0x14, +8, +0xc, puis le slot +0xc du
pointeur à +4. Les variantes terminales libèrent 0x84 ou 0x1c octets si le
bit 0 du flag est posé. Un test MSVC x86 utilise de vrais objets virtuels et
vérifie l'ordre des appels. Compilation Win32 et 17 tests CTest réussis ;
équivalence au binaire en exécution non vérifiée.

## Lot service_lifecycle_035

Cinq autres fonctions du cycle de vie sont intégrées dans
`src/platform/ServiceLifecycleExtra.cpp` : `006dcbc7`, `006dcc40`,
`006dce80`, `006dcf14`, `006dd111`. Le second allocateur vient de
`DAT_0089adcc` et son slot virtuel +4 reçoit le bloc et la taille avec
l'objet en ECX. `006dcc40` traite les membres aux offsets +0xb8 et +0xb4 :
le premier appelle le slot 0 avec le flag 3 ou libère le mot précédent selon
son marqueur ; le second appelle le slot +8 avec le pointeur objet sur la
pile. `006dcf14` appelle d'abord le membre +0xb0, puis le membre +0x20 tant
que le compteur signé à +0xa0 reste positif ; les callbacks peuvent modifier
ce compteur. Les destructeurs terminaux libèrent 0xc0 ou 0xb4 octets suivant
le bit 0 du flag. Le test Win32 vérifie l'ordre des appels, la mise à zéro
et les tailles transmises. Compilation et 17 tests CTest réussis ; aucune
équivalence au binaire exécuté n'est affirmée.

## Lot service_variants_036

Cinq wrappers liés au même allocateur secondaire (`006de65f`, `006de70d`,
`006de7a1`, `006de7d0`, `006de7e5`) sont intégrés dans
`ServiceLifecycleExtra.cpp`. Les tables écrites à l'offset 0 et les tailles
0x24, 0x78, 0x464 et 4 viennent directement des instructions x86. La variante
0x78 écrit d'abord `PTR_FUN_0071b62c`, puis `006de7d0` pose
`PTR_FUN_0071b670` avant une éventuelle libération. Le test de handles
contrôle les tailles transmises au slot virtuel de l'allocateur et les tables
finales. Compilation Win32 et 17 tests CTest réussis ; l'équivalence au jeu
en exécution reste non vérifiée.

## Lot service_cleanup_037

Les cinq fonctions `006de735`, `006de449`, `006de5b0`, `006de5e1` et
`006de615` sont intégrées dans `ServiceLifecycleExtra.cpp`. `006de5b0`
relit le pointeur à +0x468 après le premier appel virtuel : le premier slot
est +0x20, le second +8. L'assembleur pousse explicitement le pointeur objet
sur la pile, contrairement au candidat ReAgent qui employait `__thiscall` ;
l'intégration utilise le contrat d'appel observé. `006de5e1` remet ensuite
les tables virtuelles et traite toute valeur résiduelle à +0x468 avant la
libération conditionnelle de 0x46c octets par `006de615`. Le test couvre les
deux branches où le premier callback annule ou conserve le pointeur. La
compilation Win32 et les 17 tests passent ; l'équivalence avec le binaire
lancé reste non vérifiée.

`window_shell_022` a int?gr? les appels Win32 de cr?ation et
d?enregistrement de classe de fen?tre (005d8ed0, 00ff8380, thunk 005d8e30).
Le passage implicite de x/y par ECX/EAX et l?instance en ESI sont des
param?tres explicites dans l?interface C++. Un test Windows r?el valide la
fen?tre cr??e ; le gestionnaire de messages 005d8a30 reste diff?r?.

`projection_update_size_023` ajoute les trois tailles ?l?mentaires et le
calcul de taille lors d?une mise ? jour (006d896e). Le candidat ReAgent de
ce dernier lisait +0x0c m?me dans les branches qui ne le lisent pas ; cette
lecture a ?t? d?plac?e dans les branches concern?es. Les modes 1, 2 et 3
sont v?rifi?s par un test sur une cha?ne de deux enregistrements.
L?orchestrateur saute d?sormais toute fonction d?j? int?gr?e, m?me si son
candidat ReAgent avait ?t? rejet? : le lot 023 avait temporairement ?cras?
l??tat de 006d8864 avant cette correction.

`projection_lookup_024` regroupe la travers?e par offset relatif
006d6c5c et quatre wrappers pour les types 2, 4, 0x10 et 0x20. ReAgent
a rejet? 006d6c5c ? cause des 24 bits hauts du retour du classifieur ;
les instructions x86 de 006d6c09 d?finissent EAX enti?rement sur chaque
chemin. Les wrappers pr?servent le pointeur dans EAX m?me si Ghidra les
a typ?s `void`. Un test parcourt cinq enregistrements cha?n?s.

`projection_chain_mutation_025` ajoute la seconde m?thode virtuelle du
service d?allocation (lib?ration, offset +4), ainsi que l?insertion, le retrait
et la lib?ration r?cursive d?une cha?ne. Les retours des fonctions de mutation
sont expos?s par leur octet bas, seul garanti par le d?sassemblage. Le test
couvre l?insertion, le retrait du dernier ?l?ment et le marquage du premier
?l?ment ; la suppression d?un ?l?ment suivi de descendants demande encore
une v?rification des contrats d?appel et d?ownership.
