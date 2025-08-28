#include "filter.h"

static void static_deque_init(StaticDeque *deque);
static DequeNode *static_node_alloc(double data);
static int deque_push_front(StaticDeque *deque, double data);



static int node_index = 0;
static double weight[LMS_M] = {0};      // 滤波器系数
static double weight_sec_p[LMS_M] = {0};  // 次级通道参数
static double mu = MU_MAX; // 步长参数

static DequeNode node_pool[MAX_DEQUE_SIZE];
static StaticDeque lms_deque = {0};
static StaticDeque spi_deque = {0};

void filter_init(void)
{
    node_index = 0;
    for (int i = 0; i < MAX_DEQUE_SIZE; i++)
    {
        node_pool[i].prev = NULL;
        node_pool[i].next = NULL;
        node_pool[i].data = 0;
    }
    for (int i = 0; i < LMS_M; i++)
    {
        weight[i] = 0;
    }
    for (int i = 0; i < LMS_M; i++)
    {
        weight_sec_p[i] = 0;
    }
}

/* 静态队列操作函数 */
static void static_deque_init(StaticDeque *deque)
{
    deque->front = NULL;
    deque->rear = NULL;
    deque->size = 0;
}

static DequeNode *static_node_alloc(double data)
{
    if (node_index >= MAX_DEQUE_SIZE)
        node_index = 0;
    DequeNode *node = &node_pool[node_index++];
    node->data = data;
    node->prev = NULL;
    node->next = NULL;
    return node;
}

static void deque_push_front(StaticDeque *deque, double data)
{
    DequeNode *node = static_node_alloc(data);

    if (deque->size >= LMS_M)
    {
        DequeNode *old = deque->rear;
        deque->rear = old->prev;
        deque->rear->next = NULL;
        deque->size--;
    }
    else if (deque->size == 0)  // 仅在刚初始化后执行
    {
        deque->front = deque->rear = node;
        deque->size = 1;
        return;
    }

    node->next = deque->front;
    deque->front->prev = node;
    deque->front = node;
    deque->size++;
}

void weight_update(double err_signal)
{
    DequeNode *current = lms_deque.front;
    for (int i = 0; i < LMS_M && current; i++)
    {
        weight[i] += mu * err_signal * current->data;
        current = current->next;
    }
}

void weight_sec_p_update(double err_signal)
{
    double norm = 1e-8;
    DequeNode *current = spi_deque.front;
    for (int i = 0; i < LMS_M && current; i++)
    {
        norm += current->data * current->data;
    }

    for (int i = 0; i < LMS_M && current; i++)
    {
        norm = current->data * current->data;
        weight_sec_p[i] += (mu / norm) * err_signal * current->data;
        current = current->next;
    }
}

double output_get(double ref_signal)
{
    double sum = 0;
    double ref_filtered_signal = 0;
    static bool initialized = 0;
    DequeNode *current;
    if (!initialized)
    {
        static_deque_init(&lms_deque);
        initialized = 1;
    }
    
    current = lms_deque.front;
    for (int i = 0; i < LMS_M && current; i++)
    {
        ref_filtered_signal += W_sec[i] * current.data[i];
        current = current->next;
    }
    deque_push_front(&lms_deque, ref_filtered_signal);

    current = lms_deque.front;
    for (int i = 0; i < LMS_M && current; i++)
    {
        sum += current->data * W[i];
        current = current->next;
    }
    return sum;
}

void update_input_deque_only(double exc_signal)
{
    static bool initialized = 0;
    if (!initialized)
    {
        static_deque_init(&spi_deque);
        initialized = 1;
    }

    deque_push_front(&spi_deque, exc_signal)
}