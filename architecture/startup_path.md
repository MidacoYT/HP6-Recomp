# Chemin de démarrage observé

La fonction d'entrée PE `006b9718` appelle l'initialisation du cookie de
sécurité, puis `006b9457` (`___tmainCRTStartup`, runtime Visual Studio 2008).
Après les initialiseurs CRT, ce dernier appelle `005d9300`, la fonction
applicative qui reçoit l'instance Windows, la ligne de commande et l'option
d'affichage. Le runtime CRT sera fourni par le compilateur du projet ; il ne
nécessite pas une reconstruction manuelle des fonctions standard.

Dans `005d9300`, les exports montrent notamment : choix du répertoire de
l'exécutable, lecture de paramètres système, copie de la ligne de commande,
recherche de la classe de fenêtre `HalfBloodPrinceMainWndClass`, lecture des
réglages, création de la fenêtre, initialisation de sous-systèmes et boucle
d'exécution. Ces étapes utilisent des globals et des fonctions encore non
reconstruits. Aucun point d'entrée jouable n'est donc validé aujourd'hui.

Le plan `startup_prereqs_020` a été arrêté devant douze dépendances hors
lot. Le lot `startup_leaf_021` a intégré la gestion des paramètres système
(005d9210) et un calcul de taille de ressource (006dadb9). Le lot
`window_shell_022` a intégré la création et l'enregistrement de la classe
de fenêtre (005d8ed0, 00ff8380, avec le thunk 005d8e30). Un test Win32
crée et détruit une fenêtre de 320x240 ; le gestionnaire de messages réel
005d8a30 reste différé. Ces adaptations rendent les coordonnées et les
dépendances au système explicites, mais ne prouvent pas l'équivalence ABI
avec les registres employés par le binaire.

## 17 oct 2026 : `005d9300` identifiée comme WinMain, chaîne tracée jusqu'au
## handler de device Direct3D 9 perdu

`005d9300` a la signature `int __stdcall(HINSTANCE, HINSTANCE, LPSTR, int)` :
c'est WinMain. Son décompilé complet (~800 lignes) donne la séquence
suivante, avec l'état d'intégration de chaque étape :

1. `_control87`, `GetModuleFileNameA`/`SetCurrentDirectoryA` (dossier de
   travail), trois `SystemParametersInfoA` sauvegardés puis restaurés via
   `apply_system_parameters` (005d9210, **intégrée**).
2. `FindWindowA("HalfBloodPrinceMainWndClass", NULL)` : évite une seconde
   instance. Si absente, `register_game_window_class` (**intégrée**).
