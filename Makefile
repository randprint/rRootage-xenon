# rRootage makefile(XENON 2.0.0)
# $Id: Makefile,v 1.6 2003/08/10 03:21:28 kenta Exp $
ifeq ($(strip $(DEVKITXENON)),)
$(error "Please set DEVKITXENON in your environment. export DEVKITXENON=<path to>devkitPPC")
endif

include $(DEVKITXENON)/rules
 
CC = xenon-gcc
CXX = xenon-g++
NAME   = rr
O      = o
RM     = rm -f
STRIP = xenon-strip
LD = xenon-ld
AR = xenon-ar
AS = xenon-as
RANLIB := xenon-ranlib

PROG   = $(NAME)
DEPSDIR = ./deps
DEFAULT_CFLAGS = -I"$(DEVKITXENON)/usr/include" -I"$(DEVKITXENON)/usr/include/SDL" -I. -DTARGET=XENON -g0 $(MACHDEP) -DLINUX -DNDEBUG -DXENON -DLIBXENON -O3 -DXENON -ffast-math -fomit-frame-pointer -funroll-loops -ffunction-sections -fdata-sections -fno-tree-vectorize -fno-tree-slp-vectorize -ftree-vectorizer-verbose=1 -flto -fuse-linker-plugin -maltivec -mabi=altivec -fno-pic -mpowerpc64 -mhard-float -Wall -mcpu=cell -mtune=cell -m32 -fno-pic -mpowerpc64 

LDFLAGS        =  		$(MACHDEP) -L. /usr/local/xenon/usr/lib/libxenon.a -L/usr/local/xenon/usr/lib \
						-n -T /usr/local/xenon/app.lds -lgl -lbulletml -lstdc++ \
                        -static -lSDL_image -ljpeg -lpng -lz -lfat -lSDL_mixer -lvorbisidec -logg -lSDL \
                        -lgcc -lxenon -lm -lc -DLIBXENON -g0 -O3 -DXENON -ffast-math -fomit-frame-pointer -funroll-loops -ffunction-sections -fdata-sections -flto -fuse-linker-plugin -maltivec -fno-pic -mpowerpc64 -mhard-float -Wall -m32 -fno-pic -mpowerpc64 


MORE_CFLAGS =

CFLAGS   = $(DEFAULT_CFLAGS) $(MORE_CFLAGS)
CPPFLAGS = $(DEFAULT_CFLAGS) $(MORE_CFLAGS) -I./bulletml/

#$(NAME)_res.$(O)
OBJS =	$(NAME).$(O) \
	foe.$(O) foecommand.$(O) barragemanager.$(O) boss.$(O) ship.$(O) laser.$(O) \
	frag.$(O) background.$(O) letterrender.$(O) shot.$(O) \
	screen.$(O) vector.$(O) degutil.$(O) rand.$(O) mt19937int.$(O) \
	soundmanager.$(O) attractmanager.$(O) screencapture.$(O) \
	httpd.$(O)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o ./$(PROG) ./$(OBJS) $(LDFLAGS)

clean:
	$(RM) $(PROG) *.$(O)
