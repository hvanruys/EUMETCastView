TARGET = EUMETCastView
TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = bz2 \
	meteosatlib \
	QSgp4 \
        SOIL \
        core
core.depends = bz2
core.depends = meteosatlib
core.depends = QSgp4
core.depends = SOIL

DISTFILES += README.md