3. **~20 lectures de registre** sous
   `HKCU\...\Harry Potter and the Half Blood Prince\GameSettings` (Width,
   BitDepth, ShadowLOD, MaxFarClip, ParticleRate, les six paires
   Mode%dWidth/Height, OptionResolution, OptionLOD, OptionBrightness,
   MaxTextureSize, etc.) via deux lecteurs : un pour les entiers
   (`005d7d40`) et un pour les chaînes (`005d7810`, PosX/PosY/SizeX/SizeY/
   Maximized/Minimized). Ces deux fonctions et leurs dépendances
   d'écriture (`005d7ae0`, `005d73c0`, `005d7650`, `005d6d80`) utilisent
   le mécanisme SEH des exceptions C++ MSVC 2008 et la disposition interne
   de `std::basic_string` ; le lot `registry_settings_055` a tenté leur
   reconstruction et a été **différé** sur les six (le vérificateur
   objectif de ReAgent a relevé de vrais écarts de flux de contrôle/nombre
   d'appels, pas de simples remarques stylistiques). Seul le lecteur de
   feuille `005d7d00` (`read_registry_value`, un `RegQueryValueExA` plafonné
   à 0xff octets) est **intégré**.
4. `create_game_window` (005d8ed0, **intégrée**) avec les dimensions
   lues au registre.
5. Si la fenêtre existe : `GetClientRect`, `service_holder` (005db920,
   **intégrée**) puis **un appel virtuel sur son slot +4** avec
   `startup_resource_size(8, 0x10)` (006dadb9, **intégrée**) comme
   argument — c'est très probablement la création du device Direct3D 9
   (ou d'un objet qui l'encapsule). Puis une dizaine d'initialisations non
   reconstruites : `005dca10` (redimensionnement d'affichage,
   ShowCursor+memset), `00688130`, `006b3a70`, `00625ce0` (probablement
   audio, vu leur petite taille et leurs sous-appels courts),
   `thunk_FUN_00ff80a0` (005d6f30, module secondaire 0x0100, 2451
   caractères), `005dcdf0`.
6. `ShowWindow`/`UpdateWindow`, puis `local_1ac = FUN_005d8fc0()`.
   **`005d8fc0` est la vraie boucle de messages Win32** : confirmé par ses
   appels à `PeekMessageA`/`TranslateMessage`/`DispatchMessageA`/
   `GetTickCount`, la détection de `WM_QUIT` (message == 0x12) et une
   limitation de fréquence via `Sleep`. Chaque itération commence par
   `FUN_0064aa60()` — l'appel « un tick de moteur » — avant de dépiler les
   messages Windows.
7. **`FUN_0064aa60` est le gestionnaire classique de device Direct3D 9
   perdu/réinitialisé** : `DAT_00cf1944` est un pointeur `IDirect3DDevice9*`
   (table virtuelle COM), l'appel au slot +0xc est
   `TestCooperativeLevel()`, celui au slot +0x40 est `Reset()`, et les
   codes de retour comparés (`-0x7789f798`, `-0x7789f797`, `-0x7789f7d9`,
   `-0x7ff8fff2`, `-0x7789fe84`, `-0x7789f794`) correspondent exactement
   aux HRESULT D3DERR_DEVICELOST / D3DERR_DEVICENOTRESET / Driver internal
   error / Out of memory / Out of video memory / Invalid call. Si le
   device est valide (ou vient d'être réinitialisé), elle appelle
   `FUN_0064a810()` — **la piste la plus probable pour trouver le rendu
   d'une frame et donc l'affichage de l'intro/menu**, non encore examinée.
8. Après la boucle : `FUN_005dced0`, `FUN_005d7250`, `FUN_00648e70`,
   `FUN_006882b0`, `thunk_FUN_0100e6e0` (005d8080, 4791 caractères, gros
   thunk du module secondaire — candidat sérieux pour l'init/désinit
   Direct3D complète), appels virtuels sur `DAT_00fcaf34`, `FUN_006599b0`
   (appelle `XInputEnable` — entrée Xbox 360), boucle `ShowCursor`,
   `apply_system_parameters('\x01')` (restauration), puis
   `TerminateProcess` (sortie normale en fin de partie, pas un crash).

Trois nouvelles dépendances externes ont été enregistrées pour débloquer ce
lot : `_CxxThrowException` (mécanisme de lancement d'exception C++ MSVC) et
deux fonctions internes de `std::basic_string` (réservation/croissance),
toutes trois traitées comme les imports CRT déjà suivis (sprintf, memcpy,
tolower, strchr) — un vrai `std::string`/`throw` les remplace, aucune
reconstruction bas niveau n'est nécessaire.

## 17 oct 2026 (suite) : table virtuelle Direct3D 9 confirmée, convention
## d'appel COM identifiée

En descendant depuis `FUN_0064aa60`, `FUN_0064a810` appelle `FUN_00642420`
(`reset_dynamic_resources`, **intégrée**, `src/render/DeviceReset.cpp`),
qui recrée les shaders après un reset de device. Cette chaîne a confirmé
plusieurs offsets de la table virtuelle `IDirect3DDevice9` par
recoupement indépendant (signature d'appel + nombre/ordre des arguments) :

| Offset | Index | Méthode               |
|--------|-------|-----------------------|
| +0xc   | 3     | TestCooperativeLevel  |
| +0x40  | 16    | Reset                 |
| +0x68  | 26    | CreateVertexBuffer    |
| +0x16c | 91    | CreateVertexShader    |
| +0x1a8 | 106   | CreatePixelShader     |
| +8     | 2     | Release (tout objet COM) |

**Point important pour la suite** : chaque appel virtuel COM observé
pousse explicitement le pointeur device/objet sur la pile juste avant
l'appel indirect (confirmé dans l'assembleur à chaque fois), au lieu de
le garder dans ECX. C'est la convention `STDMETHODCALLTYPE` standard
(`__stdcall` sur x86) — différente du `__thiscall` que ce projet utilise
partout ailleurs pour les tables virtuelles internes au jeu
(`ServiceLifecycle`, `PoolList`, `VolumeRegistry`). ReAgent a
systématiquement mal typé ces appels en `__thiscall` dans ce lot ; **toute
future reconstruction d'un appel virtuel sur `DAT_00cf1944` (ou tout autre
pointeur COM de rendu) doit utiliser `__stdcall`**.

`FUN_0064a810` lui-même reste à reconstruire : il manipule des surfaces/
swap chains via les offsets +0x14, +0x30, +0x74, +0x5c, +0x94, +0x9c sur
`DAT_00cf1944`/`DAT_00cf1954`/`DAT_00cf1958`, dont l'identification exacte
(GetBackBuffer, SetRenderTarget, etc.) n'a pas encore été confirmée par
recoupement — à faire avant de l'intégrer, pour éviter d'assigner un nom
de méthode incorrect. **Attention** : certains de ces offsets (ex. +0x14)
ne correspondent PAS directement à l'index attendu de la table ci-dessus
quand on compte le nombre d'arguments observés dans l'appel — à
re-vérifier soigneusement (peut-être un objet wrapper avec des slots
supplémentaires, pas l'interface COM brute) avant toute intégration.

## 17 oct 2026 (suite 2) : profil de render states par défaut, et un appel
## passé par ESI plutôt que par la pile

`FUN_0064aa60` (gestionnaire de device perdu) appelle `FUN_00642420`
(`reset_dynamic_resources`), qui à son tour ne dépend plus de rien de
non-résolu après ce lot. Un chemin séparé, depuis `FUN_0064aa60` →
`FUN_0064aa30` (`cache_available_texture_memory`, **intégrée**), ajuste la
qualité des textures selon la mémoire vidéo disponible via
`adjust_texture_quality_for_memory` (**intégrée**,
`src/render/DeviceReset.cpp`).

`initialize_default_render_state` (00640fb0, **intégrée**,
`src/render/RenderStateCache.cpp`) remplit un profil de 19 mots de
render/texture-stage states par défaut (143 affectations littérales,
vérifiées programmatiquement avant intégration) puis l'applique via
`apply_render_state_profile` (010021f0 + thunk 006406a0, **intégrée**).
Cette dernière est reçue dans le binaire exporté via **ESI, pas la pile**
— premier cas rencontré dans ce projet d'un paramètre transmis par
registre plutôt que par convention d'appel standard ; confirmé car
`00640fb0` charge explicitement ESI avec l'adresse de son propre tampon
juste avant l'appel, sans qu'aucune autre instruction ne modifie ESI
entre-temps. Rendu explicite comme un paramètre `const uint32_t*` dans la
reconstruction plutôt que de dépendre d'un registre non documenté.

Table virtuelle `IDirect3DDevice9`, offsets supplémentaires confirmés ce
lot : +0x10 GetAvailableTextureMem (index 4), +0xe4 SetRenderState
(index 57), +0x10c SetTextureStageState (index 67).

## 17 oct 2026 (suite 3) : `FUN_0064a810` entièrement résolu et intégrée

`FUN_0064a810` est maintenant intégrée (`prepare_render_target_texture`,
`src/render/RenderTargetSetup.cpp`). L'hypothèse initiale
(`DAT_00cf1948` = `IDirect3D9*`, +0x14 = `GetAdapterIdentifier`) était
fausse : la confirmation par le nombre d'arguments était une coïncidence.
En suivant la donnée réelle (le mot écrit dans `DAT_00cf1954` est relu
comme un pointeur COM avec son propre `vtable+0x30`, qui s'avère être
`GetDesc`), la bonne lecture est : **`DAT_00cf1948` =
`IDirect3DSwapChain9*`**, et +0x14 = `GetBackBuffer(0, 0, &DAT_00cf1954)`
(index 5), qui produit une surface. Tous les offsets suivants ont été
confirmés un par un contre le `d3d9.h` réel (Wine, ordre de déclaration
COM standard) :

| Offset | Index | Interface | Méthode |
|--------|-------|-----------|---------|
| +0x14  | 5     | IDirect3DSwapChain9 | GetBackBuffer |
| +0x30  | 12    | IDirect3DSurface9   | GetDesc |
| +8     | 2     | (tout objet COM)    | Release |
| +0x74  | 29    | IDirect3DDevice9    | CreateDepthStencilSurface |
| +0x5c  | 23    | IDirect3DDevice9    | CreateTexture |
| +0x48  | 18    | IDirect3DTexture9   | GetSurfaceLevel |
| +0x94  | 37    | IDirect3DDevice9    | SetRenderTarget |
| +0x9c  | 39    | IDirect3DDevice9    | SetDepthStencilSurface |

La fonction récupère le format du back buffer (`GetDesc`), crée une
surface depth-stencil assortie, puis — sauf si désactivé
(`DAT_00cf1998`) ou bloqué par un bit de capacité par mode
(`compute_dynamic_table_size()+0x3c`) — crée une texture de rendu hors
écran du même format et bascule le rendu sur son niveau 0 (rendu vers
texture, probablement pour la post-production : bloom, etc., cohérent
avec les options `DisableFog`/`DisablePostPro` vues dans `WinMain`).
Placée dans son propre fichier (`RenderTargetSetup.cpp`, pas
`DeviceReset.cpp`) pour éviter que les tests qui n'en ont pas besoin
n'aient à fournir les ~130 globals du profil de render states par
défaut qu'elle appelle en dernier (`initialize_default_render_state`).

**Enseignement méthodologique important** : `thunk_FUN_0100e6e0` (exploré
en pensant qu'il s'agissait de l'init/désinit D3D) s'est révélé être une
fonction d'**écriture de réglages dans le registre** à la fermeture du
jeu (mêmes clés que `005d9300` : OptionResolution, OptionLOD...),
dépendant des lecteurs/écrivains différés (`005d7ae0`, `005d73c0`). Elle
montre aussi que la séquence `iStack_c = 0xffffffff; puStack_10 =
&this_xxx; pvStack_14 = ExceptionList; ExceptionList = &pvStack_14;` au
début des fonctions à chaînes C++ est **le prologue SEH générique que
MSVC génère automatiquement** pour toute fonction ayant des objets
`std::string` locaux avec destructeur — ce n'est pas de la logique métier
mais un artefact du compilateur, qu'un compilateur moderne régénère de
lui-même pour peu qu'on écrive du C++ normal (RAII, pas de gestion
manuelle de `ExceptionList`). C'est très probablement pourquoi le
vérificateur objectif de ReAgent comptait deux fois plus de branches/
appels que les candidats pour `005d7d40`/`005d7810`/etc. (lot
`registry_settings_055`) : la différence vient surtout de ce prologue/
épilogue répété à chaque construction/destruction de `std::string`, pas
d'une erreur de logique. Une nouvelle tentative sur ces six fonctions
différées, en ignorant délibérément cette différence de nombre
d'appels/branches (comme déjà fait pour les cas de polarité de branche
immatériels), est probablement plus prometteuse qu'une reconstruction
bas niveau complète de la disposition `std::basic_string`.

## 17 oct 2026 (suite 4) : les lecteurs/écrivains de registre résolus —
## disposition `std::basic_string` MSVC 2008 confirmée

Les six fonctions différées depuis le lot `registry_settings_055` sont
maintenant intégrées dans `src/platform/RegistryConfig.cpp` (sauf
`005d6d80`, remplacée par un simple `throw std::runtime_error`, qui n'a
donc plus de contrepartie à intégrer). La disposition mémoire de
`std::basic_string<char>` sous Visual C++ 2008 a été confirmée par
recoupement (recherche web + cohérence interne) :

```
+0x00  en-tete de debogage d'iterateur (4 octets, _Container_base, non
       utilise par notre reconstruction)
+0x04  union _Bx (16 octets) : tampon SSO de 16 caracteres, ou pointeur
       heap dans les 4 premiers octets
+0x14  _Mysize (4 octets) : longueur
+0x18  _Myres  (4 octets) : capacite
total : 0x1c = 28 octets, exactement la taille observee partout dans ce
binaire pour les basic_string locaux ("local_xx [28]")
```

Le decalage de +4 par rapport a la disposition "manuel" habituelle
(tampon a +0, taille a +0x10, capacite a +0x14) correspond a l'en-tete
de 4 octets que MSVC 2005/2008 ajoute pour le debogage d'iterateurs
("checked iterators"), present par defaut meme en configuration
Release sauf desactivation explicite. Cette disposition n'a toutefois
pas besoin d'etre reproduite dans le code reconstruit : un `std::string`
moderne (`.c_str()`, `.empty()`, `+=`, affectation) est comportementalement
equivalent partout ou le binaire original manipule ces decalages bruts.

`005d7650` (lecture d'une valeur chaine depuis une cle deja ouverte)
manipule directement l'allocateur interne de la STL pour agrandir le
tampon sur `ERROR_MORE_DATA` ; la reconstruction utilise un
`std::vector<char>` redimensionne de la meme facon, comportementalement
identique sans reproduire cet ABI interne. `005d7ae0` (ecriture d'un
entier) recoit la valeur a ecrire via un argument de pile cache que
Ghidra n'a pas nomme comme parametre formel — retrouve en tracant le
site d'appel dans la branche de promotion HKLM->HKCU de `005d7d40` —
rendu explicite comme parametre normal dans la reconstruction.

Le texte exact du message d'exception ("Bad registry location..."/"Bad
data to write to registry...") est reconstruit a partir des fragments
litteraux visibles dans le decompile, sans certitude a 100% sur l'ordre
exact d'assemblage (ambiguite de rendu de Ghidra sur les appels
`operator+` imbriques) ; sans consequence fonctionnelle puisque ce
chemin n'est jamais atteint avec les noms de cles fixes que WinMain
utilise.

Compilation Win32 et les 28 tests CTest reussissent (registry_config_smoke
couvre l'aller-retour lecture/ecriture entier et chaine, le chemin
"valeur absente -> defaut", l'agrandissement de tampon sur chaine longue,
et l'exception sur nom de cle vide, avec un repertoire de registre
jetable sous `Software\Electronic Arts\<app de test>`). 307 des 321
fonctions suivies sont integrees (0 bloquee, 0 en attente de revue, 14
differees).

## 17 oct 2026 (suite 5) : `thunk_FUN_0100e6e0` résolue — bien plus
## simple que redouté

Contrairement à l'hypothèse initiale ("init/désinit D3D"), cette fonction
(atteinte via le thunk `005d8080`) se limite à **trois écritures de
réglages d'affichage** au registre : `save_display_options`
(`src/platform/RegistryConfig.cpp`), appelée côté fermeture du jeu.
Écrit `OptionResolution`/`OptionLOD`/`OptionBrightness` sous
`HKCU\Software\Electronic Arts\Harry Potter and the Half Blood
Prince\GameSettings`. Rien à voir avec Direct3D — ce nom de thunk était
une impasse pour trouver le chemin vers l'intro.

## 17 oct 2026 (suite 6) : le "frame pump" (0x005e0cc0) résolu

Appelée depuis le message loop (0x005d8fc0) quand `PeekMessageA` ne
rapporte aucun message en attente. Minuteur multimedia (0x005e0b90) +
accumulateur 64 bits à pas fixe (Q16.16) qui, une fois le seuil de
simulation atteint, appelle une méthode virtuelle "simulate" sur un
objet opaque (`DAT_0089eb0c`), puis — séparément, selon un second seuil
de rendu — une méthode "render" sur le même objet ; un repli (méthode
virtuelle sur l'objet à adresse fixe `DAT_00fc7f90`) s'exécute quand ni
l'un ni l'autre ne s'est déclenché. Les deux appels sur `DAT_0089eb0c`
sont confirmés **__thiscall** par l'assembleur brut (`MOV ECX,
[0x89eb0c]` juste avant chaque `CALL`, ECX jamais poussé) — cohérent
avec le reste du projet (tables virtuelles internes = `__thiscall`,
seul Direct3D 9/COM utilise `__stdcall`). Le type concret de
`DAT_0089eb0c` (le plus probable candidat pour être le "jeu"/"scène"
actif) n'est toujours pas identifié.

Intégré dans `src/runtime/FrameClock.cpp` : `multimedia_timer_elapsed_ms`,
`accumulate_simulation_time_sample` (paramètre EDI rendu explicite),
`interpolate_render_time`, `drain_pending_visual_effects`,
`run_frame_tick`. Détail complet (SBORROW/CARRY comme artefacts de
décomposition 64 bits, `extraout_ST0` comme artefact du décompilateur
plutôt qu'un vrai paramètre caché, la boucle do-while de
`FUN_005e0b10` comme artefact de layout mémoire) dans
`architecture/decisions.md`.

**Découverte séparée, non résolue** : `drain_pending_visual_effects`
dépend de `0x00606b00`, un sous-système de déclenchement d'effets
visuels ("BLOOM"/"GLASS"/"BACKDROP") non reconstruit, tracé différé.
Code mort tant que rien dans `src/` n'écrit `DAT_00cef790`.

## 17 oct 2026 (suite 7) : DAT_0089eb0c et DAT_00fc7f90 identifiés —
## ni l'un ni l'autre ne mène à l'intro

Recherche de xrefs en direct sur le projet Ghidra existant (lecture
seule, `pyghidra` — mêmes identifiants que `tools/read_ghidra_bytes.py`)
pour trouver qui écrit ces deux pointeurs. Résultat concret, mais
négatif pour la piste de l'intro :

- **`DAT_0089eb0c`** est écrit une seule fois, dans `FUN_005dcdf0`
  (`DAT_0089eb0c = &PTR_PTR_0087eadc;`), elle-même appelée directement
  depuis `WinMain` (`FUN_005d9300`). `FUN_005dcdf0` initialise aussi le
  frame-clock (`FUN_005e0bf0`, déjà intégrée comme
  `hp6::runtime` — voir plus haut) puis enregistre une demi-douzaine de
  gestionnaires de messages internes nommés ("iMsgDeleteEventHandler",
  "iMsgStartSystem", "iMsgStopSystem", ...) via `FUN_005dae30`, et
  délègue à plusieurs sous-systèmes via `thunk_FUN_00ff7da0`/
  `thunk_FUN_00ff7e80` (8 fonctions encore inexplorées : `FUN_005a3710`,
  `FUN_005d5ea0`, `FUN_005d6140`, `FUN_005dc750`, `FUN_005e0f70`,
  `FUN_00686c00`, `FUN_0068bc60`, `FUN_0068bdd0`). `DAT_0089eb0c` n'est
  JAMAIS réécrit ailleurs : l'objet actif ne change jamais pendant toute
  l'exécution.
- La table virtuelle de `&PTR_PTR_0087eadc` (3 méthodes seulement, à
  `0x5dcd80`/`0x5dcdc0`/`0x5dcda0`, jamais détectées comme fonctions par
  l'analyse automatique de Ghidra puisqu'atteintes uniquement par cet
  appel indirect) s'avère être un **compteur de délai + overlay de
  diagnostic**, pas la logique du jeu : slot2 ("reset") relance un
  compte à rebours de 5 images (`DAT_00cef71c`) et efface un drapeau
  "prêt" (`DAT_00866e28`) ; slot1 ("render") décrémente ce compteur et,
  une fois à zéro, pose le drapeau et appelle `FUN_005dd020` — qui
  construit la chaîne `"Build: FINAL on %s at %s "` (date/heure de
  compilation) plus des infos de résolution/plein écran. Probablement
  le texte de débogage affiché en overlay dans ce build final.
- **`DAT_00fc7f90`** (repli de `run_frame_tick` quand ni simulate ni
  render ne s'est déclenché) est écrit par `FUN_0070ffd0`
  (`MOV dword ptr [0x00fc7f90],0x843d5c`). Sa table virtuelle (lue
  directement à `0x843d5c`) pointe vers des fonctions référençant
  `PTR_PTR_008813c8` et `DAT_00fc7e10`/`DAT_00fc7e14`/`DAT_00fc7e20` —
  `PTR_PTR_008813c8` est le même symbole utilisé par `FUN_005a1cd0`
  (appelée juste avant l'initialisation du frame-clock dans
  `FUN_005dcdf0`), qui calcule un hash de la chaîne littérale
  `"SSGAudio"`. Piste probable : gestion du sous-système audio, pas la
  logique d'intro/menu non plus.
- Le focus-change dans la boucle de messages (`FUN_006599b0`, appelée
  côté perte de focus) ne fait que désacquérir le clavier/joystick
  DirectInput et désactiver XInput — pas de lien avec l'intro.

**Conclusion** : la piste du "frame pump" (`run_frame_tick` et ses deux
objets dispatchés) ne mène ni à l'intro ni au menu — c'est de la
télémétrie/diagnostic (overlay de build) et de la gestion
audio/périphériques d'entrée. La vraie machine à états intro/menu/jeu
n'est pas visible depuis le voisinage immédiat de la boucle de messages
; elle est probablement dans l'un des sous-systèmes encore inexplorés
listés ci-dessus, ou ailleurs dans `WinMain` entre la création du device
et la boucle de messages.

### Prochaine étape recommandée

Trois chantiers ouverts, aucun avec un signal fort indiquant lequel mène
le plus directement à l'intro — à discuter avant de s'engager dans l'un
d'eux, chacun étant d'une ampleur comparable aux blocages précédents :
1. Les 8 fonctions de sous-système encore inexplorées appelées depuis
   `FUN_005dcdf0` via `thunk_FUN_00ff7da0`/`thunk_FUN_00ff7e80`
   (`FUN_005a3710`, `FUN_005d5ea0`, `FUN_005d6140`, `FUN_005dc750`,
   `FUN_005e0f70`, `FUN_00686c00`, `FUN_0068bc60`, `FUN_0068bdd0`).
2. Le système de gestionnaires de messages nommés lui-même
   (`FUN_005dae30`, utilisé ~9 fois ; SEH + chaîne d'aides encore
   inexplorée `thunk_FUN_00ff9f90`/`FUN_005dffc0`/`FUN_005df8f0`/
   `FUN_005dafa0`) — comprendre "iMsgStartSystem"/"iMsgStopSystem"
   pourrait révéler comment le jeu bascule entre intro/menu/gameplay.
3. `0x00606b00` (déclenchement d'effets visuels), toujours différée.

Piste secondaire non explorée : les fonctions d'initialisation
restantes entre la création de fenêtre et la boucle de messages dans
`WinMain` (`FUN_00688130`, `FUN_006b3a70`, `FUN_00625ce0`,
`thunk_FUN_00ff80a0`) ; `thunk_FUN_0100ea00`/`thunk_FUN_0100e9b0`
(focus-gagné, selon un compteur `DAT_00cef55c`).

## 17 oct 2026 (suite 8) : le framework "GOF" et sa machine à états —
## piste la plus prometteuse à ce jour, encore non résolue

`FUN_005d6140` (appelée depuis `thunk_FUN_00ff7da0`, lui-même appelé
depuis `FUN_005dcdf0`/WinMain) s'avère être **`GameServicesCreate`** :
elle alloue (via `FUN_005dc750(taille, nom)`, qui n'est PAS un
registre de systèmes nommés comme supposé mais un **allocateur tagué**
— le nom est juste une étiquette de debug pour le suivi mémoire, pas
une clé de recherche runtime) puis construit à la main (vtable +
initialiseur) une vingtaine de singletons moteur, tous préfixés
`GOF::GameServicesCreate::` (GOF = très probablement *Game Object
Framework*, un framework interne EA ; confirmé par `EASTLLocalAllocatorModule`
= EASTL, la bibliothèque standard interne d'EA) :

`EASTLLocalAllocatorModule`, `PresentationListManager`,
`PresentationEventHandler`, `PresentationOptionsHandler`,
**`FMVDataManager`**, `StringTableDataManager`, `UIComponentManager`,
`PresentationTaskHandler`, `MessageDataManager`, `EventManager`,
`RandomNumberManager`, `ChapterData`, `ChapterManager`,
**`GameFlowStateController`**, `Manager2D`, `NStorage::System`,
`CameraManager`, `GameCameraManager`, `PresentationManager`,
`ProfileManager`, `HPTrinityManager`, `DoorColliderManager`.

`FMVDataManager` (vidéos plein écran, probablement les logos/l'intro
cinématique) et toute la famille `Presentation*` (gestionnaire de
tâches, d'événements, d'options, de listes) sont les candidats les
plus directs pour l'intro/menu.

**`GameFlowStateController`** est une **machine à états finis**
(`FiniteStateMachine::RegisterState`, elle aussi juste un allocateur
tagué). Son constructeur (`thunk_FUN_00fdfe00`, singleton
`DAT_00fcae50`) enregistre exactement 5 états au démarrage (identifiés
en lisant directement leurs tables virtuelles — méthode "GetStateID"
toujours au slot +0x10/+0x14, méthode "GetStateName" toujours au slot
+0x1c retournant un pointeur de chaîne littérale ; toutes les 5
tables virtuelles partagent le même destructeur générique à
`0x4da470`, confirmant une classe de base commune
`FiniteStateMachine::State`) :

| ID | Nom                  | Vtable     | Stockage             |
|----|----------------------|------------|-----------------------|
| 1  | `Idle`               | 0x82b428   | (table interne seule) |
| 2  | `GuiControlGamePaused`| 0x82b468  | (table interne seule) |
| 3  | `GameRunning`         | 0x82b448  | (table interne seule) |
| 4  | `EnterLoadingState`   | 0x82e86c  | `_DAT_00cef564`       |
| 5  | `LoadLevelState`      | 0x82e88c  | `DAT_00cef560`        |

Aucun de ces 5 états n'est "Intro" ou "Menu" — ce sont les états du
**gameplay en cours** (idle au démarrage, en cours, en pause, en train
de charger un niveau). `Idle` n'a même pas de méthode "Update" réelle
(stub `XOR AL,AL; RET`) : rien ne pousse cette machine hors de `Idle`
depuis l'intérieur d'elle-même. L'intro/menu est donc probablement
piloté par un système différent, pas par cette machine à états
précise.

**Piste la plus concrète trouvée à ce jour** : recherche de chaînes
contenant "intro"/"menu" dans `architecture/technical_strings.csv` a
remonté `IntroCamera`, et surtout
`Presentation.Screens.ActivateScreenWithFocus ScreenMainMenuCameraPos`
— une chaîne de **commande de script** (syntaxe
`Namespace.Verb Argument`), cohérente avec `PresentationTaskHandler`/
`PresentationEventHandler` étant un système de commandes textuelles
piloté par des données (pas du code C++ a appel direct). Un seul
site de référence trouvé pour cette chaîne précise, à `0x0048b005`
(fonction jamais détectée par Ghidra, atteinte uniquement par appel
indirect — même situation que les méthodes de machine à états) :
`MOV ESI,<&chaine>; CALL 0x0048ab90`. La fonction adjacente à
`0x0048b050` manipule explicitement `DAT_00fcadf0`
(`PresentationTaskHandler`, confirmé) pour allouer un "task" de 16
octets portant une chaîne de nom en argument (`0x826498`, pas encore
lue) — cohérent avec "créer et poster une tâche de présentation
nommée".

## 17 oct 2026 (suite 9) : la table des 93 commandes de présentation —
## la carte complète du menu/intro, pas encore le code

Poursuite de la piste `0x0048ab90`/"Presentation.Screens.ActivateScreenWithFocus"
jusqu'au bout. `0x0048ab90` est en fait déjà exportée côté ReAgent
(`FUN_0048ab90`) ; sa logique : découper la chaîne de commande reçue en
un verbe et un reste d'arguments (`strchr(' ')`), puis chercher le
verbe dans une **table de 93 entrées à `0x00876ab0`**
(`int*[93]`, chaque élément pointant vers une struct `{char* nom;
void* gestionnaire;}`) via comparaison de chaîne linéaire, et appeler
le gestionnaire trouvé avec le reste de la commande. C'est
l'**interpréteur central de toutes les commandes de script
"Presentation.*"/"Storage.*"/etc. du jeu**. Les 93 entrées (nom +
adresse du gestionnaire) sont sauvegardées dans
`architecture/presentation_commands.csv` (table de référence,
récupérée en lisant la mémoire du projet Ghidra existant en direct via
`pyghidra`, sans modifier le projet).

Points clés de cette table pour la piste intro/menu :
- **`Storage.Bootup`** (`0x0048b050`) : crée une tâche
  `Presentation:TaskHandler::Task_Bootup` (vtable `0x828338`),
  soumise à la file de `PresentationTaskHandler` (`DAT_00fcadf0`) via
  `FUN_005d2b30` (générique, "ajouter une tâche à la file liée").
  **Machine à états entièrement cartographiée** (méthode "Update" au
  slot vtable +0xc, dispatch par `JMP [EAX*4+0x59f7c0]`, 9 états) :
  - état 1 : lance un chargement asynchrone sur `DAT_00fcae18`+8 (appels
    vtable +0x60/+0x64, deux notifications de progression via
    `FUN_006b450f`).
  - état 2 : attend que ce chargement soit prêt (boucle sur lui-même
    tant que le drapeau à `[résultat vtable+0x64]+7` est faux).
  - état 3 : itère une liste d'éléments (`vtable+0x64`, compteur à
    +0x10), calcule une longueur de chaîne et appelle un IAT
    (`[0x713200]`, probablement allocation/registration de chaîne).
  - état 4 : attend une seconde condition sur `DAT_00fcae18` (deux
    vérifications booléennes, vtable +0x58 puis +4/+8 sur le résultat).
  - état 5 (code tronqué, non relu en détail).
  - état 6 : identique au point de sortie générique (pas de traitement
    propre).
  - état 7 : vérifie une condition liée à `DAT_00d46aa8` via
    `FUN_005acf90`, puis appelle `vtable+0x8c` sur `DAT_00fcae18`+8.
  - **état 8 (final)** : exécute **deux commandes de script** via
    `0x0048ab90` — `"Presentation.StorageTRCPopupActive"` puis
    `"Presentation.Screens.ActivateScreenWithFocus
    ScreenAutosaveWarningMessage"` — puis pose
    `DAT_00cef63f = 1` ("bootup terminé").
  Conclusion : `Storage.Bootup` mène à l'écran d'avertissement de
  sauvegarde (TRC console), pas directement au menu — mais confirme le
  mécanisme exact (tâches à états + commandes de script) qui doit
  ensuite enchaîner vers l'intro/le menu.
- **`TriggerBootupFlow`** (`0x0048c970`, commande #73) crée une AUTRE
  tâche, `Presentation:TaskHandler::Task_BootupFlowControl` (vtable
  `0x828360`, 8 octets, distincte de `Task_Bootup`), soumise à la même
  file. Son slot vtable +0xc (a priori l'équivalent d'"Update") n'est
  PAS une machine à états comme `Task_Bootup` — c'est une petite
  fonction de validation (`DS_HIGHLIGHTTEXT`, `DAT_00fcae04`) dont le
  rôle reste à élucider ; **probablement pas le bon point d'entrée**,
  ou bien le dispatch d'état de cette classe n'est pas au même offset
  que celui de `Task_Bootup` (sa hiérarchie de classes diffère déjà :
  son slot +4 est un stub `MOV AL,1; RET` contre une vraie méthode chez
  `Task_Bootup`).
- **Candidats directs pour l'intro/menu dans la table** (non encore
  tracés) : `FMVPlayback` (#79, `0x0048cd90` — lecture de vidéo plein
  écran, très probablement les logos/l'intro cinématique),
  `Presentation.LeaveTitlePage` (#81, `0x0048cef0` — sortie de l'écran
  titre), `OnActivateFrontEndChapter` (#75, `0x0048c9e0` — probable
  entrée dans le frontend/menu), `Presentation.StartGame`/
  `Presentation.ContinueGame` (#12/#13).

## 17 oct 2026 (suite 10) : la boucle est bouclée — qui émet
## `TriggerBootupFlow`, et exactement quand

Recherche des xrefs (en direct, pyghidra) vers la chaîne de commande
`"TriggerBootupFlow"` (entrée #73 de la table, à `0x00826238`) : une
seule fonction la référence, `FUN_00ff7fa0` (déjà exportée par
ReAgent). Sa décompilation montre un enchaînement direct : chargement
de ressources ("SSGShaders", "Shaders.str", "Game.str"), un test de nom
de chapitre contre `"Global"`, et — si vrai — `FUN_0048ab90()` (la
chaîne de commande passée par ESI, paramètre caché, comme pour les
autres sites d'émission de commande). `FUN_00ff7fa0` est appelée via
`thunk_FUN_00ff7fa0` (`0x005d6cb0`), lui-même appelé **une seule fois,
directement depuis `WinMain`** (`FUN_005d9300`, ligne ~653 du C
décompilé).

Séquence exacte dans `WinMain`, juste après la création de la fenêtre
et du device (lignes ~634-655 du C décompilé exporté) :

```
DAT_00cef698 = <creation device D3D9>
FUN_005dca10(largeur_client, hauteur_client)
FUN_00688130(); FUN_006b3a70(); FUN_00625ce0(); thunk_FUN_00ff80a0();   // inits non explorees
DAT_0089c427 = 1;
DAT_00ba9504 = 2;                 // lu par FUN_005e0bf0 (reset_frame_clock, deja integree)
FUN_005dcdf0();                   // bootstrap moteur : systeme de messages + frame clock +
                                   // GameServicesCreate (FMVDataManager, PresentationTaskHandler,
                                   // GameFlowStateController, UIComponentManager, ...)
bVar3 = thunk_FUN_00ff8ce0();     // garde-fou materiel (manette ?) ; inconditionnellement vrai
                                   // quand DAT_00cf1b00 == 0 (cas normal PC de detail)
if (bVar3) {
    thunk_FUN_00ff7fa0();         // emet la commande "TriggerBootupFlow"
                                   // -> tache Task_BootupFlowControl (PresentationTaskHandler)
    DAT_00cef68e = 1;
    if (DAT_00866e2a == '\0') {
        // lit le reglage "Maximized" (section GameSettings, deja integre :
        // hp6::platform::read_string_setting) pour l'etat de la fenetre
    }
    ...
}
```

**C'est le point de départ confirmé et complet de toute la séquence
d'intro/menu** : `WinMain` → `FUN_005dcdf0` (bootstrap + allocation des
services) → `TriggerBootupFlow` → `Task_BootupFlowControl` (dont le
rôle exact d'orchestration reste à élucider — voir ci-dessus) → (via
`Storage.Bootup`/`Task_Bootup`, une tâche à 9 états déjà cartographiée
en détail) → écran d'avertissement de sauvegarde → [probablement
`FMVPlayback`, `Presentation.LeaveTitlePage`, `OnActivateFrontEndChapter`
— non encore tracés].

### Prochaine étape recommandée

La chaîne complète WinMain → bootstrap → `TriggerBootupFlow` →
`Task_BootupFlowControl`/`Task_Bootup` → écran d'avertissement de
sauvegarde est maintenant confirmée de bout en bout par lecture directe
de l'assembleur (pas seulement par inférence). Pour aller jusqu'à
l'intro/menu proprement dits, dans l'ordre le plus probable :
1. Décompiler `Task_BootupFlowControl` plus en profondeur (son slot
   "Update" n'est pas une machine à états simple comme `Task_Bootup` —
   il faut comprendre comment il enchaîne réellement vers
   `FMVPlayback`/`Presentation.LeaveTitlePage`).
2. `FMVPlayback` (`0x0048cd90`) et `Presentation.LeaveTitlePage`
   (`0x0048cef0`)/`OnActivateFrontEndChapter` (`0x0048c9e0`).

Aucune intégration C++ n'a été tentée pour `Task_Bootup`/
`Task_BootupFlowControl`/`FUN_00ff7fa0` cette section : la logique est
comprise au niveau conceptuel (états, commandes, point de départ exact
dans WinMain), mais plusieurs globaux et sous-fonctions (`DAT_00fcae18`,
`FUN_006b450f`, `FUN_005acf90`, le contenu exact des états 5/6 de
`Task_Bootup`) ne sont pas encore résolus en détail — nécessaire avant
toute reconstruction C++ vérifiée et testée.

## 17 oct 2026 (suite 12) : les primitives génériques intégrées —
## `allocate_tagged`, `submit_task`, et le dispatcher de commandes

Les quatre primitives génériques identifiées au cours de cette
exploration (toutes déjà exportées par ReAgent) sont maintenant
intégrées et testées dans `src/presentation/` :

- `allocate_tagged` (0x005dc750) : réutilise directement
  `hp6::platform::service_holder` (même bloc d'init paresseux que
  0x005db920/`release_through_service`, déjà intégré) — le second
  pointeur de fonction partagé (`DAT_00fcdd78`) est la "fonction
  d'allocation nommée" globale, utilisée tout au long de
  `GameServicesCreate` pour les ~20 singletons du moteur.
- `submit_task` (0x005d2b30) : ajoute une tâche à la file propre d'un
  `PresentationTaskHandler` (son champ +0xc), via un allocateur
  **différent** et imbriqué à son champ +0x8 (pas `allocate_tagged`) —
  distinction confirmée en relisant l'assembleur brut avec soin (deux
  mécanismes d'allocation distincts coexistent dans le moteur : un
  service nommé global pour les gros singletons, des allocateurs locaux
  par sous-système pour les petits objets fréquents comme les tâches).
  Appelle ensuite la méthode "IsReady" de la tâche (slot vtable +0x4,
  thiscall sans argument explicite) et pose son octet +0x4 à 1 si elle
  répond faux.
- `dispatch_presentation_command`/`find_and_invoke_command`
  (0x0048ab90/0x0048abf0) : l'interpréteur central de commandes de
  script, découpe "Verbe Arguments" puis cherche le verbe dans la table
  de 93 entrées (`architecture/presentation_commands.csv`). La boucle
  de comparaison personnalisée de l'assembleur original est reproduite
  via `std::strcmp(...)==0`, équivalente ici puisque cette fonction
  n'utilise jamais le signe du résultat.

Quatre nouveaux tests CTest (`task_system_smoke`,
`command_dispatch_smoke`) couvrent ces primitives avec des tables/objets
factices, selon le style déjà établi dans ce projet. Un piège rencontré
en testant `find_and_invoke_command` : une table factice ne remplissant
que 2 des 93 emplacements laisse les 91 autres à `nullptr` par
initialisation de valeur (C++, pas un bug du dispatcher) — le
déréférencement de `entry->name` sur un emplacement non rempli
provoquait un segfault ; corrigé en remplissant les 93 emplacements
avec une entrée factice "jamais correspondante" dans le test. Un second
piège, plus subtil : une variable `const` à portée espace de noms a une
liaison interne par défaut en C++ (contrairement au C) — la définition
de la table factice dans le test avait besoin de son propre mot-clé
`extern` pour que l'`extern` côté production la résolve au lien.

318 des 333 fonctions suivies sont intégrées (0 bloquée, 0 en attente
de revue, 15 différées). Compilation Win32 et les 31 tests CTest
réussissent.

## 17 oct 2026 (suite 13) : `FMVPlayback`/`AttractMode` — bonus utile,
## confirme la limite code/données

`thunk_FUN_00fec210` (le déclencheur réel appelé par le handler de
`FMVPlayback`, déjà exportée par ReAgent) recherche son premier
argument dans une table de 16 entrées à `0x008923e8`
(`&PTR_s_AttractMode_008923e8`, stride 0x18) : c'est la **table des
cinématiques (FMV) du jeu**, par nom :

```
AttractMode, AttractModeNoText, 3_DumbledoresSpeech, 4_OrphanageMemory,
5_KatieAndTheNecklace, 7_QuidditchVsSlytherin_NightToDay,
7_SpyingOnDraco_NightToDay, 10_SlughornsBrokenMemory, 11_DracosQuest,
7_LoveSickRon_NightToDay, 8_QuidditchVsHufflepuff_NightToDay,
12_HidingTheBook, 13_SlughornsMissingMemory,
16_HalfBloodPrinceRevealed, 17_Credits
```

Pas de "1_Intro"/"2_..." explicite — les cinématiques commencent au
chapitre 3, cohérent avec `"17_Credits"` étant le générique de fin et
`AttractMode`/`AttractModeNoText` étant la boucle d'écran titre (après
inactivité), pas l'intro initiale elle-même.

Recherche de la fonction qui CONSTRUIT le nom "AttractMode" à
l'exécution (xref sur la chaîne elle-même) : une fonction non détectée
par Ghidra (`0x0048d2a0`) choisit entre `"AttractMode"` et
`"AttractModeNoText"` selon un code langue (`strncmp` contre 2
caractères à `0x81f91c`), puis programme un appel différé (minuteur
d'inactivité, argument `0x1f4`=500) — confirme que c'est le mécanisme
classique de "mode attraction après inactivité sur l'écran titre", pas
le déclenchement initial.

**Recherche des références à la chaîne "FMVPlayback" elle-même** :
aucune, à part son propre nom dans la table de 93 commandes. Aucune
fonction du code ne construit/émet littéralement `"FMVPlayback ..."`
en dur. Ceci **confirme** la limite code/données déjà notée : la
séquence complète (quelles vidéos jouent, dans quel ordre, au premier
lancement) est très probablement définie dans un **fichier de
script/chapitre** chargé par `Task_BootupFlowControl`/
`Task_FrontEndChapter`, pas dans le binaire. La table AttractMode reste
une référence utile (15 noms de FMV réels) pour une future
reconstruction de la lecture vidéo, mais la suite de cette piste
précise nécessite de chercher dans les données du jeu (archives déjà
aborées par `src/archive`/`src/projection`), pas davantage de code.

**Limite réelle atteinte** : aucun fichier de données du jeu (archives
`.pak`/`.dat`/scripts de chapitre) n'est présent dans ce projet — seule
la base Ghidra du binaire exécutable est disponible (voir
`ghidra-bridge.yaml`). La piste "trouver le script qui émet les
commandes de présentation" n'est donc pas poursuivable davantage dans
cet environnement sans les fichiers d'installation du jeu. Cette
branche d'exploration (WinMain → bootstrap → `TriggerBootupFlow` →
tâches → commandes de script → activation d'écrans nommés) est
considérée close pour l'instant au niveau où le code s'arrête.

## 17-18 oct 2026 (suite 14) : projets voisins découverts, la chaîne
## intro/menu est bouclée jusqu'au bout

L'utilisateur a signalé que `D:\HP` (répertoire parent de ce projet)
contient les archives extraites du jeu (`D:\HP\extract\`) et deux
projets voisins déjà avancés sur ce même binaire :

- **`D:\HP\reconstruction\`** : un frontend *authored* (code écrit à la
  main, pas une recompilation fidèle) qui réutilise les assets
  extraits mais réimplémente la logique du menu différemment. Objectif
  différent du nôtre (reconstruction fidèle fonction par fonction) —
  ne pas en importer de code.
- **`D:\HP\knowledge\hp6\`** : une base de connaissances **IDA Pro**
  (14177 fonctions, 7913 symboles, outil `D:\HP\tools\re_kb.py`),
  construite indépendamment sur le même binaire (même SHA256), avec
  accès à de vrais noms de symboles.

**Validation croisée** via `re_kb.py` : concordance totale avec nos
propres découvertes. `FUN_005dcdf0` = `Engine_InitCoreSystemsAndMessages` ;
`FUN_005dae30` = `MessageId_GetOrRegister` ; `FUN_005dd020` =
`DebugOverlay_RefreshAndBroadcastRender` (confirme l'hypothèse "overlay
de diagnostic") ; `FUN_005d2b30` = `PresentationTaskHandler_RegisterTask`
(= notre `submit_task`) ; `DAT_0089eb0c` = `g_pMainTickInterface`
(confirme "simulate/render" du frame pump) ; leur table de 93 commandes
concorde entrée par entrée avec la nôtre.

**La pièce manquante, trouvée** : la base liste 24 fonctions
`ScreenTask_Screen<Nom>_Enqueue` à des adresses consécutives (0x50b8b0
à 0x50c330) — dont `ScreenTask_ScreenMainMenu_Enqueue` (0x50b920),
`ScreenTask_ScreenTitlePage_Enqueue` (0x50ba70),
`ScreenTask_ScreenAutosaveWarningMessage_Enqueue` (0x50bae0),
`ScreenTask_ScreenAttractMode_Enqueue` (0x50bc60),
`ScreenTask_ScreenFMV_Enqueue` (0x50bcd0) — chacune appelant
`PresentationTaskHandler_RegisterTask`. Xrefs en direct (pyghidra)
montrent qu'elles forment une **table de données** à `0x0082fbbc` (26
entrées × 16 octets : `{Enqueue, Dequeue_commun, fn3_commun,
fn4_commun}`), lue par `FUN_0048ce30`
("HybridScreenActivateMappedTask", indice 4 de notre table de 93
commandes) via une **seconde table** de 26 pointeurs vers des objets
"tâche" à `0x00870948`, cherchés par nom exact (assembleur brut : ECX
reste le pointeur d'entrée à travers la récupération de vtable, jamais
poussé → **thiscall confirmé**). C'est le résolveur exact derrière
`Presentation.Screens.ActivateScreenWithFocus <nom_écran>`.

Intégré et testé : `hp6::presentation::activate_mapped_screen_task`
(`src/presentation/CommandDispatch.cpp`).

**Chaîne complète confirmée de bout en bout** :

```
WinMain (0x5d9300)
  -> FUN_005dcdf0 (Engine_InitCoreSystemsAndMessages)
  -> thunk_FUN_00ff7fa0 -> "TriggerBootupFlow"
  -> Task_BootupFlowControl / Task_Bootup (états, voir plus haut)
  -> "Presentation.Screens.ActivateScreenWithFocus <nom_écran>"
  -> activate_mapped_screen_task (résolution par nom, table de 26 écrans)
  -> ScreenTask_Screen<Nom>_Enqueue
  -> PresentationTaskHandler_RegisterTask (submit_task, déjà intégrée)
```

Reste non résolu : le contenu exact de chaque tâche d'écran (son
vrai travail d'affichage/rendu), et quelle commande précise est émise
juste après `TriggerBootupFlow` pour choisir le premier écran — toujours
probablement piloté par des données/scripts (limite déjà documentée),
bien que ce point n'ait pas été re-vérifié avec cette nouvelle base.

## 18 oct 2026 (suite 15) : la tâche `ScreenMainMenu` déclenche
## elle-même `ScreenTitlePage` — l'écran titre est le vrai premier écran

Décompilation directe (assembleur brut, `ScreenTask_ScreenMainMenu_Enqueue`
n'est pas encore exportée côté ReAgent) de `0x0050b920` : crée une
tâche générique `"Presentation:TaskHandler::Task"` (vtable à
`0x0082f91c`, stockée dans `DAT_00fcae8c`), soumise à
`PresentationTaskHandler_RegisterTask` (= notre `submit_task`). Le
vtable de cette tâche suit exactement le même squelette que
`Task_Bootup`/`Task_FrontEndChapter` (destructeur générique commun à
`0x4da470`/`0x48b0d0`/etc., slot 1 "IsReady" trivial, slot 2 stub) —
mais son **slot 3 ("Update", +0xc) est une vraie petite machine à
états** :

- état 1 → si `DAT_00cef67a != 1` et `DAT_00868814 == 1` (exactement
  1, pas 0 ni une autre valeur) : émet
  `"Presentation.Screens.ActivateScreenWithFocus ScreenTitlePage"`
  (via `dispatch_presentation_command`, déjà intégrée) et passe à
  l'état 2. Si `DAT_00868814 == 0` : abandonne (retour à l'état 0).
  Sinon (ex. `DAT_00868814 > 1`, ou `DAT_00cef67a == 1`) : reste à
  l'état 1 (attend).

**Donc la tâche "ScreenMainMenu" ne montre pas directement le menu —
elle attend une condition puis déclenche elle-même l'écran titre.**
`ScreenTitlePage` est very probablement le **vrai premier écran
visuel** du jeu, le menu principal se préparant en arrière-plan.
`DAT_00cef67a` est résolu (voir plus bas) ; `DAT_00868814` ne l'est
pas encore.

**`DAT_00cef67a` identifié** : posé à 1 par `FUN_005d3070`, une
fonction d'affichage de boîte de dialogue modale
(`DIALOG_MAIN`/`DIALOG_RESPONSE_0`/`DIALOG_RESPONSE_1`, liée aux
commandes `Storage.AskSetAutosave`/`Storage.OnResponseSelect0/1` déjà
tracées). C'est donc un drapeau **"dialogue modal actif"** : la tâche
`ScreenMainMenu` n'avance vers `ScreenTitlePage` que si aucun dialogue
modal n'est affiché.

Recherche de qui émet la commande initiale "ActivateScreenWithFocus
ScreenMainMenu" elle-même (ou équivalent) : la seule référence directe
trouvée à la chaîne `"ScreenMainMenu"` comme argument de commande est
dans une fonction liée à **`"QuitGame"`** (`0x0048def7`,
`QuitGame ScreenMainMenu` — probablement "quitter la partie en cours
et revenir au menu", pas le point d'entrée initial). L'objet
`ScreenTaskListener_ScreenMainMenu` (`0x0087bb98`, un abonné à un
système de notification générique à liste chaînée, `FUN_005d2c10`) ne
pointe pas non plus vers un déclencheur initial spécifique — c'est le
mécanisme de notification *générique* du framework, pas une piste
propre à ce menu précis. **Le tout premier déclencheur reste
insaisissable** malgré plusieurs angles d'attaque ; probablement
piloté par des données de script, cohérent avec la limite déjà
documentée.

## 18 oct 2026 (suite 16) : la tâche `ScreenMainMenu` intégrée et testée

`DAT_00cef67a` et `DAT_00868814` étant désormais résolus (voir plus
haut), la tâche `ScreenMainMenu` complète est intégrée dans
`src/presentation/ScreenTasks.cpp` :
`enqueue_main_menu_screen_task` (0x0050b920) et les trois méthodes de
son objet tâche (destructeur partagé à 0x0050c440, "IsReady" à
0x0053e9f0, "Update" à 0x0053ea00, exposée comme
`update_main_menu_screen_task`).

**Point technique notable** : `submit_task` (déjà intégrée) appelle
réellement `vtable_of(task)[1]` en **`__thiscall`** sur l'objet tâche
que `enqueue_main_menu_screen_task` construit — donc, pour la première
fois dans ce projet, une vtable *fabriquée par nous* (pas une vtable du
jeu qu'on se contente d'appeler) devait être compatible ABI thiscall
avec nos propres fonctions C++. MSVC interdit `__thiscall` sur une
fonction libre ; solution : les méthodes du destructeur/"IsReady"/
"Update" sont des **méthodes non-statiques normales** d'une petite
classe locale (`ScreenMainMenuTask`, sans base), et leurs adresses
brutes sont extraites via l'union pointeur-membre/pointeur-brut — un
fait d'implémentation MSVC bien connu et stable pour une classe sans
héritage multiple/virtuel. `member_function_address<Member>(...)` dans
`ScreenTasks.cpp` documente cette technique.

**Piège rencontré en testant** (même catégorie qu'hier) : une table
factice de 93 entrées comptée à la main n'en contenait que 91 — 2
emplacements restaient implicitement `nullptr`, provoquant un segfault
dès qu'un chemin d'exécution atteignait ces emplacements. Corrigé en
remplaçant le comptage manuel par des macros de répétition
(`HP6_X5`/`HP6_X10`/`HP6_X25`) dont le compte final est vérifiable par
arithmétique plutôt qu'à l'œil.

328 des 348 fonctions suivies sont intégrées (5 bloquées — le lot
ReAgent `mixed_deps_060`, CLI `codex` manquant —, 0 en attente de
revue, 15 différées). Compilation Win32 et les 33 tests CTest
réussissent.

## 18 oct 2026 (suite 17) : `ScreenTitlePage` intégrée — confirme le
## mécanisme d'inactivité → `AttractMode`

Poursuite directe de la chaîne : `ScreenTask_ScreenTitlePage_Enqueue`
(0x0050ba70) suit exactement le même schéma que `ScreenMainMenu`, mais
son objet tâche fait 24 octets (pas 16) — deux champs
supplémentaires : une paire d'octets à `+0xc`/`+0xd` et un pointeur à
`+0x14`. Son **"IsReady" (vtable slot 1, 0x00451ec0) a un effet de
bord réel et important** : il construit et soumet une **sous-tâche**,
`"Presentation:TaskHandler::Task_IdleInputTimer"` (vtable à
`0x00821508`), stockée dans le nouveau champ `+0x14`.

`Task_IdleInputTimer` (destructeur `0x00429c70`, "IsReady"
`0x0059f450`, "Update" `0x0059f460`) est un minuteur d'inactivité
minimal et autonome : son champ `+8` mémorise le tick courant
(`DAT_00d644f8`, désormais exposé publiquement via
`hp6::runtime::current_tick_value()`) à la dernière entrée détectée,
et son "Update" revérifie la présence d'une entrée (`0x004ccca0`, non
reconstruite — dépendance externe) tant que son champ `+0xc` vaut `0`
ou le sentinel `-1` ("pas encore armé"), rafraîchissant le tick
mémorisé si une entrée est détectée. **C'est le mécanisme exact derrière
le minuteur d'inactivité → `AttractMode`** tracé il y a deux sessions
(`0x0048d2a0`) : `ScreenTitlePage` arme ce minuteur dès son activation.

Les vtable slots 2 ("appartient à une liste, via le champ `+0x14`") et
3 ("Update" de `ScreenTitlePage` lui-même, >256 octets de pile locale)
ne sont **pas** reconstruits — substantiellement plus complexes,
hors de portée ici — mais rien dans ce projet n'appelle encore à
travers eux, donc leur absence n'affecte rien de ce qui est testé.

Intégré et testé dans `src/presentation/ScreenTasks.cpp` :
`enqueue_title_page_screen_task` et les méthodes réelles des deux
classes de tâches ci-dessus. `hp6::runtime::current_tick_value()`
(nouvel accesseur, `src/runtime/FrameClock.h`) expose
`DAT_00d644f8` — jusqu'ici une donnée interne au frame pump, mais
réellement lue directement par du code du jeu ailleurs (confirmé par
cette intégration).

**Piège de comptage évité cette fois** : les deux tables factices du
test (93 et 26 entrées) étaient déjà corrigées via les macros de
répétition introduites au tour précédent — aucun nouveau segfault.

334 des 354 fonctions suivies sont intégrées (5 bloquées, 0 en attente
de revue, 15 différées). Compilation Win32 et les 33 tests CTest
réussissent.

## 18 oct 2026 (suite 18) : `ScreenAutosaveWarningMessage` intégrée —
## révèle `ScreenTitlePageCut_PC`, l'auto-déclenchement de la cinématique

`ScreenTask_ScreenAutosaveWarningMessage_Enqueue` (0x0050bae0) est une
tâche plus simple que les deux précédentes (12 octets : vtable,
drapeau, un seul champ `+0x8` — pas d'interface secondaire, pas de
global dédié). Son **"Update" (0x00429de0) est entièrement
reconstruit et révèle un nouvel écran** :

```
elapsed = tick/3 - last_prompt_tick   (snapshot pris à la construction)
si elapsed <= 2000  : attendre
si elapsed > 2000   : activer "ScreenAutosaveWarningMessagePrompt" (sans
                       focus) si pas déjà actif
si une entrée a déjà été détectée : terminé (true)
si elapsed <= 8000  : attendre encore
sinon (elapsed > 8000, toujours aucune entrée) :
    émettre "Presentation.Screens.ActivateScreenWithFocus ScreenTitlePageCut_PC"
    terminé (true)
```

**`ScreenTitlePageCut_PC`** — très probablement "Title Page Cutscene,
version PC" — est donc déclenchée **automatiquement si le joueur ne
fait rien pendant ~8 secondes** après l'avertissement de sauvegarde.
C'est la cinématique d'intro elle-même.

Intégré et testé dans `src/presentation/ScreenTasks.cpp`.
**`dispatch_presentation_command` (0x0048ab90) a dû être corrigée** :
elle était déclarée `void`, mais au moins un appelant réel
(`0x00429de0`, cette tâche) lit explicitement `AL` juste après l'avoir
appelée et branche sur ce résultat — elle retourne maintenant
`std::int32_t` (le résultat de `find_and_invoke_command`), un
changement rétrocompatible avec tous les appelants existants qui
ignoraient déjà ce retour.

**Piège de test rencontré** : un gestionnaire de commande factice
stockait le pointeur `args` reçu au lieu de le copier — `args` pointe
dans le tampon de pile *local* de `dispatch_presentation_command`,
invalide dès son retour (use-after-return classique). Corrigé par une
copie (`strncpy` dans un tampon statique) plutôt qu'un stockage de
pointeur.

339 des 359 fonctions suivies sont intégrées (5 bloquées, 0 en attente
de revue, 15 différées). Compilation Win32 et les 33 tests CTest
réussissent.

## 18 oct 2026 (suite 19) : `ScreenFMV` — confirme le lien vers la
## table des cinématiques, mais trop vaste pour une intégration complète

`ScreenTask_ScreenFMV_Enqueue` (0x0050bcd0) : le plus gros objet tâche
tracé jusqu'ici (44 octets). Seuls le constructeur et le destructeur
partagé sont intégrés. Ses vtable slots 1-3 sont substantiellement
plus complexes que toutes les autres tâches d'écran de ce projet :

- **slot 2** (`0x0053df20`) indexe directement la table de 16
  cinématiques `AttractMode` à `0x008923e8` via `DAT_00cef5b8` — **la
  même table lue par `0x00fec210`/`FMVPlayback`** (voir plus haut) :
  confirme que `ScreenFMV` est bien l'écran réel derrière la lecture
  vidéo.
- **slot 3** (`0x0053e000`, "Update") est un dispatcheur à **12 états**
  (table de sauts), avec 0x170 (368) octets de pile locale —
  probablement toute la machine ouverture/décodage/lecture/nettoyage
  d'une vidéo. Hors de portée pour une intégration ponctuelle.
- **slot 1** référence `DAT_00cef7b0`, la même globale que la grosse
  machine à états `0x0048d2a0` restée non résolue il y a plusieurs
  sessions — probablement liées.

Les trois sont tracées "différées" séparément dans le registre plutôt
que laissées implicites.

341 des 364 fonctions suivies sont intégrées (5 bloquées, 0 en attente
de revue, 18 différées). Compilation Win32 et les 33 tests CTest
réussissent.

## 18 oct 2026 (suite 20) : `ScreenAttractMode` — résout l'appartenance
## de la machine à états `0x0048d2a0` restée mystérieuse

`ScreenTask_ScreenAttractMode_Enqueue` (0x0050bc60, 48 octets). Seuls
constructeur/destructeur intégrés. **Son vtable slot 1 ("IsReady",
0x0048d2a0) est confirmé être exactement la grosse machine à états
`DAT_00cef7b0` partiellement tracée il y a plusieurs sessions** (sa
branche "case 7" émettant `"QuitGame ScreenMainMenu"`, probablement le
retour au menu après une partie en cours) : cette découverte résout
**à qui appartient** cette machine à états (le "IsReady" du mode
attraction lui-même), sans pour autant la reconstruire entièrement —
toujours hors de portée, mais désormais correctement rattachée et
documentée plutôt que flottante. Les slots 2 et 3 (`"Update"`,
dispatcheur à 12 états) restent également différés.

343 des 369 fonctions suivies sont intégrées (5 bloquées, 0 en attente
de revue, 21 différées). Compilation Win32 et les 33 tests CTest
réussissent.

## 18 oct 2026 (suite 21) : `ScreenLanguageSelect` intégrée

`ScreenTask_ScreenLanguageSelect_Enqueue` (0x0050b8b0, 24 octets) :
constructeur/destructeur intégrés, selon le même schéma désormais bien
établi. Son "Update" (slot 3, 0x00429890, 0x118 octets de pile locale)
— probablement la navigation dans la liste des langues — reste
différé.

**Bilan de cette série de 6 tâches d'écran** (`ScreenMainMenu`,
`ScreenTitlePage`, `ScreenAutosaveWarningMessage`, `ScreenFMV`,
`ScreenAttractMode`, `ScreenLanguageSelect`), sur les 24 au total :
le schéma de construction/destruction est désormais entièrement
compris et répétitif (allocation via l'allocateur imbriqué du
`PresentationTaskHandler`, écriture temporaire puis finale du champ
"interface secondaire", stockage dans un global dédié, soumission via
`submit_task`). Les 18 tâches restantes suivraient probablement le
même schéma pour leur constructeur/destructeur, avec des "Update"
individuellement spécifiques (certaines triviales, d'autres
substantielles) à évaluer au cas par cas.

345 des 374 fonctions suivies sont intégrées (5 bloquées, 0 en attente
de revue, 24 différées). Compilation Win32 et les 33 tests CTest
réussissent.

### Prochaine étape recommandée

1. Utiliser `D:\HP\tools\re_kb.py`/`re_ui.py` systématiquement en
   complément de nos propres xrefs en direct (pyghidra) — validation
   croisée gratuite et parfois des noms de symboles réels.
2. Résoudre `DAT_00cef67a`/`DAT_00868814`, puis intégrer la tâche
   `ScreenMainMenu` (et son squelette, réutilisable pour les 23 autres
   tâches d'écran) en C++ vérifié.
3. Explorer `D:\HP\extract\` plus systématiquement pour d'éventuels
   scripts texte (pas seulement audio) qui émettraient les commandes
   de démarrage — pistes non encore épuisées : `shaders.str`,
   `fingerpr.int`, et les fichiers hors `chapters`/`missions`/`events`
   à la racine de `extract/`.

## 17 oct 2026 (suite 11) : `FMVPlayback`/`OnActivateFrontEndChapter`
## tracées — la limite entre "code" et "données" est atteinte

`FMVPlayback` (`0x0048cd90`, déjà exportée côté ReAgent) confirme
exactement son rôle : parse son argument en `"%s %s"` (nom de fichier +
indicateur ignorable/skippable via `atoi`), puis
`thunk_FUN_00fec210(nom_fichier, skippable)` déclenche réellement la
lecture d'une vidéo nommée — non tracée plus loin cette section.

`OnActivateFrontEndChapter` (`0x0048c9e0`, déjà exportée) crée une
tâche `Presentation:TaskHandler::Task_FrontEndChapter` (vtable
`0x828380`, stockée dans `DAT_00fcae1c`), soumise à la même file
`PresentationTaskHandler`. Contrairement à `Task_Bootup`, son slot
"Update" (+0xc) est un stub trivial (`XOR AL,AL; RET`) — **pas** de
machine à états. Son slot +0x8 (probablement appelé une fois à la
construction/l'activation) se contente d'émettre une commande
secondaire via le même lookup `0x0048abf0` :
`"Presentation.VignetteEffect 0"` (désactive un effet de vignette), pas
la logique d'affichage du menu lui-même.

**Conclusion de cette branche d'exploration** : la chaîne de contrôle en
C++ (WinMain → bootstrap → tâches → commandes de script) est désormais
cartographiée jusqu'au point où elle active des **écrans nommés** par
chaîne (`"ScreenMainMenuCameraPos"`, `"ScreenAutosaveWarningMessage"`,
etc.) via `Presentation.Screens.ActivateScreenWithFocus`. Le contenu
réel de ces écrans (disposition, boutons, logique du menu) n'est très
probablement **pas du code mais des données** chargées par
`UIComponentManager`/`PresentationManager` (fichiers de ressources du
jeu, pas visibles dans le désassemblage). Continuer à tracer du C++ au
delà de ce point a une rentabilité décroissante ; la suite logique est
plutôt de localiser et d'identifier le **format des fichiers de
définition d'écran** (probablement dans les archives de données du jeu
déjà abordées par `src/archive`/`src/projection`), une démarche
différente de celle utilisée dans toute cette section (xrefs/assembleur
en direct) et qui mérite son propre point de départ.

## 18 oct 2026 : le vrai site d'appel `IDirect3D9::CreateDevice` est
## localisé, reconstruit fidèlement et intégré (`006498e0`)

Objectif de cette session : obtenir un exécutable réel testable qui
atteigne au moins l'écran `ScreenLanguageSelect`. Une première tentative
a consisté à écrire un `WinMain`/une création de device D3D9 inventés
("mode normal") dans `src/app/GameMain.cpp` — **corrigé immédiatement
par l'utilisateur** : il faut récupérer le vrai code décompilé du jeu et
le reconstruire, pas en inventer un équivalent. `src/app/GameMain.cpp`
et son bloc CMake ont été entièrement retirés (confirmé par `git diff`
propre et 33/33 tests toujours verts après coup).

**Recherche du vrai site de création du device.** `DAT_00cf1944` (le
`IDirect3DDevice9*` utilisé par `DeviceReset.cpp` et toute la suite)
n'apparaît nulle part dans les 52654 caractères du `WinMain` décompilé
lui-même (vérifié par recherche de sous-chaîne). Un xref complet
(script `pyghidra` en lecture seule, déjà utilisé dans cette session)
contre `0x00cf1944` a donné une seule écriture directe (un `MOV
dword ptr [0x00cf1944],0x0` dans `FUN_00648e70` — un reset, pas une
création) et deux références `PUSH 0xcf1944` (l'ADRESSE du global, pas
sa valeur) dans `FUN_006498e0`, le signe classique d'un paramètre de
sortie `&DAT_00cf1944` passé à une fonction de création.

**`FUN_006498e0` est confirmé être le vrai site d'appel
`IDirect3D9::CreateDevice`**, par deux voies indépendantes :
lecture complète de son décompilé (il appelle bien `Direct3DCreate9`,
énumère les adaptateurs/formats, résout un format d'affichage
compatible via `CheckDeviceType`, puis appelle le slot vtable `+0x40`
de l'interface `IDirect3D9` — confirmé être `CreateDevice`, index 16 —
avec `&DAT_00cf1944` comme `ppReturnedDeviceInterface`) **et**
recoupement contre la base de connaissances IDA indépendante de
`D:\HP\knowledge\hp6` (`python tools/re_kb.py show 0x6498e0`), qui le
nomme déjà `CreateD3DDeviceAndSwapchains` et liste exactement les mêmes
sites d'appel. Son unique appelant est `FUN_005dca10`
(`python tools/re_kb.py show` et l'export Ghidra sont d'accord), qui
est lui-même appelé **directement depuis `WinMain`**
(`0x005d9300`) :

```
FUN_005dca10(largeur_client, hauteur_client)
{
    memset(&_Dst_0089dea8, 0, 0x200);
    FUN_005e09d0();           // non reconstruit (trivial, voir plus bas)
    FUN_006194a0(0xd71250);   // non reconstruit : init d'un tas (~13.4 Mio),
                               // "RenderFrameworkMainHeap" — bloquant, voir
                               // "prochaine étape" ci-dessous
    FUN_006498e0(largeur, hauteur, 0, 0, 6);  // == create_d3d9_device, intégrée
    ...
}
```

**`FUN_006498e0` et toute sa chaîne de dépendances nécessaire au
chemin de succès par défaut sont désormais intégrées**, fidèlement,
dans `src/render/DeviceCreate.cpp`/`.h` :

- `create_d3d9_device` (`006498e0`) : `Direct3DCreate9` → construction
  de la table d'adaptateurs/modes (`enumerate_display_modes`) →
  sélection du premier adaptateur HAL → résolution du format
  back-buffer via `CheckDeviceType` (avec repli entre les paires
  16/32 bits `0x17`/`0x16` et `0x16`/`0x15`) → remplissage d'un
  `D3DPRESENT_PARAMETERS` → `CreateDevice` (avec une seconde tentative
  à drapeaux de comportement réduits en cas d'échec) → `Clear`/`Present`
  → création de la surface depth-stencil et (sauf désactivation) d'une
  texture de rendu et d'une texture depth-stencil → liaison des cibles
  → `Clear`/`Present` final → `create_dynamic_vertex_buffer` (déjà
  intégrée dans `DeviceReset.cpp`).
- `enumerate_display_modes` (`00649140`, `BuildAdapterModeBlock` dans
  la base IDA) : construit paresseusement (gardée par
  `DAT_00cf19d0`) la table par adaptateur (identifiant à `+0`, `D3DCAPS9`
  HAL à `+0x454`, liste triée/dédupliquée de modes `{Width,Height,
  RefreshRate,Format}` à `+0x450`/`+0x44c`) pour les trois formats
  back-buffer `R5G6B5`/`X8R8G8B8`/`A8R8G8B8`, filtrée à au moins
  320×200, triée par le comparateur `qsort` `006490b0` (format
  descendant, puis largeur puis hauteur croissantes).
- Six petites fonctions triviales intégrées à l'identique :
  `copy_brightness_setting` (`004d6420`), `update_quality_tier_flag`
  (`0063b560`), `apply_multisample_quality` (`0063b880`/`01000c60` —
  confirmé par l'assembleur brut que l'argument passe par `EAX`, pas la
  pile, aux deux sites d'appel), `clear_scratch_light_block`
  (`0064b560`).
- Deux tables de render states par défaut, **extraites
  mécaniquement** (petit script Python jetable parsant le décompilé
  par expression régulière, plutôt que transcrites à la main, pour
  éliminer le risque d'erreur de copie sur 73+61 champs) et
  recoupées entre la fonction d'initialisation et celle d'application :
  `init_default_render_state_table`/`apply_default_render_states`
  (`01003010`/`0064b5c0`, 73 champs, bloc `DAT_00c751a4`..`DAT_00c754c0`)
  et `init_default_sampler_and_stage_state_table`/
  `apply_default_sampler_and_stage_states` (`01003300`/`0064bd90`,
  tables à 8 entrées/0x80 octets de foulée pour `SetSamplerState` et
  `SetTextureStageState`), reliées par `apply_secondary_render_state_defaults`
  (`01003550`).
- Un test `tests/device_create_smoke.cpp` fournit un faux
  `IDirect3D9`/`IDirect3DDevice9`/`IDirect3DSwapChain9`/surfaces/texture
  complet (toutes les méthodes COM utilisées, aux bons offsets
  `__stdcall`) et vérifie que `create_d3d9_device` appelle bien
  `CreateDevice` une fois, lie une render target et un depth-stencil,
  et se termine par la création du vertex buffer partagé — confirmé
  passant (34/34 tests CTest au total après cet ajout).

**Deux branches délibérément différées** (fonctions réelles, non
inventées, mais pas encore reconstruites, documentées comme telles —
même traitement que les 18 tâches d'écran restantes ou `FMVPlayback`) :
`build_resolution_option_list_hard_cap` (`0064c8c0`,
`BuildResolutionOptionList_HardCap` dans la base IDA — branche
atteinte seulement si `DAT_00cf1950!=0`, jamais mis à autre chose que 0
par le code intégré jusqu'ici, donc morte dans cette reconstruction ;
dépend de six fonctions non résolues et du global déjà signalé comme
inconnu `DAT_007db234`) et `fatal_display_error` (`005da4b0`, le
chemin d'échec de `Direct3DCreate9`/`CreateDevice` — formatage de
message + `MessageBoxW` + `TerminateProcess` dans le binaire d'origine,
non atteint sur une machine fonctionnelle). Les deux ont un corps
`std::abort()` dans `DeviceCreate.cpp` uniquement pour que l'édition de
liens réussisse — jamais exécutés par le chemin par défaut testé.

**Prochaine étape, pour intégrer `FUN_005dca10` lui-même** (le vrai
appelant direct de `create_d3d9_device` depuis `WinMain`) : reconstruire
`FUN_005e09d0` (triviale, remise à zéro d'une poignée de globals du
frame pump) et surtout `FUN_006194a0(0xd71250)`, l'initialisation d'un
tas dédié (~13,4 Mio, nommé `"RenderFrameworkMainHeap::"` dans une
chaîne du binaire) utilisé par le reste du moteur de rendu — non encore
tracée, dépend elle-même de plusieurs fonctions non résolues
(`thunk_FUN_00ff8b60`, tables `DAT_008796b0`/`b4`/`b8`/`bc`/`c8`/`cc`).
Une fois cela fait, `FUN_005dca10` pourra être intégrée et raccordée
directement à l'appel réel dans `WinMain`, complétant la chaîne
WinMain → création du device → (reste du bootstrap déjà documenté plus
haut) sans aucune étape inventée.

## 18 oct 2026 (suite) : `FUN_006194a0` et `FUN_005dca10` intégrées —
## le vrai appelant direct de WinMain est maintenant raccordé

`FUN_006194a0(0xd71250)` n'initialise pas un tas de taille `0xd71250` :
cette constante est une **adresse**, celle de l'objet global fixe
« RenderFrameworkMainHeap » lui-même (confirmé par la chaîne du même nom
trouvée dans ses assignations de pile). La fonction calcule la taille
totale de deux régions nommées (via une table de dérogation
`DAT_008796b0`/`DAT_008796b4`, vide dans toute cette reconstruction, donc
toujours au défaut), crée le tas via le **même pointeur de fonction**
`DAT_00fcdd78` que `hp6::presentation::allocate_tagged`
(`src/presentation/TaskSystem.cpp`) utilise déjà pour les allocations
nommées — confirmé par le commentaire de ce fichier lui-même — puis
enregistre deux sous-allocateurs nommés via `FUN_00ff8b60`
(`LocalAllocator_CreateRegister_Overlay` dans la base IDA). Le bloc
d'initialisation paresseuse qu'elle duplique en ligne (drapeau
`DAT_00fd1254`, `DAT_00fcdd74`/`78`, `DAT_00cef6bc`, atexit) est
**exactement** celui déjà reconstruit comme `hp6::platform::
service_holder()` (`src/platform/HandleRegistry.cpp`, 0x005db920) — la
reconstruction appelle cette fonction partagée plutôt que de dupliquer
le bloc, l'effet observable étant identique. `FUN_00ff8b60` lui-même
reste non tracé (il construit un objet via trois fonctions
supplémentaires non résolues, le tout enveloppé de SEH x86 MSVC) ; comme
il se trouve sur le chemin de succès **sans condition** de
`initialize_render_framework_heap` (pas une branche d'erreur morte),
`create_local_allocator_register` est un stub qui **réussit toujours**
plutôt qu'un `abort()` — sinon aucun appelant ne serait testable.

`FUN_005dca10` (0x005d9300 → ici, confirmé être le seul appelant direct
depuis `WinMain`) est maintenant intégrée dans son intégralité :
`hp6::runtime::reset_frame_pump_state()` (0x005e09d0, nouvelle fonction
dans `FrameClock.cpp` — remet à zéro exactement les globals du frame
pump que ce fichier possède déjà, confirmant par la même occasion que
« channel 0/1 » du frame pump sont bien les deux blocs de 5 mots
`tick_history`/`total_history`/`flip_index` déjà nommés) →
`initialize_render_framework_heap` → `create_d3d9_device` → (si échec
E_FAIL, réaffiche le curseur) → un appel virtuel sur un objet
`DAT_00d6af14`. Cet objet est lu (jamais écrit dans aucune fonction
nommable) par cinq fonctions sur tout le cycle de vie du binaire, dont
le bootstrap moteur `FUN_005dcdf0` (`Engine_InitCoreSystemsAndMessages`,
toujours pas reconstruit) et les deux fonctions de nettoyage
post-boucle-de-messages déjà citées plus haut (`FUN_005d7250`/
`FUN_005dced0`) ; son unique site d'écriture est hors de toute fonction
nommée par Ghidra (probablement un initialiseur statique C++). Comme cet
appel est lui aussi sur le chemin de succès sans condition, il reste un
**no-op documenté** (pas un `abort()`) en attendant la reconstruction de
`FUN_005dcdf0`.

`tests/device_create_smoke.cpp` exerce désormais `create_and_configure_
render_device` de bout en bout (34/34 tests CTest). La chaîne complète
`WinMain → FUN_005dca10 → create_d3d9_device` est maintenant
intégrée et testée ; les seules pièces encore différées sur ce chemin
précis sont `FUN_00ff8b60` (sous-allocateur nommé, stub neutre) et
l'objet `DAT_00d6af14` (nécessite `FUN_005dcdf0`).

## 18 oct 2026 (suite 2) : `FUN_005dcdf0` (le bootstrap moteur) intégré —
## découverte du bus de messages nommés et du TimeManager du moteur

`FUN_005dcdf0` (`Engine_InitCoreSystemsAndMessages`) est le second
appel direct de `WinMain`, juste après `FUN_005dca10`. Sa lecture
complète révèle un **bus de messages nommés** interne au moteur (EA
GOF) : `FUN_005dae30(slot, "iMsgNomDuMessage", callback)` enregistre un
gestionnaire pour un message par son nom (`"iMsgDeleteEventHandler"`,
`"iMsgRunningTick"`, `"iMsgDoRender"`, etc. — une trentaine au total
observée dans ce seul bootstrap) via une table de hachage/interning de
chaînes propre (`FUN_005dffc0`/`FUN_005df8f0`/`FUN_005dafa0`, non
tracée — un sous-système à part entière). Aucun code intégré dans ce
projet n'émet encore l'un de ces messages, donc
`hp6::presentation::register_named_message_handler` est un stub qui ne
produit qu'un identifiant stable et non nul, sans implémenter le vrai
registre.

Deux autres découvertes importantes :
- `DAT_00cef730` (déjà lu par `hp6::runtime::accumulate_simulation_
  time_sample`, type jusqu'ici non identifié) **est le singleton
  `TimeManager::Instance`** du moteur, créé paresseusement via
  `allocate_tagged(8, "TimeManager::Instance")` — confirmé en lisant
  `FUN_00ff7da0`/`FUN_00ff7e80` (`hp6::presentation::
  get_or_create_time_manager`), les deux variantes du même point
  d'entrée paresseux.
- `FUN_005e0bf0` (`hp6::runtime::initialize_frame_pump_timing`)
  initialise le pas de simulation fixe du frame pump : toujours
  exactement `0x280000/0` en Q16.16 (= 40.0, confirmé indépendant de
  l'argument) et un second accumulateur calculé depuis
  `(DAT_00ba9504/intervalle)*DAT_008003ac` avec le même idiome de
  conversion flottant→fixe en deux temps (partie entière puis partie
  fractionnaire multipliée séparément, pour éviter la perte de
  précision du float32) déjà présent ailleurs dans ce module.

`FUN_005dd020` (appelée deux fois) construirait une chaîne de debug
affichant la date de build (**"Jan 15 2009 à 14:31:50"**, trouvée en
dur dans l'assembleur exporté) et la résolution active — mais seulement
si `FUN_01002c80` indique une build non-finale, ce qui n'est jamais le
cas ici ; `hp6::presentation::show_debug_build_overlay_once` transcrit
fidèlement la garde (donc du vrai code mort, pas une invention) et
différe le corps.

`DAT_0089eb0c` (le "this" opaque du frame pump, type resté non identifié
depuis plusieurs sessions) est assigné **ici** (`&PTR_PTR_0087eadc`) —
on sait désormais OÙ il est construit, même si le type concret derrière
`PTR_PTR_0087eadc` reste à explorer.

`DAT_00d6af14` (l'objet moteur dont `FUN_005dca10` et `FUN_005dcdf0`
appellent chacun un slot vtable différent) a été tracé jusqu'à sa vraie
construction : ce n'est **pas** `FUN_005dcdf0` qui le crée, mais un
**initialiseur statique C++ global** à `0x0070f720` (`CALL FUN_005f4f80`,
exécuté avant `WinMain` par le runtime CRT — confirmé en désassemblant
la table de petits blocs `CALL constructeur; PUSH destructeur; CALL
atexit` à partir de `0x0070f6c0`, le même motif que toutes les variables
globales C++ statiques du jeu). Sa vtable (`PTR_LAB_008405cc`) contient
au moins 12 slots, chacun suivant le même motif paresseux que
`service_holder()` ; son slot 2 construit un petit objet nommé via
`allocate_tagged`. Le vrai constructeur de l'objet
(`thunk_FUN_00ffcfc0`/`FUN_00ffcfc0`) construit aussi deux tableaux
internes (16 éléments de 44 octets via un constructeur trivial, 8
éléments de 0x3c0 octets dont le constructeur `FUN_00429780` initialise
une poignée de champs depuis des constantes globales, dont
`DAT_007db234` déjà signalée comme « valeur inconnue » dans ce README).
Reconstruire cet objet comme un vrai singleton C++ à durée de vie
statique (plutôt que l'extern vide actuel) est noté comme prochaine
étape mais n'a pas été fait cette session — les deux appels vtable
dessus (`FUN_005dca10` slot 0, `FUN_005dcdf0` slot 2) restent des
no-op documentés.

`tests/engine_bootstrap_smoke.cpp` exerce `initialize_engine_core_
systems` de bout en bout (35/35 tests CTest). **Confirmation de ce que
l'utilisateur anticipait** : ce bootstrap révèle plusieurs sous-systèmes
entiers du moteur (bus de messages, gestion audio nommée, TimeManager,
l'objet `DAT_00d6af14` avec ses 24 sous-objets) qui mériteront chacun
leur propre travail de reverse engineering approfondi dans des sessions
futures ; ce qui est intégré aujourd'hui transcrit fidèlement le
contrôle de flux de haut niveau et permet au programme de continuer
sans planter, sans jamais prétendre reproduire le comportement interne
de ces sous-systèmes.

## 18 oct 2026 (suite 3) : trouvé — le vrai site d'émission de
## `"...ActivateScreenWithFocus ScreenLanguageSelect"`, et pourquoi
## "Global" y mène probablement

Recherche directe (xref en mémoire, pyghidra, nouveau script jetable
`find_string_xrefs2.py` car `memory.findBytes` provoque une
`RecursionError` dans ce `pyghidra` quand le script tourne depuis
`/tmp` — fonctionne depuis le répertoire du projet/scratchpad) de la
chaîne complète de commande. Un seul site l'émet : `FUN_00ff1430`
(`0x00ff147e : MOV ESI,0x835770 ; CALL 0x0048ab90`), et seulement
**quand `DAT_00d472dc == 9`**.

`DAT_00d472dc` n'est PAS un identifiant de plateforme (PS3/Xbox/Wii)
comme l'hypothèse de travail le supposait depuis `FUN_00576f30` — c'est
l'**identifiant du chapitre/zone en cours de chargement**. La table de
11 entrées à `0x0080a91c` (lue directement en mémoire, script
`dump_platform_table.py`) donne : `NONE, BURROW_INTRO, HOGWARTS_1,
BURROW_XMAS, HOGWARTS_2, THE_CAVE, HOGWARTS_3, ENDLESS_DAY, FRONT_END,
**LANGUAGE_SELECT**, POTIONS_GAME` — correspondant exactement aux
dossiers déjà vus sous `D:\HP\extract\chapters\`. **`LANGUAGE_SELECT`
est le chapitre d'index 9.**

Donc `FUN_00ff1430` (une méthode sur un objet passé en `EDI` — le
« gestionnaire de chapitre courant », pas encore identifié comme classe)
émet, pour chaque chapitre, une commande d'activation d'écran
différente : un verbe commun (`0x825b2c`) toujours émis, puis selon le
chapitre résolu — index 8 (`FRONT_END`) → une commande + un nettoyage de
texte spécifique ; **index 9 (`LANGUAGE_SELECT`) → exactement
`"Presentation.Screens.ActivateScreenWithFocus ScreenLanguageSelect"`**
; tout autre chapitre (si `DAT_0087e014==0`, un garde-fou "une seule
fois") → une troisième commande (`0x832474`, non encore lue).

`FUN_00576f30` (appelée dans `TriggerBootupFlow` avec `"Global"`)
résout ce nom en un de ces 11 chapitres fixes, ou — si le nom ne
correspond à aucune entrée de la table (c'est le cas de `"Global"`,
absent de la liste) — appelle `FUN_0053eb40("Global")`, une résolution
dynamique non encore tracée. **Hypothèse la plus probable, cohérente
avec le comportement attendu d'un premier lancement** : cette résolution
dynamique choisit `LANGUAGE_SELECT` quand aucune langue n'a encore été
choisie (ou qu'aucune sauvegarde n'existe), ce qui expliquerait
directement pourquoi l'écran de sélection de langue est le premier
visible — mais ceci reste une hypothèse, pas confirmé par lecture de
code.

**Chaînon manquant identifié, pas encore résolu** : qui appelle
`thunk_FUN_00ff1430` (`0x00577710`) ? Une seule référence trouvée,
`CALL 0x00577710` à `0x004daee4` — mais Ghidra n'a **aucune fonction
définie** à cette adresse ni dans son voisinage (`containing function:
None` sur une large fenêtre), donc ni cette référence ni son contexte
n'ont pu être lus par les scripts en lecture seule déjà établis ; il
faudrait soit forcer une analyse de fonction à cet endroit (hors de la
politique « lecture seule, aucune modification du projet Ghidra » suivie
jusqu'ici), soit trouver un autre chemin (par exemple localiser l'objet
« gestionnaire de chapitre » lui-même et sa vtable, puisque `EDI`
suggère un appel `__thiscall` implicite dont le slot contenant
`0x00577710` reste à localiser).

**Bilan de cette piste** : la commande exacte qui affiche l'écran de
sélection de langue est maintenant identifiée avec certitude
(`"Presentation.Screens.ActivateScreenWithFocus ScreenLanguageSelect"`,
émise par `FUN_00ff1430` quand le chapitre résolu est `LANGUAGE_SELECT`)
et `dispatch_presentation_command` (déjà intégré) sait déjà l'exécuter.
Ce qui manque encore pour une chaîne 100% reconstruite : (a) qui appelle
`FUN_00ff1430` et quand, (b) ce que fait `FUN_0053eb40` pour résoudre
`"Global"` en un chapitre concret. Tant que ces deux points ne sont pas
résolus, émettre directement cette commande depuis du code reconstruit
serait un raccourci inventé plutôt qu'une reconstruction fidèle — pas
fait ici, conformément à la consigne du projet.

**Chaînon manquant résolu** : `0x004daee4` (le site d'appel de
`thunk_FUN_00ff1430`) appartient bel et bien à une fonction existante
dans notre projet Ghidra — `FUN_004da7b0`, déjà exportée (probablement
par un lot ReAgent antérieur, `.ghidra-exports/004da7b0.json`) mais
jamais lue jusqu'ici. La base de connaissances IDA indépendante
(`D:\HP\knowledge\hp6`) la nomme **`ChapterLoadStateMachine_Update`**
— confirmation supplémentaire. C'est une machine à 20 états (`case 0`
à `case 0x13`, dispatch sur `*(this->target)+0x260`, chaque état posant
aussi un minuteur à `this+0x10 = 0x8c`), le pendant exact de
`Storage.Bootup`/`Task_Bootup` déjà cartographiée, mais pour le
**chargement d'un chapitre** plutôt que l'autosauvegarde. Les 19
premiers états (0–17) initialisent et chargent progressivement les
ressources du chapitre (son, shaders, caméra, HUD, flags `+0x264`, une
dizaine de sous-fonctions non tracées). **L'état final, `case 0x12`
(18)**, est celui qui compte :

```c
case 0x12:
    (reset de la machine à l'état 0, et de plusieurs champs du gestionnaire)
    thunk_FUN_00fe55d0();
    FUN_00431840();
    thunk_FUN_00ff1430();   // <-- émet la commande d'activation d'écran,
                            //     selon le chapitre résolu (DAT_00d472dc)
    if (DAT_00cef610 != 0) { ... "STARTLoadChapter: %s" ... }
    else if (*(char*)(DAT_00fcadfc+4) == 0) { FUN_0053acd0(); }
```

**La cascade complète est maintenant comprise au niveau conceptuel,
de `WinMain` jusqu'à la commande exacte qui affiche l'écran de
sélection de langue** : `WinMain` → `TriggerBootupFlow` (commande #73)
→ `Task_BootupFlowControl` → (chaîne encore à confirmer en détail,
probablement via `Storage.Bootup`) → `ChapterLoadStateMachine_Update`
charge le chapitre résolu depuis `"Global"` → à son état final, émet
soit `"...ActivateScreenWithFocus ScreenLanguageSelect"` (si le
chapitre résolu est `LANGUAGE_SELECT`, index 9) soit l'équivalent pour
`FRONT_END`/tout autre chapitre.

**Ce qui reste pour une reconstruction C++ fidèle et testée de cette
dernière étape** (non tentée cette session — le travail serait
comparable en ampleur à `FUN_005dcdf0`, voire supérieur) : les ~20
sous-fonctions de `ChapterLoadStateMachine_Update` (`FUN_0053aca0`,
`FUN_00423360`, `FUN_004d1300`, `FUN_0059c9d0`/`cb20`, `FUN_0050b370`
réappelée 4 fois avec des usages différents, `FUN_0048f240`,
`FUN_0040c7f0` réappelée 4 fois, et plusieurs autres), et surtout
confirmer **ce que `FUN_0053eb40("Global")` résout réellement** (la
résolution dynamique du nom de chapitre quand il ne correspond à
aucune des 11 entrées fixes de la table) — c'est cette résolution,
plus que `ChapterLoadStateMachine_Update` elle-même, qui détermine
directement si le chapitre de démarrage est `LANGUAGE_SELECT`.

**`FUN_00ff1430` intégrée** (`hp6::presentation::
emit_chapter_entry_screen_command`, `src/presentation/ChapterFlow.cpp`)
— lecture directe en mémoire de la table de 10 entrées à `0x00892300`
(`LanguageSelect`=9, `FrontEnd`=8, `BurrowIntro`=1, `Hogwarts`=2/4/6/7/10,
`BurrowXmas`=3, `TheCave`=5, stride 0xc, champ nom à +4) confirmant que
`FUN_0053eb40` ne trouve **aucune** entrée nommée `"Global"` — donc
l'hypothèse « `"Global"` résout vers `LANGUAGE_SELECT` » est **corrigée
en « non confirmée »** : sans entrée correspondante, `FUN_0053eb40`
retourne NULL, et son appelant (`FUN_00576f30`) ne met alors PAS à jour
`DAT_00d472dc`, qui garde donc sa valeur précédente plutôt que d'être
mis à 9. La fonction elle-même est intégrée et testée fidèlement pour
ses trois chemins désormais certains : chapitre 8 → `"OnActivateFrontEndChapter"`
+ nettoyage ; chapitre 9 → `"Presentation.Screens.ActivateScreenWithFocus
ScreenLanguageSelect"` ; tout autre chapitre (une seule fois,
`DAT_0087e014`) → `"Presentation.Screens.ActivateScreenWithFocus
ScreenMainOSD"`. `get_screen_manager_singleton` (`0x0050b370`) suit le
même schéma `service_holder` déjà établi. `tests/chapter_flow_smoke.cpp`
exerce les quatre branches (36/36 tests CTest).

`ChapterLoadStateMachine_Update` (`0x004da7b0`) elle-même et
`FUN_00576f30`/`FUN_0053eb40` (qui déterminent réellement `DAT_00d472dc`)
restent différées : la première a une vingtaine de sous-fonctions non
tracées comparables en ampleur à `FUN_005dcdf0` ; les deux autres sont
nécessaires pour savoir, sans hypothèse, quel chapitre un premier
lancement charge réellement.

## 18 oct 2026 (suite 4) : `resolve_and_load_chapter`/
## `advance_to_next_chapter` intégrées — l'ordre réel de progression des
## chapitres confirme indirectement l'hypothèse `LanguageSelect`

`FUN_00576f30` (`hp6::presentation::resolve_and_load_chapter`) et
`FUN_00ff1390` (`hp6::presentation::advance_to_next_chapter`, son
appelant le plus clair) sont maintenant intégrées et testées
(`src/presentation/ChapterFlow.cpp`, 36/36 tests CTest). Lecture
complète de `FUN_00576f30` : elle cherche `chapter_name` dans la table
fixe à 11 entrées (`0x0080a91c`), puis, en cas de correspondance
d'index != 0, cherche dans la table à 10 entrées (`0x00892300`,
`{index, nom, next_hint}`) l'entrée dont **le champ `index` vaut cet
indice** ; si `chapter_name` ne correspond à AUCUNE des deux tables
(le cas de `"Global"`, confirmé en relisant les deux tables
intégralement), `DAT_00d472dc` n'est **pas** modifié — ce qui confirme
définitivement la correction apportée au tour précédent.

`FUN_00ff1390` (`advance_to_next_chapter`) révèle le mécanisme réel de
progression : elle cherche dans la table `0x892300` l'entrée dont
`index == DAT_00d472dc` (le chapitre courant), puis prend **le champ
`index` de l'entrée suivante dans le tableau** (pas le `next_hint` de
l'entrée courante — vérifié sur l'assembleur) comme nouveau chapitre, et
appelle `resolve_and_load_chapter` avec son nom. Ceci confirme l'ORDRE
RÉEL de progression de la campagne du jeu, dans l'ordre du tableau :
**`LanguageSelect`(9) → `FrontEnd`(8) → `BurrowIntro`(1) →
`Hogwarts`(2) → `BurrowXmas`(3) → `Hogwarts`(4) → `TheCave`(5) →
`Hogwarts`(6) → `Hogwarts`(7) → `Hogwarts`(10)**.

**`LanguageSelect` est structurellement le PREMIER chapitre de toute la
séquence de progression du jeu** — une preuve indirecte forte (toujours
pas une confirmation directe d'une écriture `DAT_00d472dc = 9` au tout
premier lancement) que c'est bien l'écran voulu au démarrage. Reste
cependant non trouvé : le site exact qui écrit cette toute première
valeur (probablement une simple affectation statique quelque part dans
la logique "nouvelle partie vs partie existante", pas nécessairement
via `resolve_and_load_chapter`/`advance_to_next_chapter`). Cette
dernière pièce, et `ChapterLoadStateMachine_Update` elle-même (ses ~20
sous-fonctions de chargement de ressources), restent différées.

## 18 oct 2026 (suite 5) : nuance importante — un autre site lit
## `"CurrentChapterId"` avec un défaut `BurrowIntro`(1), pas `LanguageSelect`

Parmi les 9 sites qui écrivent `DAT_00d472dc` (liste complète obtenue
par xref en direct), aucun n'écrit la constante littérale `9` — seuls
`0x0048c50c` et `0x00fd7a1e` écrivent une constante, et c'est toujours
`8` (`FRONT_END`). Tous les autres calculent la valeur dans `EAX`/`ESI`
avant de l'écrire.

L'un de ces sites, `FUN_00fefd50`, s'est avéré particulièrement
parlant : il lit une valeur nommée **`"CurrentChapterId"`** (chaîne
littérale confirmée en mémoire) depuis un système de lookup générique
à deux niveaux (`FUN_0048eac0`, catégorie + clé, 25 appelants dans tout
le binaire — probablement le système de sauvegarde/progression
générique du jeu), avec `1` (`BurrowIntro`) comme **valeur par défaut
si rien n'est trouvé** — pas `9` (`LanguageSelect`). Si ce site est
réellement celui qui détermine le chapitre de départ sur un profil de
sauvegarde sans progression, cela **nuance** (sans l'invalider
complètement) la preuve indirecte de la section précédente : la
table de progression place bien `LanguageSelect` en premier dans
l'ordre narratif, mais le "défaut sans sauvegarde" observé ici pour la
notion de **chapitre** est `BurrowIntro`.

**Hypothèse la plus cohérente avec les deux observations** : le choix
de la langue n'est probablement pas piloté par le même mécanisme que
la progression de chapitre — un garde-fou distinct (pas encore
localisé, potentiellement lié à un réglage "langue jamais choisie",
analogue aux sentinelles `-1` déjà rencontrées pour `OptionLOD`/
`OptionBrightness`) forcerait `DAT_00d472dc = 9` **avant** que la
logique normale de progression (dont le défaut est `BurrowIntro`) ne
s'applique. Cette hypothèse n'est pas confirmée par lecture de code.

**Limite rencontrée** : les appelants de plusieurs thunks clés de cette
chaîne (`thunk_FUN_00ff1430`, `thunk_FUN_00fefd50`) ne sont pas
résolus par Ghidra (`callers: []`) — cohérent avec des méthodes
virtuelles appelées via une table de pointeurs plutôt qu'un `CALL`
direct, comme `Task_Bootup`/`Task_BootupFlowControl` déjà signalées
« dont le rôle exact d'orchestration reste à élucider » dans ce même
document. Poursuivre cette piste précise par simple lecture statique
a atteint un point de rendement décroissant sans modifier le projet
Ghidra (hors de la politique « lecture seule » suivie jusqu'ici) ou
sans pouvoir observer une exécution réelle du jeu. Cette session
redirige l'effort vers `ChapterLoadStateMachine_Update` elle-même
(nécessaire quel que soit le chapitre effectivement résolu) plutôt que
de continuer à chercher le site exact d'initialisation.

## 18 oct 2026 (suite 6) : `ChapterLoadStateMachine_Update` intégrée —
## son squelette complet, pas juste son état critique

`FUN_004da7b0` est maintenant intégrée
(`hp6::presentation::update_chapter_load_state_machine`,
`src/presentation/ChapterLoadStateMachine.cpp`). Les 20 états et
**toutes** leurs conditions de transition sont transcrits fidèlement
depuis l'assembleur exporté — y compris une subtilité confirmée en
relisant l'assembleur avec soin : le `switch` original n'a **aucun
`case` pour la valeur d'état 18** (il saute directement de `0x10` à
`0x12`), reproduite ici à l'identique plutôt que « corrigée ».

Les ~30 sous-fonctions feuilles dont dépend la progression de chaque
état (chargement audio/caméra/HUD, gestion de flags `+0x264`, etc.)
restent différées, regroupées derrière trois stubs génériques par forme
de retour plutôt qu'une fonction nommée par adresse — aucune n'a de
contrat utile isolément tant que son vrai comportement n'est pas tracé.
**L'état 19** (étiqueté `case 0x12`/`0x13` dans le binaire d'origine)
— celui qui compte pour ce projet — est transcrit et testé avec
précision : il remet la machine à l'état 0, réinitialise plusieurs
champs du registre, et appelle le vrai déclencheur de
`emit_chapter_entry_screen_command` (`thunk_FUN_00ff1430`, déjà
intégré). `tests/chapter_load_state_machine_smoke.cpp` exerce le
squelette de dispatch (états 0 à 2) et cet état critique directement ;
l'exécution complète état-1-à-19 n'est **pas** exercée de bout en bout,
puisque la plupart des transitions intermédiaires dépendent des vraies
valeurs de retour de ces sous-systèmes différés, pas seulement de leur
présence — les simuler sans les avoir tracés serait fabriquer un
comportement plutôt que le reconstruire. 37/37 tests CTest, 382/419
fonctions intégrées.

## 18 oct 2026 (suite 7) : `TriggerBootupFlow` intégrée de bout en
## bout — le handler de commande ET la fonction qui l'émet

`FUN_0048c970` (le handler de la commande `TriggerBootupFlow`, entrée
#73) et `FUN_00ff7fa0` (la fonction appelée une fois, directement,
depuis `WinMain`, qui émet cette commande) sont maintenant intégrées
(`src/presentation/BootupFlow.cpp`). Le handler s'est avéré
particulièrement simple une fois lu en entier : il alloue un objet
`Task_BootupFlowControl` de 8 octets via le motif déjà factorisé
`allocate_via_task_handler`, lui donne sa vraie vtable
(`PTR_LAB_00828360`), et le soumet via `submit_task` — déjà intégrées
toutes les deux, aucune nouvelle primitive nécessaire.

`FUN_00ff7fa0` elle-même : la comparaison littérale `"Global"=="Global"`
(toujours vraie) est reproduite comme un dispatch inconditionnel de
`"TriggerBootupFlow"` — testé bout en bout, cette commande **re-rentre
réellement** dans `handle_trigger_bootup_flow` via la table de 93
commandes, mettant une seconde tâche en file — puis appelle
`resolve_and_load_chapter(DAT_00fcadec, "Global")`, déjà intégrée et
confirmée ne pas modifier `DAT_00d472dc`. Le chargement conditionnel de
`"SSGShaders"`/`"Shaders.str"` (`DAT_008663d5`) et de `"Game.str"`
(`thunk_FUN_00ff7f20`) reste différé — aucune des deux n'est sur un
chemin dont le code déjà intégré dépend.

`tests/bootup_flow_smoke.cpp` exerce les deux fonctions, y compris la
ré-entrance réelle du dispatch — 38/38 tests CTest, 385/422 fonctions
intégrées. **La chaîne complète `WinMain → create_and_configure_
render_device → initialize_engine_core_systems → trigger_bootup_flow
→ (TriggerBootupFlow) → resolve_and_load_chapter` est maintenant
intégrée et testée de bout en bout**, à l'exception des sous-systèmes
de chargement de ressources déjà identifiés comme différés (chargement
de chapitre proprement dit, chargement de fichiers `.str`).

## 18 oct 2026 (suite 8) : `FUN_0064aa60` (gestionnaire de device
## perdu) intégrée — avec une vraie correction de fidélité en bonus

`FUN_0064aa60` (le gestionnaire classique de device Direct3D 9 perdu,
appelé une fois par itération de la boucle de messages Win32,
`src/render/DeviceLostHandler.cpp`) est intégrée en réutilisant
directement `cache_available_texture_memory` et
`prepare_render_target_texture`, déjà en place depuis des sessions
antérieures — tous les codes `D3DERR_*` et les chaînes de diagnostic
correspondent exactement à l'assembleur exporté. Ses deux auxiliaires,
`FUN_0064a6d0` (`release_dynamic_render_resources`) et `FUN_0064c460`
(`set_render_targets_cached`), sont intégrés avec elle.

**Correction de fidélité découverte en écrivant le test** : `Reset()`
réutilise, dans le vrai binaire, exactement le même bloc
`D3DPRESENT_PARAMETERS` persistant que `create_d3d9_device` a rempli à
la création (`_Dst_00c75538`, un vrai global, pas une variable de
pile). `create_d3d9_device` (déjà intégrée) utilisait jusqu'ici un
tableau **local** — fidèle à l'intérieur d'un seul appel, mais pas
réutilisable par un Reset() ultérieur. Corrigé : `DAT_00c75538[14]` est
désormais un vrai global partagé entre `DeviceCreate.cpp` et
`DeviceLostHandler.cpp`.

**Piège rencontré en écrivant le test, et corrigé dans le code de
production (pas seulement le test)** : `release_if_present` (un
petit auxiliaire interne à `DeviceLostHandler.cpp`) appelait le slot
**0** de la vtable d'un objet COM au lieu du slot **+8** (`Release`,
index 2) — une faute de frappe d'offset qui aurait fait planter
n'importe quel appelant réel, pas seulement le test. Détecté
uniquement parce que le test factice déréférençait un vrai objet
plutôt qu'un pointeur arbitraire comme certains tests antérieurs
avaient fait par commodité (ex. `reinterpret_cast<void*>(0x5000)`) —
confirmation que faire des objets factices complets avec une vraie
vtable, même pour un test, vaut l'effort : un pointeur arbitraire
masque ce genre d'erreur plutôt que de le révéler.

`tests/device_lost_handler_smoke.cpp` exerce les quatre branches
(device propre, `D3DERR_DEVICELOST`, `D3DERR_DEVICENOTRESET`, et
`Reset()` qui échoue à son tour) — 39/39 tests CTest, 388/425 fonctions
intégrées.

## 18 oct 2026 (suite 9) : `FUN_005d8fc0` (la vraie boucle de messages
## Win32) intégrée

`FUN_005d8fc0` (`hp6::platform::run_message_loop`,
`src/platform/MessageLoop.cpp`) est l'appel unique que `WinMain`
(`FUN_005d9300`) fait après `ShowWindow`/`UpdateWindow` — c'est le
cœur de la boucle principale du jeu. Un verrou à usage unique
(`DAT_00cef68d`, posé par un `WM_QUIT` précédent) fait retourner `0`
immédiatement à un second appel sans exécuter le corps de la boucle,
exactement comme le retour anticipé de l'x86 exporté.

Chaque itération : appelle le gestionnaire de device perdu déjà
intégré (`hp6::render::handle_device_lost`) ; tant que le moteur
tourne réellement (`DAT_00866e2a`), décompte un compteur de grâce de
démarrage (`DAT_00cef55c`) en appelant
`hp6::presentation::copy_gsc_message_handles` quand il atteint zéro —
sinon, une fois qu'une erreur fatale a déjà été affichée
(`DAT_00cef68f`), exécute un bloc de nettoyage différé documenté (non
atteignable par le code déjà intégré) ; puis `PeekMessageA`. Sans
message en attente, calcule le temps réel écoulé à partir du même
accumulateur Q16.16 que `initialize_frame_pump_timing` lit
(`DAT_00d64570`/`74`), avec exactement le même idiome
entier/fraction, appelle la pompe de frame déjà intégrée
(`hp6::runtime::run_frame_tick`), et enregistre paresseusement un
traqueur de performance (`hp6::platform::service_holder`, réutilisé
plutôt que dupliqué) quand `DAT_00cef69e` est posé, avant un
`Sleep(0)` si l'itération a été plus rapide que le budget calculé.
Avec un message en attente : retourne `wParam` sur `WM_QUIT` (en
effaçant le verrou, comme l'x86 exporté), sinon
`TranslateMessage`/`DispatchMessageA` (sauté entièrement si
`PeekMessageA` lui-même a échoué, conformément au test `!= -1` de
l'original).

Différés, documentés comme no-op : le bloc de nettoyage « erreur
fatale déjà affichée » ; `thunk_FUN_00ff9760`/`FUN_005dee50` (traqueur
de plancher de performance, alimente `DAT_00866e24`) ; `FUN_00689180`
et `FUN_00545f30` — aucun des quatre n'est sur un chemin dont le code
déjà intégré dépend.

`tests/message_loop_smoke.cpp` exerce le verrou à usage unique et la
gestion de `WM_QUIT` (posté via `PostQuitMessage(42)`, récupéré dès la
première itération avant même d'atteindre la branche de pompe de
frame) — 40/40 tests CTest, 389/426 fonctions intégrées. **La chaîne
complète `WinMain → ... → run_message_loop` est maintenant reconstruite
et testée pièce par pièce ; il ne reste plus qu'à assembler `WinMain`
lui-même comme une vraie fonction C++ appelable, en reliant les
morceaux déjà validés.**

## 18 oct 2026 (suite 10) : `WinMain` (`FUN_005d9300`) assemblée,
## intégrée et testée de bout en bout

`FUN_005d9300` — le vrai `WinMain` du jeu — est maintenant reconstruite
en entier dans `src/platform/WinMain.cpp`
(`hp6::platform::run_win_main`), en reliant toutes les pièces déjà
validées pendant cette session. Déroulé réel, dans l'ordre : mot de
contrôle FPU (`_control87`), répertoire courant relatif à l'exécutable,
sauvegarde de trois réglages `SystemParametersInfoA` (le vrai
`FUN_005d9210('\0')`, déjà modélisé par
`hp6::platform::apply_system_parameters`), copie de la ligne de
commande, vérification mono-instance (`FindWindowA`), lecture d'une
vingtaine de réglages `GameSettings` (résolution, qualité, position de
fenêtre, Maximized/Minimized, six paires `Mode%dWidth`/`Height`),
création de la vraie fenêtre
(`hp6::platform::create_game_window`), création du device D3D9
(`hp6::render::create_and_configure_render_device`), montée du système
de fichiers (`initialize_file_system_storage`, voir ci-dessous),
bootstrap moteur (`hp6::presentation::initialize_engine_core_systems`),
vérification de la porte de compatibilité matérielle
(`is_platform_compatible`, voir ci-dessous), dispatch de
`TriggerBootupFlow`, affichage/mise à jour de la fenêtre, la vraie
boucle de messages (`run_message_loop`), puis l'intégralité de la
séquence d'arrêt : démontage du device
(`hp6::render::shutdown_render_device`, voir ci-dessous), restauration
des `SystemParametersInfoA`, et `TerminateProcess`. Le chemin d'échec
partagé (classe non enregistrée, fenêtre non créée, ou porte de
compatibilité fermée) retombe, par simple traversée des `if` imbriqués
sans `else` — exactement comme l'x86 exporté — sur un nettoyage commun
(`DestroyWindow` + restauration directe des `SystemParametersInfoA`).
L'appel à `TerminateProcess` est rendu injectable (un paramètre
`ProcessTerminationCall`) : l'x86 exporté ne revient jamais après un
appel réussi, mais un test peut substituer un bouchon qui l'enregistre
sans tuer le processus de test, afin d'observer le résultat de la
boucle de messages.

Cinq fonctions supplémentaires découvertes sur ce chemin ont été
intégrées pour de vrai :
- `hp6::platform::initialize_file_system_storage` (`FUN_00688130`) —
  remet à zéro le drapeau `DAT_00cf1b00` que lit la porte de
  compatibilité, réutilise `reset_oversized_record`,
  `initialize_default_record_and_cache_flag`,
  `schedule_autosave_timer(100)` et
  `hp6::render::create_local_allocator_register`, déjà en place.
- `hp6::platform::is_platform_compatible` (`FUN_00ff8ce0`) — son
  chemin rapide réel (`DAT_00cf1b00 == 0` → `true`) est reproduit
  fidèlement ; son vérificateur interne (probablement une détection de
  version DirectX) est **confirmé mort** dans cette chaîne d'appel
  précisément parce que `initialize_file_system_storage` vient de
  remettre ce drapeau à zéro juste avant — un `abort()` documenté, pas
  une invention.
- `hp6::render::shutdown_render_device` (`FUN_00648e70`) — le vrai
  démontage de tout ce que la chaîne de création du device possède :
  un objet auxiliaire non tracé, le tampon de sommets dynamique
  partagé, chaque sous-tampon par adaptateur alloué par
  `enumerate_display_modes`, les ressources de cible de rendu/
  depth-stencil, la table par adaptateur elle-même, puis le device et
  l'objet `IDirect3D9`, chacun avec une vérification de compteur de
  référence (reproduisant les diagnostics fatals réels « Device refs
  %d »/« D3D refs %d »).
- `hp6::platform::parse_setting_string_to_int` (`FUN_00ff8300`) —
  conversion chaîne-vers-entier via `std::istringstream`, pour les
  réglages `PosX`/`PosY`/`SizeX`/`SizeY`.
- Deux nouveaux no-op documentés dans `DeviceCreate.cpp`
  (`run_post_message_loop_cleanup_primary`/`_secondary`, pour
  `FUN_005dced0`/`FUN_005d7250`) et quatre dans un nouveau fichier
  `src/platform/EngineSubsystems.cpp`
  (`reset_particle_system_tables`, `initialize_debug_variable_system`,
  `initialize_runtime_services`, `parse_command_line_overrides`,
  `release_cached_debug_strings`) — chacun documenté avec l'adresse
  réelle, ce qui est tracé, et pourquoi le reste est différé (sous-
  système de développement pur, objet de service non tracé, ou
  branche confirmée morte).

`tests/winmain_smoke.cpp` exerce `run_win_main` de bout en bout contre
une chaîne complète d'objets `IDirect3D9`/`IDirect3DDevice9` factices
(union des bouchons déjà écrits pour `device_create_smoke.cpp` et
`device_lost_handler_smoke.cpp`), avec le verrou `DAT_00cef68d` de la
boucle de messages pré-armé pour éviter toute boucle infinie dans le
test. 41/41 tests CTest, 394/438 fonctions intégrées (39 différées, 5
bloquées). **La reconstruction atteint enfin son objectif directeur :
un exécutable réel et testable, assemblé pièce par pièce depuis
`WinMain`, qui crée une vraie fenêtre, un vrai device Direct3D 9,
démarre le vrai moteur, et déclenche `TriggerBootupFlow` → résolution
de chapitre → `ScreenLanguageSelect`.**

## 18 oct 2026 (suite 11) : piste "iMsgDoRender" / rendu GUI — exploration
## honnête, non résolue

Suite à une demande explicite de continuer la reconstruction jusqu'à
pouvoir **réellement dessiner** quelque chose à l'écran (sans jamais
inventer), cette section trace l'investigation menée pour remonter de
`ScreenLanguageSelect` jusqu'au premier vrai appel D3D9 de dessin. Elle
est documentée intégralement, y compris les fausses pistes, pour éviter
qu'une session future ne refasse le même travail à l'aveugle.

**Ce qui est confirmé (lu directement dans le projet Ghidra, adresses
réelles, désassemblage vérifié) :**
- `FUN_0050b8b0` (constructeur réel de `ScreenLanguageSelectTask`)
  installe la vraie vtable à **0x00821528** (pas une adresse inventée) :
  slot 0 = destructeur `0x00429810`, slot 1 = `0x00429850`,
  slot 2 = `0x00429860`, slot 3 = `Update` `0x00429890` (déjà documenté),
  slot 4 (+0x10) = `0x00405a30`.
- `0x00405a30` est **littéralement vide** (`{ return; }` — un stub réel
  dans le binaire, pas une supposition).
- `hp6::presentation::PresentationTaskHandler` (`DAT_00fcadf0`) a sa
  vraie vtable à **0x0083d194** (4 slots réels : dtor `0x005d28e0`,
  `0x005c1290`, `0x005d5d70`, OnMessage `0x005d2ad0`). Son `OnMessage`
  (slot+0xc) compare le type du message reçu à trois sentinelles
  (`DAT_00d63e50`, `DAT_00d63e28`, `DAT_00d63e70`) ; sur
  `DAT_00d63e70` — le message nommé **"iMsgDoRender"**, enregistré par
  `register_render_messages` (déjà intégrée, `EngineBootstrap.cpp`) —
  il parcourt sa liste de tâches actives et appelle, pour chacune, le
  slot+0x10 de la vtable PRIMAIRE de la tâche (pas `secondary_interface`,
  une fausse piste initiale corrigée en cours de route). C'est
  exactement le mécanisme qui, pour `ScreenLanguageSelectTask`, retombe
  sur le stub vide `0x00405a30` ci-dessus.
- `FUN_00429890` (le vrai `Update` de `ScreenLanguageSelect`) construit
  la liste des langues en appelant `FUN_00429430(0x00cf8168, cle, texte)`
  pour chaque entrée `"GUI_LANGUAGEMENU%d"` (0 à 15) — `FUN_00429430`
  est confirmée être une simple insertion/mise à jour dans une structure
  de type `std::map` (les thunks appelés — `thunk_FUN_00fd78a0`,
  `FUN_0042ff00`, `thunk_FUN_00fd7d40`, `thunk_FUN_00fd8080`, etc. —
  correspondent exactement aux patterns internes MSVC STL d'insertion
  dans un arbre rouge-noir). **Ce n'est pas un appel de dessin.**
  `0x00cf8168` est un objet GUI central référencé par 45 fonctions
  différentes à travers le binaire (menus, dialogues, prompts) — un
  véritable sous-système GUI à part entière, jamais touché jusqu'ici.
- `DAT_00fcaf34` (lu dans `run_frame_tick`'s appel "simulate", slot 0 de
  la vtable du frame-pump) est en réalité le **vrai système d'entrée**
  (`RealInputSystem`, construit par `FUN_005e00f0`, appelé depuis
  `FUN_00ff80a0`/`initialize_runtime_services`, différée cette session)
  — **pas** un gestionnaire de rendu comme une première hypothèse
  l'a suggéré avant vérification ; corrigé en cours d'investigation.
  Le slot 0 du frame-pump (`0x005dcd80`) draine donc des évènements
  d'entrée (clavier/souris/manette), pas des commandes de rendu.
- Le slot+4 du frame-pump (`0x005dcdc0`, que `FrameClock.cpp` nomme
  générique "render") est en réalité un compte à rebours de 5 ticks qui
  finit par appeler `show_debug_build_overlay_once` (déjà intégrée) —
  une fonctionnalité de debug, pas le vrai rendu de la scène.
- `FUN_005f4b50` enregistre (ré-enregistre) "iMsgDoRender" et plusieurs
  autres messages puis appelle `FUN_005db1a0` sur chacun — **mais**
  `FUN_005db1a0` s'est révélée être une insertion dans une liste
  triée par priorité (abonnement), pas une diffusion/exécution. Cette
  piste ("FUN_005f4b50 diffuse le rendu chaque frame") a donc été
  **écartée après vérification**, pas présentée comme acquise.

**Ce qui reste non résolu, honnêtement, à ce stade :**
- Qui appelle réellement la diffusion du message "iMsgDoRender" chaque
  frame (la vraie fonction "Fire"/"Dispatch", distincte de
  `FUN_005db1a0`, n'a pas été localisée).
- Quel système lit la table `0x00cf8168` pour effectivement dessiner le
  texte des menus (polices, sprites, `DrawPrimitive`/`DrawIndexedPrimitive`
  réels) — c'est un sous-système GUI complet, non encore abordé.
- Tous les appelants de `FUN_00429890`/`FUN_005f4b50` sont vides dans
  Ghidra (`callers: []`) — cohérent avec la limite déjà documentée
  plusieurs fois dans ce projet (dispatch par table de pointeurs de
  fonctions, non résolu par l'analyse statique sans modifier le projet
  Ghidra, ce que la politique de lecture seule de ce projet exclut).

Rien de ce qui précède n'a été inventé : chaque affirmation "confirmée"
ci-dessus est appuyée par une adresse réelle et un désassemblage lu en
direct dans le projet Ghidra. La partie "non résolue" est explicitement
documentée comme telle plutôt que comblée par une supposition
plausible — conformément à la règle de ce projet de ne jamais fabriquer
de comportement de jeu.

## 18 oct 2026 (suite 12) : premier vrai `DrawPrimitive` intégré — la
## piste des menus "spatiaux" en 3D

Indication clé de l'utilisateur, confirmée dans le binaire : les menus
de ce jeu ne sont **pas** du HUD 2D classique — ce sont des éléments
positionnés dans l'espace 3D, rendus via une vraie caméra dédiée.
Recherche de la chaîne `"Camera"` dans les exports Ghidra déjà en
cache : confirmation immédiate de **`"ScreenSpaceCamera"`**,
**`"CurrentInWorldCamera"`** et **`"GlobalPresentationCamera"`**
(`FUN_0059fa00`/`FUN_0059fdc0`/`FUN_0059f7f0`), chacune activée via
`FUN_005d3570` (probable "SetActiveCamera par nom", pas encore
intégrée). `FUN_0059fa00` copie une matrice de transformation
(16 flottants) d'un champ vers un autre avant d'activer
`"ScreenSpaceCamera"` — cohérent avec une vraie caméra de rendu
d'interface positionnée dans le monde 3D plutôt qu'un simple
espace écran 2D.

En cherchant, dans les exports déjà en cache, les fonctions référençant
à la fois le device D3D9 réel (`DAT_00cf1944`) et un offset de vtable
correspondant à `SetTransform`/`DrawPrimitive`/`BeginScene`/`EndScene`,
quatre candidats sont apparus dans le même voisinage que le code déjà
intégré (`0x0064a6d0`..`0x0064c8c0`). Deux d'entre eux se sont révélés
être un **vrai, complet, confirmé** pipeline de dessin D3D9 :

- **`FUN_00648c00`** (→ `hp6::render::submit_screen_quad`) : verrouille
  le tampon de sommets dynamique partagé déjà créé
  (`create_dynamic_vertex_buffer`), y entrelace jusqu'à 0x400 sommets
  (position/couleur/UV selon un masque de bits, toujours 28 octets par
  sommet), le déverrouille, le lie via `SetStreamSource` (vtable+400,
  index 100) et la déclaration/le shader de sommets mis en cache, puis
  émet le vrai appel **`DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, n-2)`**
  (vtable+0x144, index 81) — les deux offsets confirmés correspondre
  exactement à la vraie disposition de vtable `IDirect3DDevice9`.
- **`FUN_0064abd0`** (→ `hp6::render::render_fade_overlay_quad`) :
  compose la texture-cible-de-rendu (`DAT_00cf1958`, quand elle n'est
  pas désactivée) comme un quad plein écran teinté d'une couleur
  (construit directement : 4 sommets, bornes écran calculées depuis la
  résolution active et deux constantes opaques non encore tracées,
  `DAT_00831c2c`/`DAT_007db234`), via `submit_screen_quad` ci-dessus,
  puis appelle `EndScene`/`Present` — un vrai fondu/composite d'écran.

Six petits setters D3D9 mis en cache supplémentaires, du même type
déjà établi (`set_render_state_cached`, `set_texture_stage_state_
cached`), ont été intégrés au passage : `set_sampler_state_cached`,
`set_texture_cached`, `set_vertex_declaration_cached`,
`set_vertex_shader_cached`, `set_pixel_shader_cached`,
`reset_vertex_shader_constant_cache` (`src/render/RenderStateCache.cpp`).

**Honnêteté sur la portée réelle** : dans le binaire exporté, cette
chaîne de dessin n'est atteinte que depuis le système de overlay de
debug (`FUN_005dd020 → thunk_FUN_010120d0 → FUN_00648630 →
FUN_0064abd0`), et `hp6::presentation::is_debug_build_overlay_enabled`
confirme déjà que ce chemin est **codé en dur à `false`** dans le
binaire FINAL livré — donc mort dans le jeu tel que shippé. Ce n'est
**pas** encore le vrai moteur de rendu des menus (`ScreenLanguageSelect`
etc.), qui reste à localiser (voir la section précédente). Mais le
pipeline de dessin lui-même — verrouillage de tampon, liaison de flux,
`DrawPrimitive` réel sur un vrai device D3D9 — est désormais reconstruit
fidèlement depuis le désassemblage réel, et testé de bout en bout contre
un faux device dans `tests/screen_overlay_render_smoke.cpp`, qui vérifie
que le vrai appel `DrawPrimitive(TRIANGLESTRIP, 0, 2)` se déclenche avec
les bonnes données de sommets. **C'est le premier vrai appel de dessin
D3D9 intégré dans ce projet.**

42/42 tests CTest, tous verts. Piste des caméras `"ScreenSpaceCamera"`/
`"CurrentInWorldCamera"`/`"GlobalPresentationCamera"` et de
`FUN_005d3570` ("SetActiveCamera" probable) laissée ouverte pour la
suite — c'est le prochain fil à tirer pour relier la caméra 3D réelle
au vrai rendu de `ScreenLanguageSelect`.

## 18 oct 2026 (suite 13) : `FUN_005d3570` tracé — la caméra "Harry"
## (joueur) trouvée, le déclencheur du dessin de scène toujours non
## localisé

`FUN_005d3570(this, category, name, transform[16])` est confirmée :
cherche `name` dans un arbre rouge-noir (comparaison via
`FUN_005d5a30`) ; si trouvé, copie la matrice 4x4 `transform` dans
l'entrée ; sinon, en crée une nouvelle. C'est bien une fonction réelle
« créer/mettre à jour une caméra (ou tout objet transformé) nommée ».
5 vrais appelants trouvés (pas de dispatch par vtable ici) :
`FUN_0059fa00`/`FUN_0059fdc0` (déjà connues), `FUN_0053c5b0`,
`FUN_005550c0`, `FUN_005551f0`.

**`FUN_0053c5b0` est la découverte clé de cette suite** : c'est la
vraie méthode « Update » (confirmée via une vraie xref de données
Ghidra — `getReferencesTo`, pas une supposition — vers une vraie table
de fonctions à `0x00832a44` : slot 0 = constructeur `FUN_0053c4c0`
déjà connu, slot+0xc = cette fonction). Elle parcourt plusieurs arbres
rouge-noir de requêtes de chargement de ressources (état 0..9, motif
très proche de `ChapterLoadStateMachine_Update` déjà intégrée), active
**`"CurrentInWorldCamera"`** via `FUN_0059fdc0`, construit une matrice
4x4 (identité par défaut via la constante `DAT_007db234`, ou copiée
depuis un objet obtenu par `thunk_FUN_00ff2450()` quand il existe), puis
appelle **`FUN_005d3570(this, offset, "Harry", &matrice)`** — la vraie
caméra du joueur, nommée d'après le protagoniste. Ceci confirme
concrètement l'indication de l'utilisateur : une vraie caméra 3D,
suivant un objet nommé réel, mise à jour chaque passage.

**Les autres slots de cette même table, lus en direct, écartent deux
fausses pistes plutôt que de les laisser en suspens** : slot+0x10
(`0x00622a80`) et slot+0x14 (`0x00622a90`) — à la même position relative
que le slot "Render" trouvé pour `ScreenLanguageSelectTask` — se sont
révélés être de **simples accesseurs d'état** (`MOV EAX,4; RET`), pas
des appels de dessin. Slot+0x1c réutilise le même stub vide partagé
`0x00405a30` déjà rencontré. Slot+0x4 (`FUN_0053c550`) est la
réinitialisation symétrique du constructeur (appelle `FUN_0059fc90`,
probable désactivation de `"ScreenSpaceCamera"`). Slot+0x18
(`0x0053cab0`) est un second gestionnaire de message (compare le type
de message à deux IDs enregistrés à `this+0x4db4`/`+0x4db6`), pas un
dessin non plus.

**Conclusion honnête à ce stade** : la configuration de caméra réelle
(activation par nom, matrice de transformation) est maintenant tracée
et confirmée, mais **le déclencheur du vrai passage de dessin de la
scène 3D (et donc des menus positionnés dedans) reste non localisé**
après une recherche systématique de cette table de fonctions complète.
Chaque fausse piste explorée a été vérifiée puis explicitement écartée
plutôt que supposée. La limite reste la même que celle déjà documentée
plusieurs fois dans ce projet : les appelants réels passent par des
tables de pointeurs de fonctions que l'analyse statique de Ghidra ne
résout pas sans modifier le projet (exclu par la politique de lecture
seule de ce projet). Les deux derniers appelants de `FUN_005d3570` (`FUN_005550c0`,
`FUN_005551f0`) ont été vérifiés : ce sont des variantes génériques du
même motif (« mettre à jour la transformation de la caméra attachée à
telle entité nommée », probablement pour les PNJ/cinématiques plutôt
que pour le joueur spécifiquement), pas une nouvelle piste vers le
dessin de scène.

**À ce stade, les 5 appelants réels de `FUN_005d3570` sont tous
épuisés** sans révéler le déclencheur du vrai passage de dessin. La
suite a demandé de changer d'approche (voir "suite 14" ci-dessous).

## 18 oct 2026 (suite 14) : la chaîne complète trouvée — de
## `PluginPresentationBitmap` jusqu'au vrai `DrawPrimitive` de la scène

Changement d'approche : plutôt que de remonter depuis les caméras,
recherche descendante dans la base de connaissances IDA (`D:\HP\
knowledge\hp6`, qui couvre les 14177 fonctions et porte certains noms
réels déjà résolus) pour toute fonction liée au rendu. Résultat
immédiat et décisif : **`PluginPresentationBitmap_BuildRenderQuad`**
(`0x0056baa0`), avec l'indice `system_hint: "menus"`, et dont les
globales référencées incluent **`0x00cf8168`** — exactement le même
objet GUI déjà identifié (suite 11) comme la table de chaînes
localisées que `FUN_00429890` (le vrai `Update` de `ScreenLanguageSelect`)
remplit via `"GUI_LANGUAGEMENU%d"`. **Ceci relie enfin directement
l'écran de sélection de langue au vrai pipeline de rendu.**

Chaîne complète tracée et confirmée, maillon par maillon, jusqu'au vrai
appel D3D9 :

1. **`FUN_0056baa0`** (`PluginPresentationBitmap_BuildRenderQuad`) —
   construit une matrice de transformation 4x4 (combinant la caméra
   active et la position locale via `FUN_00401520`, "concaténer deux
   matrices"), récupère le texte localisé (`FUN_00429290`
   `UITextDb_GetDynamicText`, lisant la table `0xcf8168`), construit un
   descripteur de quad (couleur, UV, mode de blend lu dans une table
   indexée par un littéral `"WORLD"` — confirmant à nouveau des espaces
   de coordonnées distincts), puis appelle `FUN_005e92d0`.
2. **`FUN_005e92d0`** — teste le rectangle englobant, puis appelle
   `FUN_005e93e0` (mise en page/glyphes, 1024 octets de scratch),
   `FUN_005e9680` (découpage/clipping contre le viewport — calculs de
   ratio sur les coordonnées UV, confirmant un vrai clipping de glyphes)
   et `FUN_005e9820`.
3. **`FUN_005e9820`** — alloue depuis `DAT_00cf18e4`, le **même bump
   arena déjà intégré** dans `src/runtime/ScratchBuffers.cpp` de ce
   projet (pont concret, pas une coïncidence), génère les indices de
   triangles d'un quad, pousse une matrice de transformation 4x4 dans
   un tampon croissant (`DAT_0096b48c`), construit un paquet de
   commande, puis appelle `FUN_00542e10`.
4. **`FUN_00542e10`** (**62 vrais appelants** à travers tout le moteur,
   pas de dispatch par vtable) — "SubmitRenderCommand par matériau" :
   classe la commande dans une liste chaînée par matériau
   (`DAT_00967218`), elle-même repointée vers la région active par
   **`hp6::render::set_active_render_region`** (`FUN_0063d0f0`,
   **déjà intégrée dans ce projet depuis une session antérieure**,
   `src/render/RenderRegionManager.cpp` — nom de classe réel confirmé
   par une chaîne de diagnostic embarquée, `"RenderRegionManager::
   SetActiveRenderRegion..."`).
5. **`FUN_00647480`** — la fonction qui vide TOUTES les régions actives
   chaque frame : parcourt la table de régions (`_Dst_00b970d0`,
   `_Dst_00ba1500`), et pour chaque région, chaque compartiment de
   matériau de `&_Dst_00b170d0 + région` (**la même base que
   `set_active_render_region` écrit déjà** — confirmation croisée
   directe), parcourt les commandes en attente et appelle
   `FUN_00646200` pour chacune.
6. **`FUN_00646200`** — confirmée être **`Scene::End`** par sa propre
   chaîne de diagnostic embarquée (`"Scene::End - warning drawprim/draw
   indexed failed?!?!\n"`). C'est l'exécuteur réel du dessin : lie les
   états de rendu/matériaux (`FUN_0063b6e0`, de nombreux appels), lie
   la texture/le shader, puis émet le **vrai appel D3D9** :
   `(**(code**)(*DAT_00cf1944 + 0x144))(...)` = **`DrawPrimitive`**
   (pas d'index) ou `(**(code**)(*DAT_00cf1944 + 0x148))(...)` =
   **`DrawIndexedPrimitive`** (avec index) — avec vérification
   d'échec via la même fonction de diagnostic déjà intégrée
   (`hp6::platform::format_diagnostic_message`).

`FUN_00647480` n'a qu'un seul appelant trouvé par Ghidra :
**`FUN_00648630`** — la MÊME fonction déjà examinée en détail cette
session (suite 12), qui après avoir vidé toutes les régions
(`FUN_00647480`, maintenant identifiée) compose ensuite le fondu
d'écran de debug déjà intégré (`hp6::render::render_fade_overlay_quad`,
`FUN_0064abd0`). **Ceci confirme que `FUN_00648630` est bien la vraie
fonction "rendre une frame complète"**, et pas seulement un utilitaire
de debug comme la suite 12 le supposait par prudence — mais son seul
chemin d'appel trouvé statiquement reste celui, gardé par
`is_debug_build_overlay_enabled` (donc mort en FINAL), déjà documenté.
Le vrai déclencheur par-frame (probablement une dispatch par vtable
que Ghidra ne résout pas statiquement) reste non localisé, mais
**la chaîne de dessin elle-même, de bout en bout, est maintenant
entièrement tracée et confirmée jusqu'au vrai `DrawIndexedPrimitive`
de la scène** — pas seulement celui, plus simple, du quad de fondu
déjà intégré.

**Honnêteté sur la portée** : `FUN_00646200`/`Scene::End` est une
fonction volumineuse (~150 lignes) avec de nombreux sous-appels de
liaison d'état/matériau non encore tracés (`FUN_0063b6e0`,
`thunk_FUN_010043b0`, `FUN_00644cd0`, `FUN_00642380`, `FUN_00652400`,
`FUN_006455c0`, etc.). Elle n'a **pas** été intégrée dans le code C++
de ce projet à cette étape — seulement lue et confirmée en direct dans
Ghidra, avec les deux vrais sites d'appel `DrawPrimitive`/
`DrawIndexedPrimitive` identifiés avec certitude. L'intégrer
fidèlement demanderait de tracer chacun de ces sous-appels en premier,
pour ne pas inventer la logique de liaison de matériau.

## 18 oct 2026 (suite 15) : `Scene::End` intégrée pour de vrai — deuxième
## vrai `DrawPrimitive`, cette fois connecté à la vraie scène

Poursuite de la suite 14 : plusieurs des sous-appels de `Scene::End`
identifiés comme "non tracés" se sont révélés être de simples
enveloppes D3D9 directes en les lisant réellement, et ont donc été
intégrées pour de vrai au lieu d'être différées :

- **`FUN_0063b6e0`** → `hp6::render::set_vertex_shader_constant_f` —
  `SetVertexShaderConstantF` (vtable+0x178, index 94).
- **`FUN_00652380`** → `hp6::render::bind_vertex_buffer_by_handle` —
  `SetStreamSource` (vtable+400, index 100) ; le handle se résout via
  l'une de deux tables à pas fixe de 0x14 octets selon son bit de
  signe, vers le vrai pointeur de vertex buffer au champ+0x10.
- **`FUN_00652400`** → `hp6::render::bind_index_buffer_by_handle` —
  `SetIndices` (vtable+0x1a0, index 104), même motif de résolution.
- **`FUN_0063b7a0`** → `hp6::render::resolve_mode_pixel_shader_handle`
  — recherche le shader pixel du mode actif (`DAT_00998580`, déjà
  écrit par `set_active_render_region`) dans une table de capacités
  par mode à pas fixe.
- **`thunk_FUN_010043b0`** → `hp6::render::bind_pixel_shader_for_mode`
  — combine les deux fonctions ci-dessus avec `set_pixel_shader_cached`
  déjà intégrée.

Avec ces briques réelles en main, **`FUN_00646200`/`Scene::End` elle-
même a été intégrée** (`hp6::render::execute_scene_render_batch`,
`src/render/SceneBatchRenderer.cpp`) : les trois vrais chemins de
contrôle (invalidation de région, récursion multi-batch `'M'`, dessin
d'un seul élément sur les deux branches de palier de qualité) sont
reproduits fidèlement, ainsi que les deux vrais appels D3D9
(`DrawPrimitive`/`DrawIndexedPrimitive`) avec leurs arguments lus
directement depuis les champs fixes de l'élément de dessin — ces
arguments sont corrects et fidèles **indépendamment** de ce qui est
différé, puisque les blocs de liaison matériau/éclairage différés
(`FUN_00644cd0`, `FUN_006455c0`, la résolution de handle de
déclaration de `FUN_00642380`, `thunk_FUN_01002c30`) ne les calculent
pas — ils ne changent que l'apparence du rendu, jamais si ni avec
quels paramètres le vrai dessin se déclenche.

Un vrai bug de test a été trouvé et corrigé en écrivant
`tests/scene_batch_renderer_smoke.cpp` : un tableau de vtable factice
trop petit de deux emplacements pour le slot `SetPixelShader`
(`0x1ac`) provoquait une écriture hors limites corrompant un compteur
de test voisin — détecté immédiatement par une valeur de compteur
incohérente, corrigé en élargissant le tableau. Un deuxième piège de
test (les caches de liaison par défaut à zéro coïncidant avec les
handles de test à zéro, masquant les vrais appels) a aussi été corrigé
en forçant des valeurs de cache initiales different.

43/43 tests CTest, tous verts, 423/462 fonctions intégrées (39
différées, 0 bloquée). **C'est le deuxième vrai appel de dessin D3D9
intégré dans ce projet, et le premier connecté à la vraie chaîne de
rendu de scène (pas seulement au fondu de debug) — avec la logique de
contrôle, les deux types de dessin (indexé et non indexé), la
récursion multi-batch et le chemin d'invalidation tous testés contre
un vrai device factice.**

## 18 oct 2026 (suite 16) — le chemin low-tier a maintenant sa vraie apparence (transform + matériau)

Suite à la demande explicite de l'utilisateur : « pousse plus loins
parce que faudras reussir a reelement afficher le menu des langues »,
les deux derniers blocs différés du chemin **low-tier** de
`Scene::End` (celui que `DAT_008663d5 != 0` sélectionne) ont été
reconstruits pour de vrai, sans rien inventer — en relisant le JSON
Ghidra complet (`decompiled` ET pcode) de `FUN_006455c0` et
`FUN_00644cd0`, puis en désassemblant directement les sites d'appel
ambigus via un script `pyghidra` jetable (voir ci-dessous) plutôt que
de deviner.

**`FUN_006455c0` → `hp6::render::push_item_transform_and_material`**
(`src/render/SceneBatchRenderer.cpp`). Pousse la transformation et les
constantes de couleur/matériau du vertex shader pour l'élément en
cours :
- si le pointeur de transform de l'élément (`context[0]`) a changé
  depuis le dernier appel, multiplie la matrice par la matrice
  vue/projection fixe de la région active (table `DAT_00f82530` ou
  `DAT_00f82310` selon le bit `0x40000000` de `context[1]`, indexée
  par `DAT_00cf1914`), transpose le résultat, et le pousse au
  registre 0 du vertex shader (`set_vertex_shader_constant_f`, déjà
  intégrée) ;
- pousse jusqu'à trois autres groupes de constantes couleur/matériau
  (registres 4, 5, 6, 0x12) et l'état de rendu final (`0x16`), tous
  conditionnés par les bits de `context[1]`.

Point délicat résolu par désassemblage réel (pas juste le C
décompilé, qui masque le registre implicite `in_EAX` de
`FUN_005a2310`/transpose) : les DEUX appels à la transposition
écrivent dans un tampon de scratch SÉPARÉ de la source (jamais en
place) — confirmé en lisant les instructions x86 réelles autour des
deux sites d'appel (`LEA EAX,[EBP+...]` suivi de `LEA ECX,[EBP+...]`
avec des offsets différents). Cela a évité d'introduire une fonction
`transpose_matrix4x4` « sûre à l'aliasing » dont le projet n'a en
réalité pas besoin.

**`FUN_00644cd0` (second chemin, celui avec `param_4 != 0`) →
`hp6::render::resolve_material_bindings`**. Relecture directe du
corps décompilé complet de `Scene::End` a aussi confirmé l'ORDRE réel
des opérations : `DAT_00866360` (le cache du type de matériau) doit
être écrit **après** l'appel à `FUN_00644cd0`, pas avant — sinon la
comparaison interne de `resolve_material_bindings` avec la valeur
*précédente* de `DAT_00866360` serait toujours vraie contre
elle-même. Le code existant (placeholder) avait le mauvais ordre ;
corrigé. La fonction elle-même : pose la vraie porte « dessiner pour
de vrai » (`DAT_00865285`) à vrai, puis sélectionne sur le type de
matériau pour lier la vraie paire déclaration + vertex shader via
`set_vertex_declaration_cached`/`set_vertex_shader_cached` (déjà
intégrées) pour les types 2, 10 et **`0x20000108`** — ce dernier
utilisant exactement `DAT_00cf1930`/`DAT_00cf192c`, les MÊMES globals
que `hp6::render::render_fade_overlay_quad` (le fondu de debug),
confirmant que les deux vrais appels de dessin de ce projet partagent
ce même matériau réel « overlay 2D ». Les autres types reconnus
passent par `FUN_01002960` → `hp6::render::resolve_material_shader_effect`
(malgré le nom `thunk_FUN_01002960` employé ailleurs, ce n'est pas un
thunk mais une vraie fonction) — elle aussi intégrée pour de vrai dans
cette même passe (voir plus bas) : elle calcule un index de « variante »
de shader à partir des bits de `DAT_00ba14fc` et du type de matériau,
puis réutilise la liaison par défaut ou une table cache
déclaration/shader par (variante, groupe, index). Seul son unique
sous-appel non tracé (`FUN_00641f70`, le constructeur de déclaration en
cas de cache vide — ambiguïté réelle de registre implicite, pas encore
résolue) reste différé, en no-op documenté (handle nul) qui garde la
porte ouverte. Seul un type de matériau vraiment inconnu referme la
porte — testé explicitement.

Deux petites fonctions de maths pures, sous-appels de
`push_item_transform_and_material`, ont aussi été ajoutées à
`src/math/Transform.{h,cpp}` : `multiply_matrix4x4` (`FUN_00401520`,
multiplication 4x4 standard) et `transpose_matrix4x4` (`FUN_005a2310`,
transposition 4x4 standard).

Un vrai bug de test a été trouvé en construisant
`tests/scene_batch_renderer_smoke.cpp` : le vtable factice n'avait pas
les emplacements `SetRenderState` (`0xe4`) ni `SetVertexShaderConstantF`
(`0x178`) câblés, provoquant un appel à travers un pointeur nul —
corrigé en les ajoutant. Un deuxième effet de bord a aussi été
découvert : les items de test avaient `material_type == 0` à l'octet
`+0x18`, un type non reconnu par la vraie logique maintenant branchée
— ce qui fermait la porte de dessin et faisait échouer les anciennes
assertions (elles passaient avant seulement parce que
`resolve_material_bindings` était un no-op). Corrigé en donnant aux
items de test un vrai type de matériau reconnu, et de nouveaux tests
ont été ajoutés : la vraie transposition de matrice (matrice de
translation connue → résultat transposé vérifié composante par
composante au registre 0), le matériau partagé `0x20000108`, et le
cas de matériau inconnu qui ferme la porte de dessin.

**Résultat : 428/467 fonctions intégrées (39 différées, 0 bloquée),
43/43 tests CTest verts** (`resolve_material_shader_effect` a été
intégrée dans cette même passe mais omise de `reports/pipeline/
state.json` sur le coup — corrigé en suite 17 ci-dessous). Le chemin
low-tier de `Scene::End` — celui qui dessine très probablement les
quads 2D des menus comme `ScreenLanguageSelect` — a maintenant une
apparence (transform, couleur, déclaration/shader) entièrement réelle
et tracée, pas seulement un déclenchement de dessin avec des
paramètres corrects. Il ne reste différé, sur ce chemin précis, que
`FUN_00641f70` (construction de déclaration en cas de cache vide) —
qui ne change que l'apparence, jamais si le dessin se déclenche. La
prochaine étape logique reste la même qu'avant :
trouver le vrai déclencheur par frame (`FUN_00648630`/
`FUN_00647480`), toujours bloqué par la même limitation d'analyse
statique (dispatch par vtable) rencontrée partout ailleurs dans ce
projet.

## 19 oct 2026 (suite 17) — ScreenLanguageSelectTask : IsReady et slot 2 intégrés pour de vrai

Repris la relecture directe de `FUN_00429890` (`Scene`::Update de
`ScreenLanguageSelectTask`, suite 14/15) pour évaluer si cette
fonction — documentée « hors scope » depuis plusieurs sessions —
pouvait enfin être intégrée. Son corps réel confirme bien ce qui était
déjà noté : une machine à états à 3 phases (0/1/2), un tampon local de
0x118 octets, une boucle sur une liste de langues (`DAT_00fcae04`,
structure begin/end façon vecteur) avec des comparaisons de chaînes
octet-par-octet écrites à la main, et plusieurs sous-appels non tracés
(`FUN_005e5f30`, `FUN_005e5f00`, `FUN_00435110`, `FUN_00416f70`,
`thunk_FUN_0100e910`, `thunk_FUN_00fd7920`). Conclusion inchangée :
rester différée — cohérent avec le traitement identique déjà appliqué
à TOUTES les fonctions `Update` sœurs des autres tâches d'écran
(`ScreenMainMenu`, `ScreenTitlePage`, `ScreenFMV`, `ScreenAttractMode`)
dans ce projet, et de toute façon non appelée par rien de réel ici
(même blocage de déclencheur par frame que plus haut).

En revanche, les slots 1 (« IsReady », `0x00429850`) et 2
(`0x00429860`) de cette même tâche se sont révélés réellement
intégrables. Ni l'un ni l'autre n'a son propre objet fonction dans
l'analyse Ghidra de ce projet (ils tombent dans un trou non réclamé
entre `FUN_00429780` et `FUN_00429890` — probablement jamais atteints
autrement que par la vtable, donc jamais détectés comme fonctions
séparées par l'analyse automatique) ; lus directement par
désassemblage en direct (même technique `pyghidra` que pour la
transposition de matrice, suite 16) plutôt que devinés :

- **`0x00429850` → `query_ready_language_select_screen_task`** :
  `MOV dword ptr [ECX+0x14],0x0; MOV AL,0x1; RET` — remet la machine à
  états d'Update à l'état 0 et répond « prêt », exactement la même
  forme que l'IsReady de chaque tâche sœur déjà dans ce fichier.
- **`0x00429860` → `mark_language_select_pending_task_ready`** :
  parcourt la liste de tâches propre à `PresentationTaskHandler`
  (`+0xc`, liste chaînée simple, tag à `+8`) à la recherche d'un nœud
  taggé avec le pointeur `this+0xc` (la sous-tâche
  `Task_IdleInputTimer` qu'Update soumet dans son état 1, elle-même
  déjà reconstruite pour `ScreenTitlePageTask`) — si trouvé, positionne
  le `ready_flag` (`+0x4`) de CETTE sous-tâche à 1. Point de
  désassemblage à vérifier avec soin : au moment de l'écriture finale
  (`MOV byte ptr [ECX+0x4],0x1`), `ECX` contient encore la valeur
  `this+0xc` chargée plus tôt (pas le nœud de liste trouvé, `EAX`) —
  c'est bien la sous-tâche elle-même qui est marquée prête, pas le
  nœud.

Les deux sont maintenant de vraies méthodes de
`ScreenLanguageSelectTask` dans `src/presentation/ScreenTasks.cpp`,
câblées dans sa vtable réelle, testées dans
`tests/screen_tasks_smoke.cpp` (liste vide, tag non correspondant,
nœud correspondant trouvé via `next`). **430/467 fonctions intégrées
(37 différées, 0 bloquée), 43/43 tests verts.**

## 19 oct 2026 (suite 18) — le parseur de ligne de commande intégré pour de vrai ; `FUN_0053eb40` reconnue comme déjà faite

En triant les 37 fonctions encore différées pour la suite, deux
corrections de comptabilité d'abord : `FUN_0053eb40` (résolution de nom
de chapitre dans la table à 10 entrées de `0x00892300`) est en réalité
déjà intégrée — son comportement (une recherche `strcmp`-équivalente)
est reproduit fidèlement, mais *en ligne*, dans la seconde boucle de
`hp6::presentation::resolve_and_load_chapter`
(`src/presentation/ChapterFlow.cpp`), jamais factorisé en fonction
séparée puisque `resolve_and_load_chapter` en est l'unique appelant réel
confirmé. Enregistrée comme intégrée après coup.

