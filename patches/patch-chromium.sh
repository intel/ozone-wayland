#!/bin/sh

PATCH_DIR=`pwd`/src/ozone/patches/
HACKING_BRANCH=master-ozone

echo "Ozone-Wayland: patching Chromium"
cd src/

# we switch to $HACKING_BRANCH before anything
git checkout master

exists=`git show-ref refs/heads/$HACKING_BRANCH`
if [ -n "$exists" ]; then
  git branch -D $HACKING_BRANCH
fi

git checkout -b $HACKING_BRANCH master
git am $PATCH_DIR/00*


