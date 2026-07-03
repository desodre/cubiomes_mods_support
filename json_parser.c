#include "json_parser.h"
#include "dynamic_registry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>


typedef enum {
    JSON_EOF,
    JSON_LBRACE, JSON_RBRACE,
    JSON_LBRACKET, JSON_RBRACKET,
    JSON_COLON, JSON_COMMA,
    JSON_STRING, JSON_NUMBER,
    JSON_ERROR
} JSONTokenType;

typedef struct {
    const char *ptr;
    JSONTokenType type;
    char str_val[128];
    double num_val;
} JSONTokenizer;

static void nextToken(JSONTokenizer *tok) {
    while (*tok->ptr && (*tok->ptr == ' ' || *tok->ptr == '\t' || *tok->ptr == '\r' || *tok->ptr == '\n')) {
        tok->ptr++;
    }
    if (!*tok->ptr) {
        tok->type = JSON_EOF;
        return;
    }
    char c = *tok->ptr++;
    switch (c) {
        case '{': tok->type = JSON_LBRACE; return;
        case '}': tok->type = JSON_RBRACE; return;
        case '[': tok->type = JSON_LBRACKET; return;
        case ']': tok->type = JSON_RBRACKET; return;
        case ':': tok->type = JSON_COLON; return;
        case ',': tok->type = JSON_COMMA; return;
        case '"': {
            int len = 0;
            while (*tok->ptr && *tok->ptr != '"') {
                if (len < (int)sizeof(tok->str_val) - 1) {
                    tok->str_val[len++] = *tok->ptr;
                }
                tok->ptr++;
            }
            if (*tok->ptr == '"') tok->ptr++;
            tok->str_val[len] = '\0';
            tok->type = JSON_STRING;
            return;
        }
        default: {
            if ((c >= '0' && c <= '9') || c == '-' || c == '.') {
                tok->ptr--; // go back
                char *end;
                tok->num_val = strtod(tok->ptr, &end);
                if (end == tok->ptr) {
                    tok->ptr++; // force advance to prevent infinite loop
                    tok->type = JSON_ERROR;
                    return;
                }
                tok->ptr = end;
                tok->type = JSON_NUMBER;
                return;
            }
            tok->type = JSON_ERROR;
            return;
        }
    }
}