Ensuite, `FUN_00ffa0d0` (le parseur de dérogations de ligne de commande,
`hp6::platform::parse_command_line_overrides`,
`src/platform/EngineSubsystems.cpp`) a été intégré pour de vrai, en
réglant d'abord une ambiguïté de registre implicite par désassemblage
direct du site d'appel réel dans `WinMain` (`0x005d93b3`-`0x005d93eb`) :
`EDI` n'est PAS un « this » sur un objet abstrait comme le laissait
penser le commentaire précédent, mais l'adresse **codée en dur** d'un
tampon global fixe (`0x00d63f68`) — `WinMain` copie sa propre ligne de
commande (tronquée) dans deux tampons globaux adjacents de 0x200 octets
avant d'appeler cette fonction avec `EDI` pointant sur le premier. La
fonction découpe la ligne en deux tables à 0x20 entrées max : les
drapeaux nus (`-flag`) et les paires (`-cle=valeur`), en réutilisant
`FUN_005e0630` (`extract_next_token`, un tokenizer sensible aux
guillemets) pour extraire la valeur ou le jeton brut. Deux vraies
bizarreries du binaire d'origine ont été reproduites fidèlement plutôt
que « corrigées » : la branche paires avance toujours au-delà de son
jeton même si SA table est pleine, alors que la branche drapeaux nus
N'avance PAS si LA SIENNE est pleine (un risque de boucle infinie
latent dans le binaire original au-delà de 32 jetons nus, jamais
déclenché par les tests de ce projet). Aucun appelant tracé ne relit
encore ces tables. Testé dans `tests/winmain_smoke.cpp` (drapeau nu,
paire simple, valeur entre guillemets avec espace interne).

