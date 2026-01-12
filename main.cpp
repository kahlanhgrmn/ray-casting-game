#include "game.h"
#include "map.h"
#include "player.h"
#include "raycaster.h"


static Vector2 tileToMini(float tx, float ty){
    return Vector2{
        (float)miniPad + tx * miniTile,
        (float)miniPad + ty * miniTile
    };
}


int main(){
    InitWindow(screenWidth, screenHeight, "Maze Explorer");
    InitAudioDevice();
    if(!IsAudioDeviceReady()) {
        TraceLog(LOG_ERROR, "Audio device failed to initialize!");
    }

    RenderTexture2D viewRT = LoadRenderTexture(internalW, internalH);
    SetTextureFilter(viewRT.texture, TEXTURE_FILTER_POINT);

    Texture2D levelTextures[3];
    levelTextures[0] = LoadTexture("assets/walls/wall0.png");
    levelTextures[1] = LoadTexture("assets/walls/wall1.png");
    levelTextures[2] = LoadTexture("assets/walls/wall3.jpeg");

    for(int i = 0; i < 3; i++){
        SetTextureFilter(levelTextures[i], TEXTURE_FILTER_POINT);
        SetTextureWrap(levelTextures[i], TEXTURE_WRAP_REPEAT);
    }

    Sound footstepSound = LoadSound("assets/sounds/421152__giocosound__footstep_wood_toe_2.wav");
    Sound elevatorSound = LoadSound("assets/sounds/old_elevator_door.mp3");

    SetSoundVolume(footstepSound, 0.3f);
    SetSoundVolume(elevatorSound, 1.0f);

    Music ambientFloor0 = LoadMusicStream("assets/sounds/254783__jonathantremblay__buzzing-light.wav");
    Music ambientFloor1 = LoadMusicStream("assets/sounds/upside_down_grin2.ogg");

    SetMusicVolume(ambientFloor0, 0.1f);
    SetMusicVolume(ambientFloor1, 0.5f);

    Music currentAmbient = ambientFloor0;
    PlayMusicStream(currentAmbient);

    initMaps();
    Player player(1.5f, 1.5f);

    SetTargetFPS(60);

    float footstepTimer = 0.0f;
    Vector2 lastPlayerPos = {player.x, player.y};

    bool inElevator = false;
    float elevatorTimer = 0.0f;
    float elevatorDuration = 3.0f;
    int targetFloor = 0;

    while(!WindowShouldClose()){
        float dt = GetFrameTime();

        UpdateMusicStream(currentAmbient);

        if(inElevator){
            elevatorTimer += dt;

            if(elevatorTimer >= elevatorDuration){
                inElevator = false;
                elevatorTimer = 0.0f;
                    
                StopMusicStream(currentAmbient);
                if(targetFloor == 0){
                    currentAmbient = ambientFloor0;
                } 
                else{
                    currentAmbient = ambientFloor1;
                }

                PlayMusicStream(currentAmbient);
                    
                setFloor(targetFloor);
                player.x = currentMap->elevatorPosition.x;
                player.y = currentMap->elevatorPosition.y;
            }
        }

        else{
            if(IsKeyPressed(KEY_SPACE)){
                setFloor((currentFloor+1) % 2);
            }

            if(IsKeyPressed(KEY_E)){
                if(player.isOnElevator()){
                    inElevator = true;
                    elevatorTimer = 0.0f;
                    targetFloor = (currentFloor + 1) % 2;

                    if(elevatorSound.frameCount > 0){
                        PlaySound(elevatorSound);
                    }
                }
            }
            player.update(dt);

            float distMoved = sqrtf(
                (player.x - lastPlayerPos.x) * (player.x - lastPlayerPos.x) + (player.y - lastPlayerPos.y) * (player.y - lastPlayerPos.y)
            );

            if(distMoved > 0.01f){
                footstepTimer += dt;

                if(footstepTimer >= footstepInterval) {
                    PlaySound(footstepSound);
                    footstepTimer = 0.0f;
                }
            } 
            else{
                footstepTimer = 0.0f;
            }
            
            lastPlayerPos = {player.x, player.y};
        }

        float startingAngle = player.angle - fov * 0.5f;

        Texture2D currentTexture = levelTextures[currentFloor % 3];


        if(inElevator){
            BeginDrawing();
            ClearBackground(BLACK);
            
            float progress = elevatorTimer / elevatorDuration;
            int doorWidth = (int)(screenWidth * 0.5f * progress);

            if(progress < 0.5f){
                DrawRectangle(0, 0, doorWidth, screenHeight, Color{40, 40, 40, 255});
                DrawRectangle(screenWidth - doorWidth, 0, doorWidth, screenHeight, Color{40, 40, 40, 255});
            } 

            else{
                int openWidth = (int)(screenWidth * 0.5f * (1.0f - progress));
                DrawRectangle(0, 0, openWidth, screenHeight, Color{40, 40, 40, 255});
                DrawRectangle(screenWidth - openWidth, 0, openWidth, screenHeight, Color{40, 40, 40, 255});
            }
            
            const char* floorText = TextFormat("Going to Floor %d...", targetFloor + 1);
            int textWidth = MeasureText(floorText, 40);

            DrawText(floorText, screenWidth/2 - textWidth/2, screenHeight/2 - 20, 40, WHITE);
            
            EndDrawing();
        }

        else{
            BeginTextureMode(viewRT);
            ClearBackground(RAYWHITE);

            int horizon = internalH / 2;

            Color roofColour = Color{90, 90, 120, 255};
            Color floorColour   = Color{60, 60, 60, 255};

            DrawRectangle(0, 0, internalW, horizon, roofColour); // roof
            DrawRectangle(0, horizon, internalW, internalH - horizon, floorColour); // floor


            for(int i = 0; i < numRays; i++){
                float t = (float)i / (float)(numRays -1);
                float rayAngle = startingAngle + t * fov;

                RayHit hit = castRay(player.x, player.y, rayAngle);

                if(hit.cell == 0){
                    continue;
                }

                // calculate perpendicular distance first
                float perpDistance = hit.dist * cosf(rayAngle - player.angle);
                if(perpDistance < 0.0001f){
                    perpDistance = 0.0001f;
                }

                // then calc wall dimentions after
                float wallHeight = (1.0f / perpDistance) * projectPlaneDist;

                int sliceX = i;
                int sliceW = 1;
                int sliceY = horizon - (int)(wallHeight * 0.5f);
                int sliceH = (int)wallHeight;

                if(sliceY < 0) {sliceH += sliceY; sliceY = 0;}
                if(sliceY + sliceH > internalH) {sliceH = internalH - sliceY;}

                if(sliceH <= 0){
                    continue;
                }

                float shade = 1.0f - perpDistance / maxRayDist;
                if(shade < 0.0f) shade = 0.0f;
                if(hit.side == 1) shade *= 0.65f;

                float wallX;
                if(hit.side == 0){
                    wallX = hit.hitY;
                } 
                else{
                    wallX = hit.hitX;  
                }
                wallX -= floorf(wallX);

                int textureX = (int)(wallX * (float)currentTexture.width);
                if(textureX < 0) textureX = 0;
                if(textureX >= currentTexture.width) textureX = currentTexture.width - 1;

                if(hit.side == 0 && cosf(rayAngle) > 0) textureX = currentTexture.width - textureX - 1;
                if(hit.side == 1 && sinf(rayAngle) < 0) textureX = currentTexture.width - textureX - 1;

                Rectangle src = {(float)textureX, 0.0f, 1.0f, (float)currentTexture.height};
                Rectangle dst = {(float)sliceX, (float)sliceY, (float)sliceW, (float)sliceH};

                unsigned char tc = (unsigned char)(80 + shade * 175);
                Color tint = { tc, tc, tc, 255 };

                DrawTexturePro(currentTexture, src, dst, Vector2{0,0}, 0.0f, tint);
            }
            EndTextureMode();

            // MINIMAP

            BeginDrawing();
            ClearBackground(RAYWHITE);

            float scale = fminf((float)screenWidth / internalW, (float)screenHeight / internalH);
            float dstW = internalW * scale;
            float dstH = internalH * scale;
            float dstX = (screenWidth  - dstW) * 0.5f;
            float dstY = (screenHeight - dstH) * 0.5f;
            

            Rectangle src = { 0, 0, (float)internalW, -(float)internalH };
            Rectangle dst = { dstX, dstY, dstW, dstH };

            DrawTexturePro(viewRT.texture, src, dst, Vector2{0,0}, 0.0f, WHITE);

            int miniW = currentMap->width * miniTile;
            int miniH = currentMap->height * miniTile;
            DrawRectangle(miniPad - 6, miniPad - 6, miniW + 12, miniH + 12, Color{0,0,0,120});

            for(int y = 0; y < currentMap->height; y++){
                for(int x = 0; x < currentMap->width; x++){

                    Vector2 p = tileToMini((float)x, (float)y);


                    Color tileCol;
                    int cellType = currentMap->getCell(x, y);

                    if(cellType == tileWall){
                        tileCol = Color{70,70,70,200};
                    } 
                    else if(cellType == tileElevator){
                        tileCol = Color{100,255,100,200};
                    } 
                    else{
                        tileCol = Color{200,200,200,140};
                    }


                    DrawRectangle((int)p.x, (int)p.y, miniTile, miniTile, tileCol);
                    DrawRectangleLines((int)p.x, (int)p.y, miniTile, miniTile, Color{0,0,0,50});
                }
            }


            Vector2 pMini = tileToMini(player.x, player.y);
            DrawCircleV(pMini, player.radius * miniTile, GREEN);
            DrawLineV(pMini, tileToMini(player.x + cosf(player.angle) * 0.8f, player.y + sinf(player.angle) * 0.8f), DARKGREEN);

            if(player.isOnElevator()){
                DrawText("Press E to use elevator", screenWidth/2 - 100, screenHeight - 100, 20, YELLOW);
            }

            DrawText(TextFormat("Floor: %d | Press E to interact | Press SPACE to switch floors", 
                currentFloor + 1), 10, 10, 20, WHITE
            );

            DrawText(TextFormat("angle=%.2f rad (%.0f deg)", player.angle, player.angle * 180.0f / PI), 40, screenHeight - 70, 18, BLACK);
            EndDrawing();
        }
    }

    UnloadSound(footstepSound);
    if(elevatorSound.frameCount > 0){UnloadSound(elevatorSound);}
    UnloadMusicStream(ambientFloor0);
    UnloadMusicStream(ambientFloor1);

    for(int i = 0; i < 3; i++){
        UnloadTexture(levelTextures[i]);
    }
    UnloadRenderTexture(viewRT);
    cleanupMaps();

    CloseAudioDevice();
    CloseWindow();
    return 0;
}