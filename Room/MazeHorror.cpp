#include <cmath>
#include <ctime>
#include <iostream>
#include <algorithm>
#include <map>
#include <vector>

// GRAPHICS (DO NOT TOUCH)
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>
#include <SDL2/sdl.h>
// END GRAPHICS
#include <irrKlang.h>

#include "Direction.h"
#include "Maze.h"

#pragma comment(lib, "irrKlang.lib")

#define DEV

using namespace irrklang;
#define rha right_hand_angle

bool wall_collision(double player_x, double player_y);

struct Maniac {
	double x, y, rot_angle, right_hand_angle, dx, dy, point_x, point_y;
};

double ManiacDist(Maniac maniac);

double*** current_cube = new double** [6];

double mapwidth = 31;
double mapheight = 31;
double wallwidth = 0.5;
double wallheight = 3;
double player_x = 1;
double player_y = 8;
double pi = acos(0) * 2;
double dA = 5;
double dS = 0.05;
int Cw = 1000;
int Ch = 1000;
double rotation_angle = 0;
double d0 = 0.25;
int k = 1;
bool died = false;
int count = 0;

double check_points[6][2] = { {20, 1}, {20, 17}, {1, 7}, {1, 10}, {1, 1}, {1, 20} };
int chckpntsnum = 6;

Maniac maniac = { 1, 4, 0, 0, 0, 0, 0, 0 };

ISoundEngine* engine = createIrrKlangDevice();
ISound* sound = engine->play2D("Sounds/scary_sound.mp3", true, true, false, ESM_AUTO_DETECT, true);
ISound* sound2;

void Move(const Direction direction);

double** RotatePolygon(double** polygon, double verts_amount, double angle, bool axis_y = true)
{
	double** new_polygon = new double* [verts_amount];
	for (int i = 0; i < verts_amount; i++)
	{
		new_polygon[i] = new double[3];
		double x = polygon[i][0];
		double y = polygon[i][1];
		double z = polygon[i][2];
		if (axis_y)
		{
			double new_x = x * cos(angle) + z * sin(angle);
			double new_z = -x * sin(angle) + z * cos(angle);
			new_polygon[i][0] = new_x;
			new_polygon[i][1] = y;
			new_polygon[i][2] = new_z;
		}
		else {
			double new_y = y * cos(angle) + z * sin(angle);
			double new_z = -y * sin(angle) + z * cos(angle);
			new_polygon[i][0] = x;
			new_polygon[i][1] = new_y;
			new_polygon[i][2] = new_z;
		}
	}
	/*delete[] polygon;*/
	return new_polygon;
}



bool sees(Maniac maniac)
{
	double y = player_y - maniac.y;
	double x = player_x - maniac.x;
	double tg = y / x;
	for (double X = std::min(maniac.x, player_x); X < std::max(maniac.x, player_x); X += 0.5)
	{
		double Y = maniac.y + tg * (X - std::min(maniac.x, player_x));
		if (wall_collision(X, Y))
			return false;
	}
	return true;
}

double ManiacDist(Maniac maniac)
{
	double x = maniac.x - player_x;
	double y = maniac.y - player_y;
	return sqrt(x * x + y * y);
}

struct Point {
	int x;
	int y;
};

char** field = nullptr;

