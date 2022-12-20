#ifndef MAZE_H
#define MAZE_H

#include <iostream>
#include <cstdlib>
#include <ctime>

class Maze
{
public:
    enum CellType : char
    {
        WALL = '#',
        SPACE = ' ',
    };

    void Print(); //Печать лабиринта

    Maze(size_t width, size_t height);

    ~Maze();

    char* operator[](size_t height_index) {
        if (height_index < 0 || height_index > height_ - 1)
            throw std::out_of_range("Index is out of boundaries");
        return array_[height_index];
    }

    const char const* const* array() const {
        return array_;
    };

    const size_t& height() const {
        return height_;
    }

    const size_t& width() const {
        return width_;
    }
private:
    size_t height_, width_;
    char** array_;

    void Generate(); //Создание лабиринта
    void RemoveDeadend(int x, int y);
    void RemoveDeadends();
    bool DeadEnd(int x, int y);
};

#endif // !MAZE_H