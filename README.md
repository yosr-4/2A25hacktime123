# HackTime – Module Atelier (Qt / C++)

## Lancer le projet
1. Qt Creator → **Ouvrir un projet** → `HackTimeAtelier.pro`
2. Choisir un kit (Qt 5.15 ou Qt 6, avec les modules **Widgets, Sql, PrintSupport**)
3. **Ctrl+R**

## Base de données
`connexion.cpp` (haut du fichier) contient les paramètres de la base commune
(MySQL : hôte, port, nom `hacktime`, utilisateur, mot de passe).
Si MySQL n'est pas joignable (ou si le pilote QMYSQL manque), l'application bascule sur une
base **SQLite locale** créée automatiquement avec le même schéma (MLD du PDF) et des exemples.
Le message de connexion s'affiche dans la barre d'état en bas.

Tables utilisées : `Atelier` (CRUD) et `Competition` (lecture : liste déroulante + jointure).
Hypothèses : `duree` = entier en minutes ; `date_atelier` = DATETIME.
Adaptez les requêtes dans `atelier.cpp` si votre base diffère.

## Fonctionnalités
- Ajouter / modifier / supprimer / dupliquer un atelier (contrôle de saisie + détection de conflit de salle)
- Recherche, filtres (compétition, statut, année), tri, pagination
- KPI, histogramme par mois, détails de l'atelier sélectionné
- Export PDF et export CSV (ouvrable dans Excel)
