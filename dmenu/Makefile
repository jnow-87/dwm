bin-y := dmenu stest

dmenu-y := drw.o dmenu.o util.o
stest-y := stest.o


# flags
VERSION = 5.2

dmenu-cppflags := \
	-I/usr/X11R6/include \
	-I/usr/include/freetype2 \
	-DXINERAMA \
	-D_DEFAULT_SOURCE \
	-D_BSD_SOURCE \
	-D_XOPEN_SOURCE=700L \
	-D_POSIX_C_SOURCE=200809L \
	-DVERSION=\"$(VERSION)\"

dmenu-ldlibs := \
	-L/usr/X11R6/lib \
	-lX11 \
	-lfontconfig \
	-lXft \
	-lXinerama

stest-cppflags := \
	-D_DEFAULT_SOURCE \
	-D_BSD_SOURCE \
	-D_XOPEN_SOURCE=700L \
	-D_POSIX_C_SOURCE=200809L
