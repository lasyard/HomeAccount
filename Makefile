# define this to turn on debug
DEBUG ?= y

# Application name
TARGET := HomeAccount
VERSION := 1.0
YEAR := 2020

# Source files
CPP_SRCS := \
  $(wildcard *.cpp) \
  $(wildcard file/*.cpp) \
  $(wildcard file/except/*.cpp)
C_SRCS := \
  $(wildcard c/*.c)
LCS := $(wildcard i18n/*.po)
SRCS := $(CPP_SRCS) $(C_SRCS)

OS := $(shell uname)

ifeq ($(DEBUG), y)
  CFLAGS += -O0 -g -DDEBUG
else
  CFLAGS += -O2
endif

ifeq ($(OS), Darwin)
  ifeq ($(DEBUG), y)
    WX_BUILD_PATH := $(HOME)/devel/wxWidgets/build-cocoa-debug
  else
    WX_BUILD_PATH := $(HOME)/devel/wxWidgets/build-cocoa-release
  endif
endif

WX_CONFIG := $(WX_BUILD_PATH)/wx-config

CXXFLAGS := $(CFLAGS)
CXXFLAGS += -pipe -Wall -std=c++11 $(shell $(WX_CONFIG) --cxxflags) -DwxHAS_IMAGES_IN_RESOURCES
CXXFLAGS += -I$(HOME)/devel/cryptopp
LDFLAGS += $(shell $(WX_CONFIG) --libs)
LDFLAGS += -L$(HOME)/devel/cryptopp -lcryptopp

OBJS := $(CPP_SRCS:.cpp=.o) $(C_SRCS:.c=.o)

DEPFILE := .depend

.PHONY: all clean dep

ifeq ($(OS), Darwin)
.PHONY: app_bundle

APPDIR := $(TARGET).app
CTSDIR := $(APPDIR)/Contents
EXEDIR := $(CTSDIR)/MacOS
RCSDIR := $(CTSDIR)/Resources
APPICON := macos.icns
PNGS := $(subst res,$(RCSDIR),$(wildcard res/*.png))
APPLCS := $(subst .po,.lproj/ha.mo,$(subst i18n,$(RCSDIR),$(LCS)))

all: app_bundle

app_bundle: \
  $(CTSDIR)/Info.plist \
  $(CTSDIR)/PkgInfo \
  $(EXEDIR)/$(TARGET) \
  $(RCSDIR)/$(APPICON) \
  $(PNGS) \
  $(APPLCS)

$(CTSDIR)/Info.plist: res/Info.plist.in
	-mkdir -p $(dir $@)
	cat $< \
	  | sed -e 's/EXECUTABLE/$(TARGET)/' \
	  | sed -e 's/VERSION/$(VERSION)/' \
	  | sed -e 's/YEAR/$(YEAR)/' \
	  > $@

$(CTSDIR)/PkgInfo:
	-mkdir -p $(dir $@)
	echo "APPL????\c" > $@

$(EXEDIR)/$(TARGET): $(TARGET)
	-mkdir -p $(dir $@)
	SetFile -t APPL $(TARGET)
	cp $< $@

$(RCSDIR)/$(APPICON): res/macos.iconset
	-mkdir -p $(dir $@)
	iconutil -c icns -o $@ $<

$(RCSDIR)/%.png: res/%.png
	-mkdir -p $(dir $@)
	cp $< $@

$(RCSDIR)/%.lproj/ha.mo: i18n/%.mo
	-mkdir -p $(dir $@)
	cp $< $@
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
	-rm -f $(DEPFILE)
ifeq ($(OS), Darwin)
	-rm -rf $(APPDIR)
endif

ifeq ($(DEPFILE), $(wildcard $(DEPFILE)))
  include $(DEPFILE)
endif
