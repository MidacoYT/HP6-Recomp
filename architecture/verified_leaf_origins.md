# Origine et rôle des 31 petites fonctions

Les noms de fonctions d'origine ne sont pas présents dans le PE analysé. Les rôles ci-dessous reposent sur les octets de l'exécutable sans SecuROM, les exports du projet Ghidra et les références dans les appelants ou les tables de fonctions. Une référence Ghidra de type `COMPUTED_CALL` n'est pas un appel direct : elle désigne une cible possible d'un appel virtuel.

| Adresses | Origine établie | Rôle établi et limite |
| --- | --- | --- |
| `00401220`, `00405a30` | `0045b8a0`, `0045f8d0` et de nombreux autres constructeurs passent ces adresses à `_eh_vector_constructor_iterator_` / `_eh_vector_destructor_iterator_`. | Callback de construction qui renvoie le pointeur d'objet reçu en ECX ; callback de destruction vide. `00405a30` est aussi réutilisé comme méthode virtuelle vide. |
| `00404de0` | Lecture de `DAT_00998580`; appelée par `00648550` pendant une transformation de coordonnées. | Renvoie le global. Le rôle du global reste inconnu ; la valeur de retour n'apparaît pas dans la décompilation de cet appelant. |
| `00407b30`, `004d59c0`, `004eef20`, `00622a70` | Les quatre adresses occupent des entrées de tables virtuelles utilisées par `00492b60`, qui alloue des entrées `FiniteStateMachine::RegisterState` et interroge la méthode virtuelle à `+0x14`. | Renvoient respectivement `1`, `2`, `3`, `4`. Dans ce contexte, ce sont des identifiants d'état. Les mêmes fonctions triviales peuvent être partagées par d'autres classes ; elles ne portent donc pas un nom de classe unique. |
| `0050c0c0` | Méthode virtuelle dans des tables à `0082fbc4` et suivantes ; `005d2c10` enlève un élément d'une liste, appelle ses méthodes virtuelles puis libère sa mémoire. | Met le mot à `objet+0x8` à zéro. Le type de l'objet n'est pas suffisamment établi pour nommer ce champ. |
| `0062b2c0` | Référencée par les tables à `00842560`, `008425a4`, `008425b4`; cible virtuelle possible de `00639110`. | Lit le mot à `objet+0x8`; le rôle du champ est inconnu. |
| `00653600` | Plusieurs tables virtuelles et appels calculés, notamment dans les chemins `00508820`, `0053f280`, `0059cfa0`, `0059d030`. | Lit l'octet à `objet+0x4`. Les quatre contextes ne prouvent pas une classe ou un nom de champ commun. |
| `006a7e50` | Appel direct de `00fe9c10`. | Déréférence un pointeur de mot 32 bits. |
| `006ae620`, `006c9700`, `006dadcf` | Respectivement dans les chemins `006ae4e0`, de destruction `006896e0` et d'arrêt `006882b0`. | Méthodes vides ; leur emploi est connu, mais aucun état n'est modifié. |
| `006b477f` | Appels directs depuis le traitement de commandes `006b6be4` et la construction MemCard `0100ee80`. | Remet deux mots consécutifs à zéro. C'est un initialiseur partagé, sans type d'objet unique. |
| `006bde50`, `006bdf50`, `006bdfa0`, `006bdfc0`, `006c1d00` | Tables de fonctions en `.rdata` autour de `00716fc0` et `00718210` à `007183f0`. Les chaînes voisines mentionnent `PATH.lib` et les pistes. | Accesseurs virtuels d'objets de la bibliothèque audio PATH. Les offsets lus sont exactement `+0x64`, `+0x14`, `+0x04`, `+0x24`, `+0x58`. Il manque la définition de chaque classe et de chaque champ pour choisir des noms métier. |
| `006be290` | `006c2980` et `006c3be0` appellent la fonction de journalisation `006c1e00` (`PF_dr ... DRYLEVEL`) puis écrivent le champ `+0x10`. Les tables virtuelles placent `006be290` au slot de lecture associé. | Lit le niveau d'effet « dry » à `+0x10`. Intégrée sous le nom `hp6::audio::path_get_dry_level`. |
| `006c1d20` | La table virtuelle commençant à `00718210` place cette méthode à `+0x2c` (`0071823c`). La méthode `006c1d40`, dans la même table à `+0x30`, appelle le slot `+0x2c` avant de comparer et journaliser `VOLUME ancien -> nouveau`. | Lecture du volume courant au champ `+0x14`. Intégrée sous le nom `hp6::audio::path_get_volume` dans `src/audio/PathVolume.cpp`. |
| `006c9e80`, `006c9e90`, `006c9eb0` | Fonctions voisines dans la zone PATH, sans appel ni pointeur de fonction détecté dans ce PE. | Lisent `+0x244`, puis calculent `(+0x244)+(+0x220)` et `(+0x244)+(+0x224)`. L'appartenance à un objet particulier n'est pas démontrée. |
| `006cec20` | Fonction voisine de `006cec00`, qui écrit l'octet au même offset. Aucune référence entrante détectée. | Lit l'octet à `+0x1ac` ; la paire lecture/écriture est établie, pas le sens du champ. |
| `006d8f90` | `006d8ea0` incrémente le mot à `verrou+0x20` après acquisition. `006be390` le lit avant attente et `006be470` le teste avant libération. | Lit la profondeur d'acquisition du verrou. Cette fonction porte maintenant le nom `hp6::platform::lock_acquisition_depth` dans `src/platform/SyncAccessors.cpp`. |
| `006de5de` | Appelée au début des chemins d'interrogation de périphériques `006dd1aa`, `006dd376`, `006ddb48`. | Renvoie toujours `0` dans ce binaire ; aucun état n'est lu ou modifié. |
| `006e208d` | Enregistrée comme callback par `006deca0`, appelée depuis la mise à jour `006e0018`. | Callback vide dans ce binaire. |
| `006e6016` | Appelée par `006df12f` et `006df141` lors des calculs d'offsets audio. | Renvoie toujours `0` dans ce binaire. |
| `006e712e` | Deux appels de `006e0f39` sur les groupes de trois mots à `param_4+0x120` et `param_4+0x12c` pendant la préparation d'un tampon audio. | Efface trois mots consécutifs. Le nom des champs audio n'est pas démontré. |

Les adresses `006c9e80`, `006c9e90`, `006c9eb0` et `006cec20` ne possèdent aucune référence entrante détectable dans les sections de code et de données examinées. Leur comportement reste fonctionnel et testé, mais il serait inventé de leur attribuer une place certaine dans le flux du menu ou un nom de classe d'origine.

Pour reproduire les références et pointeurs de fonction :

```powershell
python tools/trace_verified_leafs.py D:/HP/hp6ss/build/sdk-lab/pc/hp6.exe --details --output build/verified_leaf_traces.json
```
