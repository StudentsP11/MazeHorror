#include <iostream>

#include "SDL.h"

int SDL_main(int argc, char ** argv) {
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return 0;
	}

	SDL_Window* window = SDL_CreateWindow("Maze Horror", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, 100, 100,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

  SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}