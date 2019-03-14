-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: systemd
Binary: systemd, systemd-sysv, libpam-systemd, libsystemd0, libsystemd-dev, libsystemd-login0, libsystemd-login-dev, libsystemd-daemon0, libsystemd-daemon-dev, libsystemd-journal0, libsystemd-journal-dev, libsystemd-id128-0, libsystemd-id128-dev, udev, libudev1, libudev-dev, udev-udeb, libudev1-udeb, libgudev-1.0-0, gir1.2-gudev-1.0, libgudev-1.0-dev, python3-systemd, systemd-dbg
Architecture: linux-any
Version: 215-17+deb8u11
Maintainer: Debian systemd Maintainers <pkg-systemd-maintainers@lists.alioth.debian.org>
Uploaders: Michael Biebl <biebl@debian.org>, Marco d'Itri <md@linux.it>, Michael Stapelberg <stapelberg@debian.org>, Sjoerd Simons <sjoerd@debian.org>, Martin Pitt <mpitt@debian.org>
Homepage: http://www.freedesktop.org/wiki/Software/systemd
Standards-Version: 3.9.6
Vcs-Browser: http://anonscm.debian.org/gitweb/?p=pkg-systemd/systemd.git;a=summary
Vcs-Git: git://anonscm.debian.org/pkg-systemd/systemd.git
Testsuite: autopkgtest
Testsuite-Triggers: acl, build-essential, busybox-static, cron, lightdm, network-manager, pkg-config, python3
Build-Depends: debhelper (>= 9), pkg-config, xsltproc, docbook-xsl, docbook-xml, gtk-doc-tools, m4, dh-autoreconf, automake (>= 1.11), autoconf (>= 2.63), intltool, gperf, libcap-dev, libpam0g-dev, libaudit-dev, libdbus-1-dev (>= 1.3.2), libglib2.0-dev (>= 2.22.0), libcryptsetup-dev (>= 2:1.6.0), libselinux1-dev (>= 2.1.9), libacl1-dev, liblzma-dev, libgcrypt11-dev, libkmod-dev (>= 15), libblkid-dev (>= 2.20), libgirepository1.0-dev (>= 1.31.1), gobject-introspection (>= 1.31.1), python3-all-dev, python3-lxml, libglib2.0-doc
Package-List:
 gir1.2-gudev-1.0 deb introspection optional arch=linux-any
 libgudev-1.0-0 deb libs optional arch=linux-any
 libgudev-1.0-dev deb libdevel optional arch=linux-any
 libpam-systemd deb admin optional arch=linux-any
 libsystemd-daemon-dev deb oldlibs extra arch=linux-any
 libsystemd-daemon0 deb oldlibs extra arch=linux-any
 libsystemd-dev deb libdevel optional arch=linux-any
 libsystemd-id128-0 deb oldlibs extra arch=linux-any
 libsystemd-id128-dev deb oldlibs extra arch=linux-any
 libsystemd-journal-dev deb oldlibs extra arch=linux-any
 libsystemd-journal0 deb oldlibs extra arch=linux-any
 libsystemd-login-dev deb oldlibs extra arch=linux-any
 libsystemd-login0 deb oldlibs extra arch=linux-any
 libsystemd0 deb libs optional arch=linux-any
 libudev-dev deb libdevel optional arch=linux-any
 libudev1 deb libs important arch=linux-any
 libudev1-udeb udeb debian-installer optional arch=linux-any
 python3-systemd deb python optional arch=linux-any
 systemd deb admin optional arch=linux-any
 systemd-dbg deb debug extra arch=linux-any
 systemd-sysv deb admin extra arch=linux-any
 udev deb admin important arch=linux-any
 udev-udeb udeb debian-installer optional arch=linux-any
Checksums-Sha1:
 7a592f90c0c1ac05c43de45b8fde1f23b5268cb4 2888652 systemd_215.orig.tar.xz
 7715a60637a0ccfe0ed9938f2f58bf4961c68eb8 245604 systemd_215-17+deb8u11.debian.tar.xz
Checksums-Sha256:
 ce76a3c05e7d4adc806a3446a5510c0c9b76a33f19adc32754b69a0945124505 2888652 systemd_215.orig.tar.xz
 4e9d765876b1c90a6f1155e4c04e84c9b900990d4c2ef973a35a9cf2fc4f16fd 245604 systemd_215-17+deb8u11.debian.tar.xz
Files:
 d2603e9fffd8b18d242543e36f2e7d31 2888652 systemd_215.orig.tar.xz
 0f7fae74fcaa17f990569304be3f6bc1 245604 systemd_215-17+deb8u11.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQKjBAEBCgCNFiEErPPQiO8y7e9qGoNf2a0UuVE7UeQFAlyI55VfFIAAAAAALgAo
aXNzdWVyLWZwckBub3RhdGlvbnMub3BlbnBncC5maWZ0aGhvcnNlbWFuLm5ldEFD
RjNEMDg4RUYzMkVERUY2QTFBODM1RkQ5QUQxNEI5NTEzQjUxRTQPHGFwb0BkZWJp
YW4ub3JnAAoJENmtFLlRO1HkeEgQAKlcA2/OKhppAijr60y8Q2YPq1JMq9SGv27q
uPv8elGE0b6BuA8XdC6FMMo6lqb/heFmyKZbhyML6qt5MQU/D+cX3iQJXJquqzis
3UdbIkxvOYe9D2Uxd+qXYI5uvP3F4B5CEPzHMpRR597m03xgy7qyw2QH1GelNZNa
lrQL7k7hWIQGtb/gu89LZ0VxHDQxpImmj5DJQ3DlZ10u4bS1tYEh8nJjiAu1EQd2
mCbM7OGhWkjs44LY+vI2EpCDkBMvzcdS3u9v7OZFNyYIIEnh7J+uRI5YNq//CfU7
mxzCXDLQ4zIueN668flkW83gAyhZYkLQJh0WtsYWrd6aVRNJbBjZm0HToRAIQ9Rx
xN225yc/Wz1EYItW7+kVzDKWeQkP0briyTuuRLvSD8+PbY3mPcgf8cqySD7cf6HT
g9DtpDnoIgz2SopfBFu+5j6jwrgekvB1Sb2QEqEg+c2uZLtlDg69ETrOJRzVhf4D
lPwh+seQ2dyka9S4Vl2CaRsZPK6nEl1+3AMP2BWrEugDrbwQmi703auKxgK00GT/
DV5EP7Sv6hKZ3GOMHpeGB6TFRYPJrMBd29Sfy00302WZ2HsHXN4/NBRSA+mh2NHD
BRWCe921p2aSUvWdgPkD6swcpRAQ4Je+CZXlPW+wXMo02bmW4IaOSVSPwmWmMtSx
u7jLYnDC
=lTgV
-----END PGP SIGNATURE-----
