#!/bin/sh


PATCH_DIR=`pwd`/src/ozone/patches/

echo "Ozone-Wayland: patching Chromium"
cd src/

# making sure these patches were not applied already. if so, purge them
git reset --hard origin/master

git am $PATCH_DIR/000*

# jump now to mesa dir and apply the needed patches there
cd third_party/mesa/src
git am $PATCH_DIR/100*
