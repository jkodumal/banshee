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

IBANSHEE_DIR = ../bin

.PHONY: ibanshee-check
ibanshee-check: ibanshee-regr

IBC_DIR := ./test.ibc
COR_DIR := ./test.ibc.cor

IBANSHEE_TESTS := 
IBANSHEE_TESTS += cons_def.ibc
IBANSHEE_TESTS += cons_def2.ibc
IBANSHEE_TESTS += row_norm.ibc
IBANSHEE_TESTS += row.ibc
IBANSHEE_TESTS += trans.ibc
IBANSHEE_TESTS += trans2.ibc


IBANSHEE_EXEC := $(IBANSHEE_DIR)/ibanshee.exe

.PHONY: ibanshee-regr
ibanshee-regr:  $(IBANSHEE_EXEC) ibanshee-regr/clean \
           $(patsubst %,ibanshee-tests/%,$(IBANSHEE_TESTS)) ibanshee-done

ibanshee-regr/clean:
	rm -f $(IBC_DIR)/*.out

.PHONY: ibanshee-done

ibanshee-done:; @echo "iBanshee tests pass"

ibanshee-tests/%:
	$(IBANSHEE_EXEC) -f $(IBC_DIR)/$* > $(IBC_DIR)/$*.out
	diff $(COR_DIR)/$*.cor $(IBC_DIR)/$*.out

$(IBANSHEE_DIR)/ibanshee.exe: 
	$(MAKE) -C ../ ibanshee

clean: ibanshee-regr/clean
