SRC=$(wildcard src/*.cpp)

main:
	clang++ $(SRC) BufferAlogrithm.cpp LRU.cpp main.cpp