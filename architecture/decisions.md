# Décisions et limites de reconstruction

## Lot 052 — opérateurs fichiers réservés (file_ops_052), 2026-10-18

- **Résolution du blocage « blocked »** : les 5 wrappers de
  `mixed_deps_060` étaient bloqués parce que le chaînage confondait
  *token* et *record*. L'asm tranche : les créateurs renvoient le **token
  compacté** (`index | (compteur+1)<<8`, confirmé octet par octet dans
  `006b1910`), mais les workers de fichier passent à l'étage suivant le
  **pair_lo** d'une étape précédente (paire lue par 006b2359), et le
  binaire déréférence `word(record+0xc)` pour trouver le slot moteur.
  `006b175b`/`006b1815`/`006b1783`/`006b17b7` renvoient la **paire
  64 bits**, le token n'entre en jeu que comme clef d'attente.
- `006b329a` → `construct_probe_node_00715af0` (`PoolList.cpp`) :
  thiscall RET 0x1c, vtable brute 0x715af0 ; attach moteur d'abord
  (006b20e8), record à +0x14, paires signées CDQ +0x28/+0x2c et
  +0x20/+0x24 entrelacées avec l'écriture de vtable, mot non signé +0x30
  en dernier, ordonnancement (006b2199) à la fin.
- `006b24f2` → `create_probe_node_from_record` (`PoolList.cpp`) : cdecl ;
  slot 0x40 (006b1d53) puis entonnoir décompilé
  `329a(slot, p1, p4, p2, p3, p5, p6, [p1+0xc])` ; pas de garde
  pool-vide ; répond `[slot+0xc]`.
- `006b17b7` → `create_and_wait_probe_node` (`PoolWait.cpp`) : paire
  finale (006b2359), comme 006b1815.
- Nouveau module `src/runtime/FileOps.{h,cpp}` (11 fonctions du lot) :
  - workers (cdecl, rappelés par 006b2553 avec réserve 0x64 et pointeur
    de contexte) :
    - `006d9f47` `open_file_reserved` : chaîne 175b(champ 1) → 1815 →
      descripteur 18 dwords hlsfile (6, 0xac, 5, chaîne 0x71b358, 7,
      &ctx[4], 1, chemin, drapeaux `(ctx4>>9)&2` et `(ctx4>>6)&4`,
      DAT_008513ac, …) → vcall thiscall callee-clean slot 0 de
      DAT_00851384 ; réponse ≠ 0 ⇒ 006b1783(réponse, rec2) puis 006b17e5,
      `ctx[2] = rec2`, retour de la réponse ; sinon 006b17e5 et 0.
    - `006da0c0` `write_file_reserved` : 175b(champ 1) →
      1783(0, ctx[1], ctx[2], mode réserve−1) → 17e5 ignoré ; retourne le
      pair_lo du repeat.
    - `006da462` `check_file_reserved` : 175b(champ **6**) → 17b7(0,
      ctx[1], ctx[2], mode réserve−1) → 17e5 ignoré ; SETZ sur
      `pair_lo == ctx[2]`.
  - wrappers (contexte pile EBP−0x14) : `006da081`
    `open_file_from_path` ({chemin, −, sortie, 0, arg} puis
    `*out = ctx[2]`), `006da10c`/`006da14e` `write_file_flagged/plain`
    (drapeau 1 / `AND [EBP-8],0`), `006da4b1`/`006da4f3`
    `check_file_flagged/plain` (idome DEC/NEG/SBB/INC = `résultat==1`).
- **Modèle du harnais étendu** : `fire_drain` passe en mode balayage
  quand `drain_node == nullptr` (complète la tête de la première liste
  pending vivante, se ré-arme à chaque pump via
  `wheel[5] = {callback, 96, période 0, échéance 0}`) — les chaînes
  multi-étages se drainent ainsi sans connaître les tokens à l'avance ;
  un `PathAllocator` à résultat forcé sert la réponse du rapport
  d'erreur. Piège à mémoire : le descripteur vit dans la frame *morte*
  du worker — il est copié **pendant** l'appel d'allocation (repéré par
  la longueur), tout accès depuis main après retour lit une mémoire
  écrasée par la machinerie d'appel.
- mixed_deps_060 marqué superseded (ses 5 adresses vivent désormais en
  file_ops_052, leurs dépendances externes 006b2553/006b25b8 intégrées).

## Lot 051 — franges du pool (pool_fringes_051), 2026-10-18

- `006b1815` → `create_and_wait_plain_node` (`src/runtime/PoolWait.cpp`) :
  chaîne créer (006b2527, sans mots d'attach) → attendre (006b22c3) →
  paire finale (006b2359) ; **retourne la paire elle-même** (contrairement
  à 006b17e5 qui répond un booléen) — entonnoir vérifié dans l'asm
  (`PUSH 0; PUSH [EBP+0xc]; PUSH [EBP+8]`).
- `006b2dde` → `queue_remove_006b2dde` (`src/runtime/ListQueue.cpp`) :
  retrait thiscall (RET 8) d'un nœud précis de la FIFO
  { tête +0, queue +4, compteur +8, lien à node+0 } — complète la famille
  push/pop du complément lot 039. Piège asm : la marche ne re-déréférence
  **jamais** null — `[previous]==0` est testé AVANT (2e15 / 2e21) ; une
  première écriture avec `previous = next` puis read crashait (voir test).
  La queue/l'info de précédent peut venir de `param_2` (hint) ou de la
  tête si le hint est nul.
- `006b45ae` → `reset_wrapper_block_006b45ae` (`src/runtime/WrapperStubs.cpp`)
  : fastcall, reset octet-exact du bandeau d'état 0x16 octets de la
  famille wrapper : `OR [ECX],-1; OR [ECX+4],-1`, dwords +8/+0xc/+0x10 et
  octets +0x14/+0x15 à zéro.
- `006b1a59` → `set_game_search_paths` (`src/runtime/PoolPath.cpp`) :
  installe la liste `;`-séparée des chemins — `clear_buffer` (006b3b0e, le
  **raz** une fois pour la table `root+0x88` (DAT_00851390<<2) et une fois
  pour le tampon `root+0x8c` (DAT_00851394)) ; copie la chaîne, pose
  `array[0] = tampon`, découpe au thunk strchr CRT (006b9386) : chaque `;`
  devient un terminateur (slash/barre antérieure effacée) et le jeton
  suivant est posé dans le slot suivant ; le dernier jeton reçoit le même
  traitement. Les globales de comptage ne sont jamais écrites.
- **Découverte d'enregistrement** : `006b3b0e` avec 2 arguments dans cet
  export est bien `clear_buffer(ptr, size)` (déjà intégré en
  volume_registry_044) et non memcpy — l'hypothèse « 2 pushes = memcpy »
  était fausse et corrigée avant l'intégration.
- Couverture : `queue_remove` (médiat/tête/queue/clef absente),
  reset wrapper (32-bit + stérilité des octets 0x16+), lot d'attente plain
  via le drain unique de la roue, et le splitter de chemins (têtes,
  jetons, stérilité au-delà de la zone utilisée), avec restauration de
  `root+0x88` pour le jeu de tests 047.

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

## Lot stream_priority_051

Les sept adresses du lot sont desormais integrees dans src/stream. 006dc17c
et 006dbb17 forment l'enchainement de niveau d'un en-tete STRM : le premier
ecrit le mot +0x50 et declenche le second quand la comparaison au seuil
+0x58 change de cote ; le second ecrit la cible de franchissement +0x54 et,
si l'en-tete est actif (+0x44 == 1), fait remonter la priorite du noeud de
file +0x184 a l'octet +0x4c via hp6::runtime::update_node_priority. ReAgent
a rejete ces deux candidats (score FAIL malgre un verdict objectif PASS) sur
deux points : l'usage de decalages bruts au lieu de membres nommes, deja le
style etabli de ce fichier pour cette structure non resolue, et un ordre
d'evaluation non garanti entre deux lectures independantes et sans effet de
bord (+0x4c et +0x184) qui ne change pas le resultat observable. Les deux
candidats sont retenus apres cette revue.

006f4ffd et 006f4eb9 sont des cibles de saut de queue (PUSH EBP; MOV
EBP,ESP; POP EBP; JMP) qui transmettent leurs deux arguments sans
modification vers 006dc17c ; elles sont integrees comme de simples
redirections nommees par adresse, a l'image des bouchons de
src/runtime/WrapperStubs.cpp.

006e0778, 006e3f71 et 006e71e1 forment un nouveau module,
src/stream/ChannelRegistry.cpp : une table de 256 entrees (DAT_0089b200,
bornee par l'octet DAT_0089b2fe) resolue par index, une recherche de noeud
par identifiant complet dans la liste chainee du compartiment selectionne
par l'octet bas de l'identifiant (entree+0x120), et une application de
niveau qui resout l'entree de table vers son emplacement de flux (entree+0)
avant d'appeler la chaine 006f4eb9/006dc17c. Le candidat ReAgent de 006e3f71
a ete rejete pour sa chaine de reinterpret_cast sur des decalages bruts ; le
candidat est retenu car la lecture de pointeurs correspond exactement a
l'assembleur exporte (006e3f90-006e3fa6).

Le nouveau test stream_priority_smoke couvre le franchissement de seuil
actif et inactif, les deux bouchons de redirection, et la chaine complete du
registre de canaux. L'ajout de l'appel a update_node_priority depuis
Metadata.cpp tire desormais PoolList.obj/PoolAccess.obj (et, par leur
intermediaire, Sync.obj et ThreadPriority.obj) dans tout binaire qui
reference stream::Metadata ; tests/core_smoke.cpp recoit le meme jeu de
definitions inertes que tests/pool_list_smoke.cpp pour rester linké.
Compilation Win32 et les 22 tests CTest reussissent ; l'equivalence au
binaire en execution reste non verifiee. Les 262 fonctions suivies sont
desormais integrees (0 bloquee, 0 en attente de revue, 8 differees).

## Lot pool_deps_052

006b2673 est integre dans src/runtime/VolumeRegistry.cpp : il resout le
volume servant un chemin (find_volume_by_name), puis saute sur le slot
+0x2c de la table virtuelle de l'objet du record (record+0x84), en
thiscall sans argument de pile. L'assembleur exporte montre un JMP
[EAX+0x2c] precede d'un POP EBP, donc un veritable appel terminal dont
l'epilogue de la cible renvoie directement a l'appelant de 006b2673 ; le
candidat ReAgent a ete rejete pour avoir rendu cela par un appel suivi
d'un retour et pour avoir indexe la table par sizeof(void*). Les deux
points sont evites en reprenant le style d'acces par decalage d'octet deja
en usage dans ce fichier. Aucune verification de nullite sur le record
resolu, comme dans le binaire. Le test volume_registry_smoke reutilise la
fixture VolumeNode existante (slot vb() a +0x2c) pour verifier l'appel.

Les cinq autres candidats du lot (006d9eea, 006d9f1a, 006da03f, 006da3f9,
006da42f) restent differes. Ce sont des wrappers cdecl qui resolvent un
volume puis lancent hp6::runtime::run_reserved_callback (valeur reservee
100) avec un pointeur de fonction de rappel passe par adresse plutot
qu'appele directement ; cette reference n'apparait donc pas dans les
callees suivis par function_map.jsonl et n'a pas ete detectee par
select_batch.py. Le rappel de 006d9eea/006d9f1a (006d9eac) est une
composition simple de fonctions deja integrees
(create_and_wait_bounded_path_node -> create_and_wait_plain_node ->
create_and_wait_node), mais verifier cette chaine demanderait un drainage
generique sur plusieurs attentes imbriquees au sein d'un seul appel, ce que
le harnais de test actuel (lot pool_wait_043, mecanisme a un seul coup) ne
permet pas sans extension disproportionnee. Les rappels de 006da03f/
006da3f9/006da42f (006d9f47 et 006da296) tirent en plus un mecanisme
d'assertion/rapport non resolu (table virtuelle DAT_00851384, enregistrement
source fixe hlsfile.cpp) qui merite sa propre investigation. Les deux
paires (006d9eea/006d9f1a et 006da3f9/006da42f) sont par ailleurs
observablement identiques entre elles : le seul mot de pile qui les
distingue n'est jamais lu par leur rappel commun.

Compilation Win32 et les 22 tests CTest reussissent. 263 des 276 fonctions
suivies sont integrees (0 bloquee, 0 en attente de revue, 13 differees).

## Lot mixed_deps_053

select_batch.py a regroupe ces six adresses sur le seul critere d'un
callee partage, ici memcpy (006b9730) ; le lot n'est donc pas une unite
fonctionnelle, contrairement aux lots precedents groupes par structure
reelle. Les six sont neanmoins integrees dans le nouveau fichier
src/runtime/ScratchBuffers.cpp : quatre allocateurs/copieurs de tampon
independants et une fonction qui compose deux d'entre eux.

ReAgent a obtenu un verdict FAIL sur quatre candidats malgre un
objective_verdict PASS et un parity_status green sur les quatre :
- 00422e20 (store_payload) : le reviseur n'a pas pu confirmer
  independamment la polarite des deux branches signees a partir du
  P-code/CFG seul. La relecture directe de l'assembleur expose (JGE
  0x00422e34 sur taille>=0x800, JGE 0x00422e62 sur taille>=0x2400)
  confirme la polarite du candidat.