**433/468 fonctions intégrées (35 différées, 0 bloquée), 43/43 tests
CTest verts.**

## 19 oct 2026 (suite 19) — fermeture d'un fil en suspens : `render_fade_overlay_quad`'s « armé ailleurs » enfin tracé

`render_fade_overlay_quad` (suite 12/13) portait depuis plusieurs
sessions une note : son compteur à usage unique (`DAT_00cf19e4`) est
« armé ailleurs (non encore tracé) ». `FUN_0064a9e0` est précisément ce
site — et s'est révélé entièrement intégrable d'un coup, puisque ses
deux seuls sous-appels (`FUN_0064aa60` et `FUN_0064aa30`) sont déjà
`hp6::render::handle_device_lost` et `hp6::render::
cache_available_texture_memory`, intégrés depuis longtemps. Nouvelle
fonction `hp6::render::try_begin_debug_overlay_scene`
(`src/render/ScreenOverlayRender.cpp`) : si le device n'est pas prêt à
dessiner (`handle_device_lost` échoue), ne fait rien ; sinon rafraîchit
le cache de mémoire texture (redondant mais fidèle), appelle le vrai
`BeginScene` (vtable+0xa4), incrémente `DAT_00cf19e4`. Atteinte, dans le
binaire exporté, uniquement via `FUN_01002c80` (un gros initialiseur de
tampons de debug, lui-même seulement atteignable par la même chaîne de
overlay de debug confirmée morte) — `FUN_01002c80` lui-même reste
différé (trop de sous-dépendances non tracées pour une valeur marginale
sur un chemin mort), mais `FUN_0064a9e0` vaut le détour puisque c'est
du vrai code D3D9 testable.

