/**
 * 
 * Copyright (C) 2023 NeuronBasic Co., Ltd. All rights reserved.
 * 
 * @file motion_multiple.c
 * @author karl (karl.zhang@neuronBasic.com)
 * @version 1.0
 * @date 2023-07-04
 * @brief 多motion框源文件
 * 
 */
#include "motion_multiple.h"

#if (DEBUG_BOUNDING_RECT_INFO == 1)
static void s_printMap(const uint32_t *motion_flag, uint8_t len);
static void s_printMapALL(struct BoundingRectManger *manger, const uint32_t *src, uint8_t len);
#endif
static uint32_t s_swar(uint32_t x);
static int32_t s_markEdge(const uint32_t *src, uint32_t *dst, uint8_t len, struct EdgeInfo *info);
static void s_getEdgePoint(struct BoundingRectManger *manger, const uint32_t *edgeBitsMap, uint8_t len);
static int32_t s_groupEdgePoints(struct BoundingRectManger *manger, uint8_t len);
static int32_t s_releaseBitsMapBoundingRect(struct BoundingRectManger *manger);
static void s_EdgeInfo_init(struct EdgeInfo *edgeInfo);

/**
 * @brief 获取多个motion框的demo
 * 
 * @author karl (karl.zhang@neuronBasic.com)
 * @date 2023-07-04
 * @version 1.0
 * 
 * @param manger 在调用getBitsMapBoundingRect后的多motion框manger,里面记录了框信息
 * 
 */
void getMotionBoundingRectDemo(struct BoundingRectManger *manger)
{
    struct EegeGroup *edgeGroupCurrent;
    
    for (edgeGroupCurrent = manger->edgeGroup; edgeGroupCurrent != NULL; edgeGroupCurrent = edgeGroupCurrent->last) {
        if (edgeGroupCurrent->belong == NULL) {
            /* 在这里得到 */
            //左边界:(uint32_t)(31 - edgeGroupCurrent->left) * 10
            //右边界:(uint32_t)(31 - edgeGroupCurrent->right) * 10 + 10
            //上边界:(uint32_t)(edgeGroupCurrent->up)
            //下边界:(uint32_t)(edgeGroupCurrent->down) + 10
        }
    }
}
/**
 * @brief 获取motion点图
 * 
 * @author karl (karl.zhang@neuronBasic.com)
 * @date 2023-07-05
 * @version 1.0
 * 
 * 
 * @return uint32_t* 返回获取结果的指针,为NULL说明malloc失败
 */
uint32_t *getMotionBitsMap(void)
{
    // static uint32_t *bitsMap = NULL;
    // if (bitsMap == NULL)
    //     bitsMap = (uint32_t *)malloc(sizeof(uint32_t) * 32);
    // if (bitsMap != NULL)
    //     for (uint8_t i = 0; i < 32; i++) {
    //         bitsMap[i] = __raw_readl(0x40090800 + 0x200 + i * 4);
    //     }
    // return bitsMap;
}
/**
 * @brief 只提供一个motion框
 * 
 * @author karl (karl.zhang@neuronBasic.com)
 * @date 2023-07-04
 * @version 1.0
 * 
 * @param result 结果
 * @param bitsMap motion点图
 * @param len 图长度
 * 
 * @return int32_t 0 - 没有框, 1 - 有框
 */
int32_t getSingleBoundingRect(struct MotionBoundingRect *result, const uint32_t *bitsMap, uint8_t len)
{
    uint32_t i;
    uint32_t leftAndRight = 0x00, temp;
    uint8_t up = 0, down = len - 1;
    for (i = 0; i < len; ++i) {
        if (bitsMap[i] != 0x00) {
            leftAndRight |= bitsMap[i];
            down = i;
        } else if (leftAndRight == 0x00) {
            up = i + 1;
        }
    }
    if (leftAndRight != 0x00) {
        result->down = down;
        result->up = up;
        temp = leftAndRight;
        for (i = 0; (temp & (0x01)) == 0x00; ++i, temp >>= 1)
            ;
        result->right = i;
        temp = leftAndRight;
        for (i = 0; (temp & (0x80000000)) == 0x00; ++i, temp <<= 1)
            ;
        result->left = 31 - i;
        return 1;
    } else {
        return 0;
    }
}
/**
 * @brief 初始化BoundingRectManger结构体
 * 
 * @author karl (karl.zhang@neuronBasic.com)
 * @date 2023-07-04
 * @version 1.0
 * 
 * @param manger 
 * 
 */
