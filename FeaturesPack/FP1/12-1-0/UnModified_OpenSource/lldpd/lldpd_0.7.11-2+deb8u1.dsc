-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA256

Format: 3.0 (quilt)
Source: lldpd
Binary: lldpd, liblldpctl-dev
Architecture: linux-any kfreebsd-any
Version: 0.7.11-2+deb8u1
Maintainer: Vincent Bernat <bernat@debian.org>
Homepage: http://vincentbernat.github.com/lldpd/
Standards-Version: 3.9.6
Vcs-Browser: http://anonscm.debian.org/gitweb/?p=collab-maint/lldpd.git
Vcs-Git: git://anonscm.debian.org/collab-maint/lldpd.git
Build-Depends: debhelper (>= 5), cdbs (>= 0.4.122), autotools-dev, dh-autoreconf, libsnmp-dev, libpci-dev, libxml2-dev, libevent-dev, libreadline-dev, libbsd-dev, pkg-config, dh-systemd (>= 1.5)
Package-List:
 liblldpctl-dev deb libdevel optional arch=linux-any,kfreebsd-any
 lldpd deb net optional arch=linux-any,kfreebsd-any
Checksums-Sha1:
 7e3245b51f1894b3cc1d74187674fced719f080b 1509215 lldpd_0.7.11.orig.tar.gz
 0e828cef24998d9923ee14bf91e7c54a6a0a8616 10884 lldpd_0.7.11-2+deb8u1.debian.tar.xz
Checksums-Sha256:
 5257169e0de6037e81efb1bcb26f6dd5755e3efa0a025144d6763bdfaf982e6b 1509215 lldpd_0.7.11.orig.tar.gz
 fa2098f64460f158b17ac16ebceb511fc8f301fbf3214f7ccf4c0c769e560ce9 10884 lldpd_0.7.11-2+deb8u1.debian.tar.xz
Files:
 ec71873094adf2e9ae1fcae134ebb5c0 1509215 lldpd_0.7.11.orig.tar.gz
 7dda72cf2517a451dad96c4c0e06c776 10884 lldpd_0.7.11-2+deb8u1.debian.tar.xz

-----BEGIN PGP SIGNATURE-----
Version: GnuPG v1

iQIcBAEBCAAGBQJWLNdMAAoJEJWkL+g1NSX5jPUQAJuJMZ04GiF0ouB1V6jSVqg1
al13cmECiiHCu6MCXq3uHnYJRZXUwmscjyrbelb/K8FYGq+vohuJXGvpv/fwvFjT
SIsP1ntSf6Nqjo9MOZPqb8BIYmPa7ZFsAzxE0jNIc7plp3T7ko2lrxf29dpCw27k
6UPawNUXDpDApyRlhOQJwe5TNNeUJqRbWqM4xVB8oCpEK2EQb1QhkfLZnKcEBsYJ
kuMrcHjqV/dRYr/EQDwQ9WVV5qdVBPhe+/pfPsl5pGVti+GZLnYSbJ3sUuoWrks4
cmPgZivCYoL/s2VBdn8Wa97IVi7qVD8w1ymq2hT/RPiUwclwrlnA5PHuyjcderx5
WAxFZhxx7Uc8DnlOOkrop1RQOlmSMhHBo/MBuFtXmuS1TwHClc9tbc+hOSW7SIra
bQ2feKz3QTZLdJT+vD6F2i/KhXQU9hj0g1wM2+COYbtZNuYLK8N3EUXCT0qwvAqn
jmUH1mMG1mxSXlZxhPX8ImvMFET7UzvgLjeNv9Iuew8VDRH7I34+TJc6wUsfXx1R
ju8uKogJZ8pEjMLHwt1D26n4crvH22dB95y3mpbrehy2SO9b5xKSB5UNw3jYmR0C
tbke10xkBhtYYqGG602n0svUCBRUNbp56ACC7yJQL3wvm0DExbGkWRqlD/EUHbYS
UmIMOTyQJkPZQgBCm+r1
=K9Sc
-----END PGP SIGNATURE-----