static int getVanillaBiomeIdByName(const char *name) {
    if (strncmp(name, "minecraft:", 10) == 0) {
        name += 10;
    }
    if (strcmp(name, "ocean") == 0) return ocean;
    if (strcmp(name, "plains") == 0) return plains;
    if (strcmp(name, "desert") == 0) return desert;
    if (strcmp(name, "mountains") == 0 || strcmp(name, "extreme_hills") == 0 || strcmp(name, "windswept_hills") == 0) return mountains;
    if (strcmp(name, "forest") == 0) return forest;
    if (strcmp(name, "taiga") == 0) return taiga;
    if (strcmp(name, "swamp") == 0 || strcmp(name, "swampland") == 0) return swamp;
    if (strcmp(name, "river") == 0) return river;
    if (strcmp(name, "nether_wastes") == 0 || strcmp(name, "hell") == 0) return nether_wastes;
    if (strcmp(name, "the_end") == 0 || strcmp(name, "sky") == 0) return the_end;
    if (strcmp(name, "frozen_ocean") == 0) return frozen_ocean;
    if (strcmp(name, "frozen_river") == 0) return frozen_river;
    if (strcmp(name, "snowy_tundra") == 0 || strcmp(name, "ice_plains") == 0 || strcmp(name, "snowy_plains") == 0) return snowy_tundra;
    if (strcmp(name, "snowy_mountains") == 0 || strcmp(name, "ice_mountains") == 0) return snowy_mountains;
    if (strcmp(name, "mushroom_fields") == 0 || strcmp(name, "mushroom_island") == 0) return mushroom_fields;
    if (strcmp(name, "mushroom_field_shore") == 0 || strcmp(name, "mushroom_island_shore") == 0) return mushroom_field_shore;
    if (strcmp(name, "beach") == 0) return beach;
    if (strcmp(name, "desert_hills") == 0) return desert_hills;
    if (strcmp(name, "wooded_hills") == 0 || strcmp(name, "forest_hills") == 0) return wooded_hills;
    if (strcmp(name, "taiga_hills") == 0) return taiga_hills;
    if (strcmp(name, "mountain_edge") == 0 || strcmp(name, "extreme_hills_edge") == 0) return mountain_edge;
    if (strcmp(name, "jungle") == 0) return jungle;
    if (strcmp(name, "jungle_hills") == 0) return jungle_hills;
    if (strcmp(name, "jungle_edge") == 0 || strcmp(name, "sparse_jungle") == 0) return jungle_edge;
    if (strcmp(name, "deep_ocean") == 0) return deep_ocean;
    if (strcmp(name, "stone_shore") == 0 || strcmp(name, "stone_beach") == 0 || strcmp(name, "stony_shore") == 0) return stone_shore;
    if (strcmp(name, "snowy_beach") == 0 || strcmp(name, "cold_beach") == 0) return snowy_beach;
    if (strcmp(name, "birch_forest") == 0) return birch_forest;
    if (strcmp(name, "birch_forest_hills") == 0) return birch_forest_hills;
    if (strcmp(name, "dark_forest") == 0 || strcmp(name, "roofed_forest") == 0) return dark_forest;
    if (strcmp(name, "snowy_taiga") == 0 || strcmp(name, "cold_taiga") == 0) return snowy_taiga;
    if (strcmp(name, "snowy_taiga_hills") == 0 || strcmp(name, "cold_taiga_hills") == 0) return snowy_taiga_hills;
    if (strcmp(name, "giant_tree_taiga") == 0 || strcmp(name, "mega_taiga") == 0 || strcmp(name, "old_growth_pine_taiga") == 0) return giant_tree_taiga;
    if (strcmp(name, "giant_tree_taiga_hills") == 0 || strcmp(name, "mega_taiga_hills") == 0) return giant_tree_taiga_hills;
    if (strcmp(name, "wooded_mountains") == 0 || strcmp(name, "extreme_hills_plus") == 0 || strcmp(name, "windswept_forest") == 0) return wooded_mountains;
    if (strcmp(name, "savanna") == 0) return savanna;
    if (strcmp(name, "savanna_plateau") == 0) return savanna_plateau;
    if (strcmp(name, "badlands") == 0 || strcmp(name, "mesa") == 0) return badlands;
    if (strcmp(name, "wooded_badlands_plateau") == 0 || strcmp(name, "mesa_plateau_f") == 0 || strcmp(name, "wooded_badlands") == 0) return wooded_badlands_plateau;
    if (strcmp(name, "badlands_plateau") == 0 || strcmp(name, "mesa_plateau") == 0) return badlands_plateau;
    if (strcmp(name, "warm_ocean") == 0) return warm_ocean;
    if (strcmp(name, "lukewarm_ocean") == 0) return lukewarm_ocean;
    if (strcmp(name, "cold_ocean") == 0) return cold_ocean;
    if (strcmp(name, "deep_warm_ocean") == 0) return deep_warm_ocean;
    if (strcmp(name, "deep_lukewarm_ocean") == 0) return deep_lukewarm_ocean;
    if (strcmp(name, "deep_cold_ocean") == 0) return deep_cold_ocean;
    if (strcmp(name, "deep_frozen_ocean") == 0) return deep_frozen_ocean;
    if (strcmp(name, "sunflower_plains") == 0) return sunflower_plains;
    if (strcmp(name, "desert_lakes") == 0) return desert_lakes;
    if (strcmp(name, "gravelly_mountains") == 0 || strcmp(name, "windswept_gravelly_hills") == 0) return gravelly_mountains;
    if (strcmp(name, "flower_forest") == 0) return flower_forest;
    if (strcmp(name, "taiga_mountains") == 0) return taiga_mountains;
    if (strcmp(name, "swamp_hills") == 0) return swamp_hills;
    if (strcmp(name, "ice_spikes") == 0) return ice_spikes;
    if (strcmp(name, "modified_jungle") == 0) return modified_jungle;
    if (strcmp(name, "modified_jungle_edge") == 0) return modified_jungle_edge;
    if (strcmp(name, "tall_birch_forest") == 0 || strcmp(name, "old_growth_birch_forest") == 0) return tall_birch_forest;
    if (strcmp(name, "tall_birch_hills") == 0) return tall_birch_hills;
    if (strcmp(name, "dark_forest_hills") == 0) return dark_forest_hills;
    if (strcmp(name, "snowy_taiga_mountains") == 0) return snowy_taiga_mountains;
    if (strcmp(name, "giant_spruce_taiga") == 0 || strcmp(name, "old_growth_spruce_taiga") == 0) return giant_spruce_taiga;
    if (strcmp(name, "giant_spruce_taiga_hills") == 0) return giant_spruce_taiga_hills;
    if (strcmp(name, "modified_gravelly_mountains") == 0) return modified_gravelly_mountains;
    if (strcmp(name, "shattered_savanna") == 0 || strcmp(name, "windswept_savanna") == 0) return shattered_savanna;
    if (strcmp(name, "shattered_savanna_plateau") == 0) return shattered_savanna_plateau;
    if (strcmp(name, "eroded_badlands") == 0) return eroded_badlands;
    if (strcmp(name, "modified_wooded_badlands_plateau") == 0) return modified_wooded_badlands_plateau;
    if (strcmp(name, "modified_badlands_plateau") == 0) return modified_badlands_plateau;
    if (strcmp(name, "bamboo_jungle") == 0) return bamboo_jungle;
    if (strcmp(name, "bamboo_jungle_hills") == 0) return bamboo_jungle_hills;
    if (strcmp(name, "soul_sand_valley") == 0) return soul_sand_valley;
    if (strcmp(name, "crimson_forest") == 0) return crimson_forest;
    if (strcmp(name, "warped_forest") == 0) return warped_forest;
    if (strcmp(name, "basalt_deltas") == 0) return basalt_deltas;
    if (strcmp(name, "dripstone_caves") == 0) return dripstone_caves;
    if (strcmp(name, "lush_caves") == 0) return lush_caves;
    if (strcmp(name, "meadow") == 0) return meadow;
    if (strcmp(name, "grove") == 0) return grove;
    if (strcmp(name, "snowy_slopes") == 0) return snowy_slopes;
    if (strcmp(name, "jagged_peaks") == 0) return jagged_peaks;
    if (strcmp(name, "frozen_peaks") == 0) return frozen_peaks;
    if (strcmp(name, "stony_peaks") == 0) return stony_peaks;
    if (strcmp(name, "deep_dark") == 0) return deep_dark;
    if (strcmp(name, "mangrove_swamp") == 0) return mangrove_swamp;
    if (strcmp(name, "cherry_grove") == 0) return cherry_grove;
    if (strcmp(name, "pale_garden") == 0) return pale_garden;
    return none;
}

