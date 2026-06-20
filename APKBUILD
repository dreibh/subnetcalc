# Maintainer: Thomas Dreibholz <simula.no>
pkgname=subnetcalc
pkgver=2.7.2
pkgrel=1
pkgdesc="IPv4/IPv6 Subnet Calculator"
url="https://www.nntb.no/~dreibh/subnetcalc/"
arch="all"
license="GPL-3.0-or-later"
depends=""
makedepends="cmake build-base gettext-dev libmaxminddb-dev"
subpackages="$pkgname-doc $pkgname-lang"
source="https://www.nntb.no/~dreibh/subnetcalc/download/subnetcalc-$pkgver.tar.xz"

build() {
	cmake -B build \
		-DCMAKE_INSTALL_PREFIX=/usr \
		-DCMAKE_BUILD_TYPE=None
	cmake --build build
}

package() {
	DESTDIR="$pkgdir" cmake --install build
}

sha512sums="
# You will need to generate the SHA512 sum using 'abuild checksum'
"
