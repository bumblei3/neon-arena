CXX=g++
CXXFLAGS=-O2 -Wall -std=c++17 $(shell pkg-config --cflags sdl2)
LDFLAGS=$(shell pkg-config --libs sdl2) -lGL -lm

neon-arena: main.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f neon-arena

.PHONY: clean
