# Copyright (C) 2013 Mark Hills <mark@xwax.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2, as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License version 2 for more details.
# 
# You should have received a copy of the GNU General Public License
# version 2 along with this program; if not, write to the Free
# Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA 02110-1301, USA.
#

# Import the optional configuration

-include .config

# Libraries and dependencies

INSTALL ?= install

#SDL_CFLAGS ?= -I /usr/local/include/SDL2
SDL_LIBS ?= -L/usr/local/lib -lSDL2 -lSDL2_ttf

# Installation paths

PREFIX ?= $(HOME)

BINDIR ?= $(PREFIX)/bin
EXECDIR ?= $(PREFIX)/libexec
MANDIR ?= $(PREFIX)/share/man
DOCDIR ?= $(PREFIX)/share/doc

# Build flags

#CFLAGS ?= -O3
CFLAGS ?= -g
CFLAGS += -Wall -I /usr/include/SDL2 -I /usr/local/include/SDL2
CPPFLAGS += -MMD
LDFLAGS ?= -O3

# Core objects and libraries

OBJS = interface.o \
	osc.o \
	track.o \
	button.o \
	fader.o \
	overview.o \
	closeup.o \
	label.o \
	touchwax.o
DEVICE_CPPFLAGS =
DEVICE_LIBS =
DEPS = $(OBJS:.o=.d)

# Rules

.PHONY:		all
all:		touchwax

# Main binary

touchwax:		$(OBJS)
touchwax:		LDLIBS += $(SDL_LIBS) $(DEVICE_LIBS) -lm -llo 
touchwax:		LDFLAGS += -pthread

interface.o:	CFLAGS += $(SDL_CFLAGS)

touchwax.o:		CFLAGS += $(SDL_CFLAGS)
touchwax.o:		CPPFLAGS += $(DEVICE_CPPFLAGS)
touchwax.o:		CPPFLAGS += -DEXECDIR=\"$(EXECDIR)\"

# Install to system

.PHONY:		install
install:
		$(INSTALL) -D touchwax $(DESTDIR)$(BINDIR)/touchwax

# Distribution archive from Git source code

.PHONY:		dist
dist:		.version
		./mkdist $(VERSION)

# Editor tags files

TAGS:		$(OBJS:.o=.c)
		etags $^

.PHONY:		clean
clean:
		rm -f touchwax \
			$(OBJS) $(DEPS) \
			TAGS

-include $(DEPS)
