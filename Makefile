##  Makefile for hub_port_power tool
# dependency files (*.d) are generated at compile time

# Set the version number of this package
VERSION=0.1.0

# Packages we depend on, their cflags and libs options
# Note: cross-env needs lots of std libraries added, yuck.
pkg_packages 	:= libusb-1.0
PKG_CFLAGS   	:= $(shell pkg-config --cflags $(pkg_packages))
PKG_LDFLAGS  	:= $(shell pkg-config --libs $(pkg_packages))

# To make warnings stand out, reduce noise by default.
# Run with 'make V=1' for full make verbosity
ifeq ("$(origin V)", "command line")
  BUILD_VERBOSE = $(V)
endif
ifndef BUILD_VERBOSE
  BUILD_VERBOSE = 0
endif
ifeq (${BUILD_VERBOSE},1)
  quiet =
  Q =
else
  quiet = quiet_
  Q = @
endif
export Q quiet
    quiet_cmd_link = LINK    $@
  quiet_cmd_cc_c_o = CC      $@
quiet_cmd_cc_cpp_o = CC++    $@
   quiet_cmd_clean = CLEAN
      quiet_cmd_ar = AR      $@
 quiet_cmd_install = INSTALL $@
     quiet_cmd_gen = GEN     $@


# lib_helper.c adds entry points from newer versions of libusb
# lib_helper.h declares missing API from new versions of libusb
# if we are missing libusb_error_name we will need both
LIBUSB_HELPER 	:= $(shell grep libusb_error_name `pkg-config --cflags-only-I $(pkg_packages) | sed -e 's/-I//g' -e 's/ *$$//'`/libusb.h >/dev/null; echo $$?)

EXTRA_DEFS 	:= -DLIBUSB_HELPER=$(LIBUSB_HELPER)
ifeq ($(LIBUSB_HELPER),1)
EXTRA_SRCS 	:= libusb_helper.c
endif

SRCS = hub_port_power.c $(EXTRA_SRCS)
OBJS = $(SRCS:%.c=%.o)
OBJLISTS = $(OBJS:%.o=%.lis)
DEPENDS = $(SRCS:%.c=%.d)

PROG = hub_port_power

CFLAGS=-ggdb -Wall -g $(PKG_CFLAGS) $(EXTRA_DEFS) $(IINC)
DEPFLAGS=$(PKG_CFLAGS) $(IINC)

LDLIBS = $(PKG_LDFLAGS)

# Rules to make app
$(PROG): $(OBJS)
	@echo "  $($(quiet)cmd_link)"
	$(Q)$(CC) $(LDFLAGS) $(OBJS) $(LOADLIBES) $(LDLIBS) -o $@

%.o: %.c
	@echo "  $($(quiet)cmd_cc_c_o)"
	$(Q)$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
	$(Q)$(CC) -MM $(DEPFLAGS) $< > $*.d

%.lis: %.o
	$(OBJDUMP) -d $< > $(@F)

.PHONY: obj_lists
obj_lists: $(OBJLISTS)

# Misc.
.PHONY: clean
clean:
	@echo "  $($(quiet)cmd_clean)"
	$(Q)$(RM) $(OBJS) $(DEPENDS) core $(PROG) cscope.out *.tar.gz

# install rules
bindir=$(DESTDIR)/sbin

.PHONY: install
# perform the install into $(DESTDIR)
install: $(PROG)
	@echo "  $($(quiet)cmd_install)"
	$(Q)install -d $(bindir)
	$(Q)install -t $(bindir) $(PROG)

.PHONY: cscope
cscope:
	@echo "  $($(quiet)cmd_gen)"
	$(Q)echo $(PKG_CFLAGS) $(SRCS) | fmt -1 > cscope.files
	$(Q)cscope -b


.PHONY: dist
dist:
	@echo "  $($(quiet)cmd_gen)"
	$(Q)Tcommit=`git log -1 --pretty=format:%ct` && \
	     outfile=hub_port_power-$$Tcommit.tar.gz && \
	     git archive --format=tar --prefix=hub_port_power-$$Tcommit/ HEAD | \
	     gzip > $$outfile && \
	     echo $$outfile: created distribution archive

# Process dependencies
include $(wildcard *.d)

# vim:ft=make:noet:sw=8:ts=8:
