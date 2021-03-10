# define this to turn on debug
DEBUG ?= y

# Application info
APP_NAME := HomeAccount
VERSION := 1.2
YEAR := 2021
VER_STR := v$(subst .,_,$(VERSION))

# Where the 3rd-party libs are
DEV_ROOT := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))../devel

# 3rd-party libs
WX_VERSION := 3.1.4
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

# Shell command
CP ?= cp
MV ?= mv
RM ?= rm -f
CAT ?= cat
GREP ?= grep
SED ?= sed -e
MKDIR ?= mkdir -p
ZIP ?= zip -9 -r
TAR ?= tar -cjv

OBJS := $(CPP_SRCS:.cpp=.o) $(C_SRCS:.c=.o)
XRS := resources.xrs
PNGS := $(wildcard res/*.png)

# Attempt to determine platform
SYSTEMX := $(shell $(CC) $(CFLAGS) -dumpmachine 2>/dev/null)
ARCH := $(shell echo $(MACHINEX) | cut -f 1 -d '-')
ifeq ($(ARCH),)
  ARCH := $(shell uname -m 2>/dev/null)
endif
ifeq ($(SYSTEMX),)
  SYSTEMX := $(shell uname -s 2>/dev/null)
endif
IS_DARWIN := $(shell echo "$(SYSTEMX)" | $(GREP) -i -c "Darwin")
IS_MINGW := $(shell echo "$(SYSTEMX)" | $(GREP) -i -c "MinGW")

ifneq ($(IS_DARWIN),0)
  STUB := $(ARCH)-darwin
  TARGET := $(APP_NAME)
endif
ifneq ($(IS_MINGW),0)
  STUB := $(ARCH)-mingw
  TARGET := $(APP_NAME).exe
  RES_OBJ := resources.o
  OBJS := $(OBJS) $(RES_OBJ)
endif

VER_STR := $(VER_STR)_$(STUB)

CRYPTOPP_PATH := $(CRYPTOPP_PATH)-$(STUB)-release

ifeq ($(DEBUG), y)
  CFLAGS += -O0 -g -DDEBUG
  VER_STR := $(VER_STR)-debug
  WX_BUILD_PATH := $(WX_PATH)/build-$(STUB)-debug
else
  CFLAGS += -O2
  VER_STR := $(VER_STR)-release
  WX_BUILD_PATH := $(WX_PATH)/build-$(STUB)-release
endif

CFLAGS += -DVER_STR=\"$(VER_STR)\"
CXXFLAGS := $(CFLAGS)
CXXFLAGS += -pipe -Wall -std=c++11

WX_CONFIG := $(WX_BUILD_PATH)/wx-config

CXXFLAGS += $(shell $(WX_CONFIG) --cxxflags)
ifneq ($(IS_DARWIN),0)
  CXXFLAGS += -DwxHAS_IMAGES_IN_RESOURCES
endif
CXXFLAGS += -I$(CRYPTOPP_PATH)
LDFLAGS += $(shell $(WX_CONFIG) --libs)
LDFLAGS += -L$(CRYPTOPP_PATH) -lcryptopp

DEPFILE := .depend

.PHONY: all clean dep clean-dep
.PHONY: app-bundle, app-zipped

all: app-zipped

ifneq ($(IS_DARWIN),0)
APP_DIR := $(APP_NAME).app
CTS_DIR := $(APP_DIR)/Contents
EXE_DIR := $(CTS_DIR)/MacOS
RCS_DIR := $(CTS_DIR)/Resources
FRM_DIR := $(CTS_DIR)/Frameworks
APP_ICON := macos.icns
APP_ZIP := $(APP_NAME)-$(VER_STR).tar.bz2

app-zipped: $(APP_ZIP)

$(APP_ZIP): app-bundle
	-$(RM) $(APP_ZIP)
	$(TAR) -f $@ $(APP_DIR)

app-bundle: \
  $(CTS_DIR)/Info.plist \
  $(CTS_DIR)/PkgInfo \
  $(EXE_DIR)/$(TARGET) \
  $(RCS_DIR)/$(APP_ICON) \
  $(subst res,$(RCS_DIR),$(PNGS)) \
  $(subst .po,.lproj/ha.mo,$(subst i18n,$(RCS_DIR),$(LCS))) \
  $(RCS_DIR)/$(XRS)

$(CTS_DIR) $(EXE_DIR) $(FRM_DIR) $(RCS_DIR):
	-$(MKDIR) $@

$(CTS_DIR)/Info.plist: res/Info.plist.in $(CTS_DIR)
	$(CAT) $< \
	  | $(SED) 's/EXECUTABLE/$(TARGET)/' \
	  | $(SED) 's/VERSION/$(VERSION)/' \
	  | $(SED) 's/YEAR/$(YEAR)/' \
	  > $@

$(CTS_DIR)/PkgInfo: $(CTS_DIR)
	echo "APPL????\c" > $@

$(EXE_DIR)/$(TARGET): $(TARGET) $(EXE_DIR)
	SetFile -t APPL $<
	$(CP) $< $@
	./cp-dylibs.sh $@

$(RCS_DIR)/$(APP_ICON): res/macos.iconset $(RCS_DIR)
	iconutil -c icns -o $@ $<

$(RCS_DIR)/%.lproj/ha.mo: i18n/%.mo $(RCS_DIR)
	-$(MKDIR) $(dir $@)
	$(CP) $< $@
endif

ifneq ($(IS_MINGW),0)
# Don't simply use APP_NAME for MSYS2 make a ghost executable of the same name.
APP_DIR := $(APP_NAME)_win
EXE_DIR := $(APP_DIR)
RCS_DIR := $(APP_DIR)
APP_ZIP := $(APP_NAME)-$(VER_STR).zip

app-zipped: $(APP_ZIP)

$(APP_ZIP): app-bundle
	-$(RM) $(APP_ZIP)
	$(ZIP) $@ $(APP_DIR) -x \*\/HA\/\*

$(APP_DIR):
	-$(MKDIR) $@

app-bundle: \
  $(EXE_DIR)/$(TARGET) \
  $(RCS_DIR)/$(XRS) \
  $(subst res,$(RCS_DIR),$(PNGS)) \
  $(subst .po,/ha.mo,$(subst i18n,$(RCS_DIR),$(LCS)))
	./cp-dlls.sh $(EXE_DIR)/$(TARGET) $(WX_BUILD_PATH)/lib

$(EXE_DIR)/$(TARGET): $(TARGET) $(EXE_DIR)
	$(CP) $< $@

$(RCS_DIR)/%/ha.mo: i18n/%.mo $(RCS_DIR)
	-$(MKDIR) $(dir $@)
	$(CP) $< $@

$(RES_OBJ): res/resources.rc res/appIcon.ico
	windres $< -o $@ -I$(WX_PATH)/include
endif

%.mo: %.po
	msgfmt -o $@ $^

$(RCS_DIR)/$(XRS): $(XRS) $(RCS_DIR)
	$(CP) $< $@

$(RCS_DIR)/%.png: res/%.png $(RCS_DIR)
	$(CP) $< $@

$(XRS): $(XRCS)
ifeq ($(IS_MINGW),1)
# PE executables don't know where the dll is
# and MinGw honor PATH instead of LD_LIBRARY_PATH
	PATH=$(PATH):$(WX_BUILD_PATH)/lib $(WX_BUILD_PATH)/utils/wxrc/wxrc $^ -o $@
else
	$(WX_BUILD_PATH)/utils/wxrc/wxrc $^ -o $@
endif

$(TARGET): $(OBJS)
# LDFLAGS must be put at end for MinGW build.
	$(CXX) -o $@ $^ $(LDFLAGS)

dep: $(DEPFILE)

clean:
	-$(RM) $(OBJS)
	-$(RM) $(TARGET)
	-$(RM) $(XRS)
	-$(RM) -r $(APP_DIR)
	-$(RM) $(APP_ZIP)

clean-dep:
	-$(RM) $(DEPFILE)

$(DEPFILE): $(C_SRCS) $(CPP_SRCS)
	$(CC) -MM $(CFLAGS) $(C_SRCS) > $@
	$(CXX) -MM $(CXXFLAGS) $(CPP_SRCS) >> $@

ifeq ($(filter clean%,$(MAKECMDGOALS)),)
  ifeq ($(DEPFILE),$(wildcard $(DEPFILE)))
    include $(DEPFILE)
  endif
endif
