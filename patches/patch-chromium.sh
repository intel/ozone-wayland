#!/bin/sh

PATCH_DIR=`pwd`/src/ozone/patches/
HACKING_BRANCH=master-ozone
HACKING_OZONE_BRANCH=master-ozonewayland
RELEASE_BRANCH=branch_2272
RELEASE_OZONE_BRANCH=Milestone-Carnival

echo "Ozone-Wayland: patching Chromium"
cd src/

# we switch to $HACKING_BRANCH before anything
git checkout $RELEASE_BRANCH

# Chromium patches.
exists=`git show-ref refs/heads/$HACKING_BRANCH`
if [ -n "$exists" ]; then
  git branch -D $HACKING_BRANCH
fi

git checkout -b $HACKING_BRANCH $RELEASE_BRANCH
git am $PATCH_DIR/00*

# Switch ozone-wayland branch.
cd ozone/
git checkout $RELEASE_OZONE_BRANCH
exists=`git show-ref refs/heads/$HACKING_OZONE_BRANCH`
if [ -n "$exists" ]; then
  git branch -D $HACKING_OZONE_BRANCH
fi
git checkout -b $HACKING_OZONE_BRANCH $RELEASE_OZONE_BRANCH
cd ..

# jump now to WebRTC dir and apply the needed patches there
cd third_party/webrtc/
git reset --hard origin/master
git am $PATCH_DIR/100*
