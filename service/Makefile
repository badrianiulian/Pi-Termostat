#*********************************************************************
# SH1106 DS18B20 KEYS module control service
# *********************************************************************

# Makefile itself dir
ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

# DEV MODE
#CCFLAGS=-fsanitize=address -static-libasan -fno-omit-frame-pointer -Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s
# NON DEV MODE - removed: -Ofast
CCFLAGS=-Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s

prefix := /usr/local

# define all programs
PROGRAMS = shdskeys_srv
SOURCES = ${PROGRAMS:=.cpp}

CXX=g++
CFLAGS=${CCFLAGS}

all: ${PROGRAMS}

${PROGRAMS}: ${SOURCES}
	$(CXX) $(CFLAGS) -Wall  -lSH1106_DS18B20_KEYS $@.cpp -o $@ -lpthread `mariadb_config --cflags --libs`

clean:
	rm -rf $(PROGRAMS)

install: all
	test -d $(prefix) || mkdir $(prefix)
	test -d $(prefix)/bin || mkdir $(prefix)/bin
	for prog in $(PROGRAMS); do \
	  install -m 0755 $$prog $(prefix)/bin; \
	done

.PHONY: install


