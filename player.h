#pragma once
#include "raylib.h"

class Player{
    public:
        float x;
        float y;
        float angle;
        float radius;

        Player(float startPosX, float startPosY);
        void update(float dt);
        bool checkCollision(float newX, float newY);
        bool isOnElevator();
};