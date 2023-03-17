#include <cmath>
#include <ctime>
#include <iostream>
#include <algorithm>
#include <map>
#include <vector>
#include <array>
#include <random>
#include <numbers>

// GRAPHICS (DO NOT TOUCH)
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>
#include <SDL2/sdl.h>
// END GRAPHICS
#include <numeric>
#include <valarray>
#include <SDL2/SDL_mixer.h>
#include "Direction.h"
#include "Maze.h"

#pragma comment(lib, "irrKlang.lib")

#define DEV

template<std::size_t X, std::size_t Y, std::size_t Z>
using matrix3d = std::array<std::array<std::array<float, Z>, Y>, X>;

template<std::size_t X, std::size_t Y>
using matrix2d = std::array<std::array<float, Y>, X>;

struct vec2d {
	float x;
	float y;
};

struct GameObject {
	vec2d position;
	vec2d size{ 0, 0 };
};

struct Entity {
	GameObject object;
	float viewDirectionAngle;
};

struct Maniac {
	Entity entity;
	float rightHandAngle, point_x, point_y;
};

struct Player {
	Entity entity;
	bool is_died = false;
};

bool is_wall_collided(const vec2d& player_position);

const char* SOUND_BENZOPILA = "Sounds/benzopila.wav";
const char* SOUND_KRIK = "Sounds/krik.wav";
const char* SOUND_SCARY = "Sounds/scary_sound.wav";

constexpr float pi = static_cast<float>(std::numbers::pi);

constexpr size_t map_width = 31;
constexpr size_t map_height = 31;
constexpr float angleDelta = 5;
constexpr float speedDelta = 0.05f;
constexpr float d0 = 0.25;

int k = 1;

int count = 0;

float wall_width = 0.5;
float wall_height = 3;

char** field = nullptr;

constexpr int check_points_number = 6;
constexpr float check_points[check_points_number][2] = { 
	{20, 1}, {20, 17}, {1, 7},
	{1, 10}, {1, 1}, {1, 20} 
};

matrix3d<6,4,3> current_cube;

Maniac maniac { 
	.entity {
		.object {
			.position {1, 4}, 
			.size {1,1}
		}, 
		.viewDirectionAngle = 0
	}, 
	.rightHandAngle = 0,
	.point_x = 0,
	.point_y = 0
};

Player player {
	.entity {
		.object {
			.position {1, 1},
			.size {1,1}
		},
		.viewDirectionAngle = 0
	},
	.is_died = false
};

Mix_Chunk* soundScary;
Mix_Chunk* soundKrik;
Mix_Chunk* soundBenzopila;

void Move(Entity& player, const Direction direction);

matrix2d<4, 3> RotatePolygon(
	const matrix2d<4, 3> & polygon,
	const float angle,
	const bool axis_y = true)
{
	constexpr size_t verts_amount = 4;
	matrix2d<verts_amount, 3> new_polygon{};
	for (size_t i = 0; i < verts_amount; i++)
	{
		const float x = polygon[i][0];
		const float y = polygon[i][1];
		const float z = polygon[i][2];
		if (axis_y)
		{
			const float new_x = x * cos(angle) + z * sin(angle);
			const float new_z = -x * sin(angle) + z * cos(angle);
			new_polygon[i][0] = new_x;
			new_polygon[i][1] = y;
			new_polygon[i][2] = new_z;
		}
		else {
			const float new_y = y * cos(angle) + z * sin(angle);
			const float new_z = -y * sin(angle) + z * cos(angle);
			new_polygon[i][0] = x;
			new_polygon[i][1] = new_y;
			new_polygon[i][2] = new_z;
		}
	}

	return new_polygon;
}

[[nodiscard]] bool GameObject_Sees(
	const GameObject& game_object1, 
	const GameObject& game_object2) 
{
	const float tg =
		(game_object1.position.y - game_object2.position.y)
		/ (game_object1.position.x - game_object2.position.y);

	const vec2d min_position = {
		std::min(game_object2.position.x, game_object1.position.x),
		std::min(game_object2.position.y, game_object1.position.y)
	};
	
	const vec2d	max_position = {
		std::max(game_object2.position.x, game_object1.position.x),
		0
	};

	for (float x = min_position.x; x < max_position.x; x += 0.5)
	{
		float y = game_object2.position.y + tg * (x - min_position.x);
		if (is_wall_collided({x, y}))
			return false;
	}

	return true;
}

