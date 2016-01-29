# Generic makefile for static libraries

NAME=ccore
VERSIONMAYOR=1
VERSIONMINOR=0

SOURCEDIR:=src/$(NAME)
LIBDIR=lib
INCDIR=include
TESTDIR=test

RM=rm -f
DYNAR=$(CC) -shared -Wl,-soname,$(DLIBNAME).$(VERSIONMAYOR).$(VERSIONMINOR) -o
STATAR=ar rcs
CFLAGS:=-I$(INCDIR) -fPIC -O3 -DCC_USE_ALL
LDLIBS=-lGL -lGLU -lGLEW -lm

SRCS:=$(filter-out $(wildcard ./$(SOURCEDIR)/windows/*/*.c), $(wildcard ./$(SOURCEDIR)/*/*/*.c))
OBJS:=$(subst .c,.o,$(SRCS))
DLIBNAME:=lib$(NAME).so
DLIBFILE:=$(LIBDIR)/$(DLIBNAME)
SLIBNAME:=lib$(NAME).a
SLIBFILE:=$(LIBDIR)/$(SLIBNAME)
MAKEFILEDIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
ILIBDIR:=$(DESTDIR)/usr/lib
IDLIBFILE:=$(ILIBDIR)/$(DLIBNAME)
IINCDIR:=$(DESTDIR)/usr/include

all: $(DLIBFILE).$(VERSIONMAYOR).$(VERSIONMINOR) $(SLIBFILE)

$(SLIBFILE): $(OBJS)
	@mkdir -p $(LIBDIR)
	$(STATAR) $(SLIBFILE) $(OBJS)

$(DLIBFILE).$(VERSIONMAYOR).$(VERSIONMINOR): $(OBJS)
	@mkdir -p $(LIBDIR)
	$(DYNAR) $(DLIBFILE).$(VERSIONMAYOR).$(VERSIONMINOR) $(OBJS)
	ln -sf $(abspath $(DLIBFILE).$(VERSIONMAYOR).$(VERSIONMINOR)) $(DLIBFILE).$(VERSIONMAYOR)
	ln -sf $(abspath $(DLIBFILE).$(VERSIONMAYOR)) $(DLIBFILE)

.PHONY: test
test: $(DLIBFILE).$(VERSIONMAYOR).$(VERSIONMINOR)
	@(cd $(TESTDIR); $(MAKE) INCDIR="$(MAKEFILEDIR)$(INCDIR)" LIBDIR="$(MAKEFILEDIR)$(LIBDIR)" LIBNAME="$(NAME)")

.PHONY: clean
clean:
	$(RM) $(OBJS) $(DLIBFILE).$(VERSIONMAYOR).$(VERSIONMINOR) $(DLIBFILE).$(VERSIONMAYOR) $(DLIBFILE)

.PHONY: install
install:
	mkdir -p $(IINCDIR)
	cp -R $(INCDIR)/* $(IINCDIR)
	mkdir -p $(ILIBDIR)
	cp $(SLIBFILE) $(ILIBDIR)
	cp $(DLIBFILE).$(VERSIONMAYOR).$(VERSIONMINOR) $(ILIBDIR)
	ln -sf $(DLIBNAME).$(VERSIONMAYOR).$(VERSIONMINOR) $(IDLIBFILE).$(VERSIONMAYOR)
	ln -sf $(DLIBNAME).$(VERSIONMAYOR) $(IDLIBFILE)

.PHONY: dist-clean
dist-clean: clean
	$(RM) *~ .depend

.depend: $(SRCS)
	$(RM) ./.depend
	$(CC) $(CFLAGS) -MM $(SRCS) >>./.depend;

include .depend
