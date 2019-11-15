SHELL = /bin/sh

PKG_CONFIG ?= pkg-config
CXX ?= g++
INSTALL ?= install
INSTALL_PROGRAM ?= $(INSTALL)
INSTALL_DATA ?= $(INSTALL) -m644
STRIP ?= strip

PREFIX ?= /usr/local
LV2DIR ?= $(PREFIX)/lib/lv2

CPPFLAGS += -DPIC
CXXFLAGS += -std=c++11 -fvisibility=hidden -fPIC
LDFLAGS += -shared -Wl,-z,relro,-z,now
STRIPFLAGS += -s --strip-program=$(STRIP)

DSPFLAGS =
GUIFLAGS = -DPUGL_HAVE_CAIRO

DSPFLAGS += `$(PKG_CONFIG) --cflags --libs lv2`
GUIFLAGS += `$(PKG_CONFIG) --cflags --libs lv2 x11 cairo`

BUNDLE = BSEQuencer.lv2
DSP = BSEQuencer
DSP_SRC = ./src/BSEQuencer.cpp
GUI = BSEQuencer_GUI
GUI_SRC = ./src/BSEQuencer_GUI.cpp
OBJ_EXT = .so
DSP_OBJ = $(DSP)$(OBJ_EXT)
GUI_OBJ = $(GUI)$(OBJ_EXT)
B_OBJECTS = $(addprefix $(BUNDLE)/, $(DSP_OBJ) $(GUI_OBJ))
FILES = manifest.ttl BSEQuencer.ttl surface.png DrumSymbol.png NoteSymbol.png EditSymbol.png ScaleEditor.png LICENSE
B_FILES = $(addprefix $(BUNDLE)/, $(FILES))

GUI_INCL = \
	src/BWidgets/ImageIcon.cpp \
	src/BWidgets/Icon.cpp \
	src/BWidgets/HPianoRoll.cpp \
	src/BWidgets/PianoWidget.cpp \
	src/BWidgets/TextToggleButton.cpp \
	src/BWidgets/ToggleButton.cpp \
	src/BWidgets/TextButton.cpp \
	src/BWidgets/HSwitch.cpp \
	src/BWidgets/PopupListBox.cpp \
	src/BWidgets/ListBox.cpp \
	src/BWidgets/ChoiceBox.cpp \
	src/BWidgets/ItemBox.cpp \
	src/BWidgets/Text.cpp \
	src/BWidgets/UpButton.cpp \
	src/BWidgets/DownButton.cpp \
	src/BWidgets/Button.cpp \
	src/BWidgets/DrawingSurface.cpp \
	src/BWidgets/DialValue.cpp \
	src/BWidgets/Dial.cpp \
	src/BWidgets/HSliderValue.cpp \
	src/BWidgets/HSlider.cpp \
	src/BWidgets/HScale.cpp \
	src/BWidgets/RangeWidget.cpp \
	src/BWidgets/ValueWidget.cpp \
	src/BWidgets/Knob.cpp \
	src/BWidgets/Label.cpp \
	src/BWidgets/FocusWidget.cpp \
	src/BWidgets/Window.cpp \
	src/BWidgets/Widget.cpp \
	src/BWidgets/BStyles.cpp \
	src/BWidgets/BColors.cpp \
	src/BWidgets/BItems.cpp \
	src/BWidgets/BValues.cpp \
	src/screen.c \
	src/BWidgets/cairoplus.c \
	src/BWidgets/pugl/pugl_x11_cairo.c \
	src/BWidgets/pugl/pugl_x11.c

ifeq ($(shell $(PKG_CONFIG) --exists lv2 || echo no), no)
  $(error LV2 not found. Please install LV2 first.)
endif

ifeq ($(shell $(PKG_CONFIG) --exists x11 || echo no), no)
  $(error X11 not found. Please install X11 first.)
endif

ifeq ($(shell $(PKG_CONFIG) --exists cairo || echo no), no)
  $(error Cairo not found. Please install cairo first.)
endif

$(BUNDLE): clean $(DSP_OBJ) $(GUI_OBJ)
	@cp $(FILES) $(BUNDLE)

all: $(BUNDLE)

$(DSP_OBJ): $(DSP_SRC)
	@echo -n Build $(BUNDLE) DSP...
	@mkdir -p $(BUNDLE)
	@$(CXX) $< -o $(BUNDLE)/$@ $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $(DSPFLAGS)
	@echo \ done.

$(GUI_OBJ): $(GUI_SRC)
	@echo -n Build $(BUNDLE) GUI...
	@mkdir -p $(BUNDLE)
	@$(CXX) $< $(GUI_INCL) -o $(BUNDLE)/$@ $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $(GUIFLAGS)
	@echo \ done.

install:
	@echo -n Install $(BUNDLE) to $(DESTDIR)$(LV2DIR)...
	@$(INSTALL) -d $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	@$(INSTALL_PROGRAM) -m755 $(B_OBJECTS) $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	@$(INSTALL_DATA) $(B_FILES) $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	@cp -R $(BUNDLE) $(DESTDIR)$(LV2DIR)
	@echo \ done.

install-strip:
	@echo -n "Install (stripped)" $(BUNDLE) to $(DESTDIR)$(LV2DIR)...
	@$(INSTALL) -d $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	@$(INSTALL_PROGRAM) -m755 $(STRIPFLAGS) $(B_OBJECTS) $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	@$(INSTALL_DATA) $(B_FILES) $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	@cp -R $(BUNDLE) $(DESTDIR)$(LV2DIR)
	@echo \ done.

uninstall:
	@echo -n Uninstall $(BUNDLE)...
	@rm -f $(addprefix $(DESTDIR)$(LV2DIR)/$(BUNDLE)/, $(FILES))
	@rm -f $(DESTDIR)$(LV2DIR)/$(BUNDLE)/$(GUI_OBJ)
	@rm -f $(DESTDIR)$(LV2DIR)/$(BUNDLE)/$(DSP_OBJ)
	-@rmdir $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	@echo \ done.

clean:
	@rm -rf $(BUNDLE)

.PHONY: all install install-strip uninstall clean
