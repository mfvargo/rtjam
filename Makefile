#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX, Christopher Arndt, and Patrick Desaulniers
#

include dpf/Makefile.base.mk

all: libs plugins gen

# --------------------------------------------------------------

submodules:
	git submodule update --init --recursive

libs:
	$(MAKE) -C dpf/dgl ../build/libdgl-opengl.a

plugins: libs
	$(MAKE) all -C plugins/RTJam
	$(MAKE) all -C plugins/server

ifneq ($(CROSS_COMPILING),true)
gen: plugins dpf/utils/lv2_ttl_generator
	@$(CURDIR)/dpf/utils/generate-ttl.sh
ifeq ($(MACOS),true)
	@$(CURDIR)/dpf/utils/generate-vst-bundles.sh
endif

dpf/utils/lv2_ttl_generator:
	$(MAKE) -C dpf/utils/lv2-ttl-generator
else
gen: plugins dpf/utils/lv2_ttl_generator.exe
	@$(CURDIR)/dpf/utils/generate-ttl.sh

dpf/utils/lv2_ttl_generator.exe:
	$(MAKE) -C dpf/utils/lv2-ttl-generator WINDOWS=true
endif

# --------------------------------------------------------------

clean:
	$(MAKE) clean -C dpf/dgl
	$(MAKE) clean -C dpf/utils/lv2-ttl-generator
	$(MAKE) clean -C plugins/RTJam
	$(MAKE) clean -C plugins/server
	rm -rf bin build

install: all
	$(MAKE) install -C plugins/RTJam

install-user: all
	$(MAKE) install-user -C plugins/RTJam

# --------------------------------------------------------------

deploy-pi:
	cp bin/rtjam ~/www/html/pi
	cp utils/start-rtjam.sh ~/www/html/pi
	cp utils/update-rtjam.sh ~/www/html/pi

deploy-mac:
	zip -r bin/rtjam.vst.zip bin/rtjam.vst
	scp bin/rtjam.vst.zip  pi@music.basscleftech.com:/home/pi/www/html/mac

deploy-linux:
	scp bin/rtjam  pi@music.basscleftech.com:/home/pi/www/html/linux
	scp bin/rtjam-vst.so  pi@music.basscleftech.com:/home/pi/www/html/linux
	scp readme.html  pi@music.basscleftech.com:/home/pi/www/html/index.html
	cp bin/rtjam ~/bin
	cp bin/rtjam-vst.so  ~/vsts/Bassclef

.PHONY: all clean install install-user submodule libs plugins gen