static int parseParameterRange(JSONTokenizer *tok, int32_t *min_val, int32_t *max_val) {
    if (tok->type == JSON_NUMBER) {
        int32_t val = (int32_t)(tok->num_val * 10000.0);
        *min_val = val;
        *max_val = val;
        nextToken(tok);
        return 0;
    } else if (tok->type == JSON_LBRACKET) {
        nextToken(tok);
        if (tok->type != JSON_NUMBER) return -1;
        *min_val = (int32_t)(tok->num_val * 10000.0);
        nextToken(tok);
        if (tok->type != JSON_COMMA) return -1;
        nextToken(tok);
        if (tok->type != JSON_NUMBER) return -1;
        *max_val = (int32_t)(tok->num_val * 10000.0);
        nextToken(tok);
        if (tok->type != JSON_RBRACKET) return -1;
        nextToken(tok);
        return 0;
    }
    return -1;
}

int loadDatapackBiomeSource(const char *jsonPath, BiomeParameterPoint **pointsOut, int *countOut) {
    FILE *f = fopen(jsonPath, "rb");
    if (!f) {
        return -1;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = (char *)malloc(size + 1);
    if (!buf) {
        fclose(f);
        return -1;
    }
    long bytesRead = fread(buf, 1, size, f);
    buf[bytesRead] = '\0';
    fclose(f);

    char *oldLocale = setlocale(LC_NUMERIC, NULL);
    char savedLocale[64] = "C";
    if (oldLocale) {
        strncpy(savedLocale, oldLocale, sizeof(savedLocale) - 1);
        savedLocale[sizeof(savedLocale) - 1] = '\0';
    }
    setlocale(LC_NUMERIC, "C");

    JSONTokenizer tok;
    tok.ptr = buf;
    nextToken(&tok);

    int max_points = 512;
    BiomeParameterPoint *points = (BiomeParameterPoint *)malloc(max_points * sizeof(BiomeParameterPoint));
    int count = 0;

    // Search for `"biomes"` array
    int found_biomes = 0;
    while (tok.type != JSON_EOF && tok.type != JSON_ERROR) {
        if (tok.type == JSON_STRING && strcmp(tok.str_val, "biomes") == 0) {
            nextToken(&tok);
            if (tok.type == JSON_COLON) {
                nextToken(&tok);
                if (tok.type == JSON_LBRACKET) {
                    found_biomes = 1;
                    nextToken(&tok);
                    break;
                }
            }
        }
        nextToken(&tok);
    }

    if (!found_biomes) {
        free(points);
        free(buf);
        setlocale(LC_NUMERIC, savedLocale);
        return -1;
    }

    // Parse array elements
    while (tok.type != JSON_RBRACKET && tok.type != JSON_EOF && tok.type != JSON_ERROR) {
        if (tok.type == JSON_LBRACE) {
            nextToken(&tok);
            
            char biomeName[128] = "";
            int32_t min_p[6] = {0, 0, 0, 0, 0, 0};
            int32_t max_p[6] = {0, 0, 0, 0, 0, 0};

            while (tok.type != JSON_RBRACE && tok.type != JSON_EOF && tok.type != JSON_ERROR) {
                if (tok.type == JSON_STRING) {
                    char key[128];
                    strcpy(key, tok.str_val);
                    nextToken(&tok);
                    if (tok.type == JSON_COLON) {
                        nextToken(&tok);
                        if (strcmp(key, "biome") == 0 && tok.type == JSON_STRING) {
                            strcpy(biomeName, tok.str_val);
                            nextToken(&tok);
                        } else if (strcmp(key, "parameters") == 0 && tok.type == JSON_LBRACE) {
                            nextToken(&tok);
                            while (tok.type != JSON_RBRACE && tok.type != JSON_EOF && tok.type != JSON_ERROR) {
                                if (tok.type == JSON_STRING) {
                                    char pkey[128];
                                    strcpy(pkey, tok.str_val);
                                    nextToken(&tok);
                                    if (tok.type == JSON_COLON) {
                                        nextToken(&tok);
                                        int idx = -1;
                                        if (strcmp(pkey, "temperature") == 0) idx = 0;
                                        else if (strcmp(pkey, "humidity") == 0) idx = 1;
                                        else if (strcmp(pkey, "continentalness") == 0) idx = 2;
                                        else if (strcmp(pkey, "erosion") == 0) idx = 3;
                                        else if (strcmp(pkey, "depth") == 0 || strcmp(pkey, "depth") == 0) idx = 4;
                                        else if (strcmp(pkey, "weirdness") == 0) idx = 5;
                                        
                                        if (idx != -1) {
                                            parseParameterRange(&tok, &min_p[idx], &max_p[idx]);
                                        } else {
                                            // Skip value of other parameters (e.g. offset)
                                            if (tok.type == JSON_LBRACKET) {
                                                while (tok.type != JSON_RBRACKET && tok.type != JSON_EOF) nextToken(&tok);
                                                nextToken(&tok);
                                            } else {
                                                nextToken(&tok);
                                            }
                                        }
                                    }
                                } else if (tok.type == JSON_COMMA) {
                                    nextToken(&tok);
                                } else {
                                    nextToken(&tok);
                                }
                            }
                            if (tok.type == JSON_RBRACE) nextToken(&tok);
                        } else {
                            nextToken(&tok);
                        }
                    }
                } else if (tok.type == JSON_COMMA) {
                    nextToken(&tok);
                } else {
                    nextToken(&tok);
                }
            }
            if (tok.type == JSON_RBRACE) nextToken(&tok);

            if (biomeName[0] != '\0') {
                int biomeId = getVanillaBiomeIdByName(biomeName);
                if (biomeId == none) {
                    // Try dynamic registry (register dynamically if not registered yet)
                    biomeId = getCustomBiomeIdByName(biomeName);
                    if (biomeId == -1) {
                        biomeId = registerCustomBiome(biomeName, DIM_OVERWORLD, plains);
                    }
                }
                
                if (biomeId != none) {
                    if (count >= max_points) {
                        max_points *= 2;
                        points = (BiomeParameterPoint *)realloc(points, max_points * sizeof(BiomeParameterPoint));
                    }
                    points[count].biomeId = biomeId;
                    memcpy(points[count].min_param, min_p, sizeof(min_p));
                    memcpy(points[count].max_param, max_p, sizeof(max_p));
                    count++;
                }
            }
        } else if (tok.type == JSON_COMMA) {
            nextToken(&tok);
        } else {
            nextToken(&tok);
        }
    }

    free(buf);
    *pointsOut = points;
    *countOut = count;
    setlocale(LC_NUMERIC, savedLocale);
    return count;
}
