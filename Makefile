# define this to turn on debug
DEBUG ?= y

# Application name
TARGET := HomeAccount
VERSION := 0.1
YEAR := 2020

# Source files
CPP_SRCS := \
  $(wildcard *.cpp) \
  $(wildcard file/*.cpp) \
  $(wildcard file/except/*.cpp)
C_SRCS := \
  $(wildcard c/*.c)
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
all: $(TARGET).app
else
all: $(TARGET)
endif

ifeq ($(OS), Darwin)
APP := $(TARGET).app
APP_ICON := res/macos.icns

$(APP): $(TARGET) res/Info.plist.in $(APP_ICON)
	SetFile -t APPL $(TARGET)
	-mkdir $@
	-mkdir $@/Contents
	-mkdir $@/Contents/MacOS
	-mkdir $@/Contents/Resources
	cat res/Info.plist.in \
	  | sed -e 's/EXECUTABLE/$(TARGET)/' \
	  | sed -e 's/VERSION/$(VERSION)/' \
	  | sed -e 's/YEAR/$(YEAR)/' \
	  > $@/Contents/Info.plist
	echo "APPL????\c" > $@/Contents/PkgInfo
	cp $(TARGET) $@/Contents/MacOS/
	cp $(APP_ICON) $@/Contents/Resources
	cp res/*.png $@/Contents/Resources

$(APP_ICON): res/macos.iconset
	iconutil -c icns -o $@ $^

endif

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
	-rm -rf $(APP)
	-rm -f $(APP_ICON)
endif

ifeq ($(DEPFILE), $(wildcard $(DEPFILE)))
  include $(DEPFILE)
endif
