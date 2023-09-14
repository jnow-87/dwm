bin-y := dwm

dwm-y := dwm.o drw.o util.o


dwm-cppflags := \
	-I/usr/X11R6/include \
	-I/usr/include/freetype2 \
	-D_DEFAULT_SOURCE \
	-D_BSD_SOURCE \
	-D_XOPEN_SOURCE=700L

dwm-ldlibs := \
	-L/usr/X11R6/lib \
	-lX11 \
	-lfontconfig \
	-lXft

dwm-ldlibs-$(CONFIG_XINERAMA) += -lXinerama

