prefix      := /usr/local
exec_prefix := $(prefix)
libdir       = $(exec_prefix)/lib
includedir   = $(prefix)/include

CC      = gcc
CFLAGS  = -std=gnu99 -Wall -Wextra -O3 -march=native
INSTALL = install
MKDIR   = mkdir -p

.PHONY: test clean
libnetrc.so: libnetrc.c libnetrc.h
	$(CC) $(LDFLAGS) -shared $(CFLAGS) -fPIC -o $@ $<
install: libnetrc.so libnetrc.h
	$(MKDIR) $(libdir) && $(INSTALL) libnetrc.so $(libdir); \
	$(MKDIR) $(includedir) && $(INSTALL) -m644 libnetrc.h $(includedir)
netrc: netrc.c libnetrc.so libnetrc.h
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $< libnetrc.so
test: netrc
	@./test.sh
clean:
	$(RM) *.o libnetrc.so netrc
