#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX, Christopher Arndt, and Patrick Desaulniers
#

include dpf/Makefile.base.mk

all: libs plugins gen gitversion

# --------------------------------------------------------------

submodules:
	git submodule update --init --recursive

libs:
	$(MAKE) -C dpf/dgl ../build/libdgl-opengl.a

plugins: libs
	$(MAKE) all -C plugins/RTJam
	$(MAKE) all -C plugins/server
	$(MAKE) all -C plugins/nojack
	$(MAKE) all -C plugins/rtjam-broadcast
	$(MAKE) all -C plugins/rtjam-status
	$(MAKE) all -C plugins/rtjam-sound
	$(MAKE) all -C plugins/rtjam-box

gitversion:
	git rev-parse HEAD > bin/version.txt


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
	$(MAKE) clean -C plugins/nojack
	$(MAKE) clean -C plugins/rtjam-broadcast
	$(MAKE) clean -C plugins/rtjam-status
	$(MAKE) clean -C plugins/rtjam-sound
	$(MAKE) clean -C plugins/rtjam-box
	rm -rf bin build

install: all
	$(MAKE) install -C plugins/RTJam

install-user: all
	$(MAKE) install-user -C plugins/RTJam

# --------------------------------------------------------------

deploy-pi: all
# cp bin/rtjam ~/www/html/pi
#	cp utils/JamOn.sh ~/www/html/pi
#	cp utils/Update.sh ~/www/html/pi
#	scp -i ~/.ssh/rtjam.cer bin/rtjam ubuntu@rtjam-nation.basscleftech.com:/home/ubuntu/www/html/pi
	scp -i ~/.ssh/rtjam.cer bin/rtjam-status ubuntu@rtjam-nation.basscleftech.com:/home/ubuntu/www/html/pi
	scp -i ~/.ssh/rtjam.cer bin/rtjam-sound ubuntu@rtjam-nation.basscleftech.com:/home/ubuntu/www/html/pi
	scp -i ~/.ssh/rtjam.cer bin/rtjam-box ubuntu@rtjam-nation.basscleftech.com:/home/ubuntu/www/html/pi
	scp -i ~/.ssh/rtjam.cer bin/rtjam-broadcast ubuntu@rtjam-nation.basscleftech.com:/home/ubuntu/www/html/pi
	scp -i ~/.ssh/rtjam.cer bin/version.txt ubuntu@rtjam-nation.basscleftech.com:/home/ubuntu/www/html/pi

deploy-mac:
	zip -r bin/rtjam.vst.zip bin/rtjam.vst
	scp bin/rtjam.vst.zip  pi@music.basscleftech.com:/home/pi/www/html/mac

deploy-linux:
	cp bin/rtjam ~/bin
	cp bin/rtjam-vst.so  ~/vsts

.PHONY: all clean install install-user submodule libs plugins gen
