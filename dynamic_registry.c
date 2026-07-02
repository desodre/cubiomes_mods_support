#include "dynamic_registry.h"
#include <string.h>
#include <stdlib.h>

static CustomBiomeInfo custom_biomes[MAX_CUSTOM_BIOMES];
static int custom_biome_count = 0;

int registerCustomBiome(const char *name, int dimension, int category) {
    if (custom_biome_count >= MAX_CUSTOM_BIOMES) {
        return -1;
    }
    
    // Check if already registered
    int existing_id = getCustomBiomeIdByName(name);
    if (existing_id != -1) {
        return existing_id;
    }
    
    int id = CUSTOM_BIOME_START_ID + custom_biome_count;
    CustomBiomeInfo *info = &custom_biomes[custom_biome_count];
    info->id = id;
    strncpy(info->name, name, sizeof(info->name) - 1);
    info->name[sizeof(info->name) - 1] = '\0';
    info->dimension = dimension;
    info->category = category;
    info->mutated = none;
    info->isOceanic = (category == ocean);
    info->isSnowy = 0;
    if (strstr(name, "snowy") || strstr(name, "cold") || strstr(name, "frozen") || strstr(name, "ice")) {
        info->isSnowy = 1;
    }
    
    // Generate a simple deterministic RGB color based on hash of name
    unsigned int hash = 5381;
    const char *ptr = name;
    while (*ptr) {
        hash = ((hash << 5) + hash) + *ptr++;
    }
    info->color[0] = (hash & 0xFF0000) >> 16;
    info->color[1] = (hash & 0x00FF00) >> 8;
    info->color[2] = (hash & 0x0000FF);
    
    custom_biome_count++;
    return id;
}

int getCustomBiomeIdByName(const char *name) {
    for (int i = 0; i < custom_biome_count; i++) {
        if (strcmp(custom_biomes[i].name, name) == 0) {
            return custom_biomes[i].id;
        }
    }
    return -1;
}

const CustomBiomeInfo* getCustomBiomeInfo(int id) {
    int idx = id - CUSTOM_BIOME_START_ID;
    if (idx >= 0 && idx < custom_biome_count) {
        return &custom_biomes[idx];
    }
    return NULL;
}

int isCustomBiomeId(int id) {
    return (id >= CUSTOM_BIOME_START_ID && id < CUSTOM_BIOME_START_ID + custom_biome_count);
}

int getCustomBiomeCount(void) {
    return custom_biome_count;
}

void clearCustomBiomes(void) {
    custom_biome_count = 0;
}