double dist(double pt_x, double pt_y, double X, double Y) {
	double x = X - pt_x;
	double y = Y - pt_y;
	return sqrt(x * x + y * y);
}
void check_cells(Maniac& maniac) {
	Point current_position = {(maniac.x),(maniac.y) };
	Point cell{ 0,0 };
	double distance;
	double dx, dy, cosine, sine;
	double minDist = sqrt(mapheight*mapheight + mapwidth*mapwidth);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			// TODO: Fix that behavior, wrong algoritm
			
			cell.y = current_position.y + i - 1;
			cell.x = current_position.x + j - 1;
			// Handler that doesnt accept error indexes
			if (cell.y < 0 || cell.y >= mapheight
				|| cell.x < 0 || cell.x >= mapwidth) {
				continue;
			}
			if (field[cell.y][cell.x] == '#' or (i == 1 and j == 1)) {
				continue;
			}
			distance = dist(maniac.point_x, maniac.point_y, cell.x, cell.y);
			if (minDist > distance) {
				minDist = distance;
				sine = (cell.y - maniac.y) / dist(maniac.x, maniac.y, cell.x, cell.y);
				cosine = (cell.x - maniac.x) / dist(maniac.x, maniac.y, cell.x, cell.y);
				dx = dS * cosine / 10;
				dy = dS * sine / 10;

			}
			
		}
	}
	maniac.dx = dx;
	maniac.dy = dy;

}
void switch_way(Maniac& maniac, int X, int Y) {
	if (field[Y + 1][X] == ' ')
	{
		if (maniac.dx > 0) {
			int chosen_way = rand() % 3;
			if (chosen_way == 0) {
				while (field[Y][X] != ' ') {
					X -= 1;
				}
			}
			else {
				while (field[Y][X] != ' ') {
					X += 1;
				}
			}

		}
		if (maniac.dx < 0) {
			int chosen_way = rand() % 3;
			if (chosen_way == 0) {
				while (field[Y][X] != ' ') {
					X += 1;
				}
			}
			else {
				while (field[Y][X] != ' ') {
					X -= 1;
				}
			}
		}
		int sine1 = (Y - maniac.y) / ManiacDist(maniac);
		int cosine1 = (X - maniac.x) / ManiacDist(maniac);
		maniac.dx = dS * cosine1 / 5;
		maniac.dy = 0;

	}
	else if(field[Y][X + 1] == ' ') {
		if (maniac.dy > 0) {
			int chosen_way = rand() % 3;
			if (chosen_way == 0) {
				while (field[Y][X] != ' ') {
					Y -= 1;
				}
			}
			else {
				while (field[Y][X] != ' ') {
					Y += 1;
				}
			}
		}
		else {
			int chosen_way = rand() % 3;
			if (chosen_way == 0) {
				while (field[Y][X] != ' ') {
					Y += 1;
				}
			}
			else {
				while (field[Y][X] != ' ') {
					Y -= 1;
				}
			}
		}
		int sine1 = (Y - maniac.y) / ManiacDist(maniac);
		int cosine1 = (X - maniac.x) / ManiacDist(maniac);
		maniac.dx = dS * cosine1 / 5;
		maniac.dy = 0;
	}
}

void check_checkpoints(Maniac& maniac) {
	double sine, cosine, dx, dy;
	int index = rand() % chckpntsnum;
	double a = check_points[index][0] - maniac.x;
	double b = check_points[index][1] - maniac.y;
	double S = sqrt(a * a + b * b);
	cosine = a / S;
	sine = b / S;
	double angle = asin(sine);
	if (cosine > 0 && cos(angle) < 0 || cosine < 0 && cos(angle) > 0)
		angle = pi - angle;
	maniac.rot_angle = pi - angle;

	dx = dS * cosine / 10;
	dy = dS * sine / 10;
	maniac.point_x = check_points[index][0];
	maniac.point_y = check_points[index][1];
	maniac.dx = dx;
	maniac.dy = dy;
}


//void check_cells(Maniac& maniac) {
//	int x = ceil(maniac.x);
//	int y = ceil(maniac.y);
//	Point central_cell = { x, y };
//	int standing[3][3];
//	if (maniac.y < 1) {
//		for (int i = 0; i < 3; i++) {
//			standing[0][i] = -1;
//		}
//	}
//	if (maniac.y > mapheight - 1) {
//		for (int i = 0; i < 3; i++) {
//			standing[2][i] = -1;
//		}
//	}
//	if (maniac.x < 1) {
//		for (int i = 0; i < 3; i++) {
//			standing[i][0] = -1;
//		}
//	}
//	if (maniac.x > mapwidth - 1) {
//		for (int i = 0; i < 3; i++) {
//			standing[i][2] = -1;
//		}
//	}
//	//sqrt(x * x + y * y);
//	int minDist = sqrt(mapwidth * mapwidth + mapheight * mapheight);
//	int min_x = mapheight, min_y = mapwidth;
//	for (int i = int(maniac.y) < 1 ? 0 : maniac.y - 1; i < int(maniac.y) + 2; i++) {
//		for (int j = int(maniac.x) < 1 ? 0 : maniac.x - 1; j < int(maniac.x) + 2; j++) {
//			if (field[i][j] != '#' and standing[i - int(maniac.y) + 1][j - int(maniac.x) + 1] != -1) {
//				standing[i - int(maniac.y) + 1][j - int(maniac.x) + 1] = dist(maniac.point_x, maniac.point_y, j, i);
//				if (minDist > standing[i - y + 1][j - x + 1]) {
//					minDist = standing[i - y + 1][j - x + 1];
//					min_x = j;
//					min_y = i;
//				}
//			}
//		}
//	}
//	double sine = (double(min_y) - maniac.y) / dist(maniac.x, maniac.y, min_x, min_y);
//	double cosine = (double(min_x) - maniac.x) / dist(maniac.x, maniac.y, min_x, min_y);
//	maniac.dx = dS * cosine / 5;
//	maniac.dy = dS * sine / 5;
//}

