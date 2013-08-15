Ozone-Wayland
============

Ozone-Wayland is the implementation of Chromium's Ozone for supporting Wayland graphics system. Different projects based on Chromium/Blink like the Chrome browser, ChromeOS, among others can be enabled now using Wayland.

![Alt text](https://raw.github.com/tiagovignatti/misc/master/chromium-2013-06-07-small.png "Content Shell running on Weston")

Contents
-------
  - [Design](#design)
  - [Howto](#howto)
  - [.gclient file](#gclient-file)


Design
------

Ozone is a set of classes in Chromium for abstracting different window systems on Linux. It provides abstraction for the construction of accelerated surfaces underlying Aura UI framework, input devices assignment and event handling. 

http://www.chromium.org/developers/design-documents/ozone

Before when using Aura on Linux, all the native windowing system code (X11) was spread throughout Chromium tree. Now the idea is that Ozone will abstract the native code and, because it's a set of class factories, it will switch for whoever is the window system. The biggest advantage of this API is that it allows to implement the needed window system bits externally from the Chromium tree, which is great because it is where the loaded work situates.

Worth to mention also that when Aura is used, there's no need for graphics toolkits, such as GTK+, EFL etc.

Howto
-----

My system is a Core i7 running Ubuntu 12.04 LTS (Precise Pangolin), 64-bit. I'm using a kernel from Raring though, which you can install pretty easily just `sudo apt-get install linux-generic-lts-raring` (I need this version cause I'm using Mesa master, 9.2.0-devel, for Weston).

Firstly you'd need to set up the Wayland libraries, and the Weston reference compositor that you will be running Chromium onto. The information on Wayland's web page should be enough for doing so:

http://wayland.freedesktop.org/building.html

Make sure everything is alright now, setting up the environment variable `$XDG_RUNTIME_DIR` and playing a bit with the Wayland clients, connecting them on Weston.

Then on Chromium's side, we need to setup Chromium's tree together with the Ozone-Wayland implementation "aside" of it. For that you have to use an special `.gclient` configuration that clones both of the trees (see next section). Say your depot_tools live in `~/git/chromium/depot_tools` and your chromium toplevel directory is in `~/git/chromium/src`, you will need to jump to `~/git/chromium` and run:

  ```
  $ gclient sync
  ```

It may take a considerable time for downloading the trees, but once that is done, the `.gclient` will automatically switch the chromium repository to "master-ozone" branch and apply a few patches on top of it (not everything is upstream yet). If everything went fine, now we're able to generate the needed building files:

  ```
  $ cd src/
  $ export GYP_DEFINES='component=static_library use_ash=0 use_aura=1 chromeos=0 use_ozone=1'
  $ ./build/gyp_chromium
  ```

Note that in Chromium, gyp uses pkg-config for checking where are Wayland libraries on the system, so double check that you are not mixing some that was already there with the latest that you just got and compiled from git. I have myself set a default `PKG_CONFIG_PATH` for the `$HOME/install` path and put it in .bashrc.

Now we can conclude compiling the aura_demo and Content Shell targets, which are the main targets I'm using for testing at the moment -- Content Shell takes ~20 minutes on my machine:

  ```
  $ ninja -C out/Debug -j16 aura_demo
  $ ninja -C out/Debug -j16 content_shell
  ```

That's all. At this point you should be able to connect content_shell on Weston using:

  ```
  $ ~/git/weston/src/weston &
  $ ./out/Debug/content_shell --no-sandbox
  ```

.gclient file
-------------
```python
solutions = [
  {
    u'managed': True,
    "name": "src",
    u'url': u'https://chromium.googlesource.com/chromium/src.git',
    u'custom_deps': {},
    u'deps_file': u'.DEPS.git',
    u'safesync_url': u'',
    u'custom_vars': {u'webkit_rev': u''},

    # this suppresses the execution of the gyp action from src/DEPS by setting
    # the action to nothing.
    # TODO: rjkroege recommended but in fact I'm not sure this is needed.
    "custom_hooks": [ {"name": "gyp"} ]
  },
  {
    "name"  : "src-ozone",
    "url"   : "https://github.com/otcshare/ozone-wayland.git",
    "deps_file": "gclient/DEPS"
  }
]

hooks = [
  {
    # this will apply the needed patches in Chromium tree. It pretty much
    # generates the Wayland dependencies on .gyp files there and also patch
    # other small changes. Ideally this shouldn't exist and Ozone-Wayland would
    # be just dynamically loaded within Ozone there.
    "name": "gyp",
    "pattern": ".",
    "action": [
        "bash", "src/ozone/patches/patch-chromium.sh"
    ]
  }
]
```
