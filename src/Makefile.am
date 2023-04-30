moduledir = $(IRSSI_MODULEDIR)
wrappersdir = $(datadir)/irssi/scripts
module_LTLIBRARIES = libpython.la

libpython_la_DEPENDENCIES = objects/libobjects.la
libpython_la_LIBADD = $(PYTHON_LIBS) objects/libobjects.la
libpython_la_LDFLAGS = -avoid-version

AM_CPPFLAGS = $(IRSSI_PYTHON_INCLUDES) \
		   -I$(top_srcdir)/src/objects \
		   -DSCRIPTDIR=\""$(datadir)/irssi/scripts"\"

libpython_la_SOURCES = \
	pycore.c \
	pyutils.c \
	pymodule.c \
	pyloader.c \
	pysignals.c\
	pysource.c \
	pythemes.c \
	pystatusbar.c \
	$(BUILT_SRC)

BUILT_SRC = \
	pyconstants.c

noinst_HEADERS = \
	pyconstants.h \
	pycore.h \
	pyirssi.h \
	pyirssi_irc.h \
	pyloader.h \
	pymodule.h \
	pysignals.h \
	pysource.h \
	pystatusbar.h \
	pythemes.h \
	pyutils.h \
	$(BUILT_HDR)

BUILT_HDR = \
	pysigmap.h

wrappers_DATA = irssi.py irssi_startup.py
EXTRA_DIST = $(wrappers_DATA)

SUBDIRS = objects

CLEANFILES = \
	$(BUILT_SRC) \
	$(BUILT_HDR)

pysigmap.h:
	awk -f sig2code.awk $(IRSSI_SIGNALSFILE) > $@

pyconstants.c:
	awk -f constants.awk constants.txt > $@
