NAME=test

SOURCEDIR=.

CC=gcc
RM=rm -f
CFLAGS=-O3 -Iinclude/ -g -Wall -DCC_USE_ALL -D_DEBUG
LDLIBS=-lGL -lGLU -lGLEW -lm -lccore -lX11 -lXrandr -lXinerama -lXi -lpthread

SRCS=$(shell find . -name '*.c')
OBJS=$(subst .c,.o,$(SRCS))

all: $(NAME)

.PHONY: $(NAME)
$(NAME): $(OBJS) .depend
	$(CC) $(LDFLAGS) -o $(NAME) $(OBJS) $(LDLIBS)

.PHONY: clean
clean:
	$(RM) $(OBJS)

.PHONY: dist-clean
dist-clean: clean
	$(RM) *~ .depend

.depend: $(SRCS)
	$(RM) ./.depend
	$(CC) $(CFLAGS) -MM $(SRCS) >>./.depend;

include .depend
