#include "player.h"
#include "map.h"
#include "game.h"

static const float moveSpeed = 3.0f;
static const float rotationSpeed = 2.2f;

Player::Player(float startPosX, float startPosY): x(startPosX), y(startPosY), angle(0.0f), radius(0.18f){}

bool Player::checkCollision(float newX, float newY){
    float r = radius;

    float pts[4][2] = {
        {newX - r, newY - r},
        {newX + r, newY - r},
        {newX - r, newY + r},
        {newX + r, newY + r},
    };

    for(int i = 0; i < 4; i++){
        int mx = (int)floorf(pts[i][0]);
        int my = (int)floorf(pts[i][1]);

        int cell = currentMap -> getCell(mx,my);
        
        if(cell == tileWall){
            return true;
        }
    }
    return false;
}


bool Player::isOnElevator(){
    int mx = (int)floorf(x);
    int my = (int)floorf(y);
    
    return currentMap->getCell(mx, my) == tileElevator;
}

void Player::update(float dt){
    // rotating
        if(IsKeyDown(KEY_LEFT)){angle -= rotationSpeed * dt;}
        if(IsKeyDown(KEY_RIGHT)){angle += rotationSpeed * dt;}
    

    // make sure angle is in the range
    if(angle < -PI){angle += 2*PI;}
    if(angle > PI){angle -= 2*PI;}
    
    // move
    float forward = 0.0f;
    float strafe = 0.0f;

    if(IsKeyDown(KEY_W)){forward += 1.0f;}
    if(IsKeyDown(KEY_S)){forward -= 1.0f;}
    if(IsKeyDown(KEY_D)){strafe += 1.0f;}
    if(IsKeyDown(KEY_A)){strafe -= 1.0f;}
    
    float fx = cosf(angle);
    float fy = sinf(angle);
    float rx = -fy;
    float ry = fx;
    
    float vx = (fx * forward + rx * strafe) * moveSpeed;
    float vy = (fy * forward + ry * strafe) * moveSpeed;
    
    float newX = x + vx * dt;
    float newY = y + vy * dt;
    
    if(!checkCollision(newX, y)){x = newX;}
    if(!checkCollision(x, newY)){y = newY;}
}
