# define this to turn on debug
DEBUG ?= y

# Application name
APP_NAME := HomeAccount
VERSION := 1.1
YEAR := 2021

DEV_ROOT := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))../devel

WX_VERSION := 3.1.4
WX_DYLIBS_VERSION := 3.1.4.0.0
WX_PATH := $(DEV_ROOT)/wxWidgets-$(WX_VERSION)
CRYPTOPP_VERSION := 840
CRYPTOPP_PATH := $(DEV_ROOT)/cryptopp$(CRYPTOPP_VERSION)

# Source files
CPP_SRCS := \
  $(wildcard *.cpp) \
  $(wildcard file/*.cpp) \
  $(wildcard file/except/*.cpp)
C_SRCS := \
  $(wildcard c/*.c)
LCS := $(wildcard i18n/*.po)
XRCS := $(wildcard res/*.xrc)
SRCS := $(CPP_SRCS) $(C_SRCS)

VER_STR := v$(subst .,_,$(VERSION))

OS := $(shell uname)

ifeq ($(OS), Darwin)
  VER_STR := $(VER_STR)_darwin
  STUB := cocoa
  TARGET := $(APP_NAME)
endif
ifeq ($(findstring MINGW64,$(OS)), MINGW64)
  VER_STR := $(VER_STR)_mingw64
  STUB := $(MSYSTEM_CARCH)-msw-static
  TARGET := $(APP_NAME).exe
endif

CRYPTOPP_PATH := $(CRYPTOPP_PATH)-$(STUB)

ifeq ($(DEBUG), y)
  CFLAGS += -O0 -g -DDEBUG
  VER_STR := $(VER_STR)_debug
  WX_BUILD_PATH := $(WX_PATH)/build-$(STUB)-debug
else
  CFLAGS += -O2
  VER_STR := $(VER_STR)_release
  WX_BUILD_PATH := $(WX_PATH)/build-$(STUB)-release
endif

CFLAGS += -DVER_STR=\"$(VER_STR)\"
CXXFLAGS := $(CFLAGS)
CXXFLAGS += -pipe -Wall -std=c++11

WX_CONFIG := $(WX_BUILD_PATH)/wx-config

CXXFLAGS += $(shell $(WX_CONFIG) --cxxflags)
ifeq ($(OS), Darwin)
  CXXFLAGS += -DwxHAS_IMAGES_IN_RESOURCES
endif
CXXFLAGS += -I$(CRYPTOPP_PATH)
LDFLAGS += $(shell $(WX_CONFIG) --libs)
LDFLAGS += -L$(CRYPTOPP_PATH) -lcryptopp

OBJS := $(CPP_SRCS:.cpp=.o) $(C_SRCS:.c=.o)
XRS := resources.xrs
PNGS := $(wildcard res/*.png)

DEPFILE := .depend

.PHONY: all clean dep clean-dep
.PHONY: app-bundle

all: app-bundle

ifeq ($(OS), Darwin)
APP_DIR := $(APP_NAME).app
CTS_DIR := $(APP_DIR)/Contents
EXE_DIR := $(CTS_DIR)/MacOS
RCS_DIR := $(CTS_DIR)/Resources
FRM_DIR := $(CTS_DIR)/Frameworks
APP_ICON := macos.icns

app-bundle: \
  $(CTS_DIR)/Info.plist \
  $(CTS_DIR)/PkgInfo \
  $(EXE_DIR)/$(TARGET) \
  $(RCS_DIR)/$(APP_ICON) \
  $(subst res,$(RCS_DIR),$(PNGS)) \
  $(subst .po,.lproj/ha.mo,$(subst i18n,$(RCS_DIR),$(LCS))) \
  $(RCS_DIR)/$(XRS)
	./cp-dylibs.sh

$(CTS_DIR) $(EXE_DIR) $(RCS_DIR):
	-mkdir -p $@

$(CTS_DIR)/Info.plist: res/Info.plist.in $(CTS_DIR)
	cat $< \
	  | sed -e 's/EXECUTABLE/$(TARGET)/' \
	  | sed -e 's/VERSION/$(VERSION)/' \
	  | sed -e 's/YEAR/$(YEAR)/' \
	  > $@

$(CTS_DIR)/PkgInfo: $(CTS_DIR)
	echo "APPL????\c" > $@

$(EXE_DIR)/$(TARGET): $(TARGET) $(EXE_DIR)
	SetFile -t APPL $<
	cp $< $@

$(RCS_DIR)/$(APP_ICON): res/macos.iconset $(RCS_DIR)
	iconutil -c icns -o $@ $<

$(RCS_DIR)/%.lproj/ha.mo: i18n/%.mo $(RCS_DIR)
	-mkdir -p $(dir $@)
	cp $< $@
endif

ifeq ($(findstring MINGW64,$(OS)), MINGW64)
APP_DIR := $(APP_NAME)
EXE_DIR := $(APP_DIR)
RCS_DIR := $(APP_DIR)

$(APP_DIR):
	-mkdir -p $@

app-bundle: \
  $(EXE_DIR)/$(TARGET) \
  $(RCS_DIR)/$(XRS) \
  $(subst res,$(RCS_DIR),$(PNGS)) \
  $(subst .po,/ha.mo,$(subst i18n,$(RCS_DIR),$(LCS)))
	./cp-dlls.sh

$(EXE_DIR)/$(TARGET): $(TARGET) $(EXE_DIR)
	cp $< $@

$(RCS_DIR)/%/ha.mo: i18n/%.mo $(RCS_DIR)
	-mkdir -p $(dir $@)
	cp $< $@
endif

%.mo: %.po
	msgfmt -o $@ $^

$(RCS_DIR)/$(XRS): $(XRS) $(RCS_DIR)
	cp $< $@

$(RCS_DIR)/%.png: res/%.png $(RCS_DIR)
	cp $< $@

$(XRS): $(XRCS)
	$(WX_BUILD_PATH)/utils/wxrc/wxrc $^ -o $@

$(TARGET): $(OBJS)
# LDFLAGS must be put at end for MinGW build.
	$(CXX) -o $@ $^ $(LDFLAGS)

dep:
	$(CXX) -MM $(CXXFLAGS) $(CPP_SRCS) > $(DEPFILE)
	$(CC) -MM $(CFLAGS) $(C_SRCS) >> $(DEPFILE)

clean:
	-rm -f $(OBJS)
	-rm -f $(TARGET)
	-rm -f $(XRS)
	-rm -rf $(APP_DIR)

clean-dep:
	-rm -f $(DEPFILE)

ifeq ($(DEPFILE), $(wildcard $(DEPFILE)))
  include $(DEPFILE)
endif
