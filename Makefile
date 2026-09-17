# Compiler
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -fsanitize=address

# Directories
SRC_DIR := src
CORE_DIR := $(SRC_DIR)/core
AREAS_DIR := $(CORE_DIR)/areas
MRECORDS_DIR := $(AREAS_DIR)/mrecords
GUI_DIR := $(SRC_DIR)/gui
GUI_AREAS_DIR := $(GUI_DIR)/gui_areas
CLI_DIR := $(SRC_DIR)/cli
BUILD_DIR := build
BIN_DIR := bin

# Internal includes
INCLUDES := -I$(SRC_DIR) -I$(CORE_DIR) -I$(GUI_DIR) -I$(CLI_DIR)

# Output binary
CLI_BIN := $(BIN_DIR)/fruit-cli
GUI_BIN := $(BIN_DIR)/fruit-gui
GUI_STATIC_BIN := $(BIN_DIR)/fruit-gui-static

STATIC_LIB_DIR := static_libs/
GTKMM_STATIC_LIBS := -L$(STATIC_LIB_DIR) \
	-lgtkmm-3.0 -lgdkmm-3.0 -latkmm-1.6 -lcairomm-1.0 -lpangomm-1.4

# Libs flags
GTKMM_FLAGS := $(shell pkg-config --cflags --libs gtkmm-3.0)
# Libs static flags
GTKMM_STATIC_FLAGS := -L$(STATIC_LIB_DIR) -Wl,-Bstatic $(GTKMM_STATIC_LIBS) -Wl,-Bdynamic \
	$(shell pkg-config --cflags --libs --static gtkmm-3.0) \
	-static-libgcc -static-libstdc++

# Sources
CORE_SRCS := $(wildcard $(CORE_DIR)/*.cpp)
AREAS_SRCS := $(wildcard $(AREAS_DIR)/*.cpp)
MRECORDS_SRCS := $(wildcard $(MRECORDS_DIR)/*.cpp)
GUI_AREAS_SRCS := $(wildcard $(GUI_AREAS_DIR)/*.cpp)
CLI_SRCS := $(wildcard $(CLI_DIR)/*.cpp)
GUI_SRCS := $(wildcard $(GUI_DIR)/*.cpp)

# Remove gres.c из GUI_SRCS
GUI_SRCS := $(filter-out $(GUI_DIR)/gres.c, $(GUI_SRCS))

# .o files
CORE_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(CORE_SRCS))
AREAS_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(AREAS_SRCS))
MRECORDS_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(MRECORDS_SRCS))
GUI_AREAS_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(GUI_AREAS_SRCS))
CLI_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(CLI_SRCS))
GUI_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(GUI_SRCS))

# Generated resource file
GRES_SRC := $(GUI_DIR)/gres.c
GRES_OBJ := $(BUILD_DIR)/gui/gres.o

.PHONY: all cli gui gui-static clean remove_res

all: cli gui

# Build CLI
cli: $(CLI_BIN)

$(CLI_BIN): $(CORE_OBJS) $(AREAS_OBJS) $(MRECORDS_OBJS) $(CLI_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@

# Build GUI
gui: $(GUI_BIN)

$(GUI_BIN):  $(CORE_OBJS) $(AREAS_OBJS) $(MRECORDS_OBJS) $(GUI_OBJS) $(GUI_AREAS_OBJS) $(GRES_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@ $(GTKMM_FLAGS)

gui-static: $(GUI_STATIC_BIN)

$(GUI_STATIC_BIN): $(CORE_OBJS) $(AREAS_OBJS) $(MRECORDS_OBJS) $(GUI_OBJS) $(GUI_AREAS_OBJS) $(GRES_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@ $(GTKMM_STATIC_FLAGS)

# Compile resource file
$(GRES_SRC): $(GUI_DIR)/gres.xml
	glib-compile-resources --sourcedir=src/gui --target=$@ --generate-source $<

# Build compiled resource file gres.c
$(GRES_OBJ): $(GRES_SRC)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@ $(GTKMM_FLAGS)

# Compile .cpp
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Add gtkmm flags for GUI build
$(BUILD_DIR)/gui/%.o: $(GUI_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@ $(GTKMM_FLAGS)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR) $(GRES_SRC)

remove_res:
	rm $(GRES_SRC)

# Dependencies
-include $(CORE_OBJS:.o=.d)
-include $(AREAS_OBJS:.o=.d)
-include $(MRECORDS_OBJS:.o=.d)
-include $(GUI_AREAS_OBJS:.o=.d)
-include $(CLI_OBJS:.o=.d)
-include $(GUI_OBJS:.o=.d)
-include $(GRES_OBJ:.o=.d)

$(BUILD_DIR)/%.d: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -MM -MT '$(@:.d=.o) $@' $< > $@

$(BUILD_DIR)/gui/%.d: $(GUI_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -MM -MT '$(@:.d=.o) $@' $< > $@