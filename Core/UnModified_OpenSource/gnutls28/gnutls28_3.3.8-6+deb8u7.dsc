-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: gnutls28
Binary: libgnutls28-dev, libgnutls-deb0-28, libgnutls28-dbg, gnutls-bin, gnutls-doc, guile-gnutls, libgnutlsxx28, libgnutls-openssl27
Architecture: any all
Version: 3.3.8-6+deb8u7
Maintainer: Debian GnuTLS Maintainers <pkg-gnutls-maint@lists.alioth.debian.org>
Uploaders: Andreas Metzler <ametzler@debian.org>, Eric Dorland <eric@debian.org>, James Westby <jw+debian@jameswestby.net>, Simon Josefsson <simon@josefsson.org>
Homepage: http://www.gnutls.org/
Standards-Version: 3.9.5
Vcs-Browser: http://anonscm.debian.org/gitweb/?p=pkg-gnutls/gnutls.git
Vcs-Git: git://anonscm.debian.org/pkg-gnutls/gnutls.git
Build-Depends: debhelper (>= 9), nettle-dev (>= 2.7), zlib1g-dev, libtasn1-6-dev (>= 3.9), autotools-dev, guile-2.0-dev [!ia64 !m68k], datefudge, libp11-kit-dev (>= 0.20.7), pkg-config, chrpath, libidn11-dev, autogen (>= 1:5.16-0), bison, dh-autoreconf, libgmp-dev (>= 2:6), libopts25-dev
Build-Depends-Indep: gtk-doc-tools, texinfo (>= 4.8)
Build-Conflicts: libgnutls-dev, libp11-kit-dev (= 0.21.2-1)
Package-List:
 gnutls-bin deb net optional arch=any
 gnutls-doc deb doc optional arch=all
 guile-gnutls deb lisp optional arch=amd64,arm64,armel,armhf,i386,kfreebsd-amd64,kfreebsd-i386,mips,mipsel,powerpc,ppc64el,s390,s390x,sparc,hurd-i386
 libgnutls-deb0-28 deb libs standard arch=any
 libgnutls-openssl27 deb libs standard arch=any
 libgnutls28-dbg deb debug extra arch=any
 libgnutls28-dev deb libdevel optional arch=any
 libgnutlsxx28 deb libs extra arch=any
Checksums-Sha1:
 2c07ed3f0ec3284820985085d63311e8b73cb48f 6153180 gnutls28_3.3.8.orig.tar.xz
 0a73d37a3cc86b493c15da214f416ff1d1ea6c0a 106372 gnutls28_3.3.8-6+deb8u7.debian.tar.xz
Checksums-Sha256:
 bd4642f180e19632f4ed3a1e62d60c824c7b695f5cddf41a8fba1b272eaef046 6153180 gnutls28_3.3.8.orig.tar.xz
 f5bfa892ddf437ce3c8de5f55315689d03d3e484375c47599051b8f2bb4e0c07 106372 gnutls28_3.3.8-6+deb8u7.debian.tar.xz
Files:
 b57e6b7630bdba9ea8eb28ff0eb29c2f 6153180 gnutls28_3.3.8.orig.tar.xz
 ac0f6649d16fd5397fc46366427c2431 106372 gnutls28_3.3.8-6+deb8u7.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQIzBAEBCgAdFiEE0uCSA5741Jbt9PpepU8BhUOCFIQFAllT5QwACgkQpU8BhUOC
FIS8Rw//XM4gnXCf1q2VrbY9HX24IvIufBmZWjaY7slIsxCagSPAfXAN2+BP8EfR
nXgrvfxCGvaVEe24CidgiBxI+AMKeerwkmNNUVofOCvEJd1NsyLq7uNnataditUj
Vae2DsiW2z4z9sLDF+f/3f4HhBchkjBdeskSKWCEFMkgF4R/L9kMUrz8MmgAFjI2
iU/YhY91nfoT7DyKaeCd2NrfCfEh+L6Ye++eM0gjtuqHiO+JdldpANhNk2WQjxzH
cIdjqQ6MQ+lQIAHBcQRHPN1kvBDi8QEYvz8m/2FJbuv1UfQnJy5Hk0dFaasY08cD
901MJ1ZI0qrKoZHo788SO4FznYSa8VR5ImCDI6OU72pbEF0djvYD1kK7FLLcmDDk
QW5m0+3zf0bz3cVzVJTuIBaRzKDdj2OoLRYOJSEWJIKb2Arj1qc2ORWBOuSVV8bN
vekY+xVWiFlxw0ooLA9IjJugxBqqo8stXySevxgMXzq8iYKR/BcNDxeE7pVJGIKK
WImyUGQV9toqSP7N9X+W+agok4o/zPGxsId8J5CbTXbIFH3AfA0k7l344PLy+TpU
08Zj7uTWPwRTppeabw8+CjwHCNBuvZ2VlK7XdPkH8V1YXwbWUzCQip6ImLOeiD1B
tza2zc3mBwe070LStTfwV1p0ASQgvBeZiCsyvId+BoFelsFo3ek=
=Pa3M
-----END PGP SIGNATURE-----
