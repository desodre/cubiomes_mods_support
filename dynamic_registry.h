#ifndef DYNAMIC_REGISTRY_H_
#define DYNAMIC_REGISTRY_H_

#include "biomes.h"

#define MAX_CUSTOM_BIOMES 256
#define CUSTOM_BIOME_START_ID 200

typedef struct {
    int id;
    char name[64];
    int category;
    int dimension;
    int mutated;
    int isOceanic;
    int isSnowy;
    unsigned char color[3];
} CustomBiomeInfo;

#ifdef __cplusplus
extern "C" {
#endif

int registerCustomBiome(const char *name, int dimension, int category);
int getCustomBiomeIdByName(const char *name);
const CustomBiomeInfo* getCustomBiomeInfo(int id);
int isCustomBiomeId(int id);
int getCustomBiomeCount(void);
void clearCustomBiomes(void);

#ifdef __cplusplus
}
#endif

#endif /* DYNAMIC_REGISTRY_H_ */
