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

static const float rotationSpeed = 2.2f;
static const float dirLineLength = 0.8f;

static const float fov = 60.0f * (PI / 180.0f); // converting to rad
static const int numRays = 160;


static const float maxRayDist = 24.0f;
static const float rayStep = 0.02f;

struct RayHit {
    float dist;
    float hitX;
    float hitY;
    int cell;  // what is being hit (1 = wall, 0 = nothing)
};

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

static bool inBoundary(int mx, int my){
    return mx >= 0 && mx < mapWidth && my >= 0 && my < mapHeight;
}

static int cellPos(float x, float y){
    int mx = (int)floorf(x);
    int my = (int)floorf(y);

    if(!inBoundary(mx, my)){
        return 1;
    }

    return mapGrid[my][mx];
}

static bool collides(float x, float y){
    float r = playerRadius;

    float pts[4][2] = {
        {x + r, y},
        {x - r, y},
        {x, y + r},
        {x, y - r},
    };

    for(int i = 0; i < 4; i++){
        if(cellPos(pts[i][0], pts[i][1]) != 0){
            return true;
        }
        
    }
    return false;
}

static RayHit castRay(float px, float py, float a){
    float dx = cosf(a);
    float dy = sinf(a);
    float t = 0.0f;

    while(t < maxRayDist){
        float x = px + dx * t;
        float y = py + dy *t;

        int cell = cellPos(x, y);

        if(cell != 0){
            return RayHit{t, x, y, cell};
        }
        t += rayStep;
    }
    return RayHit{maxRayDist, px + dx * maxRayDist, py + dy * maxRayDist, 0};
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
    float a  = 0.0f; // player angle

    while(!WindowShouldClose()){
        float dt = GetFrameTime();

        if(IsKeyDown(KEY_LEFT)){
            a -= rotationSpeed * dt;
        }
        if(IsKeyDown(KEY_RIGHT)){
            a += rotationSpeed * dt;
        }

        if(a < -PI){
            a += 2*PI;
        }
        if(a >  PI){
            a -= 2*PI;
        }

        float forward = 0.0f;
        float strafe  = 0.0f;

        if(IsKeyDown(KEY_W)){
            forward += 1.0f;
        }
        if(IsKeyDown(KEY_S)){
            forward -= 1.0f;
        }
        if(IsKeyDown(KEY_D)){
            strafe  += 1.0f;
        }
        if(IsKeyDown(KEY_A)){
            strafe  -= 1.0f;
        }

        float fx = cosf(a);
        float fy = sinf(a);

        float rx = -fy;
        float ry =  fx;

        float vx = (fx * forward + rx * strafe) * moveSpeed;
        float vy = (fy * forward + ry * strafe) * moveSpeed;

        float newPx = px + vx * dt;
        float newPy = py + vy * dt;

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

        float ex = px + cosf(a) * dirLineLength;
        float ey = py + sinf(a) * dirLineLength;

        Vector2 endPos = tileToScreen(ex, ey);
        DrawLineV(playerScreenPos, endPos, DARKGREEN);

        DrawText(TextFormat("angle=%.2f rad (%.0f deg)", a, a * 180.0f / PI), 40, screenHeight - 70, 18, BLACK);


        float startingAngle = a - fov * 0.5f;

        for(int i = 0; i < numRays; i++){
            float t = (float)i / (float)(numRays-1);
            float rayAngle = startingAngle + t * fov;

            RayHit hit = castRay(px, py, rayAngle);
            Vector2 endOfRay = tileToScreen(hit.hitX, hit.hitY);

            DrawLineV(playerScreenPos, endOfRay, Color{30, 80, 200, 90});
        }

        RayHit centerHit = castRay(px, py, a);
        Vector2 centerEnd = tileToScreen(centerHit.hitX, centerHit.hitY);
        
        DrawLineV(playerScreenPos, centerEnd, BLUE);
        DrawCircleV(centerEnd, 4.0f, BLUE);


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