void BoundingRectManger_init(struct BoundingRectManger *manger)
{
    if (!BOUNDING_RECT_MANAGER_NOT_INITIALIZED(manger))
        releaseBitsMapBoundingRectALL(manger);
    s_EdgeInfo_init(&manger->edgeInfo);
    manger->edgePoints = NULL;
    manger->edgeGroup = NULL;
    manger->initialized = manger;
}
/**
 * @brief 释放所有申请的内存
 * 
 * @author karl (karl.zhang@neuronBasic.com)
 * @date 2023-07-04
 * @version 1.0
 * 
 * @param manger 
 * 
 * @return int32_t 
 */
int32_t releaseBitsMapBoundingRectALL(struct BoundingRectManger *manger)
{
    if (BOUNDING_RECT_MANAGER_NOT_INITIALIZED(manger))
        return -1;
    s_releaseBitsMapBoundingRect(manger);
    if (manger->edgeInfo.edgeBitsMap != NULL) {
        free(manger->edgeInfo.edgeBitsMap);
        manger->edgeInfo.edgeBitsMap = NULL;
    }
    if (manger->edgeInfo.edgesStartIndex != NULL) {
        free(manger->edgeInfo.edgesStartIndex);
        manger->edgeInfo.edgesStartIndex = NULL;
    }
    return 0;
}
/**
 * @brief 获取motion图中所有的motion框
 * 
 * @author karl (karl.zhang@neuronBasic.com)
 * @date 2023-07-04
 * @version 1.0
 * 
 * @param manger 多motion框相关的manger
 * @param bitsMap motion图
 * @param len 图高度
 * 
 * @return int32_t 
 */
int32_t getBitsMapBoundingRect(struct BoundingRectManger *manger, const uint32_t *bitsMap, uint8_t len)
{
    // clang-format off
#if (DEBUG_PERF_TIME == 1)
    uint64_t startTick, endTick;
#endif
    int32_t ret;
DEBUG_PERF_TIME_WRAPPER("init check", startTick, endTick, {
    /* 检查manger是否初始化 */
    if (BOUNDING_RECT_MANAGER_NOT_INITIALIZED(manger))
        return -1;
});
DEBUG_PERF_TIME_WRAPPER("release", startTick, endTick, {
    /* 释放上一轮计算边缘点,边缘组申请的内存 */
    s_releaseBitsMapBoundingRect(manger);
});
    /* 为边缘点的图申请内存,一次申请后就不再释放,除非手动释放 */
    if (manger->edgeInfo.edgeBitsMap == NULL)
        manger->edgeInfo.edgeBitsMap = (uint32_t *)malloc(sizeof(uint32_t) * len);
    if (manger->edgeInfo.edgeBitsMap == NULL) {
        return -2;
    }

DEBUG_PERF_TIME_WRAPPER("markEdge", startTick, endTick, {
    /* 生成边缘点图,并得到有多少个边缘点,如果边缘点数量是0,直接返回 */
    ret = s_markEdge(bitsMap, manger->edgeInfo.edgeBitsMap, len, &manger->edgeInfo);
    if (ret != 0)
        return -3;
    if (manger->edgeInfo.edgeCount == 0)
        return 0;
});

DEBUG_PERF_TIME_WRAPPER("init edgePoints", startTick, endTick, {
    /* 根据边缘点数量,申请空间来记录边缘点信息 */
    manger->edgePoints = (struct EdgePoint *)malloc(sizeof(struct EdgePoint) * manger->edgeInfo.edgeCount);
    if (manger->edgePoints == NULL) {
        return -4;
    }
    /* 所有的边缘点信息置为0x00,主要是为了初始化边缘点信息中的所属组指针为NULL */
    memset(manger->edgePoints, 0, sizeof(struct EdgePoint) * manger->edgeInfo.edgeCount);
});
DEBUG_PERF_TIME_WRAPPER("getEdgePoint", startTick, endTick, {
    /* 从边缘点图中获取边缘点信息 */
    s_getEdgePoint(manger, manger->edgeInfo.edgeBitsMap, len);
});
DEBUG_PERF_TIME_WRAPPER("groupEdgePoints", startTick, endTick, {
    /* 将所有的边缘点分组,有连接关系的为同一连通域,即为同一组,也就是有多少个motion框 */
    if (s_groupEdgePoints(manger, len) != 0)
        return -5;
});
/* 打印debug信息 */
#if (DEBUG_BOUNDING_RECT_INFO == 1)
    struct EegeGroup *edgeGroupCurrent = manger->edgeGroup;
    printf("input\r\n");
    s_printMap(bitsMap, 32);
    printf("edge map\r\n");
    s_printMap(manger->edgeInfo.edgeBitsMap, 32);
    for (int i = 0; edgeGroupCurrent != NULL; edgeGroupCurrent = edgeGroupCurrent->last, ++i) {
        printf("i:%3d L:%3d R:%3d U:%3d D:%3d, belong:%d\r\n", i, edgeGroupCurrent->left, edgeGroupCurrent->right,
               edgeGroupCurrent->up, edgeGroupCurrent->down, edgeGroupCurrent->belong == NULL ? 0 : 1);
    }
    printf("all edge info\r\n");
    for (int i = 0; i < manger->edgeInfo.edgeCount; ++i) {
        printf("i:%d L:%d R:%d\r\n", i, manger->edgePoints[i].left, manger->edgePoints[i].right);
    }
    printf("edge per line\r\n");
    uint8_t last = 0;
    for (int i = 0; i < len; ++i) {
        printf("i:%d end:%d deta:%d\r\n", i, manger->edgeInfo.edgesStartIndex[i],
               manger->edgeInfo.edgesStartIndex[i] != 0 ? manger->edgeInfo.edgesStartIndex[i] - last : 0);
        if (manger->edgeInfo.edgesStartIndex[i] != 0)
            last = manger->edgeInfo.edgesStartIndex[i];
    }
    edgeGroupCurrent = manger->edgeGroup;
    printf("result\r\n");
    for (int j = 0; edgeGroupCurrent != NULL; edgeGroupCurrent = edgeGroupCurrent->last) {
        if (edgeGroupCurrent->belong == NULL){
            printf("j:%d L:%d R:%d U:%d D:%d\r\n", j, 31 - edgeGroupCurrent->left, 31 - edgeGroupCurrent->right,
                edgeGroupCurrent->up, edgeGroupCurrent->down);
            ++j;
        }
    }
    s_printMapALL(manger, bitsMap, len);
#endif

    return 0;
    // clang-format on
}
/**
 * @brief 对32bit的二值图进行腐蚀,结构为3X3十字
 * 
 * @author karl (karl.zhang@neuronBasic.com)
 * @date 2023-07-04
 * @version 1.0
 8
 * @param src 源图
 * @param dst 输出图
 * @param len 图高度
 *
 * @return int32_t 0 - 输入参数正常, -1 - 输入参数异常
 *
 * @note 
 * 输入指针和输出指针可以相等
 */
