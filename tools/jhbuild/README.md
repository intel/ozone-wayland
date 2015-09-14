These instructions will help you build Wayland and Weston on your Linux
operating system. They will work for most mainstream Linux distributions.

Note: it's recommended to use your operating system's Wayland and Weston
packages instead. Follow these instructions only if you need to build them
yourself, e.g. because your system's packages are too old, or if there is
some problem with your system's packages.

# Install JHBuild

Please refer to the
[JHBuild manual]
(https://developer.gnome.org/jhbuild/stable/getting-started.html.en#getting-started-install)
in order to install JHBuild into `~/.local/bin`.

# Examine require system dependencies

System dependencies must be provided by your operating system. The following
command lists system dependencies to be installed:

```
$ cd ~/chromium/src/tools/jhbuild
$ jhbuild -f wayland.jhbuildrc sysdeps
```

On most operating systems, PackageKit will speedily look up the packages to be
installed. On Debian and derivatives, a custom backend well be used instead,
which requires that you install `apt-file` and run it once. This is easy, just
slow; you will be guided through this process by the `jhbuild sysdeps` command
if you use Debian.

# Building without X11 support

You can avoid many dependencies by building without support for X11. If you do
this, you won't be able to run X11 applications under Weston, and you won't be
able to run Weston itself under X11. This option makes sense if you plan to use
a particular Weston backend other than the X11 backend, and know how to run
Weston in such an environment.

To build without X11 support, just replace `wayland.jhbuildrc` with
`wayland-no-x11.jhbuildrc` in all the instructions in this file. You will need
to create a `weston.ini` file and specify the backend that you wish to use.
See `weston.ini(5)` and `weston(1)` for details.

# Install system dependencies

`$ jhbuild -f wayland.jhbuildrc sysdeps --install`

On most systems, PackageKit will be used to install dependencies, and the
installation will not require root privileges. On Debian-based systems, a custom
backend will be used, and root is required.

# Build and install Wayland and Weston using JHBuild

`$ jhbuild -f wayland.jhbuildrc`

If you get a build error, it's usually due to a dependency missing from the
moduleset. This is usually a bug.
[Bug reports are appreciated.](https://github.com/01org/ozone-wayland/issues)

# Run Weston

```
$ jhbuild -f wayland.jhbuildrc shell
$ cd ~/chromium/src
$ out/wayland/root/bin/weston --fullscreen
```

# Run ozone-wayland in Weston

Open a weston terminal, then run:

`$ out/Release/chrome --no-sandbox`
