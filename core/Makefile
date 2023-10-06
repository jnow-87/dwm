bin-y := dwm

dwm-y := \
	main.o \
	dwm.o \
	tags.o \
	statusbar.o \
	xevents.o \
	action/ \
	config/ \
	layout/ \
	utils/ \
	xlib/


# flags
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
