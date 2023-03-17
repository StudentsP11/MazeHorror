#ifndef MAZE_H
#define MAZE_H

#include <iostream>

class Maze
{
public:
    enum CellType : char
    {
        WALL = '#',
        SPACE = ' ',
    };

    void print() const;

    Maze(size_t width, size_t height);

    ~Maze();

    [[nodiscard]] char* operator[](size_t height_index) const;

    [[nodiscard]] const char * const* array() const;
    
    [[nodiscard]] const size_t& height() const;

    [[nodiscard]] const size_t& width() const;
private:
    size_t height_, width_;
    char** array_;

    void Generate();
    void RemoveDeadEnd(int x, int y);
    void RemoveDeadEnds();
    bool DeadEnd(int x, int y);
};

#endif // !MAZE_H