int32_t erodeBitsMap(const uint32_t *src, uint32_t *dst, uint8_t len)
{
    if ((len < 3) || (src == NULL) || (dst == NULL))
        return -1;
    uint32_t i;
    uint32_t lastRow = src[0], currentRow = src[1], nextRow;
    --len;
    dst[0] = 0xFFFFFFFF & src[0] & ((src[0] << 1) | (0x00000001)) & ((src[0] >> 1) | (0x80000000)) & (src[1]);
    for (i = 1; i < len; ++i) {
        dst[i] = 0x00;
        nextRow = src[i + 1];
        if (currentRow != 0x00)
            dst[i] = 0xFFFFFFFF & lastRow & currentRow & ((currentRow << 1) | (0x00000001)) & ((currentRow >> 1) | (0x80000000)) & (nextRow);
        lastRow = currentRow;
        currentRow = nextRow;
    }
    dst[len] = 0xFFFFFFFF & lastRow & currentRow & ((currentRow << 1) | (0x00000001)) & ((currentRow >> 1) | (0x80000000));
    return 0;
}
/**
 * @brief 对32bit的二值图进行膨胀,结构为3X3十字
 * 
 * @author karl (karl.zhang@neuronBasic.com)
 * @date 2023-07-04
 * @version 1.0
 *
 * @param src 源图
 * @param dst 输出图
 * @param len 图高度
 *
 * @return int32_t 0 - 输入参数正常, -1 - 输入参数异常
 *
 * @note 
 * 输入指针和输出指针可以相等
 */
int32_t dilateBitsMap(const uint32_t *src, uint32_t *dst, uint8_t len)
{
    if ((len < 3) || (src == NULL) || (dst == NULL))
        return -1;
    uint32_t i;
    uint32_t lastRow = src[0], currentRow = src[1], nextRow;
    --len;
    
    dst[0] = 0x00000000 | src[0] | (src[0] << 1) | (src[0] >> 1) | (src[1]);
    for (i = 1; i < len; ++i) {
        dst[i] = 0x00;
        nextRow = src[i + 1];
        dst[i] = 0x00000000 | lastRow | currentRow | (currentRow << 1) | (currentRow >> 1) | (nextRow);
        lastRow = currentRow;
        currentRow = nextRow;
    }
    dst[len] = 0x00000000 | lastRow | currentRow | (currentRow << 1) | (currentRow >> 1);
    
    return 0;
}
/**
 * @brief 只释放边缘点位置申请的空间和边缘点组申请的空间
 * 
 * @author karl (karl.zhang@neuronBasic.com)
 * @date 2023-07-04
 * @version 1.0
 * 
 * @param manger 多motion框相关的manger
 * 
 * @return int32_t 0 - 正常, -1 - manger未初始化
 */