[[nodiscard]] inline float vec2d_Dist(const vec2d& vec1, const vec2d& vec2)
{
	const float x = vec1.x - vec2.x;
	const float y = vec1.y - vec2.y;
	return SDL_sqrt(x * x + y * y);
}

vec2d check_cells(const Maniac& maniac) {
	const vec2d position = { 
		maniac.entity.object.position.x,
		maniac.entity.object.position.y 
	};

	float dx = 0, dy = 0;
	float minDist = SDL_sqrt(static_cast<double>(
		map_height * map_height + map_width * map_width
	));
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			// TODO: Fix that behavior, wrong algorithm
			const size_t y = static_cast<size_t>(position.y) + i - 1;
			const size_t x = static_cast<size_t>(position.x) + j - 1;

			// Handler that doesn't accept error indexes
			if (y >= map_height
				|| x >= map_width) {
				continue;
			}

			if (field[y][x] == '#' || (i == 1 && j == 1)) {
				continue;
			}
			
			const float distance = vec2d_Dist(
				{ maniac.point_x, maniac.point_y },
				{ static_cast<float>(x), static_cast<float>(y) }
			);
			if (minDist > distance) {
				minDist = distance;

				const float sine = (static_cast<float>(y) - position.y) 
					/ vec2d_Dist(
						{ position.x, position.y },
						{ static_cast<float>(x), static_cast<float>(y) }
					);
				const float cosine = (x - position.x)
					/ vec2d_Dist(
						{ position.x, position.y },
						{ static_cast<float>(x), static_cast<float>(y) }
					);
				dx = speedDelta * cosine / 5;
				dy = speedDelta * sine / 5;
			}
		}
	}

	return { dx, dy };
}

vec2d vec2d_Sum(const vec2d & vec1, const vec2d & vec2) {
	return {
		vec1.x + vec2.x,
		vec1.y + vec2.y
	};
}

void Maniac_Move(Maniac& maniac, const vec2d& player_position)
{
	maniac.rightHandAngle += static_cast<float>(k)
		* pi / 60;
	if (maniac.rightHandAngle > pi / 4)
	{
		maniac.rightHandAngle = pi / 4;
		k = -1;
	}
	else if (maniac.rightHandAngle < -pi / 4)
	{
		maniac.rightHandAngle = -pi / 4;
		k = 1;
	}

	if (GameObject_Sees(maniac.entity.object, 
		player.entity.object)) 
	{
		const float sine = (player_position.y 
			- maniac.entity.object.position.y
			)/ vec2d_Dist(maniac.entity.object.position, maniac.entity.object.position);
		const float cosine = (player_position.x - maniac.entity.object.position.x)
			/ vec2d_Dist(maniac.entity.object.position, maniac.entity.object.position);
		float angle = asin(sine);
		if (cosine > 0 && cos(angle) < 0 || cosine < 0 && cos(angle) > 0)
			angle = pi - angle;
		maniac.entity.viewDirectionAngle = pi - angle;
		maniac.point_x = player_position.x;
		maniac.point_y = player_position.y;
	}
	else {
		const float x = maniac.entity.object.position.x - maniac.point_x;
		const float y = maniac.entity.object.position.y - maniac.point_y;
		if ((x * x + y * y) <= 0.01f) {
			auto rng = std::random_device();
			std::uniform_int_distribution distribute(0, check_points_number);
			const int index = distribute(rng);
			maniac.point_x = check_points[index][0];
			maniac.point_y = check_points[index][1];
		}
	}

	const vec2d delta = check_cells(maniac);

	maniac.entity.object.position = vec2d_Sum(
		maniac.entity.object.position, 
		delta
	);
}

