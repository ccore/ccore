NAME=ccore

SOURCEDIR=src/$(NAME)
LIBDIR=lib
TESTDIR=test

CC=gcc
RM=rm -f
AR=ar rcs
CFLAGS=-O3 -Iinclude/
LDLIBS=-lGL -lGLU -lGLEW -lm

SRCS=$(filter-out $(wildcard ./$(SOURCEDIR)/windows/*/*.c), $(wildcard ./$(SOURCEDIR)/*/*/*.c))
OBJS=$(subst .c,.o,$(SRCS))

all: $(NAME)

.PHONY: test
test:
	cd $(TESTDIR) && make && ./test

.PHONY: $(NAME)
$(NAME): $(OBJS)
	$(AR) $(LIBDIR)/lib$(NAME).a

.PHONY: clean
clean:
	$(RM) $(OBJS)

.PHONY: install
install:
	mkdir -p $(DESTDIR)/usr/include
	cp -R include/* $(DESTDIR)/usr/include
	mkdir -p $(DESTDIR)/usr/lib
	cp -R lib/* $(DESTDIR)/usr/lib

.PHONY: dist-clean
dist-clean: clean
	$(RM) *~ .depend

.depend: $(SRCS)
	$(RM) ./.depend
	$(CC) $(CFLAGS) -MM $(SRCS) >>./.depend;

include .depend
