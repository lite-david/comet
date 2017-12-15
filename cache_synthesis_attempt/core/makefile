# vim: set ts=4 nu ai:

CC := g++
CFLAGS := -std=c++11 -g
SRCDIR := src
BUILDDIR := build
COMMONDIR := ../common
HLSTOOL ?= __CATAPULT # __VIVADO if you want to make for vivado HLS

SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
COMMONOBJ := $(COMMONDIR)/build/elfFile.o
INC := -I ./include -I ../common/include/

catapult: $(OBJECTS) $(COMMONOBJ)
	@mkdir -p bin
	@echo "Linking..."
	@echo " $(CC) $^ -o ./bin/catapult.sim  "; $(CC) $^ -o ./bin/catapult.sim -D $(HLSTOOL) -D __DEBUG__
	
$(COMMONOBJ):
	make -C $(COMMONDIR)
	
$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " $(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -D $(HLSTOOL) -D __DEBUG__ -D __SIMULATOR__ -c -o $@ $<

clean:
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR) bin "; $(RM) -r $(BUILDDIR) bin

.PHONY: clean
