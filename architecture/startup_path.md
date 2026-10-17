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

Le plan `startup_prereqs_020` a ?t? arr?t? devant douze d?pendances hors
lot. Le lot `startup_leaf_021` a int?gr? la gestion des param?tres syst?me
(005d9210) et un calcul de taille de ressource (006dadb9). Le lot
`window_shell_022` a int?gr? la cr?ation et l?enregistrement de la classe
de fen?tre (005d8ed0, 00ff8380, avec le thunk 005d8e30). Un test Win32
cr?e et d?truit une fen?tre de 320?240 ; le gestionnaire de messages r?el
005d8a30 reste diff?r?. Ces adaptations rendent les coordonn?es et les
d?pendances au syst?me explicites, mais ne prouvent pas l??quivalence ABI
avec les registres employ?s par le binaire.
