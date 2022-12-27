#include "Direction.h"

Direction KeyToDirection(const SDL_Scancode key) {
	return keysAsDirections.at(key);
}

std::tuple<int, int> DirectionToVector(const Direction direction) {
	return directionsAsVectors.at(direction);
}

std::vector<SDL_Scancode> AllKeysFor(const Direction direction) {
	std::vector<SDL_Scancode> keys;

	for (const auto& [key, value] : keysAsDirections) {
		if (value == direction)
			keys.push_back(key);
	}

	return keys;
}