void maniac_Move(Maniac &maniac) {
	maniac.x += maniac.dx;
	maniac.y += maniac.dy;
	
}

void stuck_in_the_wall(Maniac& maniac) {
	Point maniac_coordinates = { maniac.x, maniac.y };
	if (field[maniac_coordinates.y][maniac_coordinates.x] == '#') {
		for (int i = int(maniac.y) < 1 ? 0 : maniac.y - 1; i < int(maniac.y) + 2; i++) {
			for (int j = int(maniac.x) < 1 ? 0 : maniac.x - 1; j < int(maniac.x) + 2; j++) {
				if (field[i][j] == ' ') {
					maniac.x = j;
					maniac.y = i;
				}
			}
		}
	}
}


void MoveManiac(Maniac& maniac)
{

	maniac.rha += k * pi / 60;
	if (maniac.rha > pi / 4)
	{
		maniac.rha = pi / 4;
		k = -1;
	}
	else if (maniac.rha < -pi / 4)
	{
		maniac.right_hand_angle = -pi / 4;
		k = 1;
	}



	if (sees(maniac)) {
		double sine, cosine, dx, dy;
		sine = (player_y - maniac.y) / ManiacDist(maniac);
		cosine = (player_x - maniac.x) / ManiacDist(maniac);
		double angle = asin(sine);
		if (cosine > 0 && cos(angle) < 0 || cosine < 0 && cos(angle) > 0)
			angle = pi - angle;
		maniac.rot_angle = pi - angle;
		maniac.point_x = player_x;
		maniac.point_y = player_y;

	}
	else {
		double x = maniac.x - maniac.point_x;
		double y = maniac.y - maniac.point_y;
		if ((x * x + y * y) <= 0.01) {
			int index = rand() % chckpntsnum;
			maniac.point_x = check_points[index][0];
			maniac.point_y = check_points[index][1];
		}
	}
	check_cells(maniac);
	maniac_Move(maniac);
}



