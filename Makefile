ARCH=arm
CC=gcc
CXX=g++

COMMON_FLAGS=

ifeq ($(ARCH),arm)
	ARCHFLAGS = -DHAVE_ARM_TLS_REGISTER -DANDROID_ARM_LINKER
else
	ARCHFLAGS = -DANDROID_X86_LINKER
endif


COMMON_SOURCES=common/strlcpy.c common/hooks.c common/properties.c

ICS_SOURCES=ics/linker.c ics/dlfcn.c ics/rt.c ics/linker_environ.c ics/linker_format.c

all:  libhybris_ics.so libEGL.so.1 libGLESv2.so.2 libhardware.so.1 libsf.so.1 test_sf test_egl test_hw test_sensors test_glesv2


libhybris_ics.so: $(COMMON_SOURCES) $(ICS_SOURCES)
	$(CC) -g -shared -o $@ -ldl -pthread -fPIC -Iics -Icommon -DLINKER_DEBUG=1 -DLINKER_TEXT_BASE=0xB0000100 -DLINKER_AREA_SIZE=0x01000000 $(ARCHFLAGS) \
		$(ICS_SOURCES) $(COMMON_SOURCES)


libhardware.so.1.0: hardware/hardware.c
	$(CC) -g -shared -o $@ -fPIC -Wl,-soname,libhardware.so.1 $< libhybris_ics.so

libhardware.so.1: libhardware.so.1.0
	ln -sf libhardware.so.1.0 libhardware.so.1

libEGL.so.1.0: egl/egl.c
	$(CC) -g -shared -o $@ -fPIC -Wl,-soname,libEGL.so.1 $< libhybris_ics.so

libsf.so.1.0: sf/sf.cpp
	$(CXX) -g -fpermissive -shared -o $@ -fPIC -Iandroid/surface_flinger -Wl,-soname,libsf.so.1 $< libhybris_ics.so

libEGL.so.1: libEGL.so.1.0
	ln -sf libEGL.so.1.0 libEGL.so.1

libGLESv2.so.2.0: glesv2/gl2.c
	$(CC) -g -shared -o $@ -fPIC -Wl,-soname,libGLESv2.so.2 $< libhybris_ics.so

libGLESv2.so.2: libGLESv2.so.2.0
	ln -sf libGLESv2.so.2.0 libGLESv2.so.2

libsf.so.1: libsf.so.1.0
	ln -sf libsf.so.1.0 libsf.so.1

test_egl: libEGL.so.1 libGLESv2.so.2 egl/test.c libhybris_ics.so
	$(CC) -g -o $@ egl/test.c libEGL.so.1 libGLESv2.so.2 libhybris_ics.so  -I .

test_sf: libsf.so.1 sf/test.cpp libhybris_ics.so
	$(CXX) -g -o $@ sf/test.cpp libsf.so.1 libEGL.so.1 libGLESv2.so.2 libhybris_ics.so  -Iandroid/surface_flinger

test_hw: libhardware.so.1 hardware/test.c libhybris_ics.so
	$(CC) -g -o $@ hardware/test.c libhardware.so.1 libhybris_ics.so  -I .

test_sensors: libhardware.so.1 hardware/sensors.c libhybris_ics.so
	$(CC) -g -o $@ hardware/sensors.c libhardware.so.1 libhybris_ics.so  -I .

test_glesv2: libEGL.so.1 libGLESv2.so.2 egl/test.c libhybris_ics.so
	$(CC) -g -o $@ glesv2/test.c -lm libEGL.so.1 libhybris_ics.so libGLESv2.so.2

clean:
	rm -rf libhybris_ics.so test_ics
	rm -rf libEGL* libGLESv2*
	rm -rf libhardware*
	rm -rf libsf*
	rm -rf test_*
