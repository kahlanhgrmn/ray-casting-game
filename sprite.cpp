#include "sprite.h"
#include <algorithm>
#include <cmath>

struct SpriteDistance{
    int idx;
    float dist;
};

SpriteManager::SpriteManager(Texture2D* spriteTexture, int count): textures(spriteTexture), textureCounter(count){}

void SpriteManager::addSprite(float x, float y, int textureIdx, int floor, int inventoryItemIdx){
    sprites.push_back({x, y, textureIdx, false, floor, inventoryItemIdx});
}

int SpriteManager::checkNearbySprite(float playerX, float playerY, float radius, int currentFloor){
    for(int i = 0; i < sprites.size(); i++){

        if(sprites[i].collected || sprites[i].floor != currentFloor) {
            continue;
        }
        
        // check sprites that are collectable
        if(sprites[i].inventoryItemIndex < 0){
            continue;
        }
        
        float dx = sprites[i].x - playerX;
        float dy = sprites[i].y - playerY;
        float distance = sqrtf(dx * dx + dy * dy);
        
        if(distance < radius){
            return i;
        }
    }
    
    return -1;
}

void SpriteManager::removeSprite(int idx){
    if(idx >= 0 && idx < sprites.size()){
        sprites.erase(sprites.begin() + idx);
    }
}

void SpriteManager::renderSprites(float playerX, float playerY, float playerAngle, RenderTexture2D& renderTarget, int horizon, float* zBuffer, int currFloor){

    std::vector<SpriteDistance> spriteDistances;
    
    for(int i = 0; i < sprites.size(); i++){

        if(sprites[i].floor != currFloor){
            continue;
        }

        if(sprites[i].inventoryItemIndex >= 0 && sprites[i].collected){
            continue;
        }

        float dx = sprites[i].x - playerX;
        float dy = sprites[i].y - playerY;
        float distance = sqrtf(dx * dx + dy * dy);
        
        spriteDistances.push_back({i, distance});
    }
    
    std::sort(spriteDistances.begin(), spriteDistances.end(), [](const SpriteDistance& a, const SpriteDistance& b){
        return a.dist > b.dist;
    });
    
    for(const auto& sd : spriteDistances){
        Sprite& sprite = sprites[sd.idx];
        
        float dx = sprite.x - playerX;
        float dy = sprite.y - playerY;

        float cosA = cosf(playerAngle);
        float sinA = sinf(playerAngle);
        float transformX = dy * cosA - dx * sinA;
        float transformY = dx * cosA + dy * sinA;

        if(transformY <= 0.1f){
            continue;
        }

        float screenX = (transformX / transformY) * projectPlaneDist;
        
        if(screenX < ((-internalW/2) - 50) || screenX > ((internalW/2) + 50)){
            continue;
        }

        float spriteHeight = (1.0f / transformY) * projectPlaneDist;
        int drawX = (int)(internalW / 2 + screenX - spriteHeight / 2);
        int drawY = (int)(horizon - spriteHeight / 2);
        int drawSize = (int)spriteHeight;
        
        if(drawSize <= 0){
            continue;
        }

        Texture2D tex = textures[sprite.textureIdx % textureCounter];
        
        float distance = transformY;
        float shade = 1.0f - (distance / maxRayDist);

        if(shade < 0.0f){
            shade = 0.0f;
        }

        unsigned char tint = (unsigned char)(80 + shade * 175);
        
        Rectangle src = {0, 0, (float)tex.width, (float)tex.height};
        Rectangle dst = {(float)drawX, (float)drawY, (float)drawSize, (float)drawSize};

        BeginTextureMode(renderTarget);
        
        int startX = (drawX < 0)? 0 : drawX;
        int endX = ((drawX + drawSize) > internalW)? internalW : (drawX + drawSize);

        for(int x = startX; x < endX; x++){

            if(transformY < zBuffer[x]){
                float columnWidth = (float)tex.width / drawSize;
                int texX = (int)((x - drawX) * columnWidth);
                
                Rectangle colSrc = {(float)texX, 0, 1, (float)tex.height};
                Rectangle colDst = {(float)x, (float)drawY, 1, (float)drawSize};
                
                DrawTexturePro(tex, colSrc, colDst, Vector2{0, 0}, 0.0f, Color{tint, tint, tint, 255});
            }
        }

        EndTextureMode();
    }
}

int SpriteManager::checkNearbyNote(float playerX, float playerY, float radius, int currentFloor){
    for(int i = 0; i < sprites.size(); i++){
        if(sprites[i].floor != currentFloor){
            continue;
        }

        if(sprites[i].inventoryItemIndex > -2){ // -2 onwards used for notes
            continue;
        }

        float dx = sprites[i].x - playerX;
        float dy = sprites[i].y - playerY;
        float distance = sqrtf((dx * dx) + (dy * dy));

        if(distance < radius) {return i;}
    }

    return -1;
}