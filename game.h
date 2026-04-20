#pragma once
#include "raylib.h"
#include <cmath>

static const int screenWidth = 900;
static const int screenHeight = 700;

static const int internalW = 400;
static const int internalH = (internalW * screenHeight) / screenWidth;

static const float fov = 66.0f * (PI / 180.0f); // converting to rad
static const int numRays = internalW;

static const float maxRayDist = 24.0f;

static const float projectPlaneDist = (internalW / 2.0f) / tanf(fov/ 2.0f);

static const int miniTile = 12;
static const int miniPad  = 12;

static const int tileFloor = 0;
static const int tileWall = 1;
static const int tileElevator = 2;

static const float footstepInterval = 0.4f;

struct Sprite {
    float x;
    float y;
    int textureIdx;
    bool collected;
    int floor;
    int inventoryItemIndex;
};

struct RayHit {
    float dist;
    float hitX;
    float hitY;
    int cell;  // what is being hit (1 = wall, 0 = nothing)
    int side; // 1 = hitting horizontal wall, 0 = hitting vertical wall
};