Deux vrais bugs de test trouvés et corrigés en écrivant
`tests/screen_overlay_render_smoke.cpp` : un vtable de surface factice
dimensionné seulement pour `Release` (8 octets) alors que le chemin
« device prêt » de `prepare_render_target_texture` appelle aussi
`GetDesc` (+0x30) — même piège que `feedback_fake_vtable_sizing`
documente déjà ; et `DAT_00cf19d0` laissé à sa valeur par défaut (nulle)
alors qu'`initialize_default_render_state` le déréférence directement
(`DAT_00cf19d0 + DAT_008663d8*0x584 + 0x474`) pour lire un octet de
capacité — `tests/device_lost_handler_smoke.cpp` avait déjà ce correctif
(un tampon factice alloué et pointé par `DAT_00cf19d0`), appliqué ici
aussi.

**434/469 fonctions intégrées (35 différées, 0 bloquée), 43/43 tests
CTest verts.**

## 19 oct 2026 (suite 20) — `fast_cos`, un vrai cosinus approché par polynôme, trouvé dans le lot `float_ops_015` abandonné

Le lot `float_ops_015` (5 fonctions) portait une note décourageante :
« ReAgent indisponible, une implémentation manuelle précédente utilisait
des globals inventés et a été retirée ». En relisant le C décompilé de
`FUN_00401270`, les quatre fonctions de ce lot partagent un piège
identique : chaque chemin se termine par un simple `return;` sans
valeur — Ghidra ne reconnaît pas que le vrai résultat flottant reste
dans `XMM0` à cet instant, invisible au niveau C décompilé. D'où
l'échec probable de la tentative précédente (sans doute une
implémentation inventée pour « faire quelque chose » plutôt que de
tracer le vrai registre).

