# Copyright (c) 2000-2004
#      The Regents of the University of California.  All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the University nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.

region_dir = ../libcompat

CC = gcc

CFLAGS = -I. -I$(region_dir) -Ddeletes= -Dtraditional= -Dsameregion= \
	 -Dparentptr= -Wall

HDRS = $(wildcard *.h)

OBJS = $(patsubst %.c,%.o,$(wildcard *.c))

NSOBJS = $(patsubst %.o,%.ns.o,$(OBJS))

TAGFILES = $(wildcard *.c) $(wildcard *.h)

TARGETS = libengine.a libnsengine.a TAGS

ifndef DEBUG
CFLAGS := -DNDEBUG -O3 ${CFLAGS}
else
CFLAGS := -pg -g ${CFLAGS}
endif

all: $(TARGETS) TAGS

libengine.a: $(OBJS)
	$(AR) rc $@ $^

libnsengine.a: $(NSOBJS) 
	$(AR) rc $@ $^

nonspec.o: nonspec.c nonspec.h
	$(CC) -c $(CFLAGS) -DNONSPEC $(CPPFLAGS) $< -o $@

%.ns.o: %.c $(HDRS)
	$(CC) -c $(CFLAGS) -DNONSPEC $(CPPFLAGS) $< -o $@

%.o : %.c $(HDRS)
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

.PHONY: clean

TAGS: $(TAGFILES)
	etags ${TAGFILES}
clean:
	-rm -f $(OBJS) $(NSOBJS) $(TARGETS) *~ 




