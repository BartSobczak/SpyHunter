all:
	g++ -I ./SDL2-2.0.10/include -L ./SDL2-2.0.10/lib -o main main.cpp -lSDL2main -lSDL2

