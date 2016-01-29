# Generic makefile for static libraries

NAME=ccore
VERSIONMAYOR=1

SOURCEDIR:=src/$(NAME)
LIBDIR=lib
INCDIR=include
TESTDIR=test

RM=rm -f
CFLAGS:=-I$(INCDIR) -g -fPIC -O3 -DCC_USE_ALL
LDLIBS=-lGL -lGLU -lGLEW -lm

SRCS:=$(filter-out $(wildcard ./$(SOURCEDIR)/windows/*/*.c), $(wildcard ./$(SOURCEDIR)/*/*/*.c))
OBJS:=$(subst .c,.o,$(SRCS))
MAKEFILEDIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))

DLIBNAME:=lib$(NAME).so
DLIBFILE:=$(LIBDIR)/$(DLIBNAME)
SLIBNAME:=lib$(NAME).a
SLIBFILE:=$(LIBDIR)/$(SLIBNAME)
ILIBDIR:=$(DESTDIR)/usr/lib
IDLIBFILE:=$(ILIBDIR)/$(DLIBNAME)
IINCDIR:=$(DESTDIR)/usr/include

DYNAR:=$(CC) $(LDFLAGS) -g -shared -Wl,-soname,$(DLIBNAME).$(VERSIONMAYOR) -o
STATAR:=ar rcs

all: $(DLIBFILE).$(VERSIONMAYOR) $(SLIBFILE)

$(SLIBFILE): $(OBJS)
	@mkdir -p $(LIBDIR)
	$(STATAR) $(SLIBFILE) $(OBJS)

$(DLIBFILE).$(VERSIONMAYOR): $(OBJS)
	@mkdir -p $(LIBDIR)
	$(DYNAR) $(DLIBFILE).$(VERSIONMAYOR) $(OBJS)
	ln -sf $(abspath $(DLIBFILE).$(VERSIONMAYOR))

.PHONY: test
test: $(DLIBFILE).$(VERSIONMAYOR)
	@(cd $(TESTDIR); $(MAKE) INCDIR="$(MAKEFILEDIR)$(INCDIR)" LIBDIR="$(MAKEFILEDIR)$(LIBDIR)" LIBNAME="$(NAME)")

.PHONY: clean
clean:
	$(RM) $(OBJS) $(DLIBFILE).$(VERSIONMAYOR) $(DLIBFILE)

.PHONY: install
install:
	mkdir -p $(IINCDIR)
	cp -R $(INCDIR)/* $(IINCDIR)
	mkdir -p $(ILIBDIR)
	cp $(SLIBFILE) $(ILIBDIR)
	cp $(DLIBFILE).$(VERSIONMAYOR) $(ILIBDIR)
	ln -sf $(DLIBNAME).$(VERSIONMAYOR) $(IDLIBFILE)

.PHONY: dist-clean
dist-clean: clean
	$(RM) *~ .depend

.depend: $(SRCS)
	$(RM) ./.depend
	$(CC) $(CFLAGS) -MM $(SRCS) >>./.depend;

include .depend