void draw_cuboid(const Maniac& maniac, float xr, float yr, float zr, 
	float x0, float y0, float z0, 
	float width_x, float width_y, float width_z, 
	std::vector<float> fillcolor, 
	std::vector<float> outcolor, 
	bool is_maniac_hands = false, bool forward = false)
{
	float x1 = x0-width_x / 2;
	float y1 = y0-width_y / 2;
	float z1 = z0-width_z / 2;
	float x2 = x0 + width_x / 2;
	float y2 = y0+width_y / 2;
	float z2 = z0+width_z / 2;
	matrix2d<8, 3> points{};

	points[0] = { x1, y1, z1 };
	points[1] = { x1, y2, z1 };
	points[2] = { x2, y2, z1 };
	points[3] = { x2, y1, z1 };
	points[4] = { x1, y1, z2 };
	points[5] = { x1, y2, z2 };
	points[6] = { x2, y2, z2 };
	points[7] = { x2, y1, z2 };

	matrix3d<6, 4, 3> trunk{};

	// TODO: Fix big condition
	if (is_maniac_hands)
	{
		float angle;
		if (forward) angle = maniac.rightHandAngle;
		else angle = -maniac.rightHandAngle;
		trunk[0] = RotatePolygon(
			matrix2d<4, 3>{ points[0], points[1], points[2], points[3] }, 
			angle, false
		);
		trunk[1] = RotatePolygon({ points[4], points[5], points[6], points[7] }, angle, false);
		trunk[2] = RotatePolygon({ points[5], points[1], points[2], points[6] }, angle, false);
		trunk[3] = RotatePolygon({ points[4], points[0], points[3], points[7] }, angle, false);
		trunk[4] = RotatePolygon({ points[5], points[1], points[0], points[4] }, angle, false);
		trunk[5] = RotatePolygon({ points[6], points[2], points[3], points[7] }, angle, false);

		for (int i = 0; i < 6; i++)
		{
			trunk[i] = RotatePolygon(trunk[i], maniac.entity.viewDirectionAngle);
		}
	}
	else
	{
		trunk[0] = RotatePolygon({ points[0], points[1], points[2], points[3] }, maniac.entity.viewDirectionAngle);
		trunk[1] = RotatePolygon({ points[4], points[5], points[6], points[7] }, maniac.entity.viewDirectionAngle);
		trunk[2] = RotatePolygon({ points[5], points[1], points[2], points[6] }, maniac.entity.viewDirectionAngle);
		trunk[3] = RotatePolygon({ points[4], points[0], points[3], points[7] }, maniac.entity.viewDirectionAngle);
		trunk[4] = RotatePolygon({ points[5], points[1], points[0], points[4] }, maniac.entity.viewDirectionAngle);
		trunk[5] = RotatePolygon({ points[6], points[2], points[3], points[7] }, maniac.entity.viewDirectionAngle);
	}

	glBegin(GL_LINE_LOOP);

	glColor3f(outcolor[0], outcolor[1], outcolor[2]);

	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			glVertex3f(trunk[i][j][0] + xr, trunk[i][j][1] + yr, trunk[i][j][2] + zr);
		}
	}

	glEnd();

	glBegin(GL_QUADS);

	glColor3f(fillcolor[0], fillcolor[1], fillcolor[2]);

	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			glVertex3f(trunk[i][j][0] + xr, trunk[i][j][1] + yr, trunk[i][j][2] + zr);
		}
	}

	glEnd();
}

