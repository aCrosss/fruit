# Compiler
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2

# Directories
SRC_DIR := src
CORE_DIR := $(SRC_DIR)/core
GUI_DIR := $(SRC_DIR)/gui
CLI_DIR := $(SRC_DIR)/cli
BUILD_DIR := build
BIN_DIR := bin

# Internal includes
INCLUDES := -I$(SRC_DIR) -I$(CORE_DIR) -I$(GUI_DIR) -I$(CLI_DIR)

# Output binary
CLI_BIN := $(BIN_DIR)/fruit-cli
GUI_BIN := $(BIN_DIR)/fruit-gui
GUI_EXT_BIN := $(BIN_DIR)/fruit-gui-external

# Libs flags
GTKMM_FLAGS := $(shell pkg-config --cflags --libs gtkmm-3.0)
GPGME_FLAGS := $(shell pkg-config --cflags --libs gpgme)

# Sources
CORE_SRCS := $(wildcard $(CORE_DIR)/*.cpp)
CLI_SRCS := $(wildcard $(CLI_DIR)/*.cpp)
GUI_SRCS := $(wildcard $(GUI_DIR)/*.cpp)

# Remove gres.c из GUI_SRCS
GUI_SRCS := $(filter-out $(GUI_DIR)/gres.c, $(GUI_SRCS))

# .o files
CORE_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(CORE_SRCS))
CLI_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(CLI_SRCS))
GUI_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(GUI_SRCS))

# Generated resource file
GRES_SRC := $(GUI_DIR)/gres.c
GRES_OBJ := $(BUILD_DIR)/gui/gres.o

.PHONY: all cli gui gui_external clean

all: cli gui

# Build CLI
cli: $(CLI_BIN)

$(CLI_BIN): $(CORE_OBJS) $(CLI_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@ $(GPGME_FLAGS)

# Build GUI
gui: $(GUI_BIN)

$(GUI_BIN): $(GRES_SRC) $(CORE_OBJS) $(GUI_OBJS) $(GRES_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@ $(GTKMM_FLAGS) $(GPGME_FLAGS)

# Build GUI external
gui_external: $(GUI_EXT_BIN)

$(GUI_EXT_BIN): CXXFLAGS += -DGUI_EXTERNAL
$(GUI_EXT_BIN): $(GRES_SRC) $(CORE_OBJS) $(GUI_OBJS) $(GRES_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@ $(GTKMM_FLAGS) $(GPGME_FLAGS)
	@cp $(GUI_DIR)/gres.xml $(BIN_DIR)/gres.xml

# Compile resource file
$(GRES_SRC): $(GUI_DIR)/gres.xml
	glib-compile-resources --target=$@ --generate-source $<

# Build compiled resource file gres.c
$(GRES_OBJ): $(GRES_SRC)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@ $(GTKMM_FLAGS)

# Compile .cpp
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@ $(GPGME_FLAGS)

# Add gtkmm flags for GUI build
$(BUILD_DIR)/gui/%.o: $(GUI_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@ $(GTKMM_FLAGS) $(GPGME_FLAGS)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR) $(GRES_SRC)

# Dependencies
-include $(CORE_OBJS:.o=.d)
-include $(CLI_OBJS:.o=.d)
-include $(GUI_OBJS:.o=.d)
-include $(GRES_OBJ:.o=.d)

$(BUILD_DIR)/%.d: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -MM -MT '$(@:.d=.o) $@' $< > $@

$(BUILD_DIR)/gui/%.d: $(GUI_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -MM -MT '$(@:.d=.o) $@' $< > $@