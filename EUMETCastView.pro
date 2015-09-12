TARGET = EUMETCastView
TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = bz2 \
	meteosatlib \
	QSgp4 \
        core

core.depends = bz2
core.depends = meteosatlib
core.depends = QSgp4

DISTFILES += README.md