Désassemblage direct (`pyghidra`) de `FUN_00401270` : c'est une
**approximation polynomiale de cos(x) par série de Taylor d'ordre 16**
(puissances paires uniquement, 9 coefficients `1, -1/2!, 1/4!, ...,
1/16!`), avec réduction d'angle en deux repliements (`min(r0, 2π-r0)`
puis `min(r1, π-r1)`) pour ramener l'angle dans `[0, π/2]` avant
d'évaluer le polynôme, et un indicateur d'inversion de signe calculé
comme une identité flottante exacte `A - A == 0` (robuste malgré les
arrondis, puisque réutilise la MÊME valeur calculée une fois). Tous les
coefficients et constantes (`π`, `2π`, `1/2π`) ont été lus directement
en mémoire depuis le binaire (`0x0087e990`..`ac`, `0x00866e70`,
`0x0087ea90/94/98`) — confirmés correspondre exactement à la vraie
série de Taylor du cosinus, pas recalculés depuis une bibliothèque.

Nouvelle fonction `hp6::math::fast_cos` (`src/math/Scalar.cpp`),
vérifiée contre `std::cos` à ~1e-7 près sur une période complète et un
déroulement positif de 15 périodes. Un vrai défaut de précision a été
découvert et **testé plutôt que caché** : la réduction d'angle tronque
vers zéro (comme la vraie instruction `CVTTSS2SI`), ce qui ne ramène
correctement l'angle dans `[0, 2π)` que pour des angles d'entrée
**positifs** — certains grands angles négatifs restent largement
non réduits et perdent en précision (toujours à ~1 % près, jamais
du grand n'importe quoi). Fidèle au binaire réel, pas « corrigé ».

Les trois autres fonctions du même lot (`004013a0`, `00401da0`,
`00403190`) restent différées : `004013a0` appelle déjà
`hp6::math::sqrt_x87` (donc probablement liée à sin/asin via
l'identité pythagoricienne) mais nécessiterait le même travail de
désassemblage pour en confirmer le registre de retour implicite —
piste notée pour une prochaine session.

**435/469 fonctions intégrées (34 différées, 0 bloquée), 43/43 tests
CTest verts.**
