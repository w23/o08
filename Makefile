ROOT=.
KAPUSHA_ROOT=$(ROOT)/3p/kapusha
include $(KAPUSHA_ROOT)/common.mk

SOURCES := \
	CommandCenter.cpp \
	Field.cpp \
	Logic.cpp \
	Network.cpp \
	Patterns.cpp \
	Game.cpp

ifeq ($(KAPUSHA_WINDOWS),1)
SOURCES += \
	main_win.cpp \
	Socket_win.cpp
LDFLAGS += -lws2_32
else
SOURCES += \
	Socket_nix.cpp
endif

MODULES=$(SOURCES:.cpp=.o)
DEPENDS=$(SOURCES:.cpp=.d)

.PHONY: all clean

proto: $(MODULES) 3p/kapusha/libkapusha.a
	$(LD) $(MODULES) -lkapusha $(LDFLAGS) -L$(KAPUSHA_ROOT) -o $@

3p/kapusha/libkapusha.a:
	make -C $(KAPUSHA_ROOT)

depend: $(DEPENDS)

-include $(DEPENDS)

clean:
	make -C $(KAPUSHA_ROOT) clean
	@rm -f $(MODULES) $(DEPENDS) proto
