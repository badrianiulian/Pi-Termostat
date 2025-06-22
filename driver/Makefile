#*********************************************************************
# This is the makefile for the SH1106 OLED library driver
# *********************************************************************

# Makefile itself dir
ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

# DEV MODE
#CCFLAGS=-fsanitize=address -static-libasan -fno-omit-frame-pointer -Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s
# NON DEV MODE - removed: -Ofast
CCFLAGS=-Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s

# Where you want it installed when you do 'make install'
PREFIX=/usr/local

# Library parameters
# where to put the lib
LIBDIR=$(PREFIX)/lib
# lib name 
LIB=libSH1106_DS18B20_KEYS
# shared library name
LIBNAME=$(LIB).so.1.0

CXX=g++
#CC=gcc
CFLAGS=$(CCFLAGS)

# make all
# reinstall the library after each recompilation
all: SH1106_DS18B20_KEYS

# Make the library
SH1106_DS18B20_KEYS: SH1106_DS18B20_KEYS.o bcm2835.o
	$(CXX) -shared -Wl,-soname,$(LIB).so.1 $(CFLAGS) $(LDFLAGS)  -o ${LIBNAME} $^

# Library parts (use -fno-rtti flag to avoid link problem)
SH1106_DS18B20_KEYS.o: SH1106_DS18B20_KEYS.cpp
	$(CXX) -Wall -fPIC -fno-rtti $(CFLAGS) -c $^

bcm2835.o: bcm2835.c
	$(CC) -Wall -fPIC $(CFLAGS) -c $^

# Install the library to LIBPATH
install: 
	@echo "[Install Library]"
	@if ( test ! -d $(PREFIX)/lib ) ; then mkdir -p $(PREFIX)/lib ; fi
	@install -m 0755 ${LIBNAME} ${LIBDIR}
	@ln -sf ${LIBDIR}/${LIBNAME} ${LIBDIR}/${LIB}.so.1
	@ln -sf ${LIBDIR}/${LIBNAME} ${LIBDIR}/${LIB}.so
	@ldconfig
	@rm -rf ${LIB}.*

	@echo "[Install Headers]"
	@if ( test ! -d $(PREFIX)/include ) ; then mkdir -p $(PREFIX)/include ; fi
	@cp -f  SH1106_*.h $(PREFIX)/include
	@cp -f  bcm2835.h $(PREFIX)/include
	@cp -f  Font.h $(PREFIX)/include
	
	
# Uninstall the library 
uninstall: 
	@echo "[Uninstall Library]"
	@rm -f ${LIBDIR}/${LIB}.*

	@echo "[Uninstall Headers]"
	@rm -rf  $(PREFIX)/include/SH1106_DS18B20_KEYS*
	@rm -rf  $(PREFIX)/include/bcm2835.h
	@rm -rf  $(PREFIX)/include/Font.h
	
# clear build files
clean:
	rm -rf *.o ${LIB}.* ${LIBDIR}/${LIB}.*


