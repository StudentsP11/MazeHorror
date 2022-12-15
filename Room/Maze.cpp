#include "Maze.h"

Maze::Maze(size_t width, size_t height) {
    if (width % 2 == 0 || height % 2 == 0)
        throw std::invalid_argument("width and height must be odd sizes");

    width_ = width;
    height_ = height;

    arr_ = new char* [height];
    for (size_t i = 0; i < height; i++) {
        arr_[i] = new char[width] {0};
    }

    Generate();
}

Maze::~Maze() {
    for (size_t i = 0; i < height_; i++) {
        delete[] arr_[i];
    }
    delete[] arr_;
}

void Maze::Print()
{
    using std::cout;

    for (int i = 0; i < height_; i++)
    {
        for (int j = 0; j < width_; j++)
            cout << (char)arr_[i][j];
        cout << '\n';
    }
    cout << '\n';
}

void Maze::Generate()
{
    // Start position must be greater then corner, 0 is border, so 1 is corner
    size_t x = 3, y = 3;
    uint8_t direction = 0;
    uint64_t count = 0;

    for (int i = 0; i < height_; i++)
        for (int j = 0; j < width_; j++)
            arr_[i][j] = WALL;

    while (count < 10000)
    {
        arr_[y][x] = SPACE;
        count++;

        while (true) {
            direction = rand() % 4; //4 разных направления
            switch (direction)
            {
            case 0:
                if (y != 1)
                    if (arr_[y - 2][x] == WALL)  // Вверх
                    {
                        arr_[y - 1][x] = SPACE;
                        arr_[y - 2][x] = SPACE;
                        y -= 2;
                    }
            case 1:
                if (y != height_ - 2)
                    if (arr_[y + 2][x] == WALL)  // Вниз
                    {
                        arr_[y + 1][x] = SPACE;
                        arr_[y + 2][x] = SPACE;
                        y += 2;
                    }
            case 2:
                if (x != 1)
                    if (arr_[y][x - 2] == WALL)  // Налево
                    {
                        arr_[y][x - 1] = SPACE;
                        arr_[y][x - 2] = SPACE;
                        x -= 2;
                    }
            case 3:
                if (x != width_ - 2)
                    if (arr_[y][x + 2] == WALL)  // Направо
                    {
                        arr_[y][x + 1] = SPACE;
                        arr_[y][x + 2] = SPACE;
                        x += 2;
                    }
            }
            if (DeadEnd(x, y))
                break;
        }
        if (DeadEnd(x, y)) //Обход тупика
        {
            do
            {
                x = 2 * (rand() % ((width_ - 1) / 2)) + 1;
                y = 2 * (rand() % ((height_ - 1) / 2)) + 1;
            } while (arr_[y][x] != SPACE);
        }
    }

    

    RemoveDeadends();
}

bool Maze::DeadEnd(int x, int y)
{
    int a = 0;

    if (x != 1)
    {
        if (arr_[y][x - 2] == SPACE)
            a += 1;
    }
    else a += 1;

    if (y != 1)
    {
        if (arr_[y - 2][x] == SPACE)
            a += 1;
    }
    else a += 1;

    if (x != width_ - 2)
    {
        if (arr_[y][x + 2] == SPACE)
            a += 1;
    }
    else a += 1;

    if (y != height_ - 2)
    {
        if (arr_[y + 2][x] == SPACE)
            a += 1;
    }
    else a += 1;

    if (a == 4)
        return 1;
    else
        return 0;
}

void Maze::RemoveDeadend(int x, int y)
{
    const int directions[4][2] = { {0, 1}, {0, -1}, {1, 0}, {-1, 0} };
    int non_possible_deadends_count = 0;
    int count = 0;
    for (auto direction : directions)
    {
        int dx = direction[0], dy = direction[1];
        if (x + dx < 1 or x + dx >= width_ - 1
            or y + dy < 1 or y + dy >= height_ - 1)
        {
            continue;
        }
        if (arr_[y + dy][x + dx] == ' ')
        {
            count++;
        }
    }

    if (count <= 1) {
        for (auto direction : directions)
        {
            int dx = direction[0], dy = direction[1];
            if (x + dx < 1 or x + dx >= width_ - 1
                or y + dy < 1 or y + dy >= height_ - 1)
            {
                continue;
            }
            if (arr_[y + dy][x + dx] != ' ')
            {
                arr_[y + dy][x + dx] = ' ';
                break;
            }
        }
    }

}

void Maze::RemoveDeadends()
{
    for (int i = 0; i < height_; i++)
        for (int j = 0; j < width_; j++)
            if (arr_[i][j] == ' ')
                RemoveDeadend(j, i);
}