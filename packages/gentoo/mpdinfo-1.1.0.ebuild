# Copyright 1999-2017 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Id$

EAPI=6

DESCRIPTION="Shows mpd informations for use on status displays"
HOMEPAGE="https://github.com/jduepmeier/mpdinfo"
SRC_URI="https://mpease.de/projects/${PN}/${P}.tar.gz"

LICENSE="WTFPL-2"
SLOT="0"
KEYWORDS="~amd64 ~x86"
IUSE=""

DEPEND=">=media-libs/libmpdclient-2.10"
RDEPEND="${DEPEND}"

src_install() {
	emake DESTDIR="${D}" INSTALL_DIR="/usr/bin" install
	dodoc README.md sample.conf
}
