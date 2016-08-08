
.PHONY: all clean

all: solve

solve: $(wildcard solver/*.cpp) $(wildcard solver/*.hpp)
	mkdir -p build
	cd build && cmake -DCMAKE_BUILD_TYPE=Release ../solver
	cd build && make

clean:
	rm -r build
