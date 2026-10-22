# Petites fonctions vérifiées dans le binaire HP6 PC

## Provenance

| Image | Taille | SHA-256 | Rôle |
| --- | ---: | --- | --- |
| `D:\HP\hp6.exe` | 10 749 200 octets | `4653e4301ed6dfc271409da2443fd817a65b6514704a3487e86895337e487954` | Exécutable original protégé par SecuROM |
| `D:\HP\hp6ss\build\sdk-lab\pc\hp6.exe` | 5 054 644 octets | `41cbb7ad4b08f6edeb5c2a06aea037d1c0634e9b71df093cc9a26b2287916e10` | Exécutable sans SecuROM utilisé pour analyser le code |

Les deux images conservent la même base et des données presque identiques. Leur section `.text` diffère fortement, car le code de l'original est protégé. Les octets de fonctions ci-dessous sont donc **prouvés pour l'image sans SecuROM** ; ils ne constituent pas une preuve d'identité octet pour octet avec la section `.text` de l'original protégé.

## Reconstruction

`tools/find_small_binary_leafs.py` lit le projet Ghidra de l'image sans SecuROM, vérifie les frontières des fonctions, leurs octets et les décompilations. `tools/generate_small_binary_leafs.py` revérifie le SHA-256 et les octets dans le fichier PE avant de produire `src/runtime/VerifiedBinaryLeafs.h` et `.cpp`. Chaque définition C++ porte l'adresse et la séquence exacte d'instructions qui la justifient.

31 fonctions sans appel sortant sont intégrées : 28 restent dans `VerifiedBinaryLeafs`, et trois portent maintenant des noms établis par leurs appelants, `hp6::platform::lock_acquisition_depth` (`006d8f90`), `hp6::audio::path_get_volume` (`006c1d20`) et `hp6::audio::path_get_dry_level` (`006be290`). Les autres couvrent les constantes, les fonctions vides, les lectures et écritures de champs, la lecture d'un global déjà typé, les accès via argument sur pile et deux sommes de champs. Les noms `fn_<adresse>` restent provisoires lorsque le rôle précis n'est pas établi. Le relevé des contextes et des limites figure dans `architecture/verified_leaf_origins.md`.

Des candidates restent volontairement hors de cette intégration : écritures dans des globaux dont le type et les alias ne sont pas vérifiés, lectures de globaux non déclarés dans la reconstruction et deux fonctions avec `ret 4` dont la convention d'appel demande encore un examen. Le flux de lancement et le menu restent en pause pendant ce relevé des petites fonctions.

## Reproduire

Depuis la racine du dépôt, avec le projet Ghidra indiqué par `ghidra-bridge.yaml` :

```powershell
python tools/find_small_binary_leafs.py --output build/small_binary_leafs.json
python tools/generate_small_binary_leafs.py D:/HP/hp6ss/build/sdk-lab/pc/hp6.exe build/small_binary_leafs.json src/runtime/VerifiedBinaryLeafs.h src/runtime/VerifiedBinaryLeafs.cpp
cmake --build build --config Debug --parallel 8
ctest --test-dir build -C Debug --output-on-failure
```
