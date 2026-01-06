#include "raylib.h"
#include <cmath>

static const int screenWidth = 900;
static const int screenHeight = 700;

static const int mapWidth = 16;
static const int mapHeight = 16;

static const float moveSpeed = 3.0f;
static const float playerRadius = 0.18f;

static const float rotationSpeed = 2.2f;
static const float dirLineLength = 0.8f;

static const float fov = 60.0f * (PI / 180.0f); // converting to rad
static const int numRays = 160;

static const float maxRayDist = 24.0f;
static const float rayStep = 0.02f;

static const float projectPlaneDist = (screenWidth / 2.0f) / tanf(fov/ 2.0f);
static const int viewH = screenHeight;

static const int miniTile = 12;
static const int miniPad  = 12;
static const int miniW = mapWidth * miniTile;
static const int miniH = mapHeight * miniTile;


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


static Vector2 tileToMini(float tx, float ty) {
    return Vector2{
        (float)miniPad + tx * miniTile,
        (float)miniPad + ty * miniTile
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

        int viewTop = 0;
        int horizon = viewTop + viewH / 2;

        Color roofColour = Color{90, 90, 120, 255};
        Color floorColour   = Color{60, 60, 60, 255};

        DrawRectangle(0, viewTop, screenWidth, viewH/2, roofColour); // roof
        DrawRectangle(0, horizon, screenWidth, viewH/2, floorColour); // floor

        float startingAngle = a - fov * 0.5f;

        for(int i = 0; i < numRays; i++){
            float t = (float)i / (float)(numRays -1);
            float rayAngle = startingAngle + t * fov;

            RayHit hit = castRay(px, py, rayAngle);

            float correctedFisheyeDist = hit.dist * cosf(rayAngle - a);
            if(correctedFisheyeDist < 0.0001f){
                correctedFisheyeDist = 0.0001f;
            }

            float wallHeight = (1.0f/ correctedFisheyeDist) * projectPlaneDist;

            int sliceX = (int)(t * screenWidth);
            int sliceW = screenWidth/ numRays + 1;
            int sliceY = horizon - (int)(wallHeight/2);
            int sliceH = (int)wallHeight;

            

            float shade = 1.0f - correctedFisheyeDist / maxRayDist;
            if(shade < 0.0f){
                shade = 0.0f;
            }

            unsigned char c = (unsigned char)(80 + shade * 175);
            Color wallColour = Color{c, c, c, 255};

            if(sliceY < 0) {sliceH += sliceY; sliceY = 0;}
            if(sliceY + sliceH > screenHeight) {sliceH = screenHeight - sliceY;}
            if(sliceH > 0) {DrawRectangle(sliceX, sliceY, sliceW, sliceH, wallColour);}
        }

        DrawRectangle(miniPad - 6, miniPad - 6, miniW + 12, miniH + 12, Color{0,0,0,120});

        for(int y = 0; y < mapHeight; y++){
            for(int x = 0; x < mapWidth; x++){

                Vector2 p = tileToMini((float)x, (float)y);

                Color tileCol = (mapGrid[y][x] == 1)? Color{70,70,70,200} : Color{200,200,200,140};

                DrawRectangle((int)p.x, (int)p.y, miniTile, miniTile, tileCol);
                DrawRectangleLines((int)p.x, (int)p.y, miniTile, miniTile, Color{0,0,0,50});
            }
        }


        Vector2 pMini = tileToMini(px, py);
        DrawCircleV(pMini, playerRadius * miniTile, GREEN);

        Vector2 dMini = tileToMini(px + cosf(a)*dirLineLength, py + sinf(a)*dirLineLength);
        DrawLineV(pMini, dMini, DARKGREEN);


        DrawText(TextFormat("angle=%.2f rad (%.0f deg)", a, a * 180.0f / PI), 40, screenHeight - 70, 18, BLACK);


        for(int i = 0; i < numRays; i++){
            float t = (float)i / (float)(numRays - 1);
            float rayAngle = startingAngle + t * fov;

            RayHit hit = castRay(px, py, rayAngle);
            Vector2 rEnd = tileToMini(hit.hitX, hit.hitY);

            DrawLineV(pMini, rEnd, Color{30,80,200,90});
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}