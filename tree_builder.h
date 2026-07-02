#ifndef TREE_BUILDER_H_
#define TREE_BUILDER_H_

#include "json_parser.h"

typedef struct {
    int split_dim;
    int32_t split_val;
    int biomeId;
    int left_idx;
    int right_idx;
    int32_t min_box[6];
    int32_t max_box[6];
} KdNode;

typedef struct KdTree {
    KdNode *nodes;
    int root_idx;
    int count;
    int capacity;
} KdTree;

#ifdef __cplusplus
extern "C" {
#endif

KdTree* buildKdTree(const BiomeParameterPoint *points, int count);
void freeKdTree(KdTree *tree);
int searchKdTree(const KdTree *tree, const uint64_t np[6]);

#ifdef __cplusplus
}
#endif

#endif /* TREE_BUILDER_H_ */