void DrawCuboid(double xr, double yr, double zr, double x0, double y0, double z0, double width_x, double width_y, double width_z, double *fillcolor, double *outcolor, bool legs_or_hands = false, bool forward = false)
{
	double x1 = x0-width_x / 2;
	double y1 = y0-width_y / 2;
	double z1 = z0-width_z / 2;
	double x2 = x0 + width_x / 2;
	double y2 = y0+width_y / 2;
	double z2 = z0+width_z / 2;

	double* point1 = new double[3]{ x1, y1, z1 };
	double* point2 = new double[3]{ x1, y2, z1 };
	double* point3 = new double[3]{ x2, y2, z1 };
	double* point4 = new double[3]{ x2, y1, z1 };

	double* point5 = new double[3]{ x1, y1, z2 };
	double* point6 = new double[3]{ x1, y2, z2 };
	double* point7 = new double[3]{ x2, y2, z2 };
	double* point8 = new double[3]{ x2, y1, z2 };

	double*** trunk = new double** [6];

	if (legs_or_hands)
	{
		double angle;
		if (forward) angle = maniac.rha;
		else angle = -maniac.rha;
		trunk[0] = RotatePolygon(new double* [4]{ point1, point2, point3, point4 }, 4, angle, false);
		trunk[1] = RotatePolygon(new double* [4]{ point5, point6, point7, point8 }, 4, angle, false);
		trunk[2] = RotatePolygon(new double* [4]{ point6, point2, point3, point7 }, 4, angle, false);
		trunk[3] = RotatePolygon(new double* [4]{ point5, point1, point4, point8 }, 4, angle, false);
		trunk[4] = RotatePolygon(new double* [4]{ point6, point2, point1, point5 }, 4, angle, false);
		trunk[5] = RotatePolygon(new double* [4]{ point7, point3, point4, point8 }, 4, angle, false);

		for (int i = 0; i < 6; i++)
		{
			trunk[i] = RotatePolygon(trunk[i], 4, maniac.rot_angle);
		}
	}
	else
	{
		trunk[0] = RotatePolygon(new double* [4]{ point1, point2, point3, point4 }, 4, maniac.rot_angle);
		trunk[1] = RotatePolygon(new double* [4]{ point5, point6, point7, point8 }, 4, maniac.rot_angle);
		trunk[2] = RotatePolygon(new double* [4]{ point6, point2, point3, point7 }, 4, maniac.rot_angle);
		trunk[3] = RotatePolygon(new double* [4]{ point5, point1, point4, point8 }, 4, maniac.rot_angle);
		trunk[4] = RotatePolygon(new double* [4]{ point6, point2, point1, point5 }, 4, maniac.rot_angle);
		trunk[5] = RotatePolygon(new double* [4]{ point7, point3, point4, point8 }, 4, maniac.rot_angle);
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
	//delete[] point1;
	//delete[] point2; 
	//delete[] point3,
	//delete[] point4;
	//delete[] point5; 
	//delete[] point6; 
	//delete[] point7; 
	//delete[] point8; 
	//for (int i = 0; i < 6; i++) {
	//	delete[] trunk[i];
	//}
	//delete[] trunk;
	//delete[] fillcolor;
	//delete[] outcolor;
}

void DrawManiac(Maniac maniac)
{
	double Sx = maniac.x - player_x;
	double Sy = player_y - maniac.y;
	
	//Trunk
	DrawCuboid(Sy * wallwidth, -wallheight / 12, Sx * wallwidth, 0, 0, 0, wallwidth / 2, wallheight / 3, wallwidth / 2, new double[3]{0.9, 0.7, 0.7}, new double[3]{1, 0.5, 0.5});

	//Head
	DrawCuboid(Sy * wallwidth, wallheight / 7, Sx * wallwidth, 0, 0, 0, wallheight / 12, wallheight / 12, wallheight / 12, new double[3]{ 0.9, 0.7, 0.7 }, new double[3]{ 1, 0.5, 0.5 });

	//Eyes
	DrawCuboid(Sy * wallwidth, wallheight / 7, Sx * wallwidth, -wallheight / 48, wallheight / 48, -wallheight / 24, wallheight / 48, wallheight / 48, wallheight / 96, new double[3]{ 0, 0, 0 }, new double[3]{ 1, 0, 0 });
	DrawCuboid(Sy * wallwidth, wallheight / 7, Sx * wallwidth, wallheight / 48, wallheight / 48, -wallheight / 24, wallheight / 48, wallheight / 48, wallheight / 96, new double[3]{ 0, 0, 0 }, new double[3]{ 1, 0, 0 });

	//Legs
	DrawCuboid(Sy * wallwidth + wallwidth / 20, -wallheight / 12 - wallheight / 6, Sx * wallwidth, wallwidth / 8, -wallheight / 12, 0, wallwidth / 4, wallheight / 6, wallwidth / 4, new double[3]{ 0.9, 0.7, 0.7 }, new double[3]{ 1, 0.5, 0.5 }, true, true);
	DrawCuboid(Sy * wallwidth - wallwidth / 20, -wallheight / 12 - wallheight / 6, Sx * wallwidth, -wallwidth / 8, -wallheight / 12, 0, wallwidth / 4, wallheight / 6, wallwidth / 4, new double[3]{ 0.9, 0.7, 0.7 }, new double[3]{ 1, 0.5, 0.5 }, true, false);

	//Hands
	DrawCuboid(Sy * wallwidth + wallwidth / 4, wallheight / 12, Sx * wallwidth, wallwidth / 8, -wallheight / 6, 0, wallwidth / 8, wallheight / 4, wallwidth / 8, new double[3]{ 0.9, 0.7, 0.7 }, new double[3]{ 1, 0.5, 0.5 }, true, false);
	DrawCuboid(Sy * wallwidth - wallwidth / 4, wallheight / 12, Sx * wallwidth, -wallwidth / 8, -wallheight / 6, 0, wallwidth / 8, wallheight / 4, wallwidth / 8, new double[3]{ 0.9, 0.7, 0.7 }, new double[3]{ 1, 0.5, 0.5 }, true, true);
}



void DrawCube(int x, int y, bool roof, bool floor, double shade);

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
		sound->stop();
		sound2->stop();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearDepth(0);
		glDepthFunc(GL_GREATER);
		glLoadIdentity();
		return;
	}

	count++;

	glClearDepth(0);
	glDepthFunc(GL_GREATER);
	glLoadIdentity();

	glBegin(GL_QUADS);
	glColor3f(1, 0, 0);

	for (int i = 0; i < 200; i++)
	{
		double x = (rand() % 1000) / 2000.0 - 0.25;
		double y = (rand() % 1000) / 2000.0 - 0.25;

		glVertex3f(x, y, -0.1);
		glVertex3f(x + 0.002, y, -0.1);
		glVertex3f(x + 0.002, y + 0.002, -0.1);
		glVertex3f(x, y + 0.002, -0.1);
	}

	glEnd();
}

