INCPATH = .
LIBPATH =
LIBS =

ifdef prefix 
	DESTDIR = ${prefix}
endif

INSTDIR_BIN = ${DESTDIR}/usr/bin
INSTDIR_MAN = ${DESTDIR}/usr/share/man/man1
INSTDIR_DOC = ${DESTDIR}/usr/share/doc/packages/plcfw

CFLAGS = -g 

SOURCES = plcfwlib.c plcfwop.c plcfwoplist.c plcfwconn.c plcfw.c

OBJS = $(SOURCES:%.c=%.o)

BINARY = plcfw

MANPAGESRC = plcfw.man
MANPAGE = plcfw.1

DOCFILES = AUTHORS LICENCE README

all: $(BINARY) notes

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@ -I$(INCPATH) 

$(BINARY): $(OBJS)
	gcc -o $@ $(OBJS)

clean:
	rm -f $(BINARY) $(OBJS) $(MANPAGE).gz *~ INSTALL

install:
	@echo "Installing binary into $(INSTDIR_BIN)"
	@cp -f $(BINARY) $(INSTDIR_BIN)/.
	@echo "Installing manpage into $(INSTDIR_MAN)"
	@rm -f $(MANPAGE).gz; cp -f $(MANPAGESRC) $(MANPAGE); gzip $(MANPAGE); cp -f $(MANPAGE).gz $(INSTDIR_MAN)
	@echo "Installing documentation files into $(INSTDIR_DOC)"
	@mkdir -p $(INSTDIR_DOC)
	@cp -f $(DOCFILES) $(INSTDIR_DOC)

notes:
	@rm -f INSTALL; VER=`grep VERSION VERSION | awk -F\= '{print $$2}'`; sed -e "{s/VERSION/$${VER}/}" INSTALL.tmpl > INSTALL; echo "Actual version: $${VER}"
	
uninstall:
	@echo "Removing binary from $(INSTDIR_BIN)"
	@rm -f $(INSTDIR_BIN)/$(BINARY)
	@echo "Removing manpage from $(INSTDIR_MAN)"
	@rm -f $(INSTDIR_MAN)/$(MANPAGE).gz
	@echo "Removing documentation files from $(INSTDIR_DOC)"
	@for i in $(DOCFILES); do rm -f $(INSTDIR_DOC)/$$i; done
	@rmdir $(INSTDIR_DOC)


