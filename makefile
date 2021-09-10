SHELL = /bin/sh

PKG_CONFIG ?= pkg-config
GUI_LIBS += lv2 x11 cairo
LV2_LIBS += lv2
ifneq ($(shell $(PKG_CONFIG) --exists fontconfig || echo no), no)
  override GUI_LIBS += fontconfig
  override GUIPPFLAGS += -DPKG_HAVE_FONTCONFIG
endif

CC ?= gcc
CXX ?= g++
INSTALL ?= install
INSTALL_PROGRAM ?= $(INSTALL)
INSTALL_DATA ?= $(INSTALL) -m644
STRIP ?= strip

PREFIX ?= /usr/local
LV2DIR ?= $(PREFIX)/lib/lv2

OPTIMIZATIONS ?=-O3 -ffast-math
CFLAGS ?=-Wall
CXXFLAGS ?=-Wall
STRIPFLAGS ?=-s
LDFLAGS ?=-Wl,-Bstatic -Wl,-Bdynamic -Wl,--as-needed

override CFLAGS += -std=c99 -fvisibility=hidden -fPIC
override CXXFLAGS += -std=c++11 -fvisibility=hidden -fPIC
override LDFLAGS += -shared -pthread

override GUIPPFLAGS += -DPUGL_HAVE_CAIRO
DSPCFLAGS += `$(PKG_CONFIG) --cflags $(LV2_LIBS)`
GUICFLAGS += `$(PKG_CONFIG) --cflags $(GUI_LIBS)`
DSPLIBS += -lm `$(PKG_CONFIG) --libs $(LV2_LIBS)`
GUILIBS += -lm `$(PKG_CONFIG) --libs $(GUI_LIBS)`

ifeq ($(shell test -e src/Locale_$(LANGUAGE).hpp && echo -n yes),yes)
  override GUIPPFLAGS += -DLOCALEFILE=\"Locale_$(LANGUAGE).hpp\"
endif

ifdef WWW_BROWSER_CMD
  override GUIPPFLAGS += -DWWW_BROWSER_CMD=\"$(WWW_BROWSER_CMD)\"
endif

BUNDLE = BSEQuencer.lv2
DSP = BSEQuencer
DSP_SRC = ./src/BSEQuencer.cpp
GUI = BSEQuencer_GUI
GUI_SRC = ./src/BSEQuencer_GUI.cpp
OBJ_EXT = .so
DSP_OBJ = $(DSP)$(OBJ_EXT)
GUI_OBJ = $(GUI)$(OBJ_EXT)
B_OBJECTS = $(addprefix $(BUNDLE)/, $(DSP_OBJ) $(GUI_OBJ))
FILES = *.ttl surface.png DrumSymbol.png NoteSymbol.png EditSymbol.png ScaleEditor.png LICENSE
B_FILES = $(addprefix $(BUNDLE)/, $(FILES))

DSP_INCL = src/BUtilities/stof.cpp

GUI_CXX_INCL = \
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
	src/BWidgets/VSliderValue.cpp \
	src/BWidgets/VSlider.cpp \
	src/BWidgets/VScale.cpp \
	src/BWidgets/HSliderValue.cpp \
	src/BWidgets/HSlider.cpp \
	src/BWidgets/HScale.cpp \
	src/BWidgets/RangeWidget.cpp \
	src/BWidgets/ValueWidget.cpp \
	src/BWidgets/Knob.cpp \
	src/BWidgets/Label.cpp \
	src/BWidgets/Window.cpp \
	src/BWidgets/Widget.cpp \
	src/BWidgets/BStyles.cpp \
	src/BWidgets/BColors.cpp \
	src/BWidgets/BItems.cpp \
	src/BUtilities/to_string.cpp \
	src/BUtilities/stof.cpp \
	src/BUtilities/vsystem.cpp

GUI_C_INCL = \
	src/screen.c \
	src/BWidgets/cairoplus.c \
	src/BWidgets/pugl/implementation.c \
	src/BWidgets/pugl/x11_stub.c \
	src/BWidgets/pugl/x11_cairo.c \
	src/BWidgets/pugl/x11.c

ifeq ($(shell $(PKG_CONFIG) --exists 'lv2 >= 1.12.4' || echo no), no)
  $(error lv2 >= 1.12.4 not found. Please install lv2 >= 1.12.4 first.)
endif
ifeq ($(shell $(PKG_CONFIG) --exists 'x11 >= 1.6.0' || echo no), no)
  $(error x11 >= 1.6.0 not found. Please install x11 >= 1.6.0 first.)
endif
ifeq ($(shell $(PKG_CONFIG) --exists 'cairo >= 1.12.0' || echo no), no)
  $(error cairo >= 1.12.0 not found. Please install cairo >= 1.12.0 first.)
endif

$(BUNDLE): clean $(DSP_OBJ) $(GUI_OBJ)
	@cp $(FILES) $(BUNDLE)

all: $(BUNDLE)

$(DSP_OBJ): $(DSP_SRC)
	@echo -n Build $(BUNDLE) DSP...
	@mkdir -p $(BUNDLE)
	@$(CXX) $(CPPFLAGS) $(OPTIMIZATIONS) $(CXXFLAGS) $(LDFLAGS) $(DSPCFLAGS) -Wl,--start-group $(DSPLIBS) $< $(DSP_INCL) -Wl,--end-group -o $(BUNDLE)/$@
	@$(STRIP) $(STRIPFLAGS) $(BUNDLE)/$@
	@echo \ done.

$(GUI_OBJ): $(GUI_SRC)
	@echo -n Build $(BUNDLE) GUI...
	@mkdir -p $(BUNDLE)
	@mkdir -p $(BUNDLE)/tmp
	@cd $(BUNDLE)/tmp; $(CC) $(CPPFLAGS) $(GUIPPFLAGS) $(CFLAGS) $(GUICFLAGS) $(addprefix ../../, $(GUI_C_INCL)) -c
	@cd $(BUNDLE)/tmp; $(CXX) $(CPPFLAGS) $(GUIPPFLAGS) $(CXXFLAGS) $(GUICFLAGS) $(addprefix ../../, $< $(GUI_CXX_INCL)) -c
	@$(CXX) $(CPPFLAGS) $(GUIPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $(GUICFLAGS) -Wl,--start-group $(GUILIBS) $(BUNDLE)/tmp/*.o -Wl,--end-group -o $(BUNDLE)/$@
	@$(STRIP) $(STRIPFLAGS) $(BUNDLE)/$@
	@rm -rf $(BUNDLE)/tmp
	@echo \ done.

install:
	@echo -n Install $(BUNDLE) to $(DESTDIR)$(LV2DIR)...
	@$(INSTALL) -d $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	@$(INSTALL_PROGRAM) -m755 $(B_OBJECTS) $(DESTDIR)$(LV2DIR)/$(BUNDLE)
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

.PHONY: all install uninstall clean

.NOTPARALLEL:
