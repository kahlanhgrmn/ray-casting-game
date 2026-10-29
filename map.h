#pragma once
#include "raylib.h"

class Map{
    public:
        int width;
        int height;
        int** grid;
        Vector2 elevatorPosition;

        Map(int w, int h);
        ~Map();

        int getCell(int x, int y)const;
        bool inBounds(int x, int y)const;
};

extern Map* currentMap;
extern int currentFloor;
void initMaps();
void setFloor(int floorNumber);
void cleanupMaps();