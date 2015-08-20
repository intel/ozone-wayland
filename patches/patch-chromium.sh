#!/bin/sh

PATCH_DIR=`pwd`/src/ozone/patches/
HACKING_BRANCH=master-ozone
RELEASE_BRANCH=45.0.2454.37

echo "Ozone-Wayland: patching Chromium"
cd src/

# we switch to $HACKING_BRANCH before anything
git checkout $RELEASE_BRANCH

exists=`git show-ref refs/heads/$HACKING_BRANCH`
if [ -n "$exists" ]; then
  git branch -D $HACKING_BRANCH
fi

git checkout -b $HACKING_BRANCH $RELEASE_BRANCH
git am $PATCH_DIR/00*
