BUILD_DIR := ../build

SOURCE_FILES := $(shell find . -name '*.cpp')
HEADER_FILES := $(shell find . -name '*.h')

BUILD_FILES := $(addprefix $(BUILD_DIR)/, $(SOURCE_FILES:%.cpp=%.o))

LDFLAGS := -shared -Wl,--output-def=libscr -Wl,--out-implib=libscr -Wl,--dll -static
LDENDFLAGS := -m32 -luser32 -lpsapi -lwinmm
CPPFLAGS := -Wall -D"BUILD_DLL" -g -m32 -std=c++14 -I./

scr.asi: $(BUILD_FILES)
	i686-w64-mingw32-g++-posix $(LDFLAGS) -o $(BUILD_DIR)/$@ $(BUILD_FILES) $(LDENDFLAGS)
	cp $(BUILD_DIR)/$@ "/home/druckis3000/Games/GTA San Andreas/GTA San Andreas/scr.asi"

safe: $(BUILD_FILES)
	i686-w64-mingw32-g++-posix $(LDFLAGS) -o $(BUILD_DIR)/$@ $(BUILD_FILES) $(LDENDFLAGS)
	
$(BUILD_DIR)/%.o: %.cpp %.h
	if [ ! -d $(dir $@) ]; then mkdir $(dir $@); fi
	i686-w64-mingw32-g++-posix $(CPPFLAGS) -c -o $@ $<

cleanx:
	rm $(BUILD_DIR)/game/gtasa.o
	rm $(BUILD_DIR)/main.o
	rm $(BUILD_DIR)/game/samp.o
	rm $(BUILD_DIR)/scr.asi

clean:
	rm -r $(BUILD_DIR)/*
