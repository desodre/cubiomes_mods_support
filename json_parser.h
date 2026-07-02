#ifndef JSON_PARSER_H_
#define JSON_PARSER_H_

#include <stdint.h>

typedef struct {
    int biomeId;
    int32_t min_param[6]; // temp, hum, cont, eros, depth, weird
    int32_t max_param[6];
} BiomeParameterPoint;

#ifdef __cplusplus
extern "C" {
#endif

// Loads a datapack dimension/worldgen JSON file, registers any custom biomes
// found in it, and populates the points array.
// Returns the number of points parsed, or -1 on error.
int loadDatapackBiomeSource(const char *jsonPath, BiomeParameterPoint **pointsOut, int *countOut);

#ifdef __cplusplus
}
#endif

#endif /* JSON_PARSER_H_ */
