# Compiler
CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2
SANITIZE ?=

ifneq ($(strip $(SANITIZE)),)
	CXXFLAGS += -fsanitize=$(SANITIZE) -fno-omit-frame-pointer
endif

# Directories
SRC_DIR               := src
CORE_DIR              := $(SRC_DIR)/core
AREAS_DIR             := $(CORE_DIR)/areas
MRECORDS_DIR          := $(AREAS_DIR)/mrecords
GUI_DIR               := $(SRC_DIR)/gui
GUI_AREAS_DIR         := $(GUI_DIR)/gui_areas
CLI_DIR               := $(SRC_DIR)/cli
IMGUI_APP_DIR         := $(SRC_DIR)/imgui
THIRD_PARTY_DIR       := third_party
IMGUI_DIR             := $(THIRD_PARTY_DIR)/imgui
IMGUI_BACKENDS_DIR    := $(IMGUI_DIR)/backends
IMGUI_FILE_DIALOG_DIR := $(THIRD_PARTY_DIR)/ImGuiFileDialog
GLFW_DIR              := $(THIRD_PARTY_DIR)/glfw
BUILD_DIR             := $(if $(strip $(SANITIZE)),build/sanitize-$(SANITIZE),build)
BIN_DIR               := $(if $(strip $(SANITIZE)),bin/sanitize-$(SANITIZE),bin)

# Internal includes
INCLUDES := -I$(SRC_DIR) -I$(CORE_DIR) -I$(GUI_DIR) -I$(CLI_DIR)

