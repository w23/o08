ROOT=.
KAPUSHA_ROOT=$(ROOT)/3p/kapusha
include $(KAPUSHA_ROOT)/common.mk

SOURCES := \
	Logic.cpp \
	proto.cpp

MODULES=$(SOURCES:.cpp=.o)
DEPENDS=$(SOURCES:.cpp=.d)

.PHONY: all clean

proto: $(MODULES) 3p/kapusha/libkapusha.a
	$(LD) $(MODULES) $(LDFLAGS) -lkapusha -L$(KAPUSHA_ROOT) -o $@

3p/kapusha/libkapusha.a:
	make -C $(KAPUSHA_ROOT)

depend: $(DEPENDS)

-include $(DEPENDS)

clean:
	make -C $(KAPUSHA_ROOT) clean
	@rm -f $(MODULES) $(DEPENDS) proto
