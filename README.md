Titre du projet: Point Zéro

Membres du projet:
1. Oleh DRAHOMIRETSKYI
2. Andrii MYKYTENKO

Date limite: 17 avril 2026

Diagramme de Gantt Prévisionel >> https://docs.google.com/spreadsheets/d/1IqbCmA31hAxSB0njCKjtLSsNYLb4gT-OUikucNKHJD8/edit?usp=sharing

Diagramme de Gantt Réel >> https://docs.google.com/spreadsheets/d/1W8gmvHZU9V74IEWx6prfptLu3vMO4HS02HlB6MIEWSU/edit?usp=sharing

Description du jeu : 

Point Zéro est une réinterprétation moderne et stratégique du jeu classique de la Bataille Navale. Tout en conservant les règles fondamentales qui ont fait le succès du jeu original, ce projet introduit un système économique et de nouvelles mécaniques qui transforment chaque partie.
Le projet s'appuie sur une interface 2D épurée et réactive, développée avec la bibliothèque SDL2. Ce choix technique garantit une fluidité exemplaire des animations et une gestion optimale du conrôle, tout en offrant une expérience visuelle moderne qui modernise radicalement l'aspect statique de la bataille navale traditionnelle.

Description de la fonctionnalité : 

Après avoir lancé le jeu, vous êtes dans le menu principal avec plusieurs boutons. Il est possible de choisir le niveau de difficulté et au début il y a le choix de deux styles de menu.
Pour commencer le jeu vous cliquez sur le bouton JOUER et là il est possible de configurer votre flotte en balayant les navires avec le souris ou en choisissant la configuration aléatoire.
Après chaque partie gagnée vous obtenez la monnaie que vous pouvez dépenser pour débloquer les nouveaux styles.
Il est également possible de consulter votre score qui comprend les nombres de victoires et défaites, la taux de réussite et la meilleure partie.


Guide d'installation :

D'abord, les bibliothèques SDL2 doivent être installées.
Vous pouvez les télécharger en cliquant sur ce lien : https://github.com/libsdl-org/SDL/releases/tag/release-2.30.8

Après il faut suivre le tutoriel d'installation : https://umtice.univ-lemans.fr/pluginfile.php/174420/mod_resource/content/3/Tutoriel_SDL2.pdf


COMPILATION ET LANCEMENT >>

1. Executer la commande git clone https://github.com/olehdrahomiretskyi/Bataille-navale ou télécharger l'archive depuis ce dépôt git et l'extraire dans un dossier
2. Aller dans le répertoire Bataille-navale
3. Compiler avec make
4. Executer bin/battleship
   

Le fichier records.dat sera créé automatiquement
  au premier lancement (sauvegarde des scores).
  
Toutes les explications a propos de déplacement sont
  indiquées dans le jeu.
  
Pour quitter le jeu en mode plein ecran : bouton QUITTER
  dans le menu principal, ou Alt+F4.
  


