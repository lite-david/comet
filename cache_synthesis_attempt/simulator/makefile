# vim: set ts=4 nu ai:

CC := g++
CFLAGS := -std=c++11
SRCDIR := src
BUILDDIR := build
COMMONDIR := ../common
TARGET := bin/simRISCV

SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
COMMONOBJ := $(COMMONDIR)/build/elfFile.o
INC := -I ./include -I ../common/include/

$(TARGET): $(OBJECTS) $(COMMONOBJ)
	@mkdir -p bin
	@echo "Linking..."
	@echo " $(CC) $^ -o $(TARGET) -D __USE_AC  "; $(CC) $^ -o $(TARGET) -D __USE_AC 

$(COMMONOBJ) :
	make -C $(COMMONDIR)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo $(OBJECTS)
	@echo $(SOURCES)
	@echo " $(CC) $(CFLAGS) $(INC) -D __USE_AC -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -D __USE_AC  -c -o $@ $<
	
clean:
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR) bin/ $(TARGET)"; $(RM) -r $(BUILDDIR) bin $(TARGET)

.PHONY: clean
