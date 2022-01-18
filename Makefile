SRC=$(wildcard src/*.cpp)
PROG=$(wildcard *.cpp)

main:
	g++ -std=c++11 $(SRC) $(PROG) -o run

lib:
	g++ -std=c++11 -shared $(SRC) -o Flash-DBSim.o