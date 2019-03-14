-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA256

Format: 3.0 (quilt)
Source: perl
Binary: perl-base, perl-doc, perl-debug, libperl5.20, libperl-dev, perl-modules, perl
Architecture: any all
Version: 5.20.2-3+deb8u11
Maintainer: Niko Tyni <ntyni@debian.org>
Uploaders: Dominic Hargreaves <dom@earth.li>
Homepage: http://dev.perl.org/perl5/
Standards-Version: 3.9.6
Vcs-Browser: http://anonscm.debian.org/gitweb/?p=perl/perl.git
Vcs-Git: git://anonscm.debian.org/perl/perl.git -b debian-5.20
Build-Depends: file, cpio, libdb-dev, libgdbm-dev, netbase [!hurd-any], procps [!hurd-any], zlib1g-dev | libz-dev, libbz2-dev, dpkg-dev (>= 1.16.0), libc6-dev (>= 2.19-9) [s390x]
Package-List:
 libperl-dev deb libdevel optional arch=any
 libperl5.20 deb libs optional arch=any
 perl deb perl standard arch=any
 perl-base deb perl required arch=any essential=yes
 perl-debug deb debug extra arch=any
 perl-doc deb doc optional arch=all
 perl-modules deb perl standard arch=all
Checksums-Sha1:
 63126c683b4c79c35008a47d56f7beae876c569f 13717128 perl_5.20.2.orig.tar.bz2
 4348cadb494865efac6dcd7389cccb6d5f4d33e8 157516 perl_5.20.2-3+deb8u11.debian.tar.xz
Checksums-Sha256:
 e5a4713bc65e1da98ebd833dce425c000768bfe84d17ec5183ec5ca249db71ab 13717128 perl_5.20.2.orig.tar.bz2
 53e0ccd3ed238614fbcd8eb577159392892bcf82c7821f94f6ef379e8ae3a7c1 157516 perl_5.20.2-3+deb8u11.debian.tar.xz
Files:
 21062666f1c627aeb6dbff3c6952738b 13717128 perl_5.20.2.orig.tar.bz2
 7340e4dcd6e352c3ec4060f88c3671fe 157516 perl_5.20.2-3+deb8u11.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQJBBAEBCAArFiEEy0llJ/kAnyscGnbawAV+cU1pT7IFAlsdctgNHGRvbUBlYXJ0
aC5saQAKCRDABX5xTWlPsjJyD/9Qtbr7F/sRArFxM/+wGaAvn/LO8IxhSOrYr5rl
RF0SuKf545/3trpn81+14rQk0IXSpYwzglVasAPljFARK4Spa9Ol+WbgvNcnaZOV
1ceu7bZLtVluecbZS286ZUodzMoRZIaMFZ9jw8W5s4Hppb2/biQ/ly02coSJOFQV
w2nhtrRi41YjK8MiEEZEaTSl6GgGvCEYik2JyclEe3dG4dBquvI+L6pWixUvtIK7
vdhv3ifRGcepgEx4T72IF+OvURvkmz/EBWOee7isJl/GXAMsFYZ3sFqnN4G1V7Ga
DBnoVjrVHSRHLUbUoGZvSc91vBJ9vM2pK1reYMsd/4WDEvbPhTLk4IwvsTrpksp6
+0Idu+UDxXMoOdq//y8kWzvxxsNvC8rfMFAYwz6Dz5PMqKRHoE0Jm3J9P206Ck0/
xe3egP0z8cq358eLnQlZCjN+3HNU9UV598JkKG9tuJ1LP+imBZ1Ub+7rA/Se7Ili
Qw7c5RBgPmmUGAnQTJMJmng81FX/KFerRSefeMHUvUldXzQi4ab1wzPdDCQxK/sG
iF2RO1vbxFTNd7yQz9MTFIGIFATXJsOXoEvTktrCuwU/kPsXIqI2h8GsUbLFuSbh
6bHR8xOQ/paCS9VfLNGnO0N3akdi5T1n2yDqcvefQepfuiWMlGqGcSlv6uW25vWO
UdyCvg==
=mBt7
-----END PGP SIGNATURE-----
