bin-y := dwm

dwm-y := \
	buttons.o \
	client.o \
	clientstack.o \
	dwm.o \
	keys.o \
	keylock.o \
	layout.o \
	main.o \
	monitor.o \
	statusbar.o \
	launcher.o \
	tags.o \
	scheme.o \
	xevents.o \
	commands/ \
	config/ \
	layouts/ \
	rc/ \
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
