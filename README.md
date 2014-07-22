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

[Ozone](http://www.chromium.org/developers/design-documents/ozone) is a set of classes in Chromium for abstracting different window systems on Linux. It provides abstraction for the construction of accelerated surfaces underlying Aura UI framework, input devices assignment and event handling:

![Alt text](https://raw.github.com/tiagovignatti/misc/master/ozone-architecture-overview-small.png "Ozone architecture overview")

Before, when using Aura on Linux all the native windowing system code (X11) was spread throughout Chromium tree. Now the idea is that Ozone will abstract the native code and because it's a set of class factories, it will switch for whatever is the window system.

One of the biggest advantages of Chromium Ozone is that it allows to implement the window system details externally from the Chromium tree, which is great because it is where the loaded work situates in many cases. Therefore, Ozone-Wayland can be seen as a sibling project of Chromium where one is empowering the other with different characteristics. Besides Ozone-Wayland, there are different [Ozone implementations](http://www.chromium.org/developers/design-documents/ozone#TOC-Ozone-Platforms) like KMS/DRM, caca, etc

## Howto

We use Ubuntu 12.04 LTS (Precise Pangolin), 32/64-bit but a kernel from Raring though. One can install it pretty easily just
`sudo apt-get install linux-generic-lts-raring`. This version is needed for using Mesa master 9.3.0-devel, a requirement for Weston. Besides, make sure you have **at least** 25 GB of disk space to store all the sources and a few targets to be compiled (with debugging symbols).

Firstly you'd need to set up the Wayland libraries (version >= 1.4.0), and the Weston reference compositor that you will be running Chromium onto. The information on Wayland's web page should be enough for doing so:

http://wayland.freedesktop.org/building.html

_Please note that at the moment support for Wayland's XDG Shell protocol only works with the 1.5 series._

Make sure everything is alright now, setting up the environment variable `$XDG_RUNTIME_DIR` and playing a bit with the Wayland clients, connecting them on Weston.

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

now we can clone Ozone-Wayland and fetch all the dependencies of it, including Chromium itself:

  ```
  $ gclient config ssh://git@github.com/01org/ozone-wayland.git --name=src/ozone --git-deps
  $ GYP_DEFINES='use_ash=1 use_aura=1 chromeos=0 use_ozone=1' gclient sync
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
  $ ninja -C out/Debug -j16 chrome
  ```
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
