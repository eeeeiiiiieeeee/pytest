/**
 * 
 * Copyright (C) 2023 NeuronBasic Co., Ltd. All rights reserved.
 * 
 * @file motion_multiple.h
 * @author karl (karl.zhang@neuronBasic.com)
 * @version 1.0
 * @date 2023-07-04
 * @brief 多motion框头文件
 * 
 * @details 
 * 1.新建一个BoundingRectManger变量,用BoundingRectManger_init初始化,这一步只需要一次
 * 2.获取motion点图getMotionBitsMap()
 * 3.如果需要进行预处理,可调用erodeBitsMap来去掉小的motion框,调用dilateBitsMap来让分散的motion框合并,二者可多次调用来提高效果
 * 4.调用getBitsMapBoundingRect来获取所有的motion框
 * 5.判断BoundingRectManger.edgeInfo.edgeCount是否大于0,大于0说明有motion
 * 6.获取示例可查看getMotionBoundingRectDemo()
 * 
 * 7.如果不需要多个框可以直接调用getSingleBoundingRect()来获取全局的单个框
 *
 * @see getMotionBoundingRectDemo()
 * @note 
 * 1.调试信息可通过DEBUG_BOUNDING_RECT_INFO和DEBUG_PERF_TIME来控制
 * 2.如果需要重新初始化需要调用releaseBitsMapBoundingRectALL()来释放内存
 * 3.目前不是线程安全的,如果在多个线程处理,需要手动加锁
 * 4.每次进入getBitsMapBoundingRect会先释放上一轮的BoundingRectManger.edgePoints和BoundingRectManger.edgeGroup
 */
#ifndef MOTION_MULTIPLE_H
#define MOTION_MULTIPLE_H

/* 在linux下测试会切换成另外的头文件包含 */
/* linux下测试不要启用DEBUG_PERF_TIME */
#define __linux__
#ifndef __linux__
// #include "nbsdk.h"
#else
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#define DEBUG_BOUNDING_RECT_INFO 1 //为1就打印debug信息
#define DEBUG_PERF_TIME          0 //为1就打印性能信息


#if (DEBUG_PERF_TIME == 1)
/* 宏实现的运行耗时装饰器,function在填入的时候需要用{}包裹 */
#define DEBUG_PERF_TIME_WRAPPER(discription, startTick, endTick, function)                    \
    (startTick) = perf_get_ticks();                                                           \
    {                                                                                         \
        function;                                                                             \
    };                                                                                        \
    (endTick) = perf_get_ticks();                                                             \
    printf("%s use Tick:%.0f use time:%.1f\r\n", (discription), (float)(endTick - startTick), \
           (float)((endTick - startTick)) / (float)(PERF_TIMER_US_CNT));
#else
#define DEBUG_PERF_TIME_WRAPPER(discription, startTick, endTick, function) \
    {                                                                      \
        function;                                                          \
    }
#endif
/**
 * @brief 单个框的信息
 * 
 * @author karl (karl.zhang@neuronBasic.com)
 * @date 2023-07-04
 * @version 1.0
 * 
 * 
 */
struct MotionBoundingRect {
    uint8_t left; //< 当前框的左边
    uint8_t right; //< 当前框的右边
    uint8_t up; //<当前框的顶边
    uint8_t down; //< 当前框的底边
};
/**
 * @brief 记录边缘点所属组的情况
 * 
 * @author karl (karl.zhang@neuronBasic.com)
 * @date 2023-07-04
 * @version 1.0
 * 
 *
 */
struct EegeGroup {
    uint8_t left; //< 当前组框的左边
    uint8_t right; //< 当前组框的右边
    uint8_t up; //<当前组框的顶边
    uint8_t down; //< 当前组框的底边
    struct EegeGroup *last; //< 新生成的组的last指向上一个生成的组的地址
    struct EegeGroup *belong; //< 记录当前组属于哪个组,如果是NULL就说明是根组,根组记录的就是框的信息
};
/**
 * @brief 记录某个边缘点的信息
 * 
 * @author karl (karl.zhang@neuronBasic.com)
 * @date 2023-07-04
 * @version 1.0
 * 
 * 
 */
struct EdgePoint {
    uint8_t left; //< 边缘点的左边界
    uint8_t right; //< 边缘点的右边界
    struct EegeGroup *group; //< 本边缘点属于哪个组
};
/**
 * @brief 记录所有边缘点的信息
 * 
 * @author karl (karl.zhang@neuronBasic.com)
 * @date 2023-07-04
 * @version 1.0
 * 
 * 
 */
struct EdgeInfo {
    uint32_t edgeCount; //< 记录总共有多少个边缘点
    uint8_t *edgesStartIndex; //< 记录截至每行累积了多少个边缘点,如果为0说明那一行没有边缘点
    uint32_t *edgeBitsMap; //< 存储边缘点点图
};
/**
 * @brief 多motion框相关的manger
 * 
 * @author karl (karl.zhang@neuronBasic.com)
 * @date 2023-07-04
 * @version 1.0
 * 
 * 
 */
struct BoundingRectManger {
/* 传入指针,返回manger是不是没有初始化 */
#define BOUNDING_RECT_MANAGER_NOT_INITIALIZED(manger) ((manger)->initialized != (manger))
    struct BoundingRectManger *initialized; //< 记录manger是否初始化,初始化的时候会存入manger的地址
    struct EdgeInfo edgeInfo; //< 记录边缘点原始信息
    struct EdgePoint *
            edgePoints; //< 记录所有边缘点的位置和所属组信息,每行的边缘点连续排放,要根据edgeInfo.edgesStartIndex来确定每行边缘点的起始位置
    struct EegeGroup *edgeGroup; //< 记录所有的组的信息
};


int32_t getSingleBoundingRect(struct MotionBoundingRect *result, const uint32_t *bitsMap, uint8_t len);

void BoundingRectManger_init(struct BoundingRectManger *manger);
uint32_t *getMotionBitsMap(void);
int32_t releaseBitsMapBoundingRectALL(struct BoundingRectManger *manger);
int32_t getBitsMapBoundingRect(struct BoundingRectManger *manger, const uint32_t *bitsMap, uint8_t len);

int32_t erodeBitsMap(const uint32_t *src, uint32_t *dst, uint8_t len);
int32_t dilateBitsMap(const uint32_t *src, uint32_t *dst, uint8_t len);

#endif
