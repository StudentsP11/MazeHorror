#ifndef DIRECTION_H
#define DIRECTION_H

#include <SDL2/SDL.h>
#include <map>
#include <vector>
#include <array>

enum class Direction {
	DOWN,
	UP,
	LEFT,
	RIGHT
};

const std::array<Direction, 4> DIRECTIONS{
	Direction::DOWN,
	Direction::UP,
	Direction::LEFT,
	Direction::RIGHT
};

const std::map<SDL_Scancode, Direction> keysAsDirections{
		{SDL_SCANCODE_S, Direction::DOWN}, {SDL_SCANCODE_DOWN, Direction::DOWN},
		{SDL_SCANCODE_W, Direction::UP}, {SDL_SCANCODE_UP, Direction::UP},
		{SDL_SCANCODE_A, Direction::LEFT}, {SDL_SCANCODE_LEFT, Direction::LEFT},
		{SDL_SCANCODE_D, Direction::RIGHT}, {SDL_SCANCODE_RIGHT, Direction::RIGHT}
};

const std::map<Direction, std::tuple<int, int>> directionsAsVectors{
	{Direction::DOWN, {0, 1} },
	{Direction::UP, {0, -1} },
	{Direction::LEFT, {-1, 0} },
	{Direction::RIGHT, {1, 0} }
};

Direction KeyToDirection(const SDL_Scancode key);

std::tuple<int, int> DirectionToVector(const Direction direction);

std::vector<SDL_Scancode> AllKeysFor(const Direction direction);

#endif // #ifndef DIRECTION_H