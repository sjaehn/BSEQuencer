BUNDLE = BSEQuencer.lv2
INSTALL_DIR = /usr/lib/lv2
CC = g++
TK = src/BWidgets/TextToggleButton.cpp src/BWidgets/ToggleButton.cpp src/BWidgets/HSwitch.cpp src/BWidgets/PopupListBox.cpp src/BWidgets/ListBox.cpp src/BWidgets/ChoiceBox.cpp src/BWidgets/ItemBox.cpp src/BWidgets/UpButton.cpp src/BWidgets/DownButton.cpp src/BWidgets/Button.cpp src/BWidgets/DrawingSurface.cpp src/BWidgets/DisplayDial.cpp src/BWidgets/Dial.cpp src/BWidgets/DisplayHSlider.cpp src/BWidgets/HSlider.cpp src/BWidgets/HScale.cpp src/BWidgets/RangeWidget.cpp src/BWidgets/ValueWidget.cpp src/BWidgets/Knob.cpp src/BWidgets/Label.cpp src/BWidgets/Widget.cpp src/BWidgets/BStyles.cpp src/BWidgets/BColors.cpp src/BWidgets/BEvents.cpp src/BWidgets/BValues.cpp src/BWidgets/cairoplus.c src/BWidgets/pugl/pugl_x11.c

$(BUNDLE): clean BSEQuencer.so BSEQuencer_GUI.so
	cp manifest.ttl BSEQuencer.ttl surface.png LICENSE $(BUNDLE)
	
all: $(BUNDLE)

BSEQuencer.so: ./src/BSEQuencer.cpp
	mkdir -p $(BUNDLE)
	$(CC) $< -o $(BUNDLE)/$@ -shared -fPIC -DPIC `pkg-config --cflags --libs lv2`
	
BSEQuencer_GUI.so: ./src/BSEQuencer_GUI.cpp	
	mkdir -p $(BUNDLE)	
	$(CC) $< $(TK) -o $(BUNDLE)/$@ -shared -DPUGL_HAVE_CAIRO -fPIC -DPIC `pkg-config --cflags --libs lv2 x11 cairo`

install: $(BUNDLE)
	mkdir -p $(INSTALL_DIR)
	rm -rf $(INSTALL_DIR)/$(BUNDLE)
	cp -R $(BUNDLE) $(INSTALL_DIR)
	
.PHONY: all

clean:
	rm -rf $(BUNDLE)