static int32_t s_releaseBitsMapBoundingRect(struct BoundingRectManger *manger)
{
    if (BOUNDING_RECT_MANAGER_NOT_INITIALIZED(manger))
        return -1;
    if (manger->edgePoints != NULL) {
        struct EegeGroup *edgeGroupCurrent = manger->edgeGroup;
        while (manger->edgeGroup != NULL) {
            edgeGroupCurrent = manger->edgeGroup;
            manger->edgeGroup = manger->edgeGroup->last;
            free(edgeGroupCurrent);
        }
        free(manger->edgePoints);
        manger->edgePoints = NULL;
    }
    return 0;
}
/**
 * @brief 将所有的边缘点根据有无连接进行分组
 * 
 * @author karl (karl.zhang@neuronBasic.com)
 * @date 2023-07-04
 * @version 1.0
 * 
 * @param manger 多motion框相关的manger
 * @param len 图高度
 * 
 * @return int32_t 0 - 正常, -1 - 申请边缘点空间失败
 */
static int32_t s_groupEdgePoints(struct BoundingRectManger *manger, uint8_t len)
{
/* 生成新的边缘点组 */
#define NEW_EDGE_GROUP()                                                                 \
    {                                                                                    \
        edgePointsCurrent->group = (struct EegeGroup *)malloc(sizeof(struct EegeGroup)); \
        if (edgePointsCurrent->group == NULL)                                            \
            return -1;                                                                   \
        edgePointsCurrent->group->last = manger->edgeGroup;                              \
        manger->edgeGroup = edgePointsCurrent->group;                                    \
        edgePointsCurrent->group->belong = NULL;                                         \
        (edgePointsCurrent->group)->left = edgePointsCurrent->left;                      \
        (edgePointsCurrent->group)->right = edgePointsCurrent->right;                    \
        (edgePointsCurrent->group)->up = row;                                            \
        (edgePointsCurrent->group)->down = row;                                          \
    }
    uint32_t row;
    struct EdgePoint *edgePointsCurrentStart, *edgePointsCurrentEnd, *edgePointsCurrent;
    struct EdgePoint *edgePointsLastEnd = manger->edgePoints, *edgePointsLastStart, *edgePointsLast;
    struct EegeGroup *lastGroup;
    /* 遍历每行 */
    for (row = 0; row < len; ++row) {
        /* manger->edgeInfo.edgesStartIndex记录了每行的边缘点数量 */
        /* 如果数量是0就跳过 */
        if (manger->edgeInfo.edgesStartIndex[row] != 0) {
            /* 当前行不是第0行,并且上一行不是空行,执行以下 */
            if ((row != 0) && (manger->edgeInfo.edgesStartIndex[row - 1] != 0)) {
                edgePointsCurrentStart = edgePointsLastEnd;
                edgePointsCurrentEnd = manger->edgePoints + manger->edgeInfo.edgesStartIndex[row];
                edgePointsCurrent = edgePointsCurrentStart;
                /* 遍历本行与上一行的边缘点 */
                while ((edgePointsLast != edgePointsLastEnd) && (edgePointsCurrent != edgePointsCurrentEnd)) {
                    /* 本行当前边缘点左边界位于上一行比对的边缘点的右边界右边,说明二者无连接 */
                    /* 切换上一行的下一个比对边缘点*/
                    if (edgePointsCurrent->left < edgePointsLast->right) {
                        ++edgePointsLast;
                        /* 本行当前边缘点右边界位于上一行比对的边缘点的左边界左边,说明二者无连接 */
                        /* 如果本行当前边缘点没有组,就分配一个组,然后切换本行当前边缘点到下一个 */
                    } else if (edgePointsCurrent->right > edgePointsLast->left) {
                        if (edgePointsCurrent->group == NULL) {
                            NEW_EDGE_GROUP();
                        }
                        ++edgePointsCurrent;
                        /* 二者有连接执行以下 */
                    } else {
                        /* 获取上一行正在比对的边缘点所属的组 */
                        /* 组是一个链表,遇到组合并的时候会将其中一个组的belong指向另外一个组 */
                        lastGroup = edgePointsLast->group;
                        while (lastGroup->belong != NULL)
                            lastGroup = lastGroup->belong;
                        /* 本行当前边缘点没有组就加入上一行正在比对的边缘点的组 */
                        if (edgePointsCurrent->group == NULL) {
                            edgePointsCurrent->group = lastGroup;
                            /* 如果本行当前边缘点有组,就将上一行正在比对的边缘点的组加入到本行当前边缘点所在的组,并更新组的大小信息 */
                        } else if ((edgePointsCurrent->group) != (lastGroup)) {
                            (edgePointsCurrent->group)->up = ((edgePointsCurrent->group)->up) > (lastGroup->up) ?
                                                                     (lastGroup->up) :
                                                                     ((edgePointsCurrent->group)->up);
                            (edgePointsCurrent->group)->left = ((edgePointsCurrent->group)->left) < (lastGroup->left) ?
                                                                       (lastGroup->left) :
                                                                       ((edgePointsCurrent->group)->left);
                            (edgePointsCurrent->group)->right = ((edgePointsCurrent->group)->right) > (lastGroup->right) ?
                                                                        (lastGroup->right) :
                                                                        ((edgePointsCurrent->group)->right);
                            lastGroup->belong = edgePointsCurrent->group;
                        }
                        /* 并更新组的大小信息 */
                        (edgePointsCurrent->group)->down = row;
                        (edgePointsCurrent->group)->left = ((edgePointsCurrent->group)->left) < edgePointsCurrent->left ?
                                                                   edgePointsCurrent->left :
                                                                   ((edgePointsCurrent->group)->left);
                        (edgePointsCurrent->group)->right = ((edgePointsCurrent->group)->right) > edgePointsCurrent->right ?
                                                                    edgePointsCurrent->right :
                                                                    ((edgePointsCurrent->group)->right);
                        /* 如果本行当前边缘点右边界不小于上一行正在比对的边缘点的右边界,切换本行当前边缘点到下一个 */
                        /* 否则就切换上一行的下一个比对边缘点 */
                        if (edgePointsCurrent->right >= edgePointsLast->right)
                            ++edgePointsCurrent;
                        else
                            ++edgePointsLast;
                    }
                }
                /* 在上一行所有边缘点都比对完后,仍存在没有分配到组的本行边缘点,就为他们分配新的组 */
                while ((edgePointsCurrent) != edgePointsCurrentEnd) {
                    if (edgePointsCurrent->group == NULL) {
                        NEW_EDGE_GROUP();
                    }
                    ++edgePointsCurrent;
                }
                /* 当前行是第0行,或者上一行是空行,直接为该行所有边缘点分配到新的组 */
            } else {
                edgePointsCurrentStart = edgePointsLastEnd;
                edgePointsCurrentEnd = manger->edgePoints + manger->edgeInfo.edgesStartIndex[row];
                edgePointsCurrent = edgePointsCurrentStart;
                do {
                    NEW_EDGE_GROUP();
                    ++edgePointsCurrent;
                } while (edgePointsCurrent != edgePointsCurrentEnd);
            }
            edgePointsLastStart = edgePointsCurrentStart;
            edgePointsLastEnd = edgePointsCurrentEnd;
            edgePointsLast = edgePointsLastStart;
        }
    }
    return 0;
#undef NEW_EDGE_GROUP
}
/**
 * @brief 类似于查表法,来获取每行的边缘点位置
 * 
 * @author karl (karl.zhang@neuronBasic.com)
 * @date 2023-07-04
 * @version 1.0
 * 
 * @param edgePoint 边缘点信息的指针的指针
 * @param row 第几行
 * @param bitsGroup 第几个半字节,最低位的半字节为第0组
 * @param data 边缘点图row行的数据
 * @param leftEdgeGot 记录是否遇到了左边界
 * 
 */
