# Site de progression (HP6-Recomp)

Petit site statique (HTML/CSS/JS vanilla, aucune dependance) qui affiche
l'avancement de la decompilation/reconstruction, dans l'esprit de
decomp.dev : pourcentage global, repartition par module, et la liste
complete des fonctions avec leur statut.

Une carte (treemap, dans l'esprit de decomp.dev) affiche les ~14 200
fonctions sous forme de carres (taille = taille en octets, couleur =
statut) : survoler ou toucher un carre l'identifie, cliquer dessus le
retrouve surligne dans le tableau en dessous. Elle est dessinee sur
`<canvas>` (pas une div par fonction) pour rester fluide avec ce volume
de donnees, et respecte les memes filtres/recherche que le tableau
(les carres qui ne correspondent pas s'estompent).

Deux pourcentages distincts sont affiches : un pourcentage **global**
(fonctions integrees / ~14 200 fonctions du binaire entier, en croisant
le catalogue Ghidra de ce depot avec le catalogue IDA independant du
projet voisin `knowledge/hp6`), et un pourcentage sur les fonctions
**deja etudiees** (le sous-ensemble que ce projet a explicitement
regarde, actuellement 466).

## Regenerer les donnees

Les chiffres viennent de `reports/pipeline/state.json` (la meme source de
verite que `architecture/address_map.csv`), croise avec
`../knowledge/hp6/functions.json` (catalogue IDA complet du projet
voisin, pas versionne dans ce depot) pour le pourcentage global. Apres
une session d'integration, relancer :

```
python tools/export_progress.py
```

Cela reecrit `docs/data/progress.json`. Si le catalogue IDA voisin est
absent (ex. clone propre sans le projet voisin), le script bascule
automatiquement sur le sous-ensemble deja etudie uniquement (comme
avant). Rien d'autre n'a besoin d'etre regenere.

## Visualiser en local

```
cd docs
python -m http.server 8080
```

puis ouvrir `http://localhost:8080/`.

## Publier sur GitHub Pages

Une fois le depot pousse sur `origin` (`main`), activer GitHub Pages dans
Settings > Pages avec la source "Deploy from a branch", branche `main`,
dossier `/docs`. Aucune etape de build n'est necessaire : le dossier est
servi tel quel.