void DrawManiac(const Maniac& maniac, const Player& player)
{
	const float Sx = maniac.entity.object.position.x - player.entity.object.position.x;
	const float Sy = player.entity.object.position.y - maniac.entity.object.position.y;
	
	//Trunk
	draw_cuboid(maniac, Sy * wall_width, -wall_height / 12, Sx * wall_width, 0, 0, 0, wall_width / 2, wall_height / 3, wall_width / 2, std::vector <float>{0.9, 0.7, 0.7}, std::vector <float>{1, 0.5, 0.5});

	//Head
	draw_cuboid(maniac, Sy * wall_width, wall_height / 7, Sx * wall_width, 0, 0, 0, wall_height / 12, wall_height / 12, wall_height / 12, std::vector <float>{ 0.9, 0.7, 0.7 }, std::vector <float>{ 1, 0.5, 0.5 });

	//Eyes
	draw_cuboid(maniac, Sy * wall_width, wall_height / 7, Sx * wall_width, -wall_height / 48, wall_height / 48, -wall_height / 24, wall_height / 48, wall_height / 48, wall_height / 96, std::vector <float>{ 0, 0, 0 }, std::vector <float>{ 1, 0, 0 });
	draw_cuboid(maniac, Sy * wall_width, wall_height / 7, Sx * wall_width, wall_height / 48, wall_height / 48, -wall_height / 24, wall_height / 48, wall_height / 48, wall_height / 96, std::vector <float>{ 0, 0, 0 }, std::vector <float>{ 1, 0, 0 });

	//Legs
	draw_cuboid(maniac, Sy * wall_width + wall_width / 20, -wall_height / 12 - wall_height / 6, Sx * wall_width, wall_width / 8, -wall_height / 12, 0, wall_width / 4, wall_height / 6, wall_width / 4, std::vector <float>{ 0.9, 0.7, 0.7 }, std::vector <float>{ 1, 0.5, 0.5 }, true, true);
	draw_cuboid(maniac, Sy * wall_width - wall_width / 20, -wall_height / 12 - wall_height / 6, Sx * wall_width, -wall_width / 8, -wall_height / 12, 0, wall_width / 4, wall_height / 6, wall_width / 4, std::vector <float>{ 0.9, 0.7, 0.7 }, std::vector <float>{ 1, 0.5, 0.5 }, true, false);

	//Hands
	draw_cuboid(maniac, Sy * wall_width + wall_width / 4, wall_height / 12, Sx * wall_width, wall_width / 8, -wall_height / 6, 0, wall_width / 8, wall_height / 4, wall_width / 8, std::vector <float>{ 0.9, 0.7, 0.7 }, std::vector <float>{ 1, 0.5, 0.5 }, true, false);
	draw_cuboid(maniac, Sy * wall_width - wall_width / 4, wall_height / 12, Sx * wall_width, -wall_width / 8, -wall_height / 6, 0, wall_width / 8, wall_height / 4, wall_width / 8, std::vector <float>{ 0.9, 0.7, 0.7 }, std::vector <float>{ 1, 0.5, 0.5 }, true, true);
}



void DrawCube(int x, int y, bool roof, bool floor, float shade);

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-1, 1, -1, 1);
	gluPerspective(120, 1, 0.1, 100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void InitGraphics()
{
	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
}

void OnDied() {
	if (count == 220)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearDepth(0);
		glDepthFunc(GL_GREATER);
		glLoadIdentity();
		return;
	}

	count++;

	Mix_VolumeChunk(soundBenzopila, 64);
	Mix_PlayChannel(-1, soundBenzopila, 1);

	Mix_VolumeChunk(soundKrik, 32);
	Mix_PlayChannel(-1, soundKrik, 1);

	glClearDepth(0);
	glDepthFunc(GL_GREATER);
	glLoadIdentity();

	glBegin(GL_QUADS);
	glColor3f(1, 0, 0);

	for (int i = 0; i < 200; i++)
	{
		float x = (rand() % 1000) / 2000.0 - 0.25;
		float y = (rand() % 1000) / 2000.0 - 0.25;

		glVertex3f(x, y, -0.1);
		glVertex3f(x + 0.002, y, -0.1);
		glVertex3f(x + 0.002, y + 0.002, -0.1);
		glVertex3f(x, y + 0.002, -0.1);
	}

	glEnd();
}

