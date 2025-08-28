#define X86test // X86测试
#define VSS_Enable  // 开启变步长
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

void filter_init(void);
double output_get(double ref_signal);
void weight_update(double err_signal);


#endif

