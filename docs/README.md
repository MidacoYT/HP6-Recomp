# Site de progression (HP6-Recomp)

Petit site statique (HTML/CSS/JS vanilla, aucune dependance) qui affiche
l'avancement de la decompilation/reconstruction, dans l'esprit de
decomp.dev : pourcentage global, repartition par module, et la liste
complete des fonctions avec leur statut.

## Regenerer les donnees

Les chiffres viennent de `reports/pipeline/state.json` (la meme source de
verite que `architecture/address_map.csv`). Apres une session
d'integration, relancer :

```
python tools/export_progress.py
```

Cela reecrit `docs/data/progress.json`. Rien d'autre n'a besoin d'etre
regenere.

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
