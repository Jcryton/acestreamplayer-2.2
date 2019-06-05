Ace Stream Player
-----------------

Tested with VLC version *2.2.4*

Depends: acestreamengine,
Optimal: acestreamengine-3.1.35

**Compile for Linux:** 

Install build tools *libtool*, *build-essential*, *pkg-config*, *autoconf*

Checkout all required [libraries] to build vlc

Also install *libqt4-webkit*, *openssl*, *sqlite3*

Edit config.sh, set VLC_VERSION="2.2.4" to build this release,

`./bootstrap.sh`

`./configure.sh`

`./make.sh`

`cd build-ace`

`make install`

[libraries]:https://wiki.videolan.org/Contrib_Status/
