#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
#
# --------------------------------------------------------------

# --------------------------------------------------------------
# Project name, used for binaries

NAME = rtjam

# --------------------------------------------------------------
# Files to build

FILES_DSP = \
	PluginRTJam.cpp \
	../common/Settings.cpp \
	../common/JamNetStuff.cpp \
	../common/JamMixer.cpp \
	../common/JitterBuffer.cpp \
	../common/ChannelMap.cpp \
	../common/TimersAndStats.cpp

FILES_UI = \
	UIRTJam.cpp \
	RTJamArt.cpp \
	../common/JamDirectory.cpp \
	../common/PartialImage.cpp \
	../common/LabelBox.cpp \
	../common/MeterBar.cpp \
	../common/NotoSans_Regular.ttf.cpp
	
# --------------------------------------------------------------
# Do some magic

include ../../dpf/Makefile.plugins.mk

# --------------------------------------------------------------
# Extra flags

LINK_FLAGS += -lpthread -lcurl
CXXFLAGS += -Wno-psabi -I../common -I../common/DSP

# --------------------------------------------------------------
# Enable all selected plugin types

ifeq ($(HAVE_JACK),true)
TARGETS += jack
endif

TARGETS += ladspa
TARGETS += vst

ifeq ($(HAVE_DGL),true)
ifeq ($(HAVE_LIBLO),true)
# TARGETS += dssi
endif
endif

ifeq ($(HAVE_DGL),true)
TARGETS += lv2_sep
else
TARGETS += lv2_dsp
endif

all: $(TARGETS)

# --------------------------------------------------------------