# Output binary
CLI_BIN        := $(BIN_DIR)/fruit-cli
GUI_BIN        := $(BIN_DIR)/fruit-gui
GUI_STATIC_BIN := $(BIN_DIR)/fruit-gui-static
IMGUI_BIN      := $(BIN_DIR)/fruit-imgui

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
CORE_SRCS      := $(wildcard $(CORE_DIR)/*.cpp)
AREAS_SRCS     := $(wildcard $(AREAS_DIR)/*.cpp)
MRECORDS_SRCS  := $(wildcard $(MRECORDS_DIR)/*.cpp)
GUI_AREAS_SRCS := $(wildcard $(GUI_AREAS_DIR)/*.cpp)
CLI_SRCS       := $(wildcard $(CLI_DIR)/*.cpp)
GUI_SRCS       := $(wildcard $(GUI_DIR)/*.cpp)
IMGUI_APP_SRCS := $(wildcard $(IMGUI_APP_DIR)/*.cpp)
IMGUI_SRCS     := $(IMGUI_DIR)/imgui.cpp                       \
                  $(IMGUI_DIR)/imgui_draw.cpp                  \
                  $(IMGUI_DIR)/imgui_tables.cpp                \
                  $(IMGUI_DIR)/imgui_widgets.cpp               \
                  $(IMGUI_BACKENDS_DIR)/imgui_impl_glfw.cpp    \
                  $(IMGUI_BACKENDS_DIR)/imgui_impl_opengl3.cpp \
                  $(IMGUI_FILE_DIALOG_DIR)/ImGuiFileDialog.cpp

# Remove gres.c из GUI_SRCS
GUI_SRCS := $(filter-out $(GUI_DIR)/gres.c, $(GUI_SRCS))

# .o files
CORE_OBJS      := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(CORE_SRCS))
AREAS_OBJS     := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(AREAS_SRCS))
MRECORDS_OBJS  := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(MRECORDS_SRCS))
GUI_AREAS_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(GUI_AREAS_SRCS))
CLI_OBJS       := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(CLI_SRCS))
GUI_OBJS       := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(GUI_SRCS))
IMGUI_APP_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(IMGUI_APP_SRCS))
IMGUI_OBJS     := $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(IMGUI_SRCS))

IMGUI_INCLUDES := -I$(SRC_DIR) -I$(CORE_DIR) -I$(IMGUI_DIR) -I$(IMGUI_BACKENDS_DIR) -I$(IMGUI_FILE_DIALOG_DIR) -I$(GLFW_DIR)/include
IMGUI_CPPFLAGS := -DUSE_STD_FILESYSTEM
IMGUI_LDFLAGS  ?=

IMGUI_RELEASE_LDFLAGS ?= -static-libgcc -static-libstdc++ -s
IMGUI_RELEASE_BIN     := $(BIN_DIR)/fruit-imgui-release

GLFW_FILES    := $(shell find $(GLFW_DIR) -type f)
TARGET_TRIPLE := $(shell $(CXX) -dumpmachine 2>/dev/null)

ifneq ($(findstring mingw,$(TARGET_TRIPLE)),)
	GLFW_BUILD_VARIANT    := win32
	IMGUI_BIN             := $(BIN_DIR)/fruit-imgui.exe
	IMGUI_RELEASE_BIN     := $(BIN_DIR)/fruit-imgui-release.exe
	GLFW_PLATFORM_OPTIONS := -DCMAKE_SYSTEM_NAME=Windows -DGLFW_BUILD_WIN32=ON
	IMGUI_PLATFORM_LIBS    = -lopengl32 -lgdi32 -luser32 -lshell32
else
	GLFW_BUILD_WAYLAND    ?= OFF
	GLFW_BUILD_VARIANT    := x11-wayland-$(GLFW_BUILD_WAYLAND)
	GLFW_PLATFORM_OPTIONS := -DGLFW_BUILD_X11=ON -DGLFW_BUILD_WAYLAND=$(GLFW_BUILD_WAYLAND)
	GLFW_PKGCONFIG_DIR     = $(abspath $(GLFW_BUILD_DIR)/src)
	GLFW_STATIC_FLAGS      = $$(PKG_CONFIG_LIBDIR="$(GLFW_PKGCONFIG_DIR)" pkg-config --static --libs glfw3 2>/dev/null | sed 's/-lglfw3//g')
	OPENGL_LIBS           := $(shell pkg-config --libs opengl 2>/dev/null)

	ifeq ($(strip $(OPENGL_LIBS)),)
		OPENGL_LIBS := -lGL
	endif

	IMGUI_PLATFORM_LIBS = $(OPENGL_LIBS) $(GLFW_STATIC_FLAGS) -pthread
endif

GLFW_BUILD_DIR := $(BUILD_DIR)/glfw/$(GLFW_BUILD_VARIANT)
GLFW_LIB       := $(GLFW_BUILD_DIR)/src/libglfw3.a

GLFW_CMAKE_OPTIONS := -DBUILD_SHARED_LIBS=OFF \
	-DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF \
	-DGLFW_BUILD_DOCS=OFF -DGLFW_INSTALL=OFF \
	-DCMAKE_C_COMPILER=$(CC) $(GLFW_PLATFORM_OPTIONS) \
	$(if $(strip $(GLFW_CMAKE_C_FLAGS)),-DCMAKE_C_FLAGS=$(GLFW_CMAKE_C_FLAGS))

# Generated resource file
GRES_SRC := $(GUI_DIR)/gres.c
GRES_OBJ := $(BUILD_DIR)/gui/gres.o

.PHONY: all cli gui gui-static imgui fruit-imgui imgui-release fruit-imgui-release clean remove_res

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

imgui fruit-imgui: $(IMGUI_BIN)

imgui-release fruit-imgui-release: $(IMGUI_RELEASE_BIN)

$(IMGUI_BIN): $(CORE_OBJS) $(AREAS_OBJS) $(MRECORDS_OBJS) $(IMGUI_APP_OBJS) $(IMGUI_OBJS) $(GLFW_LIB)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(IMGUI_INCLUDES) $^ -o $@ $(IMGUI_LDFLAGS) $(IMGUI_PLATFORM_LIBS)

$(IMGUI_RELEASE_BIN): $(CORE_OBJS) $(AREAS_OBJS) $(MRECORDS_OBJS) $(IMGUI_APP_OBJS) $(IMGUI_OBJS) $(GLFW_LIB)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(IMGUI_INCLUDES) $^ -o $@ $(IMGUI_LDFLAGS) $(IMGUI_RELEASE_LDFLAGS) $(IMGUI_PLATFORM_LIBS)

$(GLFW_LIB): $(GLFW_FILES) Makefile
	cmake -S $(GLFW_DIR) -B $(GLFW_BUILD_DIR) $(GLFW_CMAKE_OPTIONS)
	cmake --build $(GLFW_BUILD_DIR) --target glfw
	@touch $(GLFW_LIB)

# Compile resource file
$(GRES_SRC): $(GUI_DIR)/gres.xml $(GUI_DIR)/iface.ui
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

$(BUILD_DIR)/imgui/%.o: $(IMGUI_APP_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(IMGUI_CPPFLAGS) $(IMGUI_INCLUDES) -c $< -o $@

$(BUILD_DIR)/third_party/%.o: $(THIRD_PARTY_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(IMGUI_CPPFLAGS) $(IMGUI_INCLUDES) -w -c $< -o $@

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
-include $(IMGUI_APP_OBJS:.o=.d)
-include $(GRES_OBJ:.o=.d)

$(BUILD_DIR)/%.d: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -MM -MT '$(@:.d=.o) $@' $< > $@

$(BUILD_DIR)/gui/%.d: $(GUI_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -MM -MT '$(@:.d=.o) $@' $< > $@

$(BUILD_DIR)/imgui/%.d: $(IMGUI_APP_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(IMGUI_CPPFLAGS) $(IMGUI_INCLUDES) -MM -MT '$(@:.d=.o) $@' $< > $@
