TARGET = EUMETCastView
TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = PublicDecompWT-2.8.1 \
        bz2 \
	meteosatlib \
	QSgp4 \
        core

core.depends = bz2
core.depends = meteosatlib
core.depends = QSgp4

DISTFILES += README.md \
               EUMETCastView.desktop \
                Globe_48x48.png \
                Globe_256x256.png


