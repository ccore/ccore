# Generic makefile for static libraries

NAME=ccore
VERSIONMAYOR=1
VERSIONMINOR=0

SOURCEDIR:=src/$(NAME)
LIBDIR=lib
INCDIR=include
TESTDIR=test

RM=rm -f
DYNAR=$(CC) -shared -Wl,-soname,$(LIBFILE).$(VERSIONMAYOR) -o
#STATAR=ar rcs
CFLAGS:=-I$(INCDIR) -fPIC -O3 -DCC_USE_ALL
LDLIBS=-lGL -lGLU -lGLEW -lm

SRCS:=$(filter-out $(wildcard ./$(SOURCEDIR)/windows/*/*.c), $(wildcard ./$(SOURCEDIR)/*/*/*.c))
OBJS:=$(subst .c,.o,$(SRCS))
LIBNAME:=lib$(NAME).so
LIBFILE:=$(LIBDIR)/$(LIBNAME)
MAKEFILEDIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
ILIBDIR:=$(DESTDIR)/usr/lib
ILIBFILE:=$(ILIBDIR)/$(LIBNAME)
IINCDIR:=$(DESTDIR)/usr/include

all: $(LIBFILE).$(VERSIONMAYOR).$(VERSIONMINOR)

$(LIBFILE).$(VERSIONMAYOR).$(VERSIONMINOR): $(OBJS)
	@mkdir -p $(LIBDIR)
	$(DYNAR) $(LIBFILE).$(VERSIONMAYOR).$(VERSIONMINOR) $(OBJS)
	ln -sf $(abspath $(LIBFILE).$(VERSIONMAYOR).$(VERSIONMINOR)) $(LIBFILE).$(VERSIONMAYOR)
	ln -sf $(abspath $(LIBFILE).$(VERSIONMAYOR)) $(LIBFILE)

.PHONY: test
test: $(LIBFILE).$(VERSIONMAYOR).$(VERSIONMINOR)
	@(cd $(TESTDIR); $(MAKE) INCDIR="$(MAKEFILEDIR)$(INCDIR)" LIBDIR="$(MAKEFILEDIR)$(LIBDIR)" LIBNAME="$(NAME)")

.PHONY: clean
clean:
	$(RM) $(OBJS) $(LIBFILE).$(VERSIONMAYOR).$(VERSIONMINOR) $(LIBFILE).$(VERSIONMAYOR) $(LIBFILE)

.PHONY: install
install:
	mkdir -p $(IINCDIR)
	cp -R $(INCDIR)/* $(IINCDIR)
	mkdir -p $(ILIBDIR)
	cp $(LIBFILE).$(VERSIONMAYOR).$(VERSIONMINOR) $(ILIBDIR)
	ln -sf $(ILIBFILE).$(VERSIONMAYOR).$(VERSIONMINOR) $(ILIBFILE).$(VERSIONMAYOR)
	ln -sf $(ILIBFILE).$(VERSIONMAYOR) $(ILIBFILE)

.PHONY: dist-clean
dist-clean: clean
	$(RM) *~ .depend

.depend: $(SRCS)
	$(RM) ./.depend
	$(CC) $(CFLAGS) -MM $(SRCS) >>./.depend;

include .depend
