# PokemonScan
 Identification d'Images
  ![image](https://github.com/user-attachments/assets/549c166a-85b5-41b0-81f0-77d85c037b4b)

## Description
   Ce projet consiste en une application client-serveur permettant de comparer des images BMP. Le client envoie une image BMP au serveur, qui compare cette image avec une banque d'images stockées localement et renvoie l'image la plus similaire ainsi que la distance de similarité.

## Fonctionnement
### Serveur
Le serveur écoute sur le port 5555 et attend les connexions des clients. Lorsqu'un client envoie une image BMP, le serveur compare cette image avec celles de la banque d'images stockées dans le répertoire img. La comparaison est effectuée en calculant le code de hachage perceptif (pHash) de chaque image et en déterminant la distance de Hamming entre les codes de hachage.

### Client
Le client permet à l'utilisateur de spécifier le chemin d'une image BMP à envoyer au serveur. Le client envoie l'image au serveur et affiche la réponse du serveur, qui indique l'image la plus similaire trouvée dans la banque d'images ainsi que la distance de similarité.


## Structure du Projet
Le projet est organisé en plusieurs composants :

- `img-dist` : Un programme en C qui génère le code de hachage perceptif pour les images.
- `list-file` : Un script Bash qui liste les fichiers d'un dossier.
- `img-search` : Un programme en C qui recherche les images les plus similaires à une image donnée en utilisant deux processus enfants pour effectuer la recherche de manière concurrente.
- `launcher` : Un script Bash qui lance le programme img-search avec différentes options (mode automatique et interactif).

## Structure du Projet
```bash
pokedex/
├── client/
│   └── pokedex-client.c       # Code source du client
├── commun/
│   └── commun.h               # Fichier d'en-tête commun
├── img/                       # Répertoire contenant les images de la banque
├── img-dist/
│   ├── bmp-endian.h           # Fichier d'en-tête pour la gestion des BMP
│   ├── bmp.c                  # Code source pour la gestion des BMP
│   ├── bmp.h                  # Fichier d'en-tête pour la gestion des BMP
│   ├── Makefile               # Makefile pour la compilation des utilitaires d'image
│   ├── pHash.c                # Code source pour le calcul du pHash
│   ├── pHash.h                # Fichier d'en-tête pour le calcul du pHash
│   ├── verbose.c              # Code source pour les messages de débogage
│   └── verbose.h              # Fichier d'en-tête pour les messages de débogage
├── Makefile                   # Makefile principal pour la compilation du projet
├── serveur/
│   ├── img-search.c           # Code source du serveur
│   └── imgdist.h              # Fichier d'en-tête pour les fonctions de traitement d'image
└── test/
    ├── img/                   # Répertoire contenant les images de test
    ├── test-new-images.data   # Données de test pour les nouvelles images
    └── tests                  # Script pour exécuter les tests
```

## Compilation
Pour compiler le projet, utilisez le Makefile principal situé à la racine du projet. Ce Makefile compile à la fois le serveur et le client.
```bash
cd img-dist/
make

cd ..
make
```

## Exécution
### Serveur
Pour lancer le serveur, exécutez la commande suivante :
```bash
./img-search
```
### Client
Pour lancer le client, exécutez la commande suivante :
```bash
./pokedex-client [adresse_ip_serveur]
```
## Dépendances
Le projet utilise les bibliothèques standard C suivantes :

`stdio.h`
`stdlib.h`
`string.h`
`unistd.h`
`signal.h`
`pthread.h`
`sys/socket.h`
`netinet/in.h`
`arpa/inet.h`
`dirent.h`
`stdatomic.h`

## Auteurs
- paug0002
- jche0027
- rrab0007
