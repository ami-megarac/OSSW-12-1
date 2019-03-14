-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 1.0
Source: openldap
Binary: slapd, slapd-smbk5pwd, ldap-utils, libldap-2.4-2, libldap-2.4-2-dbg, libldap2-dev, slapd-dbg
Architecture: any
Version: 2.4.40+dfsg-1+deb8u4
Maintainer: Debian OpenLDAP Maintainers <pkg-openldap-devel@lists.alioth.debian.org>
Uploaders: Roland Bauerschmidt <rb@debian.org>, Steve Langasek <vorlon@debian.org>, Torsten Landschoff <torsten@debian.org>, Matthijs Möhlmann <matthijs@cacholong.nl>, Timo Aaltonen <tjaalton@ubuntu.com>, Ryan Tandy <ryan@nardis.ca>
Homepage: http://www.openldap.org/
Standards-Version: 3.9.1
Vcs-Browser: http://anonscm.debian.org/gitweb/?p=pkg-openldap/openldap.git
Vcs-Git: git://anonscm.debian.org/pkg-openldap/openldap.git
Build-Depends: debhelper (>= 8.9.0~), dpkg-dev (>= 1.16.1), libdb5.3-dev, nettle-dev, libgnutls28-dev, unixodbc-dev, libncurses5-dev, libperl-dev (>= 5.8.0), libsasl2-dev, libslp-dev, libltdl-dev | libltdl3-dev (>= 1.4.3), libwrap0-dev, perl, po-debconf, quilt (>= 0.46-7), groff-base, time, heimdal-multidev, dh-autoreconf
Build-Conflicts: autoconf2.13, bind-dev, libbind-dev, libicu-dev
Package-List:
 ldap-utils deb net optional arch=any
 libldap-2.4-2 deb libs standard arch=any
 libldap-2.4-2-dbg deb debug extra arch=any
 libldap2-dev deb libdevel extra arch=any
 slapd deb net optional arch=any
 slapd-dbg deb debug extra arch=any
 slapd-smbk5pwd deb net extra arch=any
Checksums-Sha1:
 b80c48f2b7cbf634a3d463b7eb4ca38f081ce2eb 4797667 openldap_2.4.40+dfsg.orig.tar.gz
 18aaf8e3e31328426883b1e2652be6671aeb04ab 181350 openldap_2.4.40+dfsg-1+deb8u4.diff.gz
Checksums-Sha256:
 86c0326dc3dc5f1a9b3c25f7106b96f3eafcdf5da090b1fc586dec57d56e0e7f 4797667 openldap_2.4.40+dfsg.orig.tar.gz
 dffbdc8a502302724d3b2faf1c08a0383daf41cc690ffac47516813048b7d372 181350 openldap_2.4.40+dfsg-1+deb8u4.diff.gz
Files:
 8d84a916e2312aade2a3d7b2308a9a69 4797667 openldap_2.4.40+dfsg.orig.tar.gz
 1baa4caa8bdb3d02154037f1980cade7 181350 openldap_2.4.40+dfsg-1+deb8u4.diff.gz

-----BEGIN PGP SIGNATURE-----

iQJDBAEBCgAtFiEEPSfh0nqdQTd5kOFlIp/PEvXWa7YFAlsh4rgPHHJ5YW5AbmFy
ZGlzLmNhAAoJECKfzxL11mu2mA4P/A8LQ1EzR4LC4FeIoVmp20AejlJsluIIdJ6V
06qAxZi4Acsplcf5TsWG1IMY3cFEATiYRDQ4CreTkN5gqihE2hmy+01wsLflhqSe
5nSyi1MNNokTQ0g9Lw3lN/q/NGVEDLlRz5EgN1rC2xP00gUnzGZRicZcokhBT46v
OziJ5TxU/gOO8ZFPWtOe8xy8LCqZy/uxoiZEXfnWMB8GFYgHDyDf3nlfBqSsiCSO
n50hGkXMAAQq0d1Gnj5uJSt5gG17F8FWdfw3ctjD6+vLPS3mkvwCRa7IMI25TuvR
tqY2GbqXwNgetlooYmOEuTKNG0Z8jFcqWZ/KjnbGUex6UF26C2GYX50FOfeO73YN
Ezf81EYoGAA1UfJYm/I3aTxWO4+skr5MFFbFvA0eBKXmXO4e9MGy6lWJuQyhNvXC
ixaueRz+cT9LA0HZ2hTGO3g+xHs3i29iGnx9dfvTU5otDXQxE+/QKs1FXRmZt8kU
ZYYs2oHTwxzs44LeN70QFnSmIboKK0FGY+njGepmk1j8ObudYby+ROr8J1Binkr7
NMr9yrHxMVFsmkXoBgQ+o1oX/d+4+sYsfjd+LHgC5NIuHuy2X0ldDLHh4dIqLl4p
uZ69tLY3d3NBu75aToteHFNBmGFqCbYCW7XhgKKZFtWzoLExHqmAhPVkbSuk/KlI
F99xSc6p
=7BWx
-----END PGP SIGNATURE-----