static void s_getEdgePointFromTable(struct EdgePoint **edgePoint, uint32_t row, uint32_t bitsGroup, uint32_t data,
                                    uint8_t *leftEdgeGot)
{
#define ONE_BIT_HANDLE(bit1Position)                                                                                   \
    if ((*leftEdgeGot) == 0) {                                                                                         \
        (**edgePoint).left = (bitsGroup << 2) + (bit1Position); /*edgrGroup[row].append({ "L": 4 *  \
                                              group + bit1Position})*/                  \
        *leftEdgeGot = 1;                                                                                              \
    } else {                                                                                                           \
        (**edgePoint).right = (bitsGroup << 2) + (bit1Position); /*edgrGroup[row][-1]["R"] = 4 *group + bit1Position*/ \
        ++(*edgePoint);                                                                                                \
        *leftEdgeGot = 0;                                                                                              \
    }
#define TWO_BIT_HANDLE(bit1Position, bit2Position)                                                                      \
    if ((*leftEdgeGot) == 0) {                                                                                          \
        (**edgePoint).left = (bitsGroup << 2) + (bit1Position); /*edgrGroup[row].append({"L": 4 *   \
                                              group + bit1Position})*/                   \
        (**edgePoint).right = (bitsGroup << 2) + (bit2Position); /*edgrGroup[row][-1]["R"] = 4 * group + bit2Position*/ \
        ++(*edgePoint);                                                                                                 \
    } else {                                                                                                            \
        (**edgePoint).right = (bitsGroup << 2) + (bit1Position); /*edgrGroup[row][-1]["R"] = 4 *     \
                                              group + + bit1Position*/                  \
        ++(*edgePoint);                                                                                                 \
        (**edgePoint).left = (bitsGroup << 2) + (bit2Position); /*edgrGroup[row].append({"L": 4 *   \
                                              group + bit2Position})*/                   \
    }
#define THREE_BIT_HANDLE(bit1Position, bit2Position, bit3Position)                                                      \
    if ((*leftEdgeGot) == 0) {                                                                                          \
        (**edgePoint).left = (bitsGroup << 2) + (bit1Position); /*edgrGroup[row].append({"L": 4 *   \
                                              group + bit1Position})*/                   \
        (**edgePoint).right = (bitsGroup << 2) + (bit2Position); /*edgrGroup[row][-1]["R"] = 4 * group + bit2Position*/ \
        ++(*edgePoint);                                                                                                 \
        (**edgePoint).left = (bitsGroup << 2) + (bit3Position); /*edgrGroup[row].append({"L": 4 *   \
                                              group + bit3Position})*/                   \
        *leftEdgeGot = 1;                                                                                               \
    } else {                                                                                                            \
        (**edgePoint).right = (bitsGroup << 2) + (bit1Position); /*edgrGroup[row][-1]["R"] = 4 * group + bit1Position*/ \
        ++(*edgePoint);                                                                                                 \
        (**edgePoint).left = (bitsGroup << 2) + (bit2Position); /*edgrGroup[row].append({"L": 4 *   \
                                              group + bit2Position})*/                   \
        (**edgePoint).right = (bitsGroup << 2) + (bit3Position); /*edgrGroup[row][-1]["R"] = 4 * group + bit3Position*/ \
        ++(*edgePoint);                                                                                                 \
        *leftEdgeGot = 0;                                                                                               \
    }
    uint8_t halfByteState;
    /* 获取输入数据中特定的半字节,(bitsGroup << 2)等价于(bitsGroup * 4) */
    halfByteState = (data >> (bitsGroup << 2)) & 0x0f;
    switch (halfByteState) {
    case 0x01:
        ONE_BIT_HANDLE(0);
        break;
    case 0x02:
        ONE_BIT_HANDLE(1);
        break;
    case 0x03:
        TWO_BIT_HANDLE(1, 0);
        break;
    case 0x04:
        ONE_BIT_HANDLE(2);
        break;
    case 0x05:
        TWO_BIT_HANDLE(2, 0);
        break;
    case 0x06:
        TWO_BIT_HANDLE(2, 1);
        break;
    case 0x07:
        THREE_BIT_HANDLE(2, 1, 0);
        break;
    case 0x08:
        ONE_BIT_HANDLE(3);
        break;
    case 0x09:
        TWO_BIT_HANDLE(3, 0);
        break;
    case 0x0A:
        TWO_BIT_HANDLE(3, 1);
        break;
    case 0x0B:
        THREE_BIT_HANDLE(3, 1, 0);
        break;
    case 0x0C:
        TWO_BIT_HANDLE(3, 2);
        break;
    case 0x0D:
        THREE_BIT_HANDLE(2, 2, 0);
        break;
    case 0x0E:
        THREE_BIT_HANDLE(3, 2, 1);
        break;
    }
#undef ONE_BIT_HANDLE
#undef TWO_BIT_HANDLE
#undef THREE_BIT_HANDLE
}
/**
 * @brief 获取边缘点位置信息
 * 
 * @author karl (karl.zhang@neuronBasic.com)
 * @date 2023-07-04
 * @version 1.0
 * 
 * @param manger 多motion框相关的manger
 * @param edgeBitsMap 边缘点图
 * @param len 图高度
 * 
 */
