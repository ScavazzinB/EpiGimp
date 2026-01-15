# EpiGimp

Un éditeur d'images simple inspiré de GIMP, développé en C++ avec GTKmm.

## Fonctionnalités

### Gestion des fichiers
- Ouvrir des images PNG et JPEG
- Nom du fichier affiché dans la barre de titre

### Navigation
- **Zoom** : Molette souris, `Ctrl++`, `Ctrl+-`
- **Zoom 100%** : `Ctrl+0`
- **Ajuster à la fenêtre** : `Ctrl+9`
- **Pan** : Clic gauche + glisser (ou clic molette)
- Niveau de zoom affiché dans la barre de statut

### Outils
| Outil | Bouton | Raccourci | Description |
|-------|--------|-----------|-------------|
| Pipette | 🎨 | `I` | Récupère la couleur d'un pixel |

### Couleurs
- Panneau couleurs primaire/secondaire
- Échanger les couleurs : `X`
- Valeurs RGB affichées dans la barre de statut

## Compilation

### Dépendances

```bash
sudo apt install libgtkmm-3.0-dev libsdl2-dev libpng-dev libjpeg-dev
```

### Build

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
./EpiGimp
```

## Stack technique

| Technologie | Utilisation |
|-------------|-------------|
| C++17 | Langage principal |
| GTKmm 3.0 | Interface graphique |
| Cairo | Rendu 2D |
| GdkPixbuf | Chargement d'images |
| SDL2 | (Prévu pour le rendu avancé) |
| libpng/libjpeg | Support formats image |
| CMake | Système de build |

## Structure du projet

```
EpiGimp/
├── CMakeLists.txt
├── README.md
├── include/
│   └── epigimp/
│       └── MainWindow.hpp
├── src/
│   ├── main.cpp
│   └── MainWindow.cpp
└── build/
```

## Roadmap

- [ ] Outil pinceau
- [ ] Outil gomme
- [ ] Sélection rectangulaire
- [ ] Calques
- [ ] Sauvegarde d'images
- [ ] Filtres (flou, netteté, etc.)
- [ ] Historique undo/redo

## Licence

MIT
