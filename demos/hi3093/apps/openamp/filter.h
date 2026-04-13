#define X86test     // X86测试
#define VSS_Enable  // 开启变步长
#ifndef FILTER_H
#define FILTER_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define LMS_M 32
#define SEC_M 5
#define MU_MAX 0.01
#define MAX_INPUT_SIZE (LMS_M * 2)  // 双倍长度保证滤波器需求
#define MAX_ERROR_SIZE (LMS_M * 2)  // 错误队列容量
#define MAX_DEQUE_SIZE (LMS_M * 2)  // 双倍长度保证滤波器需求

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
void filter_reinit(void);

double output_get(double ref_signal);
double update_input_deque_only(double exc_signal);

void weight_update(double err_signal);
void weight_sep_update(double err_signal);

double* get_weight_sep_array(void);
double* get_weight_array(void);

#endif