.PHONY: build run clean

build:
	cmake -S . -B build
	cmake --build build -j

run: build
	./build/MazeExplorer

clean:
	rm -rf build