void render()
{
	if (died)
	{
		OnDied();
		return;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearDepth(0);
	glDepthFunc(GL_GREATER);
	glLoadIdentity();

	glRotatef(rotation_angle, 0, 1, 0);

	DrawManiac(maniac);
	double S = ManiacDist(maniac);
	double volume = 1 / (1 + S);
	sound->setVolume(volume);
	sound->setIsPaused(false);
	MoveManiac(maniac);

	for (int x = 0; x < mapwidth; x++)
	{
		for (int y = 0; y < mapheight; y++)
		{
			char cell = field[y][x];
			double shade;
			double a = x - player_x;
			double b = y - player_y;
			double a2 = x - maniac.x;
			double b2 = y - maniac.y;
			if (cell == '#')
			{
				double S = sqrt(a * a + b * b);
				double S2 = sqrt(a2 * a2 + b2 * b2);
				shade = d0 / (d0 + S) + d0 / (d0 + S2);
				DrawCube(x, y, false, false, shade);
			}
			double c = wallheight / 2;
			double S = sqrt(a * a + b * b + c * c);
			double S2 = sqrt(a2 * a2 + b2 * b2 + c * c);
			shade = d0 / (d0 + S) + d0 / (d0 + S2);
			DrawCube(x, y, true, false, shade);
			DrawCube(x, y, false, true, shade);
		}
	}
	if (ManiacDist(maniac) <= 1) {
		died = true;
		sound->stop();
		sound = engine->play2D("Sounds/benzopila.wav", true, true, false, ESM_AUTO_DETECT, true);
		sound->setVolume(1);
		sound->setIsPaused(false);
		sound2 = engine->play2D("Sounds/krik.mp3", true, true, false, ESM_AUTO_DETECT, true);
		sound2->setVolume(0.5);
		sound2->setIsPaused(false);
	}
}

void update_cube(double x, double y, bool roof, bool floor)
{
	double x1 = x;
	double y1 = -wallheight / 2;
	if (roof) y1 += wallheight;
	else if (floor) y1 -= wallwidth;
	double z1 = y;

	double x2 = x1 + wallwidth;
	double y2 = wallheight / 2;
	if (roof) y2 = y1 + wallwidth;
	else if (floor) y2 = -wallheight / 2;
	double z2 = z1 + wallwidth;
	double** points = new double*[9];
	points[1] = new double[3]{ x1, y1, z1 };
	points[2] = new double[3]{ x1, y2, z1 };
	points[3] = new double[3]{ x2, y2, z1 };
	points[4] = new double[3]{ x2, y1, z1 };

	points[5] = new double[3]{ x1, y1, z2 };
	points[6] = new double[3]{ x1, y2, z2 };
	points[7] = new double[3]{ x2, y2, z2 };
	points[8] = new double[3]{ x2, y1, z2 };

	current_cube[0] = new double* [4]{ points[1], points[2], points[3], points[4] };
	current_cube[1] = new double* [4]{ points[5], points[6], points[7], points[8] };
	current_cube[2] = new double* [4]{ points[6], points[2], points[3], points[7] };
	current_cube[3] = new double* [4]{ points[5], points[1], points[4], points[8] };
	current_cube[4] = new double* [4]{ points[6], points[2], points[1], points[5] };
	current_cube[5] = new double* [4]{ points[7], points[3], points[4], points[8] };
	// delete[] points;
}

void DrawCube(int x, int y, bool roof, bool floor, double shade)
{
	glDisable(GL_LIGHTING);

	double Sx = x - player_x;
	double Sy = player_y - y;
	double real_x = wallwidth * Sx;
	double real_y = wallwidth * Sy;
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

double degrees_to_radians(double angle)
{
	return angle / 180 * pi;
}

// TODO: Rework
bool wall_collision(double player_x, double player_y)
{
	const double collision_border = 0.5;
	for (int x = 0; x < mapwidth; x++)
	{
		for (int y = 0; y < mapheight; y++)
		{
			char cell = field[y][x];
			if (cell == Maze::WALL)
			{
				double Sx = (x - player_x) * wallwidth;
				double Sy = (y - player_y) * wallwidth;
				if (Sx > -wallwidth + collision_border
					&& Sx < wallwidth - collision_border
					&& Sy > -wallwidth + collision_border
					&& Sy < wallwidth - collision_border)
					return true;
			}
		}
	}

	return false;
}

void HandlePlayerControl(SDL_Event event) {
	const auto is_keys_down = SDL_GetKeyboardState(nullptr);

	std::vector<SDL_Scancode> check_keys;

	for (const auto& direction : DIRECTIONS) {
		auto check_keys = AllKeysFor(direction);
		if (std::any_of(
			check_keys.begin(),
			check_keys.end(), 
			[is_keys_down](SDL_Scancode key) { return is_keys_down[key]; })
		) {
			Move(direction);
		}
	}
}

void Move(const Direction direction)
{
	double alpha = degrees_to_radians(rotation_angle);
	double dx = cos(alpha) * dS;
	double dy = sin(alpha) * dS;

	if (direction == Direction::LEFT) rotation_angle -= dA;
	else if (direction == Direction::RIGHT) rotation_angle += dA;
	if (direction == Direction::FORWARD) {
		player_y -= dy;
		player_x -= dx;
	}
	else if (direction == Direction::BACKWARD) {
		player_y += dy;
		player_x += dx;
	}

	// TODO: Rework
	if (false/*wall_collision(player_x, player_y)*/)
	{
		if (direction == Direction::FORWARD)
		{
			player_y += dy;
			player_x += dx;
		}
		else if (direction == Direction::BACKWARD) {
			player_y -= dy;
			player_x -= dx;
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

	maze.Print();

	player_y = player_position.first-0.5;
	player_x = player_position.second+0.5;
	wallheight = maze.width();
	wallwidth = maze.height();

	int screen_width = 1280;
	int screen_height = 960;

	SDL_Window* window = InitWindow(screen_width, screen_height);

	SDL_Event event;

	for (;;) {
		SDL_PollEvent(&event);
		
		if (event.type == SDL_QUIT)
			break;

		HandlePlayerControl(event);

		SDL_GetWindowSize(window, &screen_width, &screen_height);
		reshape(screen_width, screen_height);

		render();

		SDL_GL_SwapWindow(window);
	}

	SDL_DestroyWindow(window);

	SDL_Quit();

	for (size_t i = 0; i < maze.height(); i++)
		delete[] field[i];
	delete[] field;

	return 0;
}
