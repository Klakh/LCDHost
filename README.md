# LCDHost

Gestionnaire de plugins qui compose des affichages pour écrans secondaires
(à l'origine les écrans LCD des claviers Logitech G15/G19). Fork de
[linkdata/LCDHost](https://github.com/linkdata/LCDHost), dont le code n'est plus
maintenu depuis 2016. Licence GPL v3 (voir `COPYING`).

## Compiler

Prérequis : Qt **5.15** (modules de base + `qtwebengine` pour le plugin
LH_WebKit) et un compilateur C++.

### Linux

```sh
sudo apt-get install libudev-dev libgl1-mesa-dev
mkdir build && cd build
qmake ../LCDHost.pro -r
make -j"$(nproc)"
```

Les binaires se trouvent dans `build/LCDHost.app/bin/` (application et
plugins dans le même dossier).

### Windows

Qt 5.15.2 `win64_msvc2019_64` + Visual Studio 2019/2022, depuis une invite
« x64 Native Tools » :

```bat
mkdir build && cd build
qmake ..\LCDHost.pro -r
nmake
```

Pour obtenir un dossier autonome : `windeployqt` sur `LCDHost.exe` et
`WebKitServer.exe`, puis copier `layouts/` dans `build\LCDHost.app\`
(c'est ce que fait la CI).

## Tester sans matériel

Le plugin **VirtualLCD** fournit des écrans virtuels 320×240 (format G19) et
160×43 (format G15), sélectionnables dans l'onglet *Output*.

Test de fumée sans écran (démarre l'application 20 s avec le layout par défaut
et vérifie le chargement des plugins principaux) :

```sh
tests/smoke/run.sh build/LCDHost.app/bin 20
```

## Premier lancement

- Les plugins sont désactivés par défaut : les charger dans l'onglet *Plugins*.
- Les layouts livrés à côté des binaires (`LCDHost.app/layouts/`) sont copiés
  dans `~/Documents/LCDHost/layouts/` au premier lancement, si ce dossier
  n'existe pas encore.

## Binaires

La CI GitHub Actions (`.github/workflows/build.yml`) compile Linux et Windows
x64, lance un test de fumée et publie les dossiers `LCDHost.app` en artefacts
(`LCDHost-linux-x64`, `LCDHost-windows-x64`).
