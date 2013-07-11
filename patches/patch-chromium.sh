#!/bin/sh

PATCH_DIR=ozone/patches/

echo "Ozone-Wayland: patching Chromium"
touch patching_chromium_bla

cd src/

git am $PATCH_DIR/000*
