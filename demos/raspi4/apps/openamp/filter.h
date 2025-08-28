#define X86test // X86测试
#define VSS_Enable  // 开启变步长
//#define VSS_TransERR
#define VSS_TransX

#ifndef FILTER_H
#define FILTER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define LMS_M 16
#define ALPHA 0.01
#define MU_MAX 0.01
#define MAX_DEQUE_SIZE (LMS_M * 2)  // 双倍长度保证滤波器需求
#define MAX_ERR_SIZE   32           // 错误队列容量

#ifdef VSS_TransX
#define MAX_MSG_SIZE (sizeof(double) * LMS_M)
#else
#define MAX_MSG_SIZE (sizeof(double) * MAX_ERR_SIZE)
#endif

/* 静态内存池实现的双端队列 */
typedef struct DequeNode {
    double data;
    struct DequeNode* prev;
    struct DequeNode* next;
} DequeNode;

typedef struct {
    DequeNode* front;
    DequeNode* rear;
    int size;
} StaticDeque;

double outputget(double newx, double dsignal);
void FilterInit(void);
void W_Reset(void);

#endif