static void s_getEdgePoint(struct BoundingRectManger *manger, const uint32_t *edgeBitsMap, uint8_t len)
{
    uint8_t leftEdgeGot = 0;
    uint32_t edgesStartIndex = 0;
    struct EdgePoint *edgePoints = NULL;
    uint32_t i;
    for (i = 0; i < len; ++i) {
        leftEdgeGot = 0;
        if (edgeBitsMap[i]) {
            edgePoints = manger->edgePoints + edgesStartIndex;
            if (edgeBitsMap[i] & 0xFFFF0000) {
                if (edgeBitsMap[i] & 0xFF000000) {
                    if (edgeBitsMap[i] & 0xF0000000)
                        s_getEdgePointFromTable(&edgePoints, i, 7, edgeBitsMap[i], &leftEdgeGot);
                    if (edgeBitsMap[i] & 0x0F000000)
                        s_getEdgePointFromTable(&edgePoints, i, 6, edgeBitsMap[i], &leftEdgeGot);
                }
                if (edgeBitsMap[i] & 0x00FF0000) {
                    if (edgeBitsMap[i] & 0x00F00000)
                        s_getEdgePointFromTable(&edgePoints, i, 5, edgeBitsMap[i], &leftEdgeGot);
                    if (edgeBitsMap[i] & 0x000F0000)
                        s_getEdgePointFromTable(&edgePoints, i, 4, edgeBitsMap[i], &leftEdgeGot);
                }
            }
            if (edgeBitsMap[i] & 0x0000FFFF) {
                if (edgeBitsMap[i] & 0x0000FF00) {
                    if (edgeBitsMap[i] & 0x0000F000)
                        s_getEdgePointFromTable(&edgePoints, i, 3, edgeBitsMap[i], &leftEdgeGot);
                    if (edgeBitsMap[i] & 0x00000F00)
                        s_getEdgePointFromTable(&edgePoints, i, 2, edgeBitsMap[i], &leftEdgeGot);
                }
                if (edgeBitsMap[i] & 0x000000FF) {
                    if (edgeBitsMap[i] & 0x000000F0)
                        s_getEdgePointFromTable(&edgePoints, i, 1, edgeBitsMap[i], &leftEdgeGot);
                    if (edgeBitsMap[i] & 0x0000000F)
                        s_getEdgePointFromTable(&edgePoints, i, 0, edgeBitsMap[i], &leftEdgeGot);
                }
            }
            edgesStartIndex = manger->edgeInfo.edgesStartIndex[i];
        }
    }
}
/**
 * @brief 生成边缘点图,并将信息存入struct EdgeInfo *info
 *
 * @author karl (karl.zhang@neuronBasic.com)
 * @date 2023-07-04
 * @version 1.0
 * 
 * @param src 输入源图
 * @param dst 输出边缘点图
 * @param len 图高度
 * @param info 记录每行边缘点累积的数量信息,和总的数量信息
 * 
 * @return int32_t 0 - 正常, -1 - 输入参数异常, -2 - 分配记录每行的边缘点累计数量信息的数组失败
 * 
 */
