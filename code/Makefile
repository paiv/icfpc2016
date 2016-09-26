
.PHONY: all release debug clean

all: release

release: CMAKE_ARGS=-DCMAKE_BUILD_TYPE=Release
release: TARGET_DIR=build/release

debug: CMAKE_ARGS=-DCMAKE_BUILD_TYPE=Debug
debug: TARGET_DIR=build/debug

debug release: solve

solve: $(TARGET_DIR)/solve
.PHONY: solve

$(TARGET_DIR)/solve: $(wildcard solver/*.cpp) $(wildcard solver/*.hpp) $(TARGET_DIR)/Makefile
		cd $(TARGET_DIR) && $(MAKE)

$(TARGET_DIR)/Makefile:
	echo $@
	mkdir -p $(TARGET_DIR) \
		&& cd $(TARGET_DIR) \
		&& cmake $(CMAKE_ARGS) ../../solver

clean:
	[ -d build ] && rm -r build