- 006e77cb (advance_cursor_copy) : reproche purement cosmetique (l'ordre
  de calcul de param_2*4 avant plutot qu'apres le memcpy, sans effet de
  bord entre les deux lectures).
- 01003ca0 (append_to_cursor) : le reviseur a juge le double
  dereferencement de pointeur superflu ; l'assembleur expose (MOV
  EAX,[DAT_00cf1a08] ; CMP [EAX],0) effectue exactement ces deux lectures.
- 01002890 (init_payload_record) : seul reproche, les decalages de
  l'enregistrement source ne sont pas nommes (deja le style etabli de ce
  fichier pour une structure non resolue).

bump_allocate (0063b850, PASS) est un allocateur lineaire sur l'arene
pointee par DAT_00cf18e4 ([0]=base,[1]=capacite,[2]=courant) : le champ
courant est toujours avance avant la verification de depassement, et un
depassement ecrit a l'adresse 0 sans recuperation (piege fatal de
depassement de capacite). Cette ecriture est reproduite fidelement ;
scratch_buffers_smoke declare _DAT_00000000 comme une variable globale
ordinaire (la reconstruction ne peut pas placer de variable a l'adresse
litterale 0) et verifie que le piege s'est bien declenche plutot que de
l'eviter. init_payload_record compose bump_allocate avec deux copies
conditionnelles vers des tampons fixes (_Dst_00bb0a70, _Dst_0096b490) ;
seul l'octet bas du retour expose par l'assembleur est defini (AL=1), le
reste etant un residu non garanti d'EAX, reproduit ici par un simple 1.

Compilation Win32 et les 23 tests CTest reussissent du premier coup.
269 des 282 fonctions suivies sont integrees (0 bloquee, 0 en attente de
revue, 13 differees).

## Lot mixed_deps_054 : premier module de rendu