void render(Player& player)
{
	if (player.is_died)
	{
		OnDied();
		return;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearDepth(0);
	glDepthFunc(GL_GREATER);
	glLoadIdentity();

	glRotatef(player.entity.viewDirectionAngle, 0, 1, 0);

	DrawManiac(maniac, player);
	float S = vec2d_Dist(maniac.entity.object.position, maniac.entity.object.position);
	
	Maniac_Move(maniac, player.entity.object.position);

	for (size_t x = 0; x < map_width; x++)
	{
		for (size_t y = 0; y < map_height; y++)
		{
			char cell = field[y][x];
			float shade;
			float a = x - player.entity.object.position.x;
			float b = y - player.entity.object.position.y;
			float a2 = x - maniac.entity.object.position.x;
			float b2 = y - maniac.entity.object.position.y;
			if (cell == '#')
			{
				float S = sqrt(a * a + b * b);
				float S2 = sqrt(a2 * a2 + b2 * b2);
				shade = d0 / (d0 + S) + d0 / (d0 + S2);
				DrawCube(x, y, false, false, shade);
			}
			float c = wall_height / 2;
			float S = sqrt(a * a + b * b + c * c);
			float S2 = sqrt(a2 * a2 + b2 * b2 + c * c);
			shade = d0 / (d0 + S) + d0 / (d0 + S2);
			DrawCube(x, y, true, false, shade);
			DrawCube(x, y, false, true, shade);
		}
	}
	if (vec2d_Dist(maniac.entity.object.position, maniac.entity.object.position) <= 1) {
		player.is_died = true;
	}
}

void update_cube(float x, float y, bool roof, bool floor)
{
	float x1 = x;
	float y1 = -wall_height / 2;
	if (roof) y1 += wall_height;
	else if (floor) y1 -= wall_width;
	float z1 = y;

	float x2 = x1 + wall_width;
	float y2 = wall_height / 2;
	if (roof) y2 = y1 + wall_width;
	else if (floor) y2 = -wall_height / 2;
	float z2 = z1 + wall_width;
	matrix2d<8, 3> points{};
	points[0] = { x1, y1, z1 };
	points[1] = { x1, y2, z1 };
	points[2] = { x2, y2, z1 };
	points[3] = { x2, y1, z1 };

	points[4] = { x1, y1, z2 };
	points[5] = { x1, y2, z2 };
	points[6] = { x2, y2, z2 };
	points[7] = { x2, y1, z2 };

	current_cube[0] = { points[0], points[1], points[2], points[3] };
	current_cube[1] = { points[4], points[5], points[6], points[7] };
	current_cube[2] = { points[5], points[1], points[2], points[6] };
	current_cube[3] = { points[4], points[0], points[3], points[7] };
	current_cube[4] = { points[5], points[1], points[0], points[4] };
	current_cube[5] = { points[6], points[2], points[3], points[7] };
}

void DrawCube(int x, int y, bool roof, bool floor, float shade)
{
	glDisable(GL_LIGHTING);

	const float Sx = x - player.entity.object.position.x;
	const float Sy = player.entity.object.position.y - y;
	const float real_x = wall_width * Sx;
	const float real_y = wall_width * Sy;
	update_cube(real_y, real_x, roof, floor);

	glBegin(GL_LINE_STRIP);

	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	glColor3f(shade + 0.1, shade + 0.1, shade + 0.1);
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			glVertex3f(current_cube[i][j][0], current_cube[i][j][1], current_cube[i][j][2]);
		}
		glVertex3f(current_cube[i][0][0], current_cube[i][0][1], current_cube[i][0][2]);
	}

	glEnd();

	glColor3f(shade, shade, shade);
	glBegin(GL_QUADS);

	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			glVertex3f(current_cube[i][j][0], current_cube[i][j][1], current_cube[i][j][2]);
		}
	}
	
	glEnd();

	//for (int i = 0; i < 6; i++) {
	//	delete[] current_cube[i];
	//	current_cube[i] = nullptr;
	//}
}

float degrees_to_radians(float angle)
{
	return angle / 180 * pi;
}

// TODO: Rework
bool is_wall_collided(const vec2d& player_position)
{
	constexpr float collision_border = 0.5;

	const float player_x = player_position.x, player_y = player_position.y;
	
	for (size_t x = 0; x < map_width; x++)
	{
		for (size_t y = 0; y < map_height; y++)
		{
			const char cell = field[y][x];
			if (cell == Maze::WALL)
			{
				float Sx = (x - player_x) * wall_width;
				float Sy = (y - player_y) * wall_width;
				if (Sx > -wall_width + collision_border
					&& Sx < wall_width - collision_border
					&& Sy > -wall_width + collision_border
					&& Sy < wall_width - collision_border)
					return true;
			}
		}
	}

	return false;
}

void HandlePlayerControl(SDL_Event event, Player& player) {
	const auto is_keys_down = SDL_GetKeyboardState(nullptr);
	
	for (const auto& direction : DIRECTIONS) {
		auto check_keys = AllKeysFor(direction);
		if (std::any_of(
			check_keys.begin(),
			check_keys.end(), 
			[is_keys_down](SDL_Scancode key) { return is_keys_down[key]; })
		) {
			Move(player.entity, direction);
		}
	}
}

