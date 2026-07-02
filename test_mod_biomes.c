#include "generator.h"
#include "dynamic_registry.h"
#include "json_parser.h"
#include "tree_builder.h"
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <string.h>

int main()
{
    printf("=== Starting Mod Support Tests ===\n");

    // 1. Load Datapack
    printf("Loading test_overworld.json...\n");
    BiomeParameterPoint *points = NULL;
    int count = 0;
    int parsed = loadDatapackBiomeSource("test_overworld.json", &points, &count);
    assert(parsed > 0);
    assert(count == 4);
    printf("Parsed %d parameter points successfully.\n", count);

    // Verify registry assignments
    int plainsId = getCustomBiomeIdByName("minecraft:plains");
    // plains is vanilla, so it shouldn't be in the custom registry
    assert(plainsId == -1); 
    
    int lavenderId = getCustomBiomeIdByName("biomesoplenty:lavender_fields");
    assert(lavenderId == CUSTOM_BIOME_START_ID);
    
    int tundraId = getCustomBiomeIdByName("biomesoplenty:tundra");
    assert(tundraId == CUSTOM_BIOME_START_ID + 1);

    const CustomBiomeInfo *lavenderInfo = getCustomBiomeInfo(lavenderId);
    assert(lavenderInfo != NULL);
    assert(strcmp(lavenderInfo->name, "biomesoplenty:lavender_fields") == 0);
    assert(lavenderInfo->isOceanic == 0);

    printf("Dynamic Biome Registry looks correct.\n");

    // 2. Build KdTree
    printf("Building Kd-Tree from parameter points...\n");
    KdTree *tree = buildKdTree(points, count);
    assert(tree != NULL);
    assert(tree->count > 0);
    printf("Kd-Tree built successfully with %d nodes.\n", tree->count);

    // 3. Test Search directly on KdTree
    printf("Testing Kd-Tree nearest-neighbor queries directly...\n");
    
    // Test plains query: np = {0, 0, -5000, 0, 0, 0}
    uint64_t np_plains[6] = {0, 0, -5000, 0, 0, 0};
    int id = searchKdTree(tree, np_plains);
    assert(id == plains);
    
    // Test desert query: np = {10000, -5000, 5000, 0, 0, 0}
    uint64_t np_desert[6] = {10000, -5000, 5000, 0, 0, 0};
    id = searchKdTree(tree, np_desert);
    assert(id == desert);

    // Test custom lavender fields query: np = {5000, 7000, 0, 0, 0, 0}
    uint64_t np_lavender[6] = {5000, 7000, 0, 0, 0, 0};
    id = searchKdTree(tree, np_lavender);
    assert(id == lavenderId);

    // Test custom tundra query: np = {-7500, -7500, 2500, 0, 0, 0}
    uint64_t np_tundra[6] = {-7500, -7500, 2500, 0, 0, 0};
    id = searchKdTree(tree, np_tundra);
    assert(id == tundraId);

    printf("Kd-Tree search queries matched exactly.\n");

    // 4. Test Integration with Generator
    printf("Testing Generator integration...\n");
    Generator g;
    setupGenerator(&g, MC_1_18, 0);
    applySeed(&g, DIM_OVERWORLD, 12345ULL);

    // Inject custom tree
    g.bn.customTree = tree;

    // Verify getBiomeAt routes to our KdTree
    int scale = 4;
    // We expect coordinates that map to the lavender climate to evaluate to lavenderId
    // Let's sample coordinate x=0, y=64, z=0. 
    // We can sample at biome coordinate scale and print the result.
    int test_biome = getBiomeAt(&g, scale, 0, 64, 0);
    printf("Sampled biome ID at (0,64,0): %d\n", test_biome);
    assert(test_biome == plains || test_biome == desert || test_biome == lavenderId || test_biome == tundraId);

    // 5. Benchmark Performance
    printf("Benchmarking lookups (1 million iterations)...\n");
    clock_t start = clock();
    uint64_t np_bench[6] = {4500, 6500, 200, -100, 50, 0};
    volatile int dummy = 0;
    for (int i = 0; i < 1000000; i++) {
        np_bench[0] = (np_bench[0] + i) % 10000;
        dummy += searchKdTree(tree, np_bench);
    }
    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    printf("1,000,000 Kd-tree lookups took %.3f seconds (%.2f million lookups/sec).\n", 
           time_spent, 1.0 / time_spent);

    // Compare with vanilla btree18 lookup
    start = clock();
    for (int i = 0; i < 1000000; i++) {
        np_bench[0] = (np_bench[0] + i) % 10000;
        dummy += climateToBiome(MC_1_18, np_bench, NULL, NULL);
    }
    end = clock();
    time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    printf("1,000,000 vanilla btree18 lookups took %.3f seconds (%.2f million lookups/sec).\n", 
           time_spent, 1.0 / time_spent);

    // Clean up
    freeKdTree(tree);
    free(points);
    clearCustomBiomes();

    printf("=== All Tests Passed Successfully! ===\n");
    return 0;
}
