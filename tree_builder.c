#include "tree_builder.h"
#include <stdlib.h>
#include <string.h>

static int allocateNode(KdTree *tree) {
    if (tree->count >= tree->capacity) {
        return -1;
    }
    return tree->count++;
}

static void swapPoints(BiomeParameterPoint *a, BiomeParameterPoint *b) {
    BiomeParameterPoint tmp = *a;
    *a = *b;
    *b = tmp;
}

static void sortPoints(BiomeParameterPoint *points, int start, int end, int d) {
    if (start >= end) return;
    int32_t pivot = (points[end].min_param[d] + points[end].max_param[d]) / 2;
    int i = start - 1;
    for (int j = start; j < end; j++) {
        int32_t val = (points[j].min_param[d] + points[j].max_param[d]) / 2;
        if (val < pivot) {
            i++;
            swapPoints(&points[i], &points[j]);
        }
    }
    swapPoints(&points[i + 1], &points[end]);
    int p = i + 1;
    sortPoints(points, start, p - 1, d);
    sortPoints(points, p + 1, end, d);
}

static int buildRecursive(KdTree *tree, BiomeParameterPoint *points, int start, int end) {
    if (start > end) return -1;
    
    int node_idx = allocateNode(tree);
    if (node_idx == -1) return -1;
    
    // Copy/Compute values first since points sorting won't affect bounding box of node
    int32_t min_box[6];
    int32_t max_box[6];
    
    for (int d = 0; d < 6; d++) {
        min_box[d] = points[start].min_param[d];
        max_box[d] = points[start].max_param[d];
    }
    for (int i = start + 1; i <= end; i++) {
        for (int d = 0; d < 6; d++) {
            if (points[i].min_param[d] < min_box[d]) min_box[d] = points[i].min_param[d];
            if (points[i].max_param[d] > max_box[d]) max_box[d] = points[i].max_param[d];
        }
    }
    
    memcpy(tree->nodes[node_idx].min_box, min_box, sizeof(min_box));
    memcpy(tree->nodes[node_idx].max_box, max_box, sizeof(max_box));
    
    if (start == end) {
        tree->nodes[node_idx].split_dim = -1;
        tree->nodes[node_idx].split_val = 0;
        tree->nodes[node_idx].biomeId = points[start].biomeId;
        tree->nodes[node_idx].left_idx = -1;
        tree->nodes[node_idx].right_idx = -1;
        return node_idx;
    }
    
    int split_dim = 0;
    int32_t max_spread = max_box[0] - min_box[0];
    for (int d = 1; d < 6; d++) {
        int32_t spread = max_box[d] - min_box[d];
        if (spread > max_spread) {
            max_spread = spread;
            split_dim = d;
        }
    }
    
    sortPoints(points, start, end, split_dim);
    int mid = (start + end) / 2;
    int32_t split_val = (points[mid].min_param[split_dim] + points[mid].max_param[split_dim]) / 2;
    
    tree->nodes[node_idx].split_dim = split_dim;
    tree->nodes[node_idx].split_val = split_val;
    tree->nodes[node_idx].biomeId = -1;
    
    int left = buildRecursive(tree, points, start, mid);
    int right = buildRecursive(tree, points, mid + 1, end);
    
    tree->nodes[node_idx].left_idx = left;
    tree->nodes[node_idx].right_idx = right;
    
    return node_idx;
}

KdTree* buildKdTree(const BiomeParameterPoint *points, int count) {
    if (count <= 0) return NULL;
    
    KdTree *tree = (KdTree *)malloc(sizeof(KdTree));
    if (!tree) return NULL;
    
    tree->capacity = count * 2;
    tree->nodes = (KdNode *)malloc(tree->capacity * sizeof(KdNode));
    if (!tree->nodes) {
        free(tree);
        return NULL;
    }
    tree->count = 0;
    
    // We need a temporary copy of points because buildRecursive sorts/partitions them in-place
    BiomeParameterPoint *points_copy = (BiomeParameterPoint *)malloc(count * sizeof(BiomeParameterPoint));
    if (!points_copy) {
        free(tree->nodes);
        free(tree);
        return NULL;
    }
    memcpy(points_copy, points, count * sizeof(BiomeParameterPoint));
    
    tree->root_idx = buildRecursive(tree, points_copy, 0, count - 1);
    free(points_copy);
    
    if (tree->root_idx == -1) {
        free(tree->nodes);
        free(tree);
        return NULL;
    }
    
    return tree;
}

void freeKdTree(KdTree *tree) {
    if (tree) {
        if (tree->nodes) free(tree->nodes);
        free(tree);
    }
}

static void searchRecursive(const KdTree *tree, int node_idx, const uint64_t np[6], uint64_t *best_ds, int *best_biome) {
    if (node_idx == -1) return;
    const KdNode *node = &tree->nodes[node_idx];
    
    uint64_t box_ds = 0;
    for (int i = 0; i < 6; i++) {
        int64_t a = (int64_t)np[i] - node->max_box[i];
        int64_t b = (int64_t)node->min_box[i] - np[i];
        int64_t d = a > 0 ? a : b > 0 ? b : 0;
        box_ds += d * d;
    }
    if (box_ds >= *best_ds) {
        return;
    }
    
    if (node->left_idx == -1 && node->right_idx == -1) {
        if (box_ds < *best_ds) {
            *best_ds = box_ds;
            *best_biome = node->biomeId;
        }
        return;
    }
    
    int64_t val = (int64_t)np[node->split_dim];
    if (val < node->split_val) {
        searchRecursive(tree, node->left_idx, np, best_ds, best_biome);
        searchRecursive(tree, node->right_idx, np, best_ds, best_biome);
    } else {
        searchRecursive(tree, node->right_idx, np, best_ds, best_biome);
        searchRecursive(tree, node->left_idx, np, best_ds, best_biome);
    }
}

int searchKdTree(const KdTree *tree, const uint64_t np[6]) {
    if (!tree || tree->count == 0) return -1;
    uint64_t best_ds = -1ULL;
    int best_biome = -1;
    searchRecursive(tree, tree->root_idx, np, &best_ds, &best_biome);
    return best_biome;
}
