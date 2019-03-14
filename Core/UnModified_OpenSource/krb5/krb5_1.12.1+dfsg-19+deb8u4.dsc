-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA256

Format: 3.0 (quilt)
Source: krb5
Binary: krb5-user, krb5-kdc, krb5-kdc-ldap, krb5-admin-server, krb5-multidev, libkrb5-dev, libkrb5-dbg, krb5-pkinit, krb5-otp, krb5-doc, libkrb5-3, libgssapi-krb5-2, libgssrpc4, libkadm5srv-mit9, libkadm5clnt-mit9, libk5crypto3, libkdb5-7, libkrb5support0, libkrad0, krb5-gss-samples, krb5-locales, libkrad-dev
Architecture: any all
Version: 1.12.1+dfsg-19+deb8u4
Maintainer: Sam Hartman <hartmans@debian.org>
Uploaders: Russ Allbery <rra@debian.org>, Benjamin Kaduk <kaduk@mit.edu>
Homepage: http://web.mit.edu/kerberos/
Standards-Version: 3.9.5
Vcs-Browser: http://git.debian.org/?p=pkg-k5-afs/debian-krb5-2013.git
Vcs-Git: git://git.debian.org/git/pkg-k5-afs/debian-krb5-2013.git
Build-Depends: debhelper (>= 8.1.3), byacc | bison, comerr-dev, docbook-to-man, doxygen, libkeyutils-dev [linux-any], libldap2-dev, libncurses5-dev, libssl-dev, ss-dev, libverto-dev (>= 0.2.4), pkg-config, dh-systemd
Build-Depends-Indep: python-cheetah, python-lxml, python-sphinx, doxygen-latex
Package-List:
 krb5-admin-server deb net optional arch=any
 krb5-doc deb doc optional arch=all
 krb5-gss-samples deb net extra arch=any
 krb5-kdc deb net optional arch=any
 krb5-kdc-ldap deb net extra arch=any
 krb5-locales deb localization standard arch=all
 krb5-multidev deb libdevel optional arch=any
 krb5-otp deb net extra arch=any
 krb5-pkinit deb net extra arch=any
 krb5-user deb net optional arch=any
 libgssapi-krb5-2 deb libs standard arch=any
 libgssrpc4 deb libs standard arch=any
 libk5crypto3 deb libs standard arch=any
 libkadm5clnt-mit9 deb libs standard arch=any
 libkadm5srv-mit9 deb libs standard arch=any
 libkdb5-7 deb libs standard arch=any
 libkrad-dev deb libdevel extra arch=any
 libkrad0 deb libs standard arch=any
 libkrb5-3 deb libs standard arch=any
 libkrb5-dbg deb debug extra arch=any
 libkrb5-dev deb libdevel extra arch=any
 libkrb5support0 deb libs standard arch=any
Checksums-Sha1:
 d211e7d605bd992d33b7cbca1da14d68f0770258 11792370 krb5_1.12.1+dfsg.orig.tar.gz
 192eaf02d62d68d40cf344f501e42b832df35948 125856 krb5_1.12.1+dfsg-19+deb8u4.debian.tar.xz
Checksums-Sha256:
 eb29959f1e9f8d71e7401f5809daefae067296eb5b0da1176366280a16bdd784 11792370 krb5_1.12.1+dfsg.orig.tar.gz
 9a2b93059db021f025eb1e65c9c4974a863cbe872543c6a91a8a11eb568e0a46 125856 krb5_1.12.1+dfsg-19+deb8u4.debian.tar.xz
Files:
 dd0367010b3d2385d9f23db25457a0bf 11792370 krb5_1.12.1+dfsg.orig.tar.gz
 e3cecf6c5e551acc7057924b117c4021 125856 krb5_1.12.1+dfsg-19+deb8u4.debian.tar.xz
Dgit: bc3c719f87aaef0d8cbaae38ac740328c6466674 debian archive/debian/1.12.1+dfsg-19+deb8u4 https://git.dgit.debian.org/krb5

-----BEGIN PGP SIGNATURE-----

iQGfBAEBCAAdFiEEz1cSziAwmFRQyTi4fJpR9iayVp8FAlmkQMYACgkQfJpR9iay
Vp8d4AtgpLVJgQ8Oq27EeN7hPKTb9gB6smm3entKCvwxOwg0BIlM4NwPDZWLzOHy
ytGcUKLVNxB3GHl6NNvimJ9NVu/MPzD0fO/Ih0feKldnpAk8qgdLT9nr5PgSliVN
gGlM2wyAy8oHrsJzuf0x8xt0DxxVXIpZPvTQCpswGHavPEOieaaBok2CL43RRyF3
xNMeOR/TcfBJVBKXoP5K9UilGAiAWD9zIvPE6VA5FULGEX4X/BhPWKZzrD7XD5e+
cYWafIY/2uTBFRiGgteSi0HB81pqM+IsUOYpi946s0r58JCYRsEvzgeDtf+INiDI
As37UPuBaQ4YxJyR2fKY8fLEwdlE4u8xpwKNNyDR1FZPwznBBJBqFNjSdYaMUMEi
llRGISOIjRnscTJdjA+QtLaqRpwJDNpSMfhukLgRMQ+JyJia3METYBCg+VeQ/HN0
5Z2RjyGAGYnpkigm4XLncXkpYkJP9O+knuSA+p12UQv09Q==
=2GMZ
-----END PGP SIGNATURE-----
