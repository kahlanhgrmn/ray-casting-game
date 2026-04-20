#include "map.h"
#include "game.h"
#include <cmath>
#include <cstdlib>

Map* currentMap = nullptr;
int currentFloor = 0;
Map* floors[10] = {nullptr}; 

Map::Map(int w, int h): width(w), height(h), elevatorPosition{0,0}{
    grid = new int*[height];

    for(int i = 0; i < height; i++){
        grid[i] = new int[width];

        for(int j = 0; j < width; j++){
            grid[i][j] = 0;
        }
    }
}

Map::~Map(){
    for(int i = 0; i < height; i++){
        delete[]grid[i];
    }
    delete[]grid;
}

int Map::getCell(int x, int y)const{
    if(!inBounds(x,y)){
        return 1;
    }

    return grid[y][x];
}

bool Map::inBounds(int x, int y)const{
    return x >= 0 && x < width && y >= 0 && y < height;
}

void initMaps(){

    // floor 0 (current map - lobby)
    floors[0] = new Map(16,16);
    floors[0] -> elevatorPosition = {7.5f, 0.5f};

    int floor0Data[16][16]={
        {1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1},
        {1,0,1,1,1,0,1,0,1,1,1,1,1,1,0,1},
        {1,0,1,0,0,0,0,0,1,0,0,0,0,1,0,1},
        {1,0,1,0,1,1,1,0,1,0,1,1,0,1,0,1},
        {1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1},
        {1,0,1,0,1,0,1,1,1,0,1,0,1,1,0,1},
        {1,0,1,0,0,0,0,0,1,0,0,0,0,1,0,1},
        {1,0,1,1,1,1,1,0,1,1,1,1,0,1,0,1},
        {1,0,0,0,0,0,1,0,0,0,0,1,0,1,0,1},
        {1,1,1,1,1,0,1,1,1,1,0,1,0,1,0,1},
        {1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,1},
        {1,0,1,0,1,1,1,1,0,1,1,1,0,1,0,1},
        {1,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1},
        {1,0,0,0,1,1,0,0,0,1,0,0,0,1,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    };

    for(int y = 0; y < 16; y++){
        for(int x = 0; x < 16; x++){
            floors[0] -> grid[y][x] = floor0Data[y][x];
        }
    }

    // other floors
    for(int f = 1; f <= 4; f++){
        floors[f] = new Map(16,16);

        // Put elevator in different spots
        if(f == 1) floors[f]->elevatorPosition = {7.5f, 7.5f};
        if(f == 2) floors[f]->elevatorPosition = {1.5f, 13.5f};
        if(f == 3) floors[f]->elevatorPosition = {13.5f, 13.5f};
        if(f == 4) floors[f]->elevatorPosition = {7.5f, 1.5f};

        for(int y = 0; y < 16; y++){
            for(int x = 0; x < 16; x++){
                // border walls
                if(x == 0 || x == 15 || y == 0 || y == 15) floors[f]->grid[y][x] = tileWall;
                else floors[f]->grid[y][x] = tileFloor;
            }
        }

        // tile for elevator
        int ex = (int)floorf(floors[f]->elevatorPosition.x);
        int ey = (int)floorf(floors[f]->elevatorPosition.y);
        floors[f]->grid[ey][ex] = tileElevator;

        
        if(f >= 2){
            for(int x = 2; x <= 13; x++){
                floors[f]->grid[8][x] = tileWall;
            }
            floors[f]->grid[8][7] = tileFloor;
        }
        
        if(f >= 3){
            for(int y = 2; y <= 13; y++){
                floors[f]->grid[y][6] = tileWall;
            }
            floors[f]->grid[10][6] = tileFloor;
        }
    }


    setFloor(0); // starting on floor 0/ ground floor
}

void setFloor(int floorNum){
    if(floorNum >= 0 && floorNum < 10 && floors[floorNum] != nullptr){
        currentFloor = floorNum;
        currentMap = floors[floorNum];
    }
}

void cleanupMaps(){
    for(int i = 0; i < 10; i++){
        if(floors[i] != nullptr){
            delete floors[i];
            floors[i] = nullptr;
        }
    }
}