static int32_t s_markEdge(const uint32_t *src, uint32_t *dst, uint8_t len, struct EdgeInfo *info)
{
    /* 检查输入参数 */
    if ((src == dst) || (src == NULL) || (dst == NULL) || (info == NULL))
        return -1;

    /* 分配空间给记录每行的边缘点累计数量信息的数组,分配后不再释放,除非需要更改图高度,那么需要手动释放后重新初始化 */
    if (info->edgesStartIndex == NULL)
        info->edgesStartIndex = (uint8_t *)malloc(sizeof(uint8_t) * (len));
    if (info->edgesStartIndex == NULL)
        return -2;

    uint32_t i;
    uint8_t *edgesStartIndex = info->edgesStartIndex;
    info->edgeCount = 0;
    for (i = 0; i < len; ++i) {
        dst[i] = 0x00;
        if (src[i] != 0x00) {
            /* ((src[i] & (src[i] << 1)) ^ src[i])能得到右边界 */
            /* ((src[i] & (src[i] >> 1)) ^ src[i])能得到左边界 */
            /* 左边界与右边界异或以去掉既是左边界又是右边界的点,这样才能保证左右边界一一对应 */
            dst[i] = ((src[i] & (src[i] << 1)) ^ src[i]) ^ ((src[i] & (src[i] >> 1)) ^ src[i]);
            if (dst[i] != 0x00) {
                /* 左右边缘为一组,所以得到1的数量后要除以2,这里用右移1位实现 */
                info->edgeCount += s_swar(dst[i]) >> 1;
                /* 记录到当前行总共有多少个边缘点,后续可根据这个来确定连续的边缘点信息(BoundingRectManger.edgePoints)中每行的起点 */
                *(edgesStartIndex++) = info->edgeCount;
                continue;
            }
        }
        *(edgesStartIndex++) = 0;
    }
    return 0;
}
/**
 * @brief EdgeInfo结构体初始化
 * 
 * @author karl (karl.zhang@neuronBasic.com)
 * @date 2023-07-04
 * @version 1.0
 * 
 * @param edgeInfo 需要初始化的EdgeInfo结构体指针
 * 
 */
