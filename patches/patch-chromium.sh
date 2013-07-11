#!/bin/sh

PATCH_DIR=ozone/patches/

echo "Ozone-Wayland: patching Chromium"
touch patching_chromium_bla

cd src/

# making sure these patches were not applied already. if so, purge them
git reset --hard origin/master

git am $PATCH_DIR/000*
