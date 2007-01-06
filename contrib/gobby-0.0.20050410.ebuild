# Copyright 1999-2004 Gentoo Technologies, Inc.
# Distributed under the terms of the GNU General Public License v2
# $Header: /home/cvsroot/gentoo-x86/dev-util/darcs/darcs-0.9.17.ebuild,v 1.2 2004/03/18 08:27:31 kosmikus Exp $

DESCRIPTION="GTKmm-based obby client"
HOMEPAGE="http://darcs.0x539.de/gobby"
LICENSE="GPL-2"
SLOT="0"
KEYWORDS="~x86 ~ppc ~amd64"
IUSE=""
EDARCS_REPOSITORY="http://darcs.0x539.de/gobby"
EDARCS_GET_CMD="get --verbose"

DEPEND=">=dev-cpp/gtkmm-2.6
        >=dev-libs/libsigc++-2.0
	net-libs/libobby"

RDEPEND=""

inherit darcs

src_compile() {
	sh ./autogen.sh
	econf || die "./configure failed"
	emake || die "make failed"
}

src_install() {
	make DESTDIR=${D} install || die
}

pkg_postinst() {
	ewarn "ui.xml versagt immernoch :("
}