static void s_EdgeInfo_init(struct EdgeInfo *edgeInfo)
{
    edgeInfo->edgesStartIndex = NULL;
    edgeInfo->edgeBitsMap = NULL;
}
/**
 * @brief swar算法,用于获取32位无符号整数的二进制中有多少个1
 * 
 * @author karl (karl.zhang@neuronBasic.com)
 * @date 2023-07-04
 * @version 1.0
 * 
 * @param x 需要计算的32位整数
 * 
 * @return uint32_t 有多少个1
 *
 * @note
 * 要注意最长支持32bit的整数,如果需要64位的整数可以分成两个32位整数,调用本函数两次
 */
static uint32_t s_swar(uint32_t x)
{
    x = (x & 0x55555555) + ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (x & 0x0F0F0F0F) + ((x >> 4) & 0x0F0F0F0F);
    x = (x * 0x01010101) >> 24;
    return x;
}
#if (DEBUG_BOUNDING_RECT_INFO == 1)
static const uint8_t s_binaryString[16][5] = {
    "----", "---1", "--1-", "--11", "-1--", "-1-1", "-11-", "-111",
    "1---", "1--1", "1-1-", "1-11", "11--", "11-1", "111-", "1111",
};
static void s_printMap(const uint32_t *src, uint8_t len)
{
    for (uint8_t i = 0; i < 32; i++) {
        uint32_t temp = src[i];
        for (uint8_t j = 0; j < 8; ++j) {
            printf("%s", s_binaryString[(temp & 0xf0000000) >> 28]);
            temp <<= 4;
        }
        printf("\r\n");
    }
    printf("\r\n");
}
static void s_createFrameBitsMap(struct BoundingRectManger *manger, uint32_t *dst)
{
    struct EegeGroup *edgeGroupCurrent = manger->edgeGroup;
    for (edgeGroupCurrent = manger->edgeGroup; edgeGroupCurrent != NULL; edgeGroupCurrent = edgeGroupCurrent->last) {
        if (edgeGroupCurrent->belong == NULL) {
            dst[edgeGroupCurrent->up] |= ((uint32_t)(0xFFFFFFFF) >> (32 - edgeGroupCurrent->left + edgeGroupCurrent->right - 1))
                                         << edgeGroupCurrent->right;
            for (int i = edgeGroupCurrent->up; i <= edgeGroupCurrent->down; ++i) {
                dst[i] |= (0x01 << edgeGroupCurrent->left) | (0x01 << edgeGroupCurrent->right);
            }
            dst[edgeGroupCurrent->down] |=
                    ((uint32_t)(0xFFFFFFFF) >> (32 - edgeGroupCurrent->left + edgeGroupCurrent->right - 1))
                    << edgeGroupCurrent->right;
        }
    }
}
static void s_printMapALL(struct BoundingRectManger *manger, const uint32_t *src, uint8_t len)
{
    uint32_t temp;
    uint32_t *frame;
    frame = (uint32_t *)malloc(sizeof(uint32_t) * len);
    memset(frame, 0x00, sizeof(uint32_t) * len);
    s_createFrameBitsMap(manger, frame);
    for (uint8_t i = 0; i < 32; i++) {
        temp = src[i];
        for (uint8_t j = 0; j < 8; ++j) {
            printf("%s", s_binaryString[(temp & 0xf0000000) >> 28]);
            temp <<= 4;
        }
        printf("\t");
        temp = manger->edgeInfo.edgeBitsMap[i];
        for (uint8_t j = 0; j < 8; ++j) {
            printf("%s", s_binaryString[(temp & 0xf0000000) >> 28]);
            temp <<= 4;
        }
        printf("\t");
        temp = frame[i];
        for (uint8_t j = 0; j < 8; ++j) {
            printf("%s", s_binaryString[(temp & 0xf0000000) >> 28]);
            temp <<= 4;
        }
        printf("\r\n");
    }
    printf("\r\n");
    free(frame);
}
#endif