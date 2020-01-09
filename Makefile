# define this to turn on debug
DEBUG ?= y

# Application name
TARGET := HomeAccount
VERSION := 1.0
YEAR := 2020

WX_VERSION := 3.1.3
WX_DYLIBS_VERSION := 3.1.3.0.0
WX_PATH := $(HOME)/workspace/devel/wxWidgets-$(WX_VERSION)
CRYPTOPP_VERSION := CRYPTOPP_8_2_0
CRYPTOPP_PATH := $(HOME)/workspace/devel/cryptopp-$(CRYPTOPP_VERSION)

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
  ifeq ($(DEBUG), y)
    WX_BUILD_PATH := $(WX_PATH)/build-cocoa-debug
  else
    WX_BUILD_PATH := $(WX_PATH)/build-cocoa-release
  endif
endif

ifeq ($(DEBUG), y)
  CFLAGS += -O0 -g -DDEBUG
  VER_STR := $(VER_STR)_debug
else
  CFLAGS += -O2
  VER_STR := $(VER_STR)_release
endif

CFLAGS += -DVER_STR=\"$(VER_STR)\"

WX_CONFIG := $(WX_BUILD_PATH)/wx-config

CXXFLAGS := $(CFLAGS)
CXXFLAGS += -pipe -Wall -std=c++11 $(shell $(WX_CONFIG) --cxxflags) -DwxHAS_IMAGES_IN_RESOURCES
CXXFLAGS += -I$(CRYPTOPP_PATH)
LDFLAGS += $(shell $(WX_CONFIG) --libs)
LDFLAGS += -L$(CRYPTOPP_PATH) -lcryptopp

OBJS := $(CPP_SRCS:.cpp=.o) $(C_SRCS:.c=.o)

DEPFILE := .depend

.PHONY: all clean dep clean-dep

ifeq ($(OS), Darwin)
.PHONY: app-bundle

APP_DIR := $(TARGET).app
CTS_DIR := $(APP_DIR)/Contents
EXE_DIR := $(CTS_DIR)/MacOS
RCS_DIR := $(CTS_DIR)/Resources
FRM_DIR := $(CTS_DIR)/Frameworks
APP_ICON := macos.icns
APP_PNGS := $(subst res,$(RCS_DIR),$(wildcard res/*.png))
APP_LCS := $(subst .po,.lproj/ha.mo,$(subst i18n,$(RCS_DIR),$(LCS)))
APP_XRS := $(RCS_DIR)/resources.xrs

all: app-bundle

app-bundle: \
  $(CTS_DIR)/Info.plist \
  $(CTS_DIR)/PkgInfo \
  $(EXE_DIR)/$(TARGET) \
  $(APP_DYLIBS) \
  $(RCS_DIR)/$(APP_ICON) \
  $(APP_PNGS) \
  $(APP_LCS) \
  $(APP_XRS)
	./cp-dylibs.sh

$(CTS_DIR)/Info.plist: res/Info.plist.in
	-mkdir -p $(dir $@)
	cat $< \
	  | sed -e 's/EXECUTABLE/$(TARGET)/' \
	  | sed -e 's/VERSION/$(VERSION)/' \
	  | sed -e 's/YEAR/$(YEAR)/' \
	  > $@

$(CTS_DIR)/PkgInfo:
	-mkdir -p $(dir $@)
	echo "APPL????\c" > $@

$(EXE_DIR)/$(TARGET): $(TARGET)
	-mkdir -p $(dir $@)
	SetFile -t APPL $(TARGET)
	cp $< $@

$(RCS_DIR)/$(APP_ICON): res/macos.iconset
	-mkdir -p $(dir $@)
	iconutil -c icns -o $@ $<

$(RCS_DIR)/%.png: res/%.png
	-mkdir -p $(dir $@)
	cp $< $@

$(RCS_DIR)/%.lproj/ha.mo: i18n/%.mo
	-mkdir -p $(dir $@)
	cp $< $@

$(RCS_DIR)/%.xrs: $(XRCS)
	-mkdir -p $(dir $@)
	$(WX_BUILD_PATH)/utils/wxrc/wxrc $^ -o $@

else
all: $(TARGET)
endif

%.mo: %.po
	msgfmt -o $@ $^

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^

dep:
	$(CXX) -MM $(CXXFLAGS) $(CPP_SRCS) > $(DEPFILE)
	$(CC) -MM $(CFLAGS) $(C_SRCS) >> $(DEPFILE)

clean:
	-rm -f $(OBJS)
	-rm -f $(TARGET)
ifeq ($(OS), Darwin)
	-rm -rf $(APP_DIR)
endif

clean-dep:
	-rm -f $(DEPFILE)

ifeq ($(DEPFILE), $(wildcard $(DEPFILE)))
  include $(DEPFILE)
endif
