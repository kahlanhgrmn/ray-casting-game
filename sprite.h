#pragma once
#include "raylib.h"
#include "game.h"
#include <vector>

class SpriteManager{
    public:
        std::vector<Sprite> sprites;
        Texture2D* textures;
        int textureCounter;

        SpriteManager(Texture2D* spriteTexture, int count);
        void addSprite(float x, float y, int textureIdx, int floor, int inventoryItemIndex = -1);
        void removeSprite(int index);
        int checkNearbySprite(float playerX, float playerY, float radius, int currentFloor);
        void renderSprites(float playerX, float playerY, float playerAngle, RenderTexture2D& renderTarget, int horizon, float* zBuffer, int currFloor);
        int checkNearbyNote(float playerX, float playerY, float radius, int currentFloor);
};