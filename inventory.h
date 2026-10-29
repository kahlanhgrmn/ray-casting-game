#pragma once
#include "raylib.h"
#include <string>
#include <vector>

struct Item {
    std::string name;
    std::string description;
    int spriteTextureIndex;  // inventory display
    bool collected;
};

class Inventory{
    public:
        std::vector<Item> items;
        int totalItems;
        int collectedCount;
        bool inventoryOpen;
        
        Inventory();
        void addItem(const std::string& name, const std::string& description, int spriteIdx);
        void collectItem(int itemIndx);
        bool isCollected(int itemIdx);
        int getCollectedCount();
        void toggleInventory();
        void renderHUD(int screenWidth, int screenHeight);
        void renderFullInventory(int screenWidth, int screenHeight, Texture2D* spriteTextures);
};