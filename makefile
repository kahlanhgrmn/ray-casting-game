.PHONY: build run clean makerun

build:
	cmake -S . -B build
	cmake --build build -j

run: build
	./build/MazeExplorer

makerun: build run

clean:
	rm -rf build