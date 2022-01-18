SRC=$(wildcard src/*.cpp)
PROG=$(wildcard *.cpp)

main:
	clang++ $(SRC) $(PROG) -o main