void Move(Entity& player, const Direction direction)
{
	const float alpha = degrees_to_radians(player.viewDirectionAngle);
	float dx = cos(alpha) * speedDelta;
	float dy = sin(alpha) * speedDelta;
	
	switch(direction) {
	case Direction::LEFT: 
		player.viewDirectionAngle -= angleDelta;
		break;
	case Direction::RIGHT: 
		player.viewDirectionAngle += angleDelta;
		break;
	case Direction::FORWARD:
		player.object.position.y -= dy;
		player.object.position.x -= dx;
		break;
	case Direction::BACKWARD:
		player.object.position.y -= dy;
		player.object.position.x -= dx;
		break;
	}

	// TODO: Rework
	if (false/*is_wall_collided(player_x, player_y)*/)
	{
		if (direction == Direction::FORWARD)
		{
			player.object.position.y += dy;
			player.object.position.x += dx;
		}
		else if (direction == Direction::BACKWARD) {
			player.object.position.y -= dy;
			player.object.position.x -= dx;
		}
	}
}

void InitSDLAttributes() {
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);
}

SDL_Window* InitWindow(const int screen_width, const int screen_height) {
	InitSDLAttributes();

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return nullptr;
	}

	SDL_Window* window = SDL_CreateWindow("Maze Horror", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, screen_width, screen_height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

	SDL_GLContext gl_context = SDL_GL_CreateContext(window);

	SDL_GL_SetSwapInterval(-1);

	if (window == NULL) {
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return nullptr;
	}

	SDL_Surface* screen_surface = SDL_GetWindowSurface(window);

	InitGraphics();

	return window;
}

std::pair<size_t, size_t> random_index_2d(size_t height, size_t width)
{
	size_t row_index = rand() % height,
		col_index = rand() % width;

	return { row_index, col_index };
}

std::pair<size_t, size_t> RandomPosition(const Maze& maze) {
	auto position = random_index_2d(maze.height(), maze.width());
	return { position.first % (maze.height() - 2) + 1,
		position.first % (maze.height() - 2) + 1};
}

int SDL_main(int argc, char ** argv) {
#ifdef DEV
	srand(1);
#else
	srand(time(NULL));
#endif
	Maze maze(31, 31);
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

	soundScary = Mix_LoadWAV(SOUND_SCARY);
	soundKrik = Mix_LoadWAV(SOUND_KRIK);
	soundBenzopila = Mix_LoadWAV(SOUND_BENZOPILA);

	Mix_VolumeChunk(soundScary, 32);
	const int maniacMusicChannel = Mix_PlayChannel(-1, soundScary, -1);

	field = new char*[maze.height()];
	for (size_t i = 0; i < maze.height(); i++) {
		field[i] = new char[maze.width()];
		for (size_t j = 0; j < maze.width(); j++) {
			field[i][j] = maze.array()[i][j];
		}
	}

	std::pair<size_t, size_t> player_position;

	do {
		player_position = RandomPosition(maze);
	} while (field[player_position.first][player_position.second] == Maze::WALL);

	maze.print();

	player.entity.object.position.y = player_position.first-0.5;
	player.entity.object.position.x = player_position.second+0.5;
	wall_height = maze.width();
	wall_width = maze.height();

	int screen_width = 1280;
	int screen_height = 960;

	SDL_Window* window = InitWindow(screen_width, screen_height);

	SDL_Event event;
	
	for (;;) {
		SDL_PollEvent(&event);
		
		if (event.type == SDL_QUIT)
			break;

		HandlePlayerControl(event, player);

		SDL_GetWindowSize(window, &screen_width, &screen_height);
		reshape(screen_width, screen_height);
		
		const float maniacDistance = std::abs(vec2d_Dist(maniac.entity.object.position, maniac.entity.object.position));
		const uint8_t maniacSoundDistance = maniacDistance > 255.f ? 255 : static_cast<uint8_t>(maniacDistance);
		
		Mix_SetPosition(maniacMusicChannel,
			std::atan2(maniac.entity.object.position.y- player.entity.object.position.y,
				maniac.entity.object.position.x - maniac.entity.object.position.x),
			maniacSoundDistance);

		render(player);

		SDL_GL_SwapWindow(window);
	}

	SDL_DestroyWindow(window);

	SDL_Quit();

	for (size_t i = 0; i < maze.height(); i++)
		delete[] field[i];
	delete[] field;

	return 0;
}
