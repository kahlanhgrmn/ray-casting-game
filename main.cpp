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
    int side; // 1 = hitting horizontal wall, 0 = hitting vertical wall
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
        {x - r, y - r},
        {x + r, y - r},
        {x - r, y + r},
        {x + r, y + r},
    };

    for(int i = 0; i < 4; i++){
        if(cellPos(pts[i][0], pts[i][1]) != 0){
            return true;
        }
        
    }
    return false;
}

static RayHit castRay(float px, float py, float a){ // casting DDA rays
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

        if(!inBoundary(mapX, mapY)){
            float distance = (side==0)? (sideDistanceX - deltaDistanceX) : (sideDistanceY - deltaDistanceY);

            if(distance > maxRayDist){
                distance = maxRayDist;
            }

            float hitX = px + rayDirectionX * distance;
            float hitY = py + rayDirectionY * distance;

            return RayHit{distance, hitX, hitY, 1, side};
        }

        cell = mapGrid[mapY][mapX];
        if(cell != 0){
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

    float distance;

    if(side == 0){
        distance = (mapX - px + (1-stepX) / 2.0f)/ rayDirectionX;
    }
    else{
        distance = (mapY - py + (1-stepY) / 2.0f) / rayDirectionY;
    }

    if(distance < 0.0001f){
        distance = 0.0001f;
    }
    if(distance > maxRayDist){
        distance = maxRayDist;
    }

    float hitX = px + rayDirectionX * distance;
    float hitY = py + rayDirectionY *distance;

    return RayHit{distance, hitX, hitY, cell, side};

}


static Vector2 tileToMini(float tx, float ty) {
    return Vector2{
        (float)miniPad + tx * miniTile,
        (float)miniPad + ty * miniTile
    };
}


int main(){
    InitWindow(screenWidth, screenHeight, "Maze Explorer");

    Texture2D wallTexture = LoadTexture("assets/walls/wall0.png");
    if(wallTexture.id == 0){
        TraceLog(LOG_ERROR, "Failed to load wall texture.");
    }

    SetTextureFilter(wallTexture, TEXTURE_FILTER_BILINEAR);
    SetTextureWrap(wallTexture, TEXTURE_WRAP_CLAMP);

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

            float perpDistance = hit.dist;
            if(perpDistance < 0.0001f){
                perpDistance = 0.0001f;

            }

            float wallHeight = (1.0f / perpDistance)* projectPlaneDist;
            

            float colWf = (float)screenWidth / (float)numRays;
            int sliceX = (int)floorf(i * colWf);
            int nextX  = (int)floorf((i + 1) * colWf);
            int sliceW = nextX - sliceX;
            if(sliceW < 1){
                sliceW = 1;
            }
            int sliceY = horizon - (int)(wallHeight/2);
            int sliceH = (int)wallHeight;


            float shade = 1.0f - perpDistance / maxRayDist;
            if(shade < 0.0f){
                shade = 0.0f;
            }

            if(hit.side == 1){
                shade *= 0.65f;
            }

            unsigned char c = (unsigned char)(80 + shade * 175);

            if(sliceY < 0) {sliceH += sliceY; sliceY = 0;}
            if(sliceY + sliceH > screenHeight) {sliceH = screenHeight - sliceY;}

            if(sliceH > 0){
                if(hit.cell == 0){
                    continue;
                }

                float rayDirectionX = cosf(rayAngle);
                float rayDirectionY = sinf(rayAngle);

                float u;

                if(hit.side == 0){
                    u = hit.hitY - floorf(hit.hitY);
                }
                else{
                    u = hit.hitX - floorf(hit.hitX);
                }

                if(hit.side == 0 && (rayDirectionX > 0)){
                    u = 1.0f - u;
                }
                if( hit.side == 1 && (rayDirectionY < 0)){
                    u = 1.0f - u;
                }

                int textureX = (int)( u * (wallTexture.width - 1));
                // clamping textures for safety
                if(textureX < 0){
                    textureX = 0;
                }
                if(textureX >= wallTexture.width){
                    textureX = wallTexture.width - 1;
                }

                Rectangle src = { (float)textureX, 0.0f, 1.0f, (float)wallTexture.height };
                Rectangle dst = { (float)sliceX, (float)sliceY, (float)sliceW, (float)sliceH };

                unsigned char tc = (unsigned char)(80 + shade * 175);
                Color tint = { tc, tc, tc, 255 };

                DrawTexturePro(wallTexture, src, dst, Vector2{0,0}, 0.0f, tint);

            }
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
            if(hit.cell != 0){
                Vector2 rEnd = tileToMini(hit.hitX, hit.hitY);

                DrawLineV(pMini, rEnd, Color{30,80,200,90});
            }
            
        }
        EndDrawing();
    }

    UnloadTexture(wallTexture);

    CloseWindow();
    return 0;
}