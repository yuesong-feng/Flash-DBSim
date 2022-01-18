SRC=$(wildcard src/*.cpp)
PROG=$(wildcard *.cpp)

main:
	g++ -std=c++11 $(SRC) $(PROG) -o run