#include "inventory.h"

Inventory::Inventory(): totalItems(0), collectedCount(0), inventoryOpen(false){}

void Inventory::addItem(const std::string& name, const std::string& description, int spriteIdx){
    items.push_back({name, description, spriteIdx, false});

    totalItems++;
}

void Inventory::collectItem(int itemIdx){
    if(itemIdx >= 0 && itemIdx < items.size() && !items[itemIdx].collected){
        items[itemIdx].collected = true;

        collectedCount++;
    }
}

bool Inventory::isCollected(int itemIdx){
    if(itemIdx >= 0 && itemIdx < items.size()){
        return items[itemIdx].collected;
    }

    return false;
}

int Inventory::getCollectedCount(){
    return collectedCount;
}

void Inventory::toggleInventory(){
    inventoryOpen = !inventoryOpen;
}

void Inventory::renderHUD(int screenWidth, int screenHeight){
    const char* hudText = TextFormat("Parts: %d/%d", collectedCount, totalItems);
    int textWidth = MeasureText(hudText, 24);

    DrawRectangle(screenWidth - textWidth - 30, 10, textWidth + 20, 40, Color{0, 0, 0, 150});

    DrawText(hudText, screenWidth - textWidth - 20, 20, 24, WHITE);
}

void Inventory::renderFullInventory(int screenWidth, int screenHeight, Texture2D* spriteTextures){
    if(!inventoryOpen){
        return;
    }

    DrawRectangle(0, 0, screenWidth, screenHeight, Color{0, 0, 0, 200});

    int boxWidth = 600;
    int boxHeight = 500;
    int boxX = (screenWidth - boxWidth) / 2;
    int boxY = (screenHeight - boxHeight) / 2;
    
    DrawRectangle(boxX, boxY, boxWidth, boxHeight, Color{20, 20, 20, 255});
    DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, Color{100, 100, 100, 255});

    const char* title = "ELEVATOR COMPONENTS";
    int titleWidth = MeasureText(title, 30);
    DrawText(title, boxX + (boxWidth - titleWidth) / 2, boxY + 20, 30, Color{200, 200, 200, 255});


    int itemY = boxY + 80;
    int itemSpacing = 80;
    
    for(int i = 0; i < items.size(); i++){
        Item& item = items[i];
        
        int itemX = boxX + 40;
        
        // status
        if(item.collected){
            DrawText("[X]", itemX, itemY, 24, GREEN);
            
            // small sprite icons
            if(spriteTextures && item.spriteTextureIndex >= 0){
                Texture2D tex = spriteTextures[item.spriteTextureIndex];
                Rectangle src = {0, 0, (float)tex.width, (float)tex.height};
                Rectangle dst = {(float)(itemX + 50), (float)itemY, 48, 48};

                DrawTexturePro(tex, src, dst, Vector2{0, 0}, 0.0f, WHITE);
            }
            
            // name of item
            DrawText(item.name.c_str(), itemX + 110, itemY, 22, WHITE);
            
            // descr
            DrawText(item.description.c_str(), itemX + 110, itemY + 28, 16, Color{150, 150, 150, 255});
        } 
        
        else{
            DrawText("[ ]", itemX, itemY, 24, Color{100, 100, 100, 255});
            DrawText(item.name.c_str(), itemX + 50, itemY, 22, Color{100, 100, 100, 255});
        }
        
        itemY += itemSpacing;
    }
    
    const char* instructions = "Press TAB to close";
    int instWidth = MeasureText(instructions, 18);
    
    DrawText(instructions, boxX + (boxWidth - instWidth) / 2, boxY + boxHeight - 40, 18, Color{150, 150, 150, 255});
}