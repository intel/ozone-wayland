# Introduction

Ozone-Wayland is the implementation of Chromium's Ozone for supporting [Wayland](http://wayland.freedesktop.org/) graphics system. Different projects based on Chromium/Blink like the Chrome Browser, Chrome OS, [Crosswalk](https://crosswalk-project.org/), among others can be enabled now using Wayland.

![Alt text](https://raw.github.com/tiagovignatti/misc/master/chromium-2013-06-07-small.png "Content Shell running on Weston")

## Contents

  - [Design](#design) - the architecture behind
  - [Howto](#howto) - set up the system environment, build and run
  - [Gardening](#gardening) - updating to the latest Chromium codebase
  - [Contributing](#contributing) - help develop and send patches
  - [License](#license)

## Design

Ozone-Wayland [architecture](https://docs.google.com/document/d/118Cmq_dedHOr4jfyVeE4jBhV7hXzhnaVCqegNMGano0/) can be found here.

## Howto

We use Ubuntu 12.04 LTS (Precise Pangolin), 32/64-bit but a kernel from Raring though. One can install it pretty easily just
`sudo apt-get install linux-generic-lts-raring`. This version is needed for using Mesa master 9.3.0-devel, a requirement for Weston. Besides, make sure you have **at least** 25 GB of disk space to store all the sources and a few targets to be compiled (with debugging symbols).

Firstly you'd need to set up the Wayland libraries (version >= 1.4.0), and the Weston reference compositor that you will be running Chromium onto. The information on Wayland's web page should be enough for doing so:

http://wayland.freedesktop.org/building.html

_Please note that at the moment support for Wayland's XDG Shell protocol only works with the 1.5 series._

Make sure everything is alright now, setting up the environment variable `$XDG_RUNTIME_DIR` and playing a bit with the Wayland clients, connecting them on Weston.

Then you'd need some distribution tools and development packages :

  ```
  $ sudo apt-get install gperf libnss3-dev libpulse-dev libdbus-1-dev libexif-dev libpci-dev libpango1.0-dev libelf-dev libcap-dev libasound2-dev libssl-dev libcups2-dev libgconf2-dev libgnome-keyring-dev
  ```

If you are running a 64-bit Ubuntu distribution, you'd additionally need :

  ```
  $ sudo apt-get install ia32-libs
  ```

Then on Chromium's side, we need to setup Chromium's tree together with the
Ozone-Wayland implementation. For that you need to use gclient to clone
Ozone-Wayland; but first you need to download
[depot\_tools](http://dev.chromium.org/developers/how-tos/install-depot-tools)
and configure it. Say your Chromium top-level will be in `~git/chromium`, you will
need:

  ```
  $ mkdir -p ~git/chromium
  $ cd ~git/chromium
  $ git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
  $ export PATH=`pwd`/depot_tools:"$PATH"
  ```

We support HWA Video Decoding using Libva and VAAPI. To enable the support one needs to build
and install these packages. Note: This is still work in progress. You can skip these dependencies if
you don’t need this support.

One needs to set the installation prefix $WLD(Below)  to be same as the one used for setting up Wayland environment.
  ```
  $ git clone git://anongit.freedesktop.org/libva  
  $ cd libva/
  $ git checkout libva-1.2.1
  $ ./autogen.sh --prefix=$WLD  
  $ make -j4 && make install
  ```
  ```
  $ git clone git://anongit.freedesktop.org/vaapi/intel-driver
  $ cd intel-driver/
  $ git checkout 1.2.1
  $ ./autogen.sh --prefix=$WLD  
  $ make -j4 && make install
  ```
now we can clone Ozone-Wayland and fetch all the dependencies of it, including Chromium itself:

  ```
  $ gclient config ssh://git@github.com/01org/ozone-wayland.git --name=src/ozone --deps-file ".DEPS.git"
  $ export GYP_DEFINES='use_ash=1 use_aura=1 chromeos=0 use_ozone=1' 
  ```
  If you want to enable HWA Video Decoding, you need to have the following GYP_DEFINES:
  ```
   $ export GYP_DEFINES='use_ash=1 use_aura=1 chromeos=0 use_ozone=1 proprietary_codecs=1 ffmpeg_branding=Chrome'
   ```
  ```
   $ gclient sync 
   ```
It may take a considerable time for downloading the trees. If everything went
fine, now we're able to build.

For now, also apply some patches:

  ```
  ./src/ozone/patches/patch-chromium.sh
  ```

TIP: to speed up debug builds you can disable Blink debugging symbols by setting remove_webcore_debug_symbols=1 in GYP_DEFINES.

Note that in Chromium, gyp uses pkg-config for checking where are Wayland libraries on the system, so double check that you are not mixing some that was already there with latest that you just got and compiled from git.

TIP: if you followed Wayland's web page instructions, then you probably want to set the `PKG_CONFIG_PATH` variable as `$HOME/install` and add it in your .bashrc to be as default.

Now we can conclude compiling a few targets like Content Shell (content_shell) or the Chromium browser:

  ```
  $ cd src/
  $ ./build/gyp_chromium
  $ ninja -C out/Debug -j16 chrome
  ```
(if you get any build errors, please execute "git log" and verify that the first "commit" line matches the "chromium_rev" one found in `src/ozone/.DEPS.git`. If it doesn't, run "git checkout $ID", apply the patches and compile again)

That's all. At this point you should be able to connect Chromium on Weston using:

  ```
  $ ~/git/weston/src/weston &
  $ ./out/Debug/chrome --no-sandbox
  ```

###Working with Ozone-Wayland Release branch:

Instructions can be found here: https://github.com/01org/ozone-wayland/wiki/Releasing#wiki-releasebranch

## Gardening

We pin chromium to a particular revision in order to keep upstream changes from
breaking our build. Updating that revision to a newer one and fixing any
resulting breakage is called gardening. To sync a different version of chromium,
update chromium_rev in .DEPS.git to a newer revision then run the gclient sync
again. Fix any build errors, and commit both changes.

## Contributing

Instructions can be found here: https://github.com/otcshare/ozone-wayland/wiki

## License

Ozone-Wayland's code uses the BSD license (check the LICENSE file in the project).
