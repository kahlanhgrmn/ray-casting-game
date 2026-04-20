#include "game.h"
#include "map.h"
#include "player.h"
#include "raycaster.h"
#include "sprite.h"
#include "inventory.h"

// for note text
static void DrawMultilineText(const char* text, int x, int y, int fontSize, int lineGap, Color col){
    int start = 0;
    int len = 0;

    while(text[start] != '\0'){
        len = 0;

        while(text[start + len] != '\0' && text[start + len] != '\n'){
            len++;
        }

        char line[512];
        int copyLen = (len < 511) ? len : 511;

        for(int i = 0; i < copyLen; i++){
            line[i] = text[start + i];
        }
        line[copyLen] = '\0';

        DrawText(line, x, y, fontSize, col);

        y += fontSize + lineGap;

        start += len;
        if(text[start] == '\n'){
            start++;
        }
    }
}


static Vector2 tileToMini(float tx, float ty){
    return Vector2{
        (float)miniPad + tx * miniTile,
        (float)miniPad + ty * miniTile
    };
}


int main(){
    // COUNTERS AND FLOORS
    const int finalFloor = 4; // basement
    float acceptanceScore = 0.0f;
    float avoidanceScore = 0.0f;

    float timeOnFloor = 0.0f;
    int lastFloor = 0;

    // ENDINGS
    bool repressionMode = false;  // once repression ending is decided, elevator return to lobby
    bool acceptanceEnding = false; // acceptance cutscene mode
    float endingTimer = 0.0f;

    // HUD
    const char* hudMessage = nullptr;
    float hudMessageTimer = 0.0f;

    // NOTES
    bool noteOpen = false;
    int openNoteId = -1;
    int openNoteSpriteIdx = -1;

    std::vector<const char*> notes;
    notes.push_back(""); // note so that notes[1] will be valid
    notes.push_back("The light hum is getting worse.\nI can't tell if it's the fixture or me.\n\nIf you hear the elevator move by itself:\nDO NOT LOOK UP.");
    notes.push_back("MAINTENANCE LOG:\nPanel keeps resetting to FLOOR 0.\nWe \"fix\" it, and it returns.\n\nGuests say the lobby feels like home.");
    notes.push_back("They keep telling me to wake up.\nBut every time I try,\nthe hotel pulls me back down.");




    InitWindow(screenWidth, screenHeight, "Maze Explorer");
    InitAudioDevice();
    if(!IsAudioDeviceReady()) {
        TraceLog(LOG_ERROR, "Audio device failed to initialize!");
    }

    RenderTexture2D viewRT = LoadRenderTexture(internalW, internalH);
    SetTextureFilter(viewRT.texture, TEXTURE_FILTER_POINT);

    // WALLS
    Texture2D levelTextures[3];
    levelTextures[0] = LoadTexture("assets/walls/wall0.png");
    levelTextures[1] = LoadTexture("assets/walls/wall1.png");
    levelTextures[2] = LoadTexture("assets/walls/wall3.jpeg");

    for(int i = 0; i < 3; i++){
        SetTextureFilter(levelTextures[i], TEXTURE_FILTER_POINT);
        SetTextureWrap(levelTextures[i], TEXTURE_WRAP_REPEAT);
    }

    // SPRITES
    Texture2D spriteTextures[2];
    spriteTextures[0] = LoadTexture("assets/sprites/crate0.png");
    spriteTextures[1] = LoadTexture("assets/sprites/barrel0.png");

    TraceLog(LOG_INFO, "Crate loaded: %dx%d", spriteTextures[0].width, spriteTextures[0].height);
    TraceLog(LOG_INFO, "Barrel loaded: %dx%d", spriteTextures[1].width, spriteTextures[1].height);


    for(int i = 0; i < 2; i++){
        SetTextureFilter(spriteTextures[i], TEXTURE_FILTER_POINT);
    }

    SpriteManager spriteManager(spriteTextures, 2);
    float* zBuffer = new float[internalW];

    // INVENTORY AND COLLECTABLES
    Inventory inventory;

    inventory.addItem("Worn Cable Spool", "Frayed and stained with age", 1);
    inventory.addItem("Control Panel", "Buttons stick when pressed", 0);
    inventory.addItem("Circuit Board", "Burnt smell lingers", 0);


    spriteManager.addSprite(8.5f, 8.5f, 1, 1, 0);
    spriteManager.addSprite(10.5f, 6.5f, 0, 2, 1);
    spriteManager.addSprite(12.5f, 12.5f, 0, 3, 2);


    // NOTE SPRITES
    spriteManager.addSprite(3.5f, 3.5f, 0, 1, -2);
    spriteManager.addSprite(12.5f, 3.5f, 0, 2, -3);
    spriteManager.addSprite(8.5f, 12.5f, 0, 3, -4);


    // SOUNDS
    Sound footstepSound = LoadSound("assets/sounds/421152__giocosound__footstep_wood_toe_2.wav");
    Sound elevatorSound = LoadSound("assets/sounds/old_elevator_door.mp3");

    SetSoundVolume(footstepSound, 0.1f);
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

        if(hudMessageTimer > 0.0f){
            hudMessageTimer -= dt;
        }
        else{
            hudMessage = nullptr;
        }

        timeOnFloor += dt;
        // acceptence ending - lingering and exploring will give more points to this ending
        if(!player.isOnElevator()){
            acceptanceScore += dt * 0.05f;
        }

        UpdateMusicStream(currentAmbient);

        if(inElevator){
            elevatorTimer += dt;

            if(elevatorTimer >= elevatorDuration){
                inElevator = false;
                elevatorTimer = 0.0f;
                    
                StopMusicStream(currentAmbient);
                if(targetFloor == finalFloor){
                    currentAmbient = ambientFloor1;
                } 
                else{
                    currentAmbient = ambientFloor0;
                }

                PlayMusicStream(currentAmbient);
                    
                setFloor(targetFloor);

                // after arriving
                if(acceptanceEnding){
                    // start ending/ fade out timer
                    endingTimer = 0.0f;
                }


                player.x = currentMap->elevatorPosition.x;
                player.y = currentMap->elevatorPosition.y;
            }
        }

        else{
            int nearbyItemSprite = spriteManager.checkNearbySprite(player.x, player.y, 0.8f, currentFloor);
            int nearbyNoteSprite = spriteManager.checkNearbyNote(player.x, player.y, 0.8f, currentFloor);



            if(IsKeyPressed(KEY_TAB)){ // inventory
                inventory.toggleInventory();
            }

            if(noteOpen){ // interaction with sprites
                if(IsKeyPressed(KEY_F)){
                    noteOpen = false;
                    openNoteId = -1;
                    openNoteSpriteIdx = -1;
                }
                // skip movement and interaction while reading a note
            }
            else{
                if(IsKeyPressed(KEY_F)){
                    // notes are first priority then pickup items
                    if(nearbyNoteSprite >= 0){
                        Sprite &s = spriteManager.sprites[nearbyNoteSprite];
                        int noteId = -s.inventoryItemIndex - 1;

                        noteOpen = true;
                        openNoteId = noteId;
                        openNoteSpriteIdx = nearbyNoteSprite;

                        // adds to acceptance ending score when reading notes for first time
                        if(!s.collected){
                            s.collected = true;
                            acceptanceScore += 1.0f;
                        }
                    }

                    else if(nearbyItemSprite >= 0){
                        Sprite& sprite = spriteManager.sprites[nearbyItemSprite];

                        if(sprite.inventoryItemIndex >= 0 && !sprite.collected){
                            sprite.collected = true;
                            inventory.collectItem(sprite.inventoryItemIndex);
                        }
                    }
                }

            }

            if(IsKeyPressed(KEY_E)){ // interact elevator
                if(player.isOnElevator()){
                    inElevator = true;
                    elevatorTimer = 0.0f;

                    // if repression/ avoidance ending, elevator always leads to the lobby
                    if(repressionMode){
                        targetFloor = 0;
                    }
                    else{
                        // game chooses your ending in the basement
                        if(currentFloor >= finalFloor){
                            // avoidance vs acceptance decision:
                            // rushing and not exploring leads to the avoidance ending
                            bool chooseRepression = (avoidanceScore > acceptanceScore + 1.5f);

                            if(chooseRepression){
                                repressionMode = true;
                                targetFloor = 0;
                            } 
                            else{
                                acceptanceEnding = true;
                                targetFloor = 0;
                            }
                        }

                        else{
                            int nextFloor = currentFloor + 1;

                            // restrict access to basement (floor 4)
                            if(currentFloor == finalFloor - 1){ // on floor 3 going to 4
                                if(inventory.getCollectedCount() < inventory.totalItems){
                                    // deny elevator use
                                    inElevator = false;
                                    elevatorTimer = 0.0f;

                                    hudMessage = TextFormat("Elevator panel: Missing components (%d/%d).", inventory.getCollectedCount(), inventory.totalItems);
                                    hudMessageTimer = 2.5f;

                                    // play "locked" sound later
                                    continue;
                                }
                            }

                            targetFloor = nextFloor;
                        }
                    }

                    // score based on how fast you left
                    if(timeOnFloor < 25.0f){
                        avoidanceScore += 1.0f;
                    }
                    else{
                        acceptanceScore += 0.5f;
                    }

                    timeOnFloor = 0.0f; // reset for next floor

                    if(elevatorSound.frameCount > 0){
                        PlaySound(elevatorSound);
                    }
                }
            }

            if(!noteOpen){
                player.update(dt);
            }

            if(acceptanceEnding && !inElevator){
                endingTimer += dt;

                if(endingTimer > 2.5f){
                    // end the game cleanly for now
                    break;
                }
            }


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
        if(currentTexture.id == 0 || currentTexture.width == 0){
            currentTexture = levelTextures[0]; // if textures aren't working
        }


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
            
            const char* floorText = TextFormat("Going to Floor %d...", targetFloor);
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
                zBuffer[i] = maxRayDist;
            }

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

                zBuffer[i] = perpDistance;

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

            spriteManager.renderSprites(player.x, player.y, player.angle, viewRT, horizon, zBuffer, currentFloor);

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

            int nearbyItem = spriteManager.checkNearbySprite(player.x, player.y, 0.8f, currentFloor);
            int nearbyNote = spriteManager.checkNearbyNote(player.x, player.y, 0.8f, currentFloor);

            if(!noteOpen){

                float dxMove = player.x - lastPlayerPos.x;
                float dyMove = player.y - lastPlayerPos.y;
                float distMoved = sqrtf(dxMove*dxMove + dyMove*dyMove);


                if(distMoved > 0.01f){
                    footstepTimer += dt;
                    if(footstepTimer >= footstepInterval){
                        PlaySound(footstepSound);
                        footstepTimer = 0.0f;
                    }
                } 
                else {footstepTimer = 0.0f;}

                lastPlayerPos = {player.x, player.y};


                if(nearbyNote >= 0){
                    DrawText("Press F to read", screenWidth/2 - 90, screenHeight - 150, 20, YELLOW);
                }
                else if(nearbyItem >= 0){
                    DrawText("Press F to collect", screenWidth/2 - 100, screenHeight - 150, 20, YELLOW);
                }
            }
            else{
                DrawText("Press F to put down", screenWidth/2 - 130, screenHeight - 150, 20, GRAY);
            }

            if(player.isOnElevator()){
                DrawText("Press E to use elevator", screenWidth/2 - 100, screenHeight - 100, 20, YELLOW);
            }

            DrawText(TextFormat("Floor: %d | Press E to interact", currentFloor), 10, 10, 20, WHITE);

            DrawText(TextFormat("angle=%.2f rad (%.0f deg)", player.angle, player.angle * 180.0f / PI), 40, screenHeight - 70, 18, BLACK);

            if(!inElevator){
                inventory.renderHUD(screenWidth, screenHeight);


                // inventory / hud
                if(hudMessage){
                    DrawText(hudMessage, screenWidth/2 - MeasureText(hudMessage, 22)/2, screenHeight - 60, 22, ORANGE);
                }

                // repression ending text
                if(currentFloor == 0 && repressionMode){
                    DrawText("WELCOME HOME.", screenWidth/2 - 120, 60, 40, WHITE);
                    DrawText("Your room is ready.", screenWidth/2 - 110, 105, 20, GRAY);
                }

                // when reading note
                if(noteOpen && openNoteId > 0 && openNoteId < (int)notes.size()){
                DrawRectangle(0, 0, screenWidth, screenHeight, Color{0,0,0,160});

                int boxW = (int)(screenWidth * 0.75f);
                int boxH = (int)(screenHeight * 0.65f);
                int boxX = (screenWidth - boxW)/2;
                int boxY = (screenHeight - boxH)/2;

                DrawRectangle(boxX, boxY, boxW, boxH, Color{20,20,20,240});
                DrawRectangleLines(boxX, boxY, boxW, boxH, Color{200,200,200,120});

                DrawText("NOTE", boxX + 20, boxY + 15, 30, RAYWHITE);

                DrawMultilineText(notes[openNoteId], boxX + 20, boxY + 60, 22, 8, RAYWHITE);

                DrawTextEx(GetFontDefault(), "Press F to put it down",
                    (Vector2){(float)boxX + 20, (float)(boxY + boxH - 40)}, 22.0f, 2.0f, GRAY
                );
            }
            }

            inventory.renderFullInventory(screenWidth, screenHeight, spriteTextures);



            EndDrawing();
        }
    }

    StopMusicStream(currentAmbient);
    StopSound(elevatorSound);
    StopSound(footstepSound);


    // CLEANUP

    UnloadSound(footstepSound);
    if(elevatorSound.frameCount > 0){UnloadSound(elevatorSound);}
    UnloadMusicStream(ambientFloor0);
    UnloadMusicStream(ambientFloor1);

    for(int i = 0; i < 3; i++){
        UnloadTexture(levelTextures[i]);
    }
    UnloadRenderTexture(viewRT);
    cleanupMaps();
    delete[] zBuffer;

    CloseAudioDevice();
    CloseWindow();
    return 0;
}