Ce lot ouvre src/render/, le premier module touchant le moteur de rendu
(les chaines D3DX9 Shader Compiler confirment un moteur Direct3D 9 dans
le binaire). RenderRegionManager.cpp integre assert_region_capacity
(0063cd60, garde sans effet observable propre) et
set_active_render_region (0063d0f0, FAIL ReAgent leve : le reviseur
reprochait l'absence de [[noreturn]] sur le diagnostic, sans consequence
puisque rien ne suit l'appel dans aucune des deux fonctions).
ShaderManager.cpp integre allocate_shader_id (01003ea0, PASS) et
free_shader_id (01003f20). Le candidat ReAgent de ce dernier laissait la
branche fatale retomber dans la comptabilite de la pile de liberation,
contrairement a l'assembleur expose (les deux chemins convergent sur la
meme epilogue unique) ; le verificateur objectif l'a releve
independamment ("P-code has 2 returns, candidate has no explicit
return"). La fonction a ete reconstruite directement depuis l'assembleur
avec le retour anticipe correct.

DisplayMode.cpp integre report_mode_option (01003650), seule fonction du
lot qui n'est pas un garde-fou fatal : son message
"ModeOption:%d:%d by %d (%d bit)" est une ligne de journal
d'enumeration de mode d'affichage, plafonnee aux dix premiers. Ghidra
sous-compte ses parametres (signature void(void) alors que la branche lit
[EBP+8]/[EBP+0xc]), le meme defaut que 01002890 ; les deux mots sont
restitues comme largeur et hauteur.

hp6::platform::format_diagnostic_message (0063ccf0, nouveau fichier
src/platform/Diagnostics.cpp) formate ses arguments dans un tampon de
2048 octets local puis l'abandonne ; Ghidra le marque noreturn mais son
assembleur expose est un RET ordinaire sans appel d'arret. Chaque
appelant integre ici n'a aucun code apres l'appel, donc l'ecart avec
l'etiquette noreturn n'a aucun effet observable.

Ce lot corrige aussi une speculation erronee de l'integration precedente
(mixed_deps_053) : le commentaire de DAT_00866368 dans
src/runtime/ScratchBuffers.cpp le decrivait comme "shared table/vtable
constant" ; report_mode_option l'imprime avec %d, confirmant une simple
valeur entiere plutot qu'un pointeur de table virtuelle. Le commentaire
est corrige ; aucun comportement ne change.

Compilation Win32 et les 24 tests CTest reussissent. 275 des 288
fonctions suivies sont integrees (0 bloquee, 0 en attente de revue, 13
differees).

## Lot registry_settings_055 : bascule vers le chemin critique de demarrage

A partir de cette session, la selection de lots cesse de suivre l'heuristique
de select_batch.py (callee partage) : elle suit desormais la chaine d'appels
reelle de demarrage, documentee en detail dans architecture/startup_path.md.
005d9300 est identifiee comme WinMain ; sa decompilation complete montre
qu'elle lit une vingtaine de reglages depuis le registre
(HKCU\...\Harry Potter and the Half Blood Prince\GameSettings) avant de
creer la fenetre et le device.

Le lot cible les six fonctions de lecture/ecriture de registre
(005d7d40, 005d7810, 005d7ae0, 005d73c0, 005d7650, 005d6d80) plus un thunk
simple (005d7d00). Trois nouvelles dependances externes ont ete necessaires
pour que le plan se resolve : _CxxThrowException (006f51a2, mecanisme de
lancement d'exception C++ MSVC) et deux fonctions internes de
std::basic_string (reservation/croissance, 005da7c0/005da820) ; les trois
sont enregistrees comme dependances externes au meme titre que sprintf/
memcpy/tolower/strchr.

Seul 005d7d00 (read_registry_value, nouveau fichier
src/platform/RegistryConfig.cpp) est integre : un RegQueryValueExA plafonne
a 0xff octets, verifie par un aller-retour reel sous une sous-cle HKCU
jetable. Les six autres sont differees : le verificateur objectif de
ReAgent a releve de vrais ecarts de flux de controle et de nombre d'appels
(par exemple 10 branches contre 5, 22 appels contre 18 pour 005d7d40 ; 12
branches contre 6 pour 005d7650), pas de simples remarques stylistiques
comme dans les lots precedents. Ces fonctions combinent le mecanisme SEH
des exceptions C++ MSVC 2008 et la disposition interne de std::basic_string
(28 octets, tampon court integre) ; leur reconstruction fidele demandera une
lecture directe de l'assembleur plutot qu'une nouvelle tentative ReAgent.

En descendant la chaine de 005d9300, deux decouvertes importantes sans
nouvelle integration de code : FUN_005d8fc0 est la vraie boucle de messages
Win32 (PeekMessageA/TranslateMessage/DispatchMessageA, detection WM_QUIT,
limitation de frequence), et chaque iteration commence par FUN_0064aa60,
confirme comme le gestionnaire standard de device Direct3D 9 perdu/
reinitialise (DAT_00cf1944 est un IDirect3DDevice9*, les appels aux slots
+0xc et +0x40 sont TestCooperativeLevel et Reset, les codes de retour
compares correspondent exactement aux HRESULT D3DERR_DEVICELOST/
D3DERR_DEVICENOTRESET/etc.). FUN_0064a810, appelee quand le device est
valide, est la piste la plus probable vers le rendu d'une frame et donc
l'affichage de l'intro et du menu ; elle n'a pas encore ete examinee.

Compilation Win32 et les 25 tests CTest reussissent. 279 des 298 fonctions
suivies sont integrees (0 bloquee, 0 en attente de revue, 19 differees).

## Lot render_reset_056b : recreation des shaders apres reset du device, et
## une correction systematique de convention d'appel COM

Huit fonctions integrees dans src/render/DeviceReset.cpp : deux triviales
(00650d80 no-op, 0063b4a0 calcul arithmetique pur) et six formant la chaine
de recreation des ressources Direct3D 9 apres un device reset, toutes
appelees depuis FUN_0064a810/FUN_0064aa60 documentees dans
architecture/startup_path.md.

Six des huit candidats ReAgent ont ete rejetes ou invalides, mais pour deux
raisons differentes cette fois :

1. **Une vraie erreur de convention d'appel, systematique.** Chaque appel
   virtuel Direct3D 9 observe dans ce lot (00648e10, 006525d0, 00653430)
   pousse explicitement le pointeur device/objet sur la pile juste avant
   l'appel indirect, au lieu de le garder dans ECX — confirme a chaque fois
   dans l'assembleur expose. C'est la convention STDMETHODCALLTYPE standard
   des interfaces COM (__stdcall sur x86), differente du __thiscall que ce
   projet utilise partout ailleurs pour les tables virtuelles internes au
   jeu (ServiceLifecycle, PoolList, VolumeRegistry). Les trois candidats
   ReAgent avaient declare ces appels en __thiscall ; les trois ont ete
   corriges en __stdcall.

2. **Un vrai bug dans le candidat de 006534a0** (release_shader_if_present) :
   ReAgent avait caste l'appel via void(***)(int*), lisant un niveau de
   pointeur de trop. Son propre reviseur objectif l'a signale
   independamment ("the P-code reads the function pointer directly from
   *param_2+8, candidate added one more deref"). Reconstruit directement
   depuis l'assembleur avec les deux lectures exactes (vtable puis
   methode).

Les offsets de table virtuelle se confirment mutuellement : +0xc et +0x40
(deja vus sur le device dans FUN_0064aa60) restent TestCooperativeLevel et
Reset ; +0x68 correspond a CreateVertexBuffer (index 26), +0x16c a
CreateVertexShader (index 91), +0x1a8 a CreatePixelShader (index 106), et
+8 sur un objet quelconque reste Release (index 2) — coherent avec la
disposition standard de IDirect3DDevice9 et confirme par les arguments
passes a chaque appel (ex. Length/Usage/FVF/Pool/ppVertexBuffer/
pSharedHandle pour CreateVertexBuffer).

reset_dynamic_resources (00642420) recree toujours les shaders de la petite
table DAT_00a68820, et recree en plus la table de 0x600 entrees
(DAT_00f38b10, 0xc4 octets/entree) quand DAT_00cf19bc > 2. Le test
device_reset_smoke a d'abord echoue par segfault : il reutilisait les
memes emplacements DAT_00a68820 entre deux appels sans les re-semer avec de
vrais objets valides, alors que recreate_vertex_shaders libere
inconditionnellement (sans verification de nullite, fidele au binaire) ce
qui s'y trouve — corrige en re-semant les emplacements avant chaque appel.

Compilation Win32 et les 26 tests CTest reussissent. 287 des 306 fonctions
suivies sont integrees (0 bloquee, 0 en attente de revue, 19 differees).

## Lot caps_table_057b : profil de render states par defaut, et une
## convention d'appel non standard (parametre passe par ESI)

Huit fonctions integrees dans src/render/RenderStateCache.cpp (nouveau
fichier) et deux de plus ajoutees a src/render/DeviceReset.cpp.

set_render_state_cached (0064c300, SetRenderState, device vtable+0xe4,
index 57) et set_texture_stage_state_cached (0064c3b0,
SetTextureStageState, vtable+0x10c, index 67) suivent le meme schema de
cache deja vu : un tableau indexe par etat (ou par stage*0x20+type) evite
l'appel D3D9 quand la valeur ne change pas. Les deux candidats ReAgent
etaient corrects sauf la meme erreur de convention d'appel que le lot
precedent (__thiscall au lieu de __stdcall).

initialize_default_render_state (00640fb0) est la plus grosse fonction
integree a ce jour par nombre de lignes : 143 affectations litterales
remplissant un profil de render/texture-stage states par defaut, verifiees
une a une par comparaison programmatique avec le decompile expose avant
toute integration (script jetable, pas de relecture a l'oeil sur un bloc
de cette taille). Son appel final a "thunk_FUN_010021f0()" sans argument
visible cachait en realite un parametre transmis par ESI (non une
pile) : 00640fb0 charge explicitement ESI avec l'adresse de son propre
tampon de 19 mots (&_Dst_00881370, rempli par le memset 0xcd puis par les
affectations qui precedent l'appel) juste avant l'appel, confirme par
l'assembleur expose (aucune autre ecriture dans ESI entre les deux). Cette
fonction est rendue explicite comme apply_render_state_profile(const
uint32_t* settings) plutot que de dependre d'un registre non documente.

apply_render_state_profile (010021f0, thunk 006406a0) applique 12 render
states et 6 texture-stage states (stage 0) depuis le tampon recu, puis
copie ses 19 mots dans des globals fixes qui, pour cet unique appelant
reconstruit, sont exactement le meme tampon qu'il vient de lire —
confirme sans effet observable pour ce site d'appel, et fidelement
reproduit pour tout autre appelant qui serait decouvert plus tard avec un
tampon different.

cache_available_texture_memory (0064aa30, GetAvailableTextureMem, device
vtable+0x10, index 4) et adjust_texture_quality_for_memory (0063b4c0,
thunk vers 01011a50) completent la chaine d'adaptation qualite/memoire
amorcee par FUN_0064aa60. Le candidat ReAgent de 0064aa30 inversait la
seconde moitie de sa condition de garde ; la relecture directe de
l'assembleur expose (JZ puis JA 0x20) confirme que l'appel a lieu pour
0 < valeur <= 0x20, comme le decompile d'origine et non comme le
candidat rejete.

Compilation Win32 et les 27 tests CTest reussissent. 295 des 314 fonctions
suivies sont integrees (0 bloquee, 0 en attente de revue, 19 differees).

## Lot strncpy_leaves_058

Six petites fonctions de construction/effacement d'enregistrement
integrees dans un nouveau fichier, src/runtime/RecordDescriptor.cpp,
sans lien identifie avec le chemin de demarrage. clear_quad_word,
clear_full_record/construct_full_record et clear_named_record/
construct_named_record forment une famille coherente (deux variantes
d'enregistrement, l'une avec quatre champs entiers supplementaires et un
octet terminal, l'autre sans) ; initialize_object_name_header est non
liee, operant sur des decalages differents (+0xb4 et +0x4xx) d'un
enregistrement non identifie. Les cinq candidats rejetes ne l'etaient
que pour des decalages bruts (style etabli pour ces structures non
resolues) ; le candidat de 006b6833 a echoue a la validation et a ete
reconstruit directement depuis l'assembleur expose.

Compilation Win32 et les 28 tests CTest reussissent. 301 des 320
fonctions suivies sont integrees (0 bloquee, 0 en attente de revue, 19
differees).

## FUN_0064a810 : resolution complete, hors pipeline ReAgent

Reconstruite directement depuis l'assembleur expose (pas de lot ReAgent,
adresse ajoutee manuellement a reports/pipeline/state.json), integree
dans un nouveau fichier src/render/RenderTargetSetup.cpp separe de
DeviceReset.cpp. L'hypothese initiale documentee dans
architecture/startup_path.md (DAT_00cf1948 = IDirect3D9*, +0x14 =
GetAdapterIdentifier) etait fausse : la correspondance sur le nombre
d'arguments etait une coincidence. La bonne lecture, confirmee par
recoupement systematique contre le d3d9.h reel (Wine, ordre de
declaration COM standard) : DAT_00cf1948 = IDirect3DSwapChain9*, et la
chaine d'appels est GetBackBuffer -> GetDesc -> CreateDepthStencilSurface
-> [CreateTexture -> Release -> GetSurfaceLevel] -> SetRenderTarget ->
SetDepthStencilSurface -> initialize_default_render_state. Chaque offset
(+0x14, +0x30, +8, +0x74, +0x5c, +0x48, +0x94, +0x9c) correspond
exactement a l'index de methode et au nombre d'arguments de la vraie
interface. Place dans son propre fichier pour eviter qu'un test qui
n'exerce pas cette fonction precise doive fournir les ~130 globals du
profil de render states par defaut (RenderStateCache.obj) qu'elle tire
transitivement via initialize_default_render_state.

Compilation Win32 et les 28 tests CTest reussissent (render_state_cache_smoke
couvre trois scenarios : rendu-vers-texture desactive, chemin normal avec
creation de texture, et blocage par bit de capacite). 302 des 321
fonctions suivies sont integrees (0 bloquee, 0 en attente de revue, 19
differees).

## Lecteurs/ecrivains de registre : disposition std::basic_string MSVC 2008
## confirmee, cinq fonctions differees integrees

Les six fonctions differees du lot registry_settings_055 sont resolues
(src/platform/RegistryConfig.cpp), hors pipeline ReAgent (reconstruction
directe depuis l'assembleur expose). 005d6d80 n'a plus de contrepartie :
remplacee par un simple `throw std::runtime_error`.

Disposition confirmee de std::basic_string<char> sous VC2008 : en-tete de
debogage d'iterateur de 4 octets (_Container_base, ignorable dans la
reconstruction), puis union _Bx de 16 octets (tampon SSO ou pointeur
heap), _Mysize a +0x14, _Myres a +0x18 ; taille totale 0x1c = 28 octets,
exactement ce qui est observe partout dans ce binaire pour les
basic_string locaux. Le decalage de +4 par rapport a la disposition
"manuel" s'explique par cet en-tete de debogage, present par defaut sous
VC 2005/2008 meme en Release. Cette disposition n'a pas besoin d'etre
reproduite : un std::string moderne (.c_str(), .empty(), +=, affectation)
est comportementalement equivalent partout.

005d7650 manipule l'allocateur interne de la STL pour agrandir son
tampon sur ERROR_MORE_DATA ; remplace par un std::vector<char>
redimensionne de la meme facon. 005d7ae0 recoit la valeur a ecrire via
un argument de pile cache que Ghidra n'a pas nomme comme parametre
formel, retrouve en tracant le site d'appel dans la branche de
promotion HKLM->HKCU de 005d7d40.

Compilation Win32 et les 28 tests CTest reussissent (registry_config_smoke
couvre l'aller-retour lecture/ecriture, le defaut sur absence, l'agrandissement
de tampon et l'exception sur cle vide). 307 des 321 fonctions suivies
sont integrees (0 bloquee, 0 en attente de revue, 14 differees). Voir
architecture/startup_path.md pour le detail complet.

## thunk_FUN_0100e6e0 : bien plus simple que redoute

Contrairement a l'hypothese initiale ("init/desinit D3D"), 0x0100e6e0
(atteinte via le thunk 0x005d8080) se limite a trois ecritures de
reglages d'affichage au registre, maintenant accessibles grace a
write_int_setting (lot precedent). Integree comme
hp6::platform::save_display_options (src/platform/RegistryConfig.cpp) :
ecrit OptionResolution=DAT_00cf19a4, OptionLOD=(DAT_008663bc!=-1 ?
DAT_008663bc : DAT_008663b8), OptionBrightness=DAT_008663c0, dans
HKCU\Software\Electronic Arts\Harry Potter and the Half Blood
Prince\GameSettings.

Cette fonction ecrit sous le vrai chemin de registre du jeu (le nom de
l'application est compile en dur, pas un parametre) ; le test enregistre
et restaure les valeurs OptionResolution/OptionLOD/OptionBrightness deja
presentes avant de s'executer, sans effet net sur une installation
reelle. Verifie en pratique : la machine de developpement avait une
cle GameSettings existante avec des valeurs reelles (OptionResolution=2,
OptionLOD=2, OptionBrightness=5, SizeX/SizeY=1920x1080), intactes apres
le test.

Compilation Win32 et les 28 tests CTest reussissent. 309 des 323
fonctions suivies sont integrees (0 bloquee, 0 en attente de revue, 14
differees).

## Le "frame pump" du message loop (0x005e0cc0) et ses helpers

Suite logique vers l'intro/menu depuis le message loop (0x005d8fc0, pas
encore reconstruite) : la fonction appelee a chaque iteration ou
PeekMessageA ne rapporte aucun message en attente. Resolue entierement
par trace de l'assembleur brut (pas seulement du C decompile), ce qui a
permis de clarifier plusieurs astuces du decompilateur :

- Les acrobaties SBORROW4/CARRY4 pour le delta de temps et son
  accumulateur 64 bits ne sont que la facon dont Ghidra decompose une
  soustraction/addition 64 bits en morceaux de 32 bits ; traduites en
  arithmetique native `int64_t`, elles deviennent une simple
  comparaison/clamp (`if (delta > 0x640000) delta = 0x640000;`), sans
  risque de mal interpreter les cas de bord signes.
- `extraout_ST0` dans FUN_005e0a30 n'est PAS un parametre cache transmis
  par l'appelant (contrairement a `unaff_EDI`, qui lui est un vrai
  parametre cache confirme par `LEA EDI,[ESP+0x18]` juste avant l'appel) :
  c'est un artefact du decompilateur qui n'a pas su suivre un FILD local.
  Le FNSTCW/OR 0xc00/FLDCW autour du FISTP force explicitement la
  troncature (RC=11, round toward zero) pour cette seule conversion, pas
  l'arrondi au plus proche : traduit simplement par un cast C++
  (`static_cast<int>(...)`, qui tronque deja vers zero).
- FUN_005e0b10's boucle do-while dont la condition compare un pointeur a
  l'adresse litterale de DAT_00d644f8 est un artefact de layout (ces
  globales sont adjacentes en memoire dans le binaire original) : la
  boucle fait en realite exactement 2 iterations fixes, reconstruites
  comme telles plutot que via une comparaison de pointeurs qui n'a plus
  aucun sens dans une reconstruction moderne.
- DAT_0089eb0c (l'objet "simulate"/"render" actif) est confirme
  **__thiscall** par l'assembleur brut : `MOV ECX,[0x89eb0c]` juste avant
  chaque `CALL`, ECX jamais pousse sur la pile. Son type concret n'est
  pas identifie ; traite comme une interface opaque (style maison deja
  utilise pour les vtables non resolues), a affiner plus tard — decision
  prise explicitement avec l'utilisateur, qui a choisi de continuer sur
  cette base plutot que d'arreter la session.
- DAT_00fc7f90 (l'objet de repli, appele quand ni simulate ni render ne
  s'est declenche) est un objet a adresse fixe (pas un pointeur vers un
  objet) : sa propre valeur EST son pointeur de vtable, et son `this` est
  `&DAT_00fc7f90` lui-meme — confirme par `MOV ECX,0xfc7f90` (l'adresse
  litterale, pas sa valeur) juste avant l'appel.

Integrees dans `src/runtime/FrameClock.cpp` :
- `multimedia_timer_elapsed_ms` (0x005e0b90)
- `accumulate_simulation_time_sample` (0x005e0a30, parametre EDI rendu
  explicite)
- `interpolate_render_time` (0x005e0b10)
- `drain_pending_visual_effects` (0x005ff790)
- `run_frame_tick` (0x005e0cc0)

**Decouverte separee, non resolue** : `drain_pending_visual_effects`
depend de 0x00606b00, un sous-systeme de declenchement d'effets visuels
("BLOOM"/"GLASS"/"BACKDROP", bit-manipulation profondement dependante de
structures non identifiees, ~100 lignes). Ampleur comparable aux deux
blocages precedents (objet COM, ABI std::basic_string). Non reconstruite ;
tracee "differee" dans reports/pipeline/state.json. Elle est injoignable
dans la couverture actuelle du projet : rien dans `src/` n'ecrit encore
DAT_00cef790 (la tete de la file d'effets en attente), donc le corps de
`drain_pending_visual_effects` qui en depend reste du code mort — verifie
par recherche exhaustive dans `src/` avant integration.

Nouveau test `tests/frame_clock_smoke.cpp` (29e test CTest) : verifie le
minuteur multimedia (monotone), l'accumulation + interpolation (deux
echantillons synthetiques, interpolation exacte a mi-chemin), et le
dispatcher complet (etat initial a zero declenche simulate ET render des
le premier appel, jamais le repli).

Compilation Win32 et les 29 tests CTest reussissent. 314 des 329
fonctions suivies sont integrees (0 bloquee, 0 en attente de revue, 15
differees).

## DAT_0089eb0c et DAT_00fc7f90 identifies : ni l'un ni l'autre ne mene a l'intro

Recherche de xrefs en direct sur le projet Ghidra existant (pyghidra,
lecture seule) pour resoudre les deux objets diffuses par
`run_frame_tick`. Resultat concret mais negatif pour la piste de
l'intro/menu : voir `architecture/startup_path.md` pour le detail
complet (adresses, assembleur, raisonnement). En resume :

- `DAT_0089eb0c` est ecrit une seule fois dans `FUN_005dcdf0` (appelee
  depuis WinMain) et ne change plus jamais. Sa table virtuelle (3
  methodes, jamais detectees comme fonctions par l'analyse automatique
  de Ghidra car atteintes uniquement par cet appel indirect) est un
  compteur de delai + overlay de diagnostic ("Build: FINAL on ... at
  ..."), pas la logique du jeu.
- `DAT_00fc7f90` (repli de `run_frame_tick`) pointe vers un sous-systeme
  lie a l'audio (meme symbole `PTR_PTR_008813c8` que la fonction qui
  hash la chaine "SSGAudio").
- Le focus-change dans la boucle de messages ne gere que
  l'acquisition/desacquisition DirectInput/XInput.

La vraie machine a etats intro/menu/jeu n'est pas visible depuis le
voisinage immediat de la boucle de messages. Trois chantiers ouverts de
taille comparable aux blocages precedents, sans signal fort pour
departager : les 8 sous-systemes encore inexplores appeles depuis
`FUN_005dcdf0`, le systeme de gestionnaires de messages nommes
(`FUN_005dae30`, "iMsgStartSystem"/"iMsgStopSystem"), et `0x00606b00`
(effets visuels, deja differee). Aucune integration de code cette
section ; exploration documentaire uniquement.

## Le framework "GOF" et sa machine a etats (piste intro/menu, non resolue)

Exploration en direct du projet Ghidra (pyghidra, lecture seule) a
l'interieur de `FUN_005d6140` (= `GameServicesCreate`, appelee depuis
`FUN_005dcdf0`/WinMain) : une vingtaine de singletons moteur "GOF::"
(Game Object Framework, confirme par EASTLLocalAllocatorModule) sont
alloues puis construits a la main, dont `FMVDataManager` et toute la
famille `Presentation*`. `GameFlowStateController` est une machine a
etats finis avec 5 etats de demarrage identifies directement via leurs
tables virtuelles (Idle/GuiControlGamePaused/GameRunning/
EnterLoadingState/LoadLevelState) : aucun n'est "Intro"/"Menu". Piste
la plus concrete trouvee : la chaine de commande de script
"Presentation.Screens.ActivateScreenWithFocus ScreenMainMenuCameraPos",
referencee une seule fois a 0x0048b005, dans une fonction jamais
detectee par l'analyse automatique de Ghidra (atteinte uniquement par
appel indirect), qui manipule `PresentationTaskHandler`
(`DAT_00fcadf0`). Detail complet, tableau des etats et adresses dans
`architecture/startup_path.md`. Aucune integration de code cette
section ; exploration documentaire uniquement, avec une nouvelle
methode (lecture de chaines + xrefs en direct via pyghidra plutot que
les seuls exports Ghidra deja generes).

## La table des 93 commandes de presentation (carte complete, pas encore de code)

Poursuite en direct (pyghidra, lecture seule) de la piste
PresentationTaskHandler jusqu'a l'interpreteur central de commandes de
script : `FUN_0048ab90` decoupe une commande ("Namespace.Verbe Arg") et
cherche le verbe dans une table de 93 entrees a `0x00876ab0`
(pointeurs vers des structs {nom, gestionnaire}). Les 93 entrees sont
sauvegardees dans `architecture/presentation_commands.csv` (table de
reference reutilisable). Deux taches liees au demarrage identifiees et
tracees via leurs vtables (meme la ou Ghidra n'avait jamais detecte de
fonction, atteintes uniquement par appel indirect) :

- `Storage.Bootup` -> tache `Task_Bootup` (vtable 0x828338), machine a
  9 etats entierement cartographiee (dispatch par table de sauts a
  0x59f7c0) ; l'etat final execute deux commandes de script
  ("Presentation.StorageTRCPopupActive" puis
  "Presentation.Screens.ActivateScreenWithFocus
  ScreenAutosaveWarningMessage") et pose DAT_00cef63f=1.
- `TriggerBootupFlow` -> tache `Task_BootupFlowControl` (vtable
  0x828360), dont le slot "Update" n'est PAS une machine a etats comme
  `Task_Bootup` ; role encore a elucider.

Candidats directs pour l'intro/menu, non encore traces : `FMVPlayback`
(#79), `Presentation.LeaveTitlePage` (#81), `OnActivateFrontEndChapter`
(#75). Detail complet (adresses, etats, raisonnement) dans
`architecture/startup_path.md`. Aucune integration de code cette
section ; exploration documentaire + un nouveau fichier de reference
(presentation_commands.csv).

## Point de depart confirme de la sequence intro/menu : WinMain -> TriggerBootupFlow

Dernier maillon de cette section d'exploration : recherche des xrefs
(pyghidra, lecture seule) vers la chaine de commande
"TriggerBootupFlow" remonte a `FUN_00ff7fa0` (deja exportee par
ReAgent), elle-meme appelee une seule fois, directement depuis
`WinMain` (`FUN_005d9300`), juste apres `FUN_005dcdf0()` (bootstrap
moteur + GameServicesCreate) et derriere un garde-fou materiel
(`thunk_FUN_00ff8ce0`, inconditionnellement vrai en configuration PC de
detail normale). La chaine complete WinMain -> bootstrap ->
TriggerBootupFlow -> Task_BootupFlowControl/Task_Bootup -> ecran
d'avertissement de sauvegarde est donc confirmee de bout en bout par
lecture directe de l'assembleur. Detail complet (sequence exacte,
adresses) dans `architecture/startup_path.md`. Aucune integration de
code cette section.

## FMVPlayback / OnActivateFrontEndChapter : la limite code/donnees est atteinte

Dernieres deux commandes tracees cette session : `FMVPlayback`
(0x0048cd90, deja exportee) parse "nom_fichier flag_skippable" et
declenche la lecture video via `thunk_FUN_00fec210`.
`OnActivateFrontEndChapter` (0x0048c9e0, deja exportee) cree une tache
`Task_FrontEndChapter` dont le slot "Update" est un stub trivial (pas
de machine a etats comme Task_Bootup) ; son seul travail reel observe
est d'emettre "Presentation.VignetteEffect 0".

Conclusion de cette branche d'exploration (detail dans
`architecture/startup_path.md`) : la chaine de controle C++ est
cartographiee jusqu'au point ou elle active des ecrans nommes par
chaine. Le contenu reel des ecrans (menu, boutons) est tres
probablement pilote par des DONNEES (fichiers de ressources UI), pas du
code visible en desassemblage. Continuer necessiterait une methode
differente (localiser le format des fichiers de definition d'ecran)
plutot que la poursuite des xrefs assembleur utilisee dans toute cette
section. Aucune integration de code cette section.

## Integration des primitives generiques : allocate_tagged, submit_task, dispatcher de commandes

Quatre fonctions deja exportees par ReAgent, identifiees au cours de
l'exploration precedente, integrees et testees dans
`src/presentation/` :

- `allocate_tagged` (0x005dc750) reutilise directement
  `hp6::platform::service_holder` (meme bloc d'init paresseux que
  0x005db920, deja integre) via son second slot de fonction
  (`DAT_00fcdd78`), qui est le service d'allocation nommee global.
- `submit_task` (0x005d2b30) ajoute une tache a la file propre d'un
  PresentationTaskHandler, via un allocateur DIFFERENT et imbrique a
  son champ +0x8 (pas allocate_tagged) : deux mecanismes d'allocation
  distincts coexistent dans le moteur (service nomme global pour les
  singletons, allocateurs locaux par sous-systeme pour les objets
  frequents). Appelle ensuite la methode "IsReady" de la tache
  (thiscall confirme).
- `dispatch_presentation_command`/`find_and_invoke_command`
  (0x0048ab90/0x0048abf0) : l'interpreteur central des 93 commandes de
  script (architecture/presentation_commands.csv).

Deux pieges rencontres en testant (detail complet dans
architecture/startup_path.md) : une table factice incomplete en C++
laisse des nullptr par initialisation de valeur (deref -> segfault,
corrige en remplissant les 93 emplacements) ; un `const` a portee
espace de noms a une liaison interne par defaut en C++, necessitant un
second mot-cle `extern` explicite cote test pour resoudre l'`extern`
cote production au lien.

318 des 333 fonctions suivies sont integrees (15 differees). 31 tests
CTest, tous verts.

## FMVPlayback/AttractMode : bonus (15 cinematiques), confirme la limite code/donnees

`thunk_FUN_00fec210` (declencheur reel de FMVPlayback) recherche son
argument dans une table de 16 entrees (`architecture/startup_path.md`)
: la table complete des cinematiques du jeu par nom (AttractMode,
AttractModeNoText, 3_DumbledoresSpeech ... 17_Credits). Aucune fonction
du code n'emet litteralement la chaine "FMVPlayback" ailleurs que dans
sa propre entree de la table de commandes : confirme que la sequence
reelle (quelles videos, dans quel ordre, au premier lancement) est tres
probablement definie dans un fichier de script/chapitre de donnees, pas
dans le binaire. Aucune integration de code cette section.

## Decouverte de projets voisins (knowledge/hp6, reconstruction/) et validation croisee

L'utilisateur a signale que `D:\HP` (le repertoire parent) contient les
archives extraites du jeu (`D:\HP\extract\`) ainsi que deux projets
voisins bien plus avances portant sur le MEME binaire :

- `D:\HP\reconstruction\` : un frontend "authored" (ecrit a la main,
  pas une recompilation binaire) qui reutilise les assets extraits du
  jeu mais reimplemente la logique du menu de facon nouvelle. Objectif
  different de HP6_Reconstructed (qui vise une reconstruction fidele,
  fonction par fonction, du binaire original) ; a ne pas melanger.
- `D:\HP\knowledge\hp6\` : une base de connaissances IDA Pro (14177
  fonctions, 7913 symboles de donnees, 309 classes "Hybrid", outil CLI
  `D:\HP\tools\re_kb.py`) construite independamment sur le meme binaire
  (SHA256 identique). Contient de vrais noms de symboles (probablement
  issus d'une base de symboles de debug) absents de notre projet
  Ghidra.

Verification croisee via `re_kb.py` sur nos propres decouvertes de la
veille : concordance totale. `FUN_005dcdf0` = `Engine_InitCoreSystemsAndMessages` ;
`FUN_005dae30` = `MessageId_GetOrRegister` ; `FUN_005dd020` =
`DebugOverlay_RefreshAndBroadcastRender` (confirme notre hypothese
"overlay de diagnostic/build string") ; `FUN_005d2b30` =
`PresentationTaskHandler_RegisterTask` (= notre `submit_task`,
deja integre) ; `DAT_0089eb0c` = `g_pMainTickInterface` (confirme notre
interpretation "simulate/render" du frame pump) ; notre table de 93
commandes a `0x00876ab0` concorde entree par entree avec
`ui_commands.json` de cette base. `GameFlowStateController` et sa
machine a etats (`thunk_FUN_00fdfe00`/`FUN_00492b60`) restent non
documentees la-bas : contribution reellement nouvelle de ce projet.

**Decouverte cle, qui boucle la piste intro/menu** : la base liste 24
fonctions `ScreenTask_Screen<Nom>_Enqueue` (adresses consecutives,
0x50b8b0 a 0x50c330), dont `ScreenTask_ScreenMainMenu_Enqueue`
(0x50b920) et `ScreenTask_ScreenTitlePage_Enqueue` (0x50ba70) ; chacune
appelle `PresentationTaskHandler_RegisterTask` (notre `submit_task`).
Recherche de xrefs en direct (pyghidra) sur ces adresses confirme
qu'elles forment une table de donnees a `0x0082fbbc` (26 entrees x 16
octets : {Enqueue, Dequeue_commun, fn3_commun, fn4_commun}), elle-meme
lue par `FUN_0048ce30` ("HybridScreenActivateMappedTask", indice 4 de
notre table de 93 commandes) via une AUTRE table de 26 pointeurs vers
des objets de type "tache" a `0x00870948`, cherchee par nom exact. Ceci
est **le resolveur derriere `Presentation.Screens.ActivateScreenWithFocus
<nom_ecran>`**. Integre et teste comme
`hp6::presentation::activate_mapped_screen_task` (thiscall confirme par
l'assembleur brut).

Chaine complete desormais confirmee de bout en bout : WinMain ->
bootstrap moteur -> TriggerBootupFlow -> Task_BootupFlowControl/
Task_Bootup -> `ActivateScreenWithFocus <nom>` -> resolution par nom
dans la table de 26 ecrans -> `ScreenTask_Screen<Nom>_Enqueue` ->
`PresentationTaskHandler_RegisterTask`. Reste non resolu : le contenu
exact de chaque tache d'ecran (son veritable travail d'affichage), et
quelle commande precise est emise juste apres `TriggerBootupFlow` (vu
hier : probablement pilote par des donnees/scripts, pas du code).

## Lot ReAgent mixed_deps_059 integre (StorageInit)

5 fonctions du lot generique `mixed_deps_059` (PASS ReAgent), verifiees
individuellement contre le decompile Ghidra original avant integration,
plus les deux utilitaires partages (`FUN_006b3b0e` memset generique,
`FUN_006b3b04` hook generique) : integrees dans
`src/platform/StorageInit.cpp`. `enumerate_logical_drives_into_table`
(GetLogicalDrives) et `schedule_autosave_timer` (minuteur Windows
attendable) sont toutes deux appelees depuis `FUN_00688130`, une des
inits non explorees de WinMain — coherent avec les commandes
"Storage.Autosave"/"Storage.CreateTimedAutosaveTrigger" deja tracees.
`0x006d95d0` (enregistrement de completion de minuteur) reste une
dependance externe non reconstruite.

324 des 339 fonctions suivies sont integrees (0 bloquee, 0 en attente
de revue, 15 differees). Compilation Win32 et les 32 tests CTest
reussissent.

## La tache ScreenMainMenu declenche elle-meme ScreenTitlePage

Decompilation directe (assembleur brut) de `ScreenTask_ScreenMainMenu_Enqueue`
(0x50b920) et de l'objet tache qu'elle construit (vtable 0x82f91c) :
sa machine a etats (slot "Update" +0xc) emet elle-meme
"Presentation.Screens.ActivateScreenWithFocus ScreenTitlePage" a son
premier update (sous conditions sur DAT_00cef67a/DAT_00868814, non
resolues), puis passe a l'etat 2. Donc ScreenTitlePage, pas
ScreenMainMenu, est tres probablement le premier ecran visuel reel ; le
menu principal se prepare en arriere-plan. Recherche du veritable
declencheur initial ("ActivateScreenWithFocus ScreenMainMenu" ou
equivalent) : seule reference directe a la chaine "ScreenMainMenu"
trouvee est liee a la commande "QuitGame" (retour menu apres partie),
pas un point d'entree initial. L'objet "ScreenTaskListener_ScreenMainMenu"
s'avere etre un abonne au mecanisme de notification GENERIQUE du
framework (liste chainee, FUN_005d2c10), pas une piste specifique.
Declencheur initial toujours insaisissable malgre plusieurs angles
d'attaque ; coherent avec la limite deja documentee (probablement
pilote par des donnees de script). Detail complet dans
architecture/startup_path.md. Aucune integration de code cette
section.

## DAT_00cef67a identifie : drapeau "dialogue modal actif"

Suite de l'investigation sur la tache ScreenMainMenu : `DAT_00cef67a`
est pose a 1 par `FUN_005d3070`, une fonction d'affichage de boite de
dialogue modale (DIALOG_MAIN/DIALOG_RESPONSE_0/DIALOG_RESPONSE_1, liee
aux commandes "Storage.AskSetAutosave"/"Storage.OnResponseSelect0/1"
deja tracees). Confirme l'hypothese : la tache ScreenMainMenu n'avance
vers ScreenTitlePage que si aucun dialogue modal n'est actif.
`DAT_00868814` reste non resolu (plusieurs lecteurs/ecrivains, dont un
qui le pose a -1 comme sentinelle "non defini") ; rendement marginal
decroissant sur ce fil precis, mis en pause ici.

## Lot ReAgent mixed_deps_060 : echec technique (CLI codex absent)

Le lot generique suivant a echoue entierement : le CLI `codex` (LLM
utilise par ReAgent, configure dans re-agent.yaml) n'est plus installe
a l'emplacement attendu (`npm list -g` ne le liste pas). Ce n'est PAS
une limite d'usage (contrairement au 16 oct) mais une dependance
d'environnement manquante. Pas corrige sans l'accord de l'utilisateur
(installation logicielle). Pipeline ReAgent generique indisponible
jusqu'a reinstallation de `codex`.

## DAT_00868814 identifie : reponse utilisateur a un dialogue de confirmation

`DAT_00868814` est lu dans une immense machine a etats
(`FUN_0042a1e0`, 9655 caracteres decompiles, >24 etats, liee a
`DAT_00fcae18` comme Task_Bootup) juste apres l'affichage d'un
dialogue "GUI_CONFIRMSAVE" via `FUN_005d3070("GUI_CONFIRMSAVE",...)`
(la meme fonction qui pose `DAT_00cef67a`) : `DAT_00868814==0` ->
reponse "non", `==1` -> reponse "oui". Cette grosse machine a etats
est tres probablement le flux complet "verifier carte memoire/profil
de sauvegarde, demander confirmation avant ecrasement" evoque par
"ScreenBootupMemcardCheck"/"Storage.CreateNewProfile" (deja trouves) ;
hors de portee d'une integration ponctuelle (ampleur d'un sous-systeme
a part entiere). Les deux globales qui bloquaient l'integration de la
tache ScreenMainMenu sont maintenant comprises au niveau conceptuel.

## La tache ScreenMainMenu integree et testee (premiere vtable fabriquee par nous)

`enqueue_main_menu_screen_task` (0x0050b920) et les trois methodes de
son objet tache (destructeur 0x0050c440, IsReady 0x0053e9f0, Update
0x0053ea00) sont integrees dans `src/presentation/ScreenTasks.cpp`.
Point technique notable : `submit_task` appelle reellement
`vtable_of(task)[1]` en **thiscall reel** sur l'objet que nous
construisons nous-memes — premiere fois dans ce projet qu'une vtable
fabriquee par nous (pas une vtable du jeu qu'on se contente d'appeler)
doit etre compatible ABI thiscall avec nos propres fonctions C++. MSVC
interdisant `__thiscall` sur une fonction libre, les methodes sont des
methodes non-statiques normales d'une petite classe locale sans base,
leurs adresses brutes extraites via union pointeur-membre/pointeur-brut
(fait d'implementation MSVC stable pour ce cas simple).

Piege rencontre en testant (meme categorie qu'hier) : une table
factice de 93 entrees comptee a la main n'en contenait que 91,
provoquant un segfault. Corrige avec des macros de repetition dont le
compte est verifiable par arithmetique.

328 des 348 fonctions suivies sont integrees (5 bloquees : lot ReAgent
mixed_deps_060, CLI codex manquant ; 15 differees). 33 tests CTest,
tous verts.

## ScreenTitlePage integree : confirme le mecanisme d'inactivite -> AttractMode

`ScreenTask_ScreenTitlePage_Enqueue` (0x0050ba70) suit le meme schema
que ScreenMainMenu (objet tache de 24 octets cette fois). Son "IsReady"
(0x00451ec0) construit et soumet une sous-tache
"Presentation:TaskHandler::Task_IdleInputTimer" (vtable 0x00821508) :
un minuteur d'inactivite minimal dont le champ+8 memorise le tick
courant (DAT_00d644f8, desormais expose via
hp6::runtime::current_tick_value()) a chaque entree detectee
(0x004ccca0, dependance externe non reconstruite). C'est le mecanisme
exact derriere le minuteur d'inactivite -> AttractMode trace il y a
deux sessions. Integre et teste dans src/presentation/ScreenTasks.cpp.
Les slots "liste"/"Update" de ScreenTitlePage elle-meme restent non
reconstruits (hors de portee, rien ne les appelle encore).

334 des 354 fonctions suivies sont integrees (5 bloquees, 15
differees). 33 tests CTest, tous verts.

## ScreenAutosaveWarningMessage integree : revele ScreenTitlePageCut_PC

L'Update de cette tache (0x00429de0, entierement reconstruit) :
apres >=2000 (tick/3) unites, active ScreenAutosaveWarningMessagePrompt
sans focus si pas deja actif ; apres >=8000 unites sans aucune entree
detectee, emet "Presentation.Screens.ActivateScreenWithFocus
ScreenTitlePageCut_PC" — la cinematique d'intro se declenche
automatiquement si le joueur ne fait rien. dispatch_presentation_command
corrigee pour retourner son resultat (un appelant reel en depend,
decouvert ici). Piege de test : un pointeur `args` stocke au lieu
d'etre copie pointait dans la pile locale de dispatch_presentation_command
(use-after-return), corrige par une copie.

339 des 359 fonctions suivies sont integrees (5 bloquees, 15
differees). 33 tests CTest, tous verts.

## ScreenFMV : confirme le lien vers la table AttractMode, trop vaste pour integration complete

ScreenTask_ScreenFMV_Enqueue (0x0050bcd0, 44 octets, le plus gros objet
tache a ce jour) : seuls constructeur/destructeur integres. Son slot 2
(0x0053df20) indexe directement la table de 16 cinematiques AttractMode
(0x008923e8) via DAT_00cef5b8 — la MEME table lue par FMVPlayback —
confirmant que ScreenFMV est l'ecran reel derriere la lecture video.
Son slot 3 (Update, 0x0053e000) est un dispatcheur a 12 etats avec
368 octets de pile locale : hors de portee, trace "differee"
explicitement dans le registre plutot que laissee implicite.

341 des 364 fonctions suivies sont integrees (5 bloquees, 18
differees). 33 tests CTest, tous verts.

## ScreenAttractMode : resout l'appartenance de la machine a etats 0x0048d2a0

ScreenTask_ScreenAttractMode_Enqueue (0x0050bc60, 48 octets) : seuls
constructeur/destructeur integres. Son vtable slot 1 ("IsReady",
0x0048d2a0) est confirme etre exactement la grosse machine a etats
DAT_00cef7b0 partiellement tracee il y a plusieurs sessions (branche
"case 7" -> "QuitGame ScreenMainMenu"). Resout a QUI appartient cette
machine (le IsReady du mode attraction), sans la reconstruire
entierement. Slots 2/3 egalement differes.

343 des 369 fonctions suivies sont integrees (5 bloquees, 21
differees). 33 tests CTest, tous verts.

## ScreenLanguageSelect integree ; bilan de la serie de 6 taches d'ecran

ScreenTask_ScreenLanguageSelect_Enqueue (0x0050b8b0, 24 octets) :
constructeur/destructeur integres. Update (slot3, 0x00429890, gros
buffer local) differe. Bilan de cette serie (ScreenMainMenu,
ScreenTitlePage, ScreenAutosaveWarningMessage, ScreenFMV,
ScreenAttractMode, ScreenLanguageSelect — 6 sur 24 taches d'ecran) : le
schema construction/destruction est maintenant entierement compris et
repetitif ; les 18 restantes suivraient probablement le meme schema,
avec des Update individuellement specifiques a evaluer au cas par cas.

345 des 374 fonctions suivies sont integrees (5 bloquees, 24
differees). 33 tests CTest, tous verts.

## Tentative de construction d'un executable "en mode normal", corrigee

Pour faire progresser vers un executable testable, j'ai d'abord ecrit
src/app/GameMain.cpp : WinMain, creation de device D3D9 et allocateur
de PresentationTaskHandler tous ecrits a la main (operator new, objets
"debug tick"/"fallback" invente pour DAT_0089eb0c/DAT_00fc7f90) plutot
que reconstruits depuis le binaire. L'utilisateur a corrige
immediatement : « Construit pas ca en mode normal tu recupere les
truc du jeu decompiler pour le reconstruire ». src/app/GameMain.cpp et
le bloc CMake associe ont ete entierement retires (verifie par git
diff propre, 33/33 tests toujours verts). Le reste de cette session
suit la methode normale du projet (xrefs/assembleur en direct, pas
d'invention) pour la meme question.

## Vrai site IDirect3D9::CreateDevice localise et integre (006498e0)

Voir architecture/startup_path.md ("18 oct 2026") pour le detail
complet. Resume : FUN_006498e0 (create_d3d9_device,
src/render/DeviceCreate.cpp) est le vrai appel CreateDevice de WinMain,
trouve par xref sur DAT_00cf1944 (deux PUSH de son ADRESSE, signe
classique d'un parametre de sortie) et confirme independamment par la
base de connaissances IDA (CreateD3DDeviceAndSwapchains). Toute sa
chaine de dependances necessaire au chemin de succes par defaut est
integree (12 fonctions/thunks : enumerate_display_modes et son
comparateur qsort, 4 fonctions triviales, 2 tables de render states par
defaut extraites mecaniquement via un script Python jetable plutot que
transcrites a la main — risque de coquille elimine sur 73+61 champs).
Deux branches reelles mais non prioritaires restent deliberement
differees avec un corps abort() (jamais executees par le chemin
teste) : build_resolution_option_list_hard_cap (0x0064c8c0, custom
resolution list, morte tant que DAT_00cf1950 reste a 0) et
fatal_display_error (0x005da4b0, chemin d'echec CreateDevice).
tests/device_create_smoke.cpp fournit un IDirect3D9/IDirect3DDevice9
factice complet et confirme le chemin de succes. Prochaine etape :
FUN_006194a0 (init d'un tas ~13,4 Mio, "RenderFrameworkMainHeap"),
seul bloquant restant avant d'integrer FUN_005dca10 lui-meme (le vrai
appelant direct depuis WinMain).

361 des 392 fonctions suivies sont integrees (5 bloquees, 26
differees : 24 anciennes + build_resolution_option_list_hard_cap et
fatal_display_error). 34 tests CTest, tous verts.

## FUN_006194a0 et FUN_005dca10 integrees : WinMain -> creation du device
## est maintenant une chaine testee de bout en bout

Voir architecture/startup_path.md ("18 oct 2026, suite") pour le detail.
Resume : FUN_006194a0(0xd71250) initialise le tas "RenderFrameworkMainHeap"
(0xd71250 est une ADRESSE, pas une taille) en reutilisant
hp6::platform::service_holder() plutot que de dupliquer son bloc d'init
paresseuse (meme effet observable que l'original, qui le duplique en
ligne), et en reutilisant le meme pointeur DAT_00fcdd78 qu'allocate_tagged.
FUN_005dca10 (l'appelant direct reel de WinMain) est integree en entier :
reset du frame pump (nouvelle fonction FrameClock.cpp), init du tas,
creation du device, puis un appel virtuel sur un objet encore non
identifie (DAT_00d6af14, necessite FUN_005dcdf0). Deux stubs sur cette
chaine (create_local_allocator_register, invoke_engine_service_hook)
sont volontairement des no-op/toujours-succes plutot que des abort(),
parce qu'ils sont sur le chemin de succes SANS CONDITION -- contrairement
a fatal_display_error/build_resolution_option_list_hard_cap qui restent
abort() car confirmes morts par construction. tests/device_create_smoke.cpp
exerce desormais toute la chaine WinMain -> FUN_005dca10 -> create_d3d9_device.

364 des 396 fonctions suivies sont integrees (5 bloquees, 27
differees). 34 tests CTest, tous verts.

## FUN_005dcdf0 (bootstrap moteur) integre : decouverte du bus de
## messages nommes et du TimeManager

Voir architecture/startup_path.md ("18 oct 2026, suite 2") pour le
detail. Resume : FUN_005dcdf0 (Engine_InitCoreSystemsAndMessages, le
second appel direct de WinMain apres FUN_005dca10) revele un bus de
messages nommes interne au moteur (~30 messages "iMsg*" enregistres
dans ce seul bootstrap, via une table de hachage non tracee) ; stub
honnete qui produit un handle stable sans implementer le vrai registre,
puisque rien n'emet encore ces messages. Deux identifications
importantes : DAT_00cef730 (deja lu par FrameClock.cpp) EST le
singleton TimeManager::Instance ; FUN_005e0bf0 initialise le pas de
simulation fixe du frame pump (toujours 40.0 en Q16.16, independant de
l'argument) et un second accumulateur avec le meme idiome de precision
flottant->fixe deja vu ailleurs. DAT_00d6af14 (l'objet moteur utilise
par FUN_005dca10 ET FUN_005dcdf0) est trace jusqu'a sa vraie
construction : un initialiseur statique C++ global a 0x0070f720,
exécuté avant WinMain par le runtime CRT -- pas FUN_005dcdf0 lui-meme.
Sa reconstruction comme vrai singleton C++ reste a faire ; les deux
appels vtable dessus restent des no-op documentes.
tests/engine_bootstrap_smoke.cpp exerce tout le bootstrap de bout en
bout.

375 des 411 fonctions suivies sont integrees (5 bloquees, 31
differees). 35 tests CTest, tous verts.

## FUN_00ff1430 integree : la commande exacte d'activation de
## ScreenLanguageSelect, et correction d'une hypothese

Voir architecture/startup_path.md ("18 oct 2026, suite 3" et la note de
cloture) pour le detail. Resume : trouve par recherche directe de la
chaine "...ActivateScreenWithFocus ScreenLanguageSelect" (un seul site,
FUN_00ff1430) ; integree et testee pour ses 3 branches certaines
(chapitre 8=FrontEnd, chapitre 9=LanguageSelect, autre=ScreenMainOSD).
Correction importante : mon hypothese precedente ("Global" resout vers
LANGUAGE_SELECT) etait fausse -- la table reelle (0x892300) ne contient
aucune entree nommee "Global", donc DAT_00d472dc n'est PAS mis a 9 par
cette voie. Qui le met reellement a 9 sur un premier lancement reste
non resolu (FUN_00576f30/FUN_0053eb40, differees). ChapterLoadStateMachine_Update
(0x4da7b0, confirmee par la base IDA independante comme etant la
machine a etats dont l'etat final appelle FUN_00ff1430) reste differee
dans son ensemble : une vingtaine de sous-fonctions non tracees,
ampleur comparable a FUN_005dcdf0.

377 des 415 fonctions suivies sont integrees (5 bloquees, 33
differees). 36 tests CTest, tous verts.

## resolve_and_load_chapter / advance_to_next_chapter integrees :
## LanguageSelect est structurellement le premier chapitre du jeu

Voir architecture/startup_path.md ("18 oct 2026, suite 4"). Resume :
FUN_00576f30 et FUN_00ff1390 integrees (src/presentation/ChapterFlow.cpp).
FUN_00ff1390 revele l'ordre reel de progression de la campagne via la
table 0x892300 : LanguageSelect(9) -> FrontEnd(8) -> BurrowIntro(1) ->
Hogwarts(2) -> BurrowXmas(3) -> ... -- LanguageSelect est
structurellement LE PREMIER chapitre de toute la sequence, preuve
indirecte forte (pas encore une confirmation directe) de l'hypothese de
depart. Reste non trouve : le site exact qui ecrit DAT_00d472dc=9 au
tout premier lancement.

381 des 419 fonctions suivies sont integrees (5 bloquees, 33
differees). 36 tests CTest, tous verts.

## ChapterLoadStateMachine_Update integree : squelette complet des 20
## etats, pas juste l'etat critique

Voir architecture/startup_path.md ("18 oct 2026, suite 6"). Tous les 20
etats et leurs conditions de transition transcrits fidelement
(y compris une subtilite confirmee sur l'assembleur : le switch
original n'a aucun case pour l'etat 18, reproduit a l'identique). Les
~30 sous-fonctions feuilles restent differees (3 stubs generiques par
forme de retour). L'etat 19 (celui qui declenche
emit_chapter_entry_screen_command) est transcrit et teste avec
precision. L'execution complete etat1->etat19 n'est pas exercee de
bout en bout (depend des vraies valeurs de retour des sous-systemes
differes).

382 des 419 fonctions suivies sont integrees (5 bloquees, 32
differees). 37 tests CTest, tous verts.

## TriggerBootupFlow integree de bout en bout

Voir architecture/startup_path.md ("18 oct 2026, suite 7"). FUN_0048c970
(handler de la commande, #73) et FUN_00ff7fa0 (la fonction qui l'emet
depuis WinMain) integrees dans src/presentation/BootupFlow.cpp, sans
nouvelle primitive (reutilise allocate_via_task_handler/submit_task/
resolve_and_load_chapter deja integrees). Teste bout en bout, y
compris la re-entrance reelle du dispatch de commande. La chaine
complete WinMain -> device D3D9 -> bootstrap moteur -> TriggerBootupFlow
-> resolution de chapitre est maintenant integree et testee de bout en
bout, modulo les sous-systemes de chargement de ressources deja
differes.

385 des 422 fonctions suivies sont integrees (5 bloquees, 32
differees). 38 tests CTest, tous verts.

## Gestionnaire de device perdu integre + vraie correction de fidelite

Voir architecture/startup_path.md ("18 oct 2026, suite 8"). FUN_0064aa60
(handle_device_lost) integree, reutilise cache_available_texture_memory
et prepare_render_target_texture deja en place. Deux corrections
importantes faites en ecrivant le test (pas juste pour le test) :
(1) create_d3d9_device utilisait un tableau LOCAL pour les
D3DPRESENT_PARAMETERS, alors que le vrai Reset() reutilise le meme
bloc PERSISTANT -- corrige en un vrai global partage DAT_00c75538 ;
(2) release_if_present appelait le slot 0 de la vtable au lieu du
slot +8 (Release) -- une vraie faute de frappe dans le code de
production, detectee seulement parce que le test utilisait un objet
factice complet (vraie vtable) plutot qu'un pointeur arbitraire comme
certains tests precedents. Lecon retenue : faire des objets factices
complets vaut l'effort, un pointeur arbitraire masque ce genre
d'erreur.

388 des 425 fonctions suivies sont integrees (5 bloquees, 32
differees). 39 tests CTest, tous verts.

## La vraie boucle de messages Win32 integree

Voir architecture/startup_path.md ("18 oct 2026, suite 9"). FUN_005d8fc0
(hp6::platform::run_message_loop, src/platform/MessageLoop.cpp) est
l'appel unique que WinMain fait apres ShowWindow/UpdateWindow. Reutilise
handle_device_lost, copy_gsc_message_handles et run_frame_tick, deja
integrees ; applique le meme idiome Q16.16 entier/fraction que
initialize_frame_pump_timing pour calculer le budget de frame. Verrou
WM_QUIT a usage unique et dispatch Translate/DispatchMessage reproduits
fidelement. Quatre sous-fonctions restent differees en no-op documente
(bloc de nettoyage "erreur fatale deja affichee", traqueur de plancher
de performance, FUN_00689180, FUN_00545f30) -- aucune n'est sur un
chemin dont le code deja integre depend.

389 des 426 fonctions suivies sont integrees (5 bloquees, 32
differees). 40 tests CTest, tous verts. La chaine complete WinMain ->
creation du device D3D9 -> bootstrap moteur -> TriggerBootupFlow ->
resolution de chapitre -> boucle de messages est maintenant
reconstruite et testee piece par piece ; il ne reste plus qu'a
assembler WinMain lui-meme comme une vraie fonction C++ appelable, en
reliant les morceaux deja valides.

## WinMain assemblee, integree et testee de bout en bout

Voir architecture/startup_path.md ("18 oct 2026, suite 10"). FUN_005d9300
(hp6::platform::run_win_main, src/platform/WinMain.cpp) relie toutes
les pieces deja integrees : SystemParametersInfoA save/restore,
verification mono-instance, ~27 lectures de reglages GameSettings,
creation de fenetre et de device D3D9, bootstrap moteur,
TriggerBootupFlow, boucle de messages reelle, et toute la sequence
d'arret (demontage du device, restauration SystemParametersInfoA,
TerminateProcess injectable pour rester testable). Cinq fonctions
supplementaires integrees pour de vrai sur ce chemin :
initialize_file_system_storage (FUN_00688130), is_platform_compatible
(FUN_00ff8ce0, dont le chemin interne est confirme mort car
initialize_file_system_storage vient de remettre son drapeau a zero
juste avant), shutdown_render_device (FUN_00648e70, le vrai demontage
du device avec verification de compteur de reference),
parse_setting_string_to_int (FUN_00ff8300), plus six no-op documentes
repartis entre DeviceCreate.cpp et un nouveau fichier
EngineSubsystems.cpp (sous-systemes de developpement pur ou objets de
service non traces, chacun avec sa propre justification documentee).

tests/winmain_smoke.cpp exerce la fonction entiere contre une chaine
complete de faux objets Direct3D9, avec le verrou de la boucle de
messages pre-arme pour eviter toute boucle infinie dans le test.

394 des 438 fonctions suivies sont integrees (5 bloquees, 39
differees). 41 tests CTest, tous verts. **L'objectif directeur de
cette reconstruction est atteint : un executable reel et testable,
assemble piece par piece depuis WinMain, qui cree une vraie fenetre,
un vrai device Direct3D 9, demarre le vrai moteur, et declenche
TriggerBootupFlow -> resolution de chapitre -> ScreenLanguageSelect.**

## Premier vrai DrawPrimitive integre

Voir architecture/startup_path.md ("18 oct 2026, suite 12"). Indication
utilisateur confirmee dans le binaire : les menus sont des elements 3D
spatiaux, rendus via de vraies cameras nommees ("ScreenSpaceCamera",
"CurrentInWorldCamera", "GlobalPresentationCamera"). En cherchant les
fonctions referencant a la fois le device D3D9 reel et des offsets de
vtable SetTransform/DrawPrimitive/BeginScene/EndScene, deux fonctions
reelles ont ete trouvees et integrees : submit_screen_quad (FUN_00648c00,
verrouille le vertex buffer partage, construit les sommets, SetStreamSource,
DrawPrimitive(TRIANGLESTRIP) reel) et render_fade_overlay_quad
(FUN_0064abd0, compose la texture-cible-de-rendu comme un quad plein
ecran, EndScene/Present). Six setters D3D9 mis en cache supplementaires
integres au passage (sampler/texture/vertex declaration/vertex shader/
pixel shader states). Honnetete sur la portee : cette chaine n'est
atteinte, dans le binaire exporte, que depuis le systeme d'overlay de
debug, confirme mort (hardcode false) dans le build FINAL -- ce n'est
pas encore le vrai moteur de rendu des menus. Teste de bout en bout
dans tests/screen_overlay_render_smoke.cpp, qui verifie que le vrai
DrawPrimitive(TRIANGLESTRIP, 0, 2) se declenche avec les bonnes donnees.

42/42 tests CTest, tous verts. Piste ouverte pour la suite : FUN_005d3570
("SetActiveCamera" probable) et les trois cameras nommees, pour relier
la vraie camera 3D au rendu reel de ScreenLanguageSelect.

## La chaine complete trouvee, puis Scene::End integree pour de vrai

Voir architecture/startup_path.md ("18 oct 2026, suite 14 et 15").
Recherche descendante dans la base IDA (14177 fonctions) : trouve
PluginPresentationBitmap_BuildRenderQuad, dont les globales referencees
incluent 0xcf8168 -- la meme table de chaines que ScreenLanguageSelect
remplit. Chaine complete tracee jusqu'au vrai dessin : BuildRenderQuad
-> clip/mise en page -> SubmitRenderCommand (62 vrais appelants) ->
RenderAllRegions (vide chaque region active, liee a
set_active_render_region deja integree) -> Scene::End (confirmee par
sa propre chaine de diagnostic), qui emet les vrais DrawPrimitive/
DrawIndexedPrimitive.

Scene::End (FUN_00646200) a ensuite ete integree pour de vrai
(hp6::render::execute_scene_render_batch, src/render/
SceneBatchRenderer.cpp) : les trois chemins de controle reels
(invalidation, recursion multi-batch, dessin simple sur deux paliers
de qualite) et les deux vrais appels D3D9 sont reproduits fidelement ;
les blocs de liaison materiau/eclairage non traces sont differes sans
affecter les parametres du vrai appel de dessin. Six petites fonctions
D3D9 supplementaires integrees au passage (SetVertexShaderConstantF,
SetStreamSource/SetIndices par handle, resolution de shader pixel par
mode). Un vrai bug de test (tableau de vtable factice trop petit,
ecriture hors limites) a ete trouve et corrige en ecrivant le test.

43/43 tests CTest, tous verts, 423/462 fonctions integrees (39
differees, 0 bloquee). Deuxieme vrai DrawPrimitive integre dans ce
projet, et premier connecte a la vraie chaine de rendu de scene.

Suite (18 oct 2026, suite 16) : les deux derniers blocs differes du
chemin low-tier de Scene::End ont ete integres pour de vrai --
FUN_006455c0 -> hp6::render::push_item_transform_and_material
(transform multipliee par la matrice vue/projection de la region
active, transposee, poussee au registre 0 du vertex shader ; groupes
de constantes couleur/materiau aux registres 4/5/6/0x12 ; etat de
rendu 0x16) et le second chemin de FUN_00644cd0 (param_4 != 0) ->
hp6::render::resolve_material_bindings (porte "dessiner" DAT_00865285,
liaison declaration+shader reelle pour les types 2, 10 et 0x20000108 --
ce dernier partage avec le fondu de debug deja integre). Resolu par
desassemblage reel (pas seulement le C decompile, qui masque le
registre implicite in_EAX de FUN_005a2310/transpose) : confirme que
les deux appels de transposition ecrivent dans un tampon separe, jamais
en place. Deux fonctions de maths pures ajoutees a src/math/Transform
(multiply_matrix4x4 = FUN_00401520, transpose_matrix4x4 = FUN_005a2310).
Un bug de vtable factice (SetRenderState/SetVertexShaderConstantF non
cables) et un changement de comportement des anciens tests (material
type 0 desormais rejete par la vraie porte) ont ete trouves et corriges
en construisant le test.

428/467 fonctions integrees (resolve_material_shader_effect / FUN_01002960
etait deja reellement integree dans cette meme passe mais son
enregistrement dans reports/pipeline/state.json avait ete oublie sur
le coup -- corrige a la suite 18 ci-dessous), 43/43 tests CTest verts.
Le chemin low-tier de Scene::End a desormais une apparence (transform,
couleur, declaration/shader) entierement reelle, plus seulement un
declenchement de dessin avec des parametres corrects.

Suite (19 oct 2026, suite 17) : ScreenLanguageSelectTask's IsReady
(0x00429850) et slot 2 (0x00429860) integres pour de vrai --
hp6::presentation::query_ready_language_select_screen_task (remet la
machine a etats d'Update a l'etat 0, toujours "pret") et
mark_language_select_pending_task_ready (parcourt la liste de taches
de PresentationTaskHandler a la recherche du pointeur this+0xc --
la sous-tache Task_IdleInputTimer qu'Update soumet -- et marque son
ready_flag +0x4 a 1 si trouvee). Ni l'une ni l'autre n'a son propre
objet fonction Ghidra (trou non reclame entre FUN_00429780 et
FUN_00429890) ; lues directement par desassemblage pyghidra plutot que
devinees. La fonction Update elle-meme (0x00429890, machine a etats a
3 phases, tampon de 0x118 octets, plusieurs sous-appels non traces)
reste deliberement differee -- meme traitement que les Update
equivalentes de toutes les autres taches d'ecran de ce fichier, et de
toute facon non appelee par rien de reel ici. L'oubli d'enregistrement
de resolve_material_shader_effect (FUN_01002960) a aussi ete corrige
dans cette passe.

430/467 fonctions integrees (37 differees, 0 bloquee), 43/43 tests
CTest verts.

Suite (19 oct 2026, suite 18) : FUN_0053eb40 (recherche de nom de
chapitre) reconnue comme deja integree -- deja reproduite en ligne dans
resolve_and_load_chapter, jamais factorisee separement. FUN_00ffa0d0 ->
hp6::platform::parse_command_line_overrides integree pour de vrai :
resolu par desassemblage direct du site d'appel reel dans WinMain que
EDI est l'adresse codee en dur d'un tampon global (0x00d63f68), pas un
"this" abstrait -- WinMain copie sa ligne de commande dans deux tampons
fixes de 0x200 octets avant l'appel. Decoupe en deux tables (drapeaux
nus / paires cle=valeur, 0x20 entrees max chacune), reutilisant
FUN_005e0630 (extract_next_token, tokenizer sensible aux guillemets).
Deux bizarreries reelles reproduites fidelement (pas "corrigees") :
asymetrie d'avancement du curseur quand l'une ou l'autre table est
pleine. Teste dans tests/winmain_smoke.cpp.

433/468 fonctions integrees (35 differees, 0 bloquee), 43/43 tests
CTest verts.

Suite (19 oct 2026, suite 19) : FUN_0064a9e0 -> hp6::render::
try_begin_debug_overlay_scene integree -- c'etait le vrai site "arme
ailleurs" que render_fade_overlay_quad (suite 12/13) laissait en
suspens pour DAT_00cf19e4. Ses deux sous-appels etaient deja integres
(handle_device_lost, cache_available_texture_memory), donc entierement
tractable : gate sur handle_device_lost, puis le vrai BeginScene
(vtable+0xa4), incremente DAT_00cf19e4. Deux vrais bugs de test
corriges en l'ecrivant : vtable de surface factice trop petite pour
GetDesc (meme piege que feedback_fake_vtable_sizing), et DAT_00cf19d0
laisse nul alors qu'initialize_default_render_state le dereference
directement -- meme correctif que tests/device_lost_handler_smoke.cpp
avait deja applique.

434/469 fonctions integrees (35 differees, 0 bloquee), 43/43 tests
CTest verts.

Suite (19 oct 2026, suite 20) : hp6::math::fast_cos (FUN_00401270,
src/math/Scalar.cpp) integree -- une approximation de cos(x) par serie
de Taylor d'ordre 16 (9 coefficients + constantes pi/2pi/1-sur-2pi, tous
lus directement en memoire, confirmes exacts). Reconstruite par
desassemblage direct (pas le C decompile seul, qui montre chaque chemin
finissant en un `return;` sans valeur -- le vrai resultat reste dans
XMM0). Verifiee contre std::cos a ~1e-7 pres ; un vrai defaut de
precision pour certains grands angles negatifs (reduction tronquee vers
zero, pas vers le bas) a ete teste explicitement plutot que cache.
Les 3 autres fonctions du meme lot float_ops_015 restent differees
(meme travail de desassemblage necessaire, pas fait cette session).

435/469 fonctions integrees (34 differees, 0 bloquee), 43/43 tests
CTest verts.
