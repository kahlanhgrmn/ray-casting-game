#include "raycaster.h"
#include "map.h"
#include "game.h"

RayHit castRay(float px, float py, float a){
    float rayDirectionX = cosf(a);
    float rayDirectionY = sinf(a);
    int mapX = (int)floorf(px);
    int mapY = (int)floorf(py);

    float deltaDistanceX = (rayDirectionX == 0.0f)? 1e30f : fabsf(1.0f/ rayDirectionX);
    float deltaDistanceY = (rayDirectionY == 0.0f)? 1e30f : fabsf(1.0f/ rayDirectionY);

    int stepX;
    int stepY;
    float sideDistanceX;
    float sideDistanceY;

    if(rayDirectionX < 0){
        stepX = -1;
        sideDistanceX = (px - (float)mapX) * deltaDistanceX;
    }

    else{
        stepX = 1;
        sideDistanceX = ((float)mapX + 1.0f - px)* deltaDistanceX;
    }

    if(rayDirectionY < 0){
        stepY = -1;
        sideDistanceY = (py - (float)mapY) * deltaDistanceY;
    }

    else{
        stepY = 1;
        sideDistanceY = ((float)mapY + 1.0f - py)* deltaDistanceY;
    }

    int side = 0;
    int cell = 0;

    // stepping through the grid
    while(true){
        if(sideDistanceX < sideDistanceY){
            sideDistanceX += deltaDistanceX;
            mapX += stepX;

            side = 0;
        }

        else{
            sideDistanceY += deltaDistanceY;
            mapY += stepY;

            side = 1;
        }

        if(!currentMap->inBounds(mapX, mapY)){
            float distance = (side==0)? (sideDistanceX - deltaDistanceX) : (sideDistanceY - deltaDistanceY);

            if(distance > maxRayDist){
                distance = maxRayDist;
            }

            float hitX = px + rayDirectionX * distance;
            float hitY = py + rayDirectionY * distance;

            return RayHit{distance, hitX, hitY, 1, side};
        }

        cell = currentMap->getCell(mapX, mapY);
        if(cell == tileWall){
            break;
        }

        float distanceCovered = (side ==0)? (sideDistanceX - deltaDistanceX) : (sideDistanceY - deltaDistanceY);

        if(distanceCovered > maxRayDist){
            float distance = maxRayDist;
            float hitX = px + rayDirectionX * distance;
            float hitY = py + rayDirectionY * distance;

            return RayHit{distance, hitX, hitY, 0, side};
        }
    }

    float distance = (side == 0)? (sideDistanceX - deltaDistanceX): (sideDistanceY - deltaDistanceY);

    if(distance < 0.0001f){
        distance = 0.0001f;
    }
    if( distance > maxRayDist){
        distance = maxRayDist;
    }

    float hitX = px + rayDirectionX * distance;
    float hitY = py + rayDirectionY * distance;

    return RayHit{distance, hitX, hitY, cell, side};
}