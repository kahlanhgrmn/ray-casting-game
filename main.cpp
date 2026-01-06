#include "raylib.h"
#include <cmath>

static const int screenWidth = 900;
static const int screenHeight = 700;

static const int mapWidth = 16;
static const int mapHeight = 16;

static const int tileSize = 36;
static const int mapOffsetX = 40;
static const int mapOffsetY = 40;

static const float moveSpeed = 3.0f;
static const float playerRadius = 0.18f;

static int mapGrid[mapHeight][ mapWidth] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
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

static bool inBoundry(int mx, int my){
    return mx >= 0 && mx < mapWidth && my >= 0 && my < mapHeight;
}

static int cellPos(float x, float y){
    int mx = (int)floorf(x);
    int my = (int)floorf(y);

    if(!inBoundry(mx, my)){
        return 1;
    }

    return mapGrid[my][mx];
}

static bool collides(float x, float y){
    float corners = playerRadius;

    float pts[4][2] = {
        {x + corners, y},
        {x - corners, y},
        {x, y + corners},
        {x, y - corners},
    };

    for(int i = 0; i < 4; i++){
        if(cellPos(pts[i][0], pts[i][1]) != 0){
            return true;
        }
        
    }
    return false;
}

static Vector2 tileToScreen(float tx, float ty){
    return Vector2{
        (float)mapOffsetX + tx * tileSize,
        (float)mapOffsetY + ty * tileSize
    };
}

int main(){
    InitWindow(screenWidth, screenHeight, "Maze Explorer");

    SetTargetFPS(60);

    float px = 1.5f;
    float py = 1.5f;

    while(!WindowShouldClose()){
        float dt = GetFrameTime();

        float dx = 0.0f;
        float dy = 0.0f;

        if(IsKeyDown(KEY_W)){
            dy -= 1.0f;
        }
        if(IsKeyDown(KEY_S)){
            dy += 1.0f;
        }
        if(IsKeyDown(KEY_A)){
            dx -= 1.0f;
        }
        if(IsKeyDown(KEY_D)){
            dx += 1.0f;
        }

        float length = sqrtf(dx * dx + dy * dy);

        if(length > 0.0001f){
            dx /= length;
            dy /= length;
        }

        float newPx = px + dx * moveSpeed * dt;
        float newPy = py + dy * moveSpeed * dt;

        if(!collides(newPx, py)){
            px = newPx;
        }
        if(!collides(px, newPy)){
            py = newPy;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        for(int y= 0; y < mapHeight; y++){
            for(int x = 0; x < mapWidth; x++){
                Rectangle r = {
                    (float)mapOffsetX + x * tileSize,
                    (float)mapOffsetY + y * tileSize,
                    (float)tileSize,
                    (float)tileSize
                };

                if(mapGrid[y][x] == 1){
                    DrawRectangleRec(r, DARKGRAY);
                } 
                else {
                    DrawRectangleRec(r, Color{ 230, 230, 230, 255 });
                }

                DrawRectangleLines((int)r.x, (int)r.y, (int)r.width, (int)r.height, GRAY);

            }
        }

        Vector2 playerScreenPos = tileToScreen(px, py);
        DrawCircleV(playerScreenPos, playerRadius * tileSize, GREEN);

        int tx = (int)floorf(px);
        int ty = (int)floorf(py);

        DrawText(TextFormat("Player (tile units): x=%.2f y=%.2f | tile=(%d,%d)",
                            px, py, tx, ty), 40, screenHeight - 40, 18, BLACK);

        DrawText("Use WASD to move the player", 40, 10, 18, BLACK);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}