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

git checkout -b $HACKING_BRANCH origin/master
git am $PATCH_DIR/00*

# jump now to mesa dir and apply the needed patches there
cd third_party/mesa/src
git reset --hard origin/master
git am $PATCH_DIR/100*
