#include "filter.h"
// #include <pthread.h>

static void static_deque_init(StaticDeque *deque);
static DequeNode *static_node_alloc(double data);
static int deque_push_front(StaticDeque *deque, double data);
static double Error_Update(double desire, double output);
static void W_Update(StaticDeque *deque, double error);

/* 全局共享资源保护 */
// static pthread_mutex_t deque_mutex = PTHREAD_MUTEX_INITIALIZER;
// static pthread_mutex_t filter_mutex = PTHREAD_MUTEX_INITIALIZER;

/* 静态内存池和索引 */
static DequeNode node_pool[MAX_DEQUE_SIZE];
static int node_index = 0;

/* 滤波器相关全局变量 */
static double W[LMS_M] = {0};      // 滤波器系数
static double MU_current = MU_MAX; // 步长参数
#ifdef VSS_TransX
static double InputArray[LMS_M]; // 输入缓存
#endif
#ifdef VSS_TransERR
static double ErrorArray[MAX_ERR_SIZE]; // 错误缓存
static int error_index = 0;
#endif

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
        node_index = 0; // 重置索引以重用节点

    DequeNode *node = &node_pool[node_index++];
    node->data = data;
    node->prev = NULL;
    node->next = NULL;
    return node;
}

static int deque_push_front(StaticDeque *deque, double data)
{
    // pthread_mutex_lock(&deque_mutex);

    if (deque->size >= LMS_M)
    { // 滤波器只需要保留LMS_M长度
        // 移除最旧节点并重用内存
        DequeNode *old = deque->rear;
        if (deque->size > 1)
        {
            deque->rear = old->prev;
            deque->rear->next = NULL;
        }
        else
        {
            deque->front = deque->rear = NULL;
        }
        deque->size--;
        // node_index--; // 允许重用该节点位置
    }

    DequeNode *node = static_node_alloc(data);
    if (!node)
    {
        // pthread_mutex_unlock(&deque_mutex);
        return -1;
    }

    if (deque->size == 0)
    {
        deque->front = deque->rear = node;
    }
    else
    {
        node->next = deque->front;
        deque->front->prev = node;
        deque->front = node;
    }
    deque->size++;

    // pthread_mutex_unlock(&deque_mutex);
    return 0;
}

/* 残差计算函数 */
static double Error_Update(double err_signal, double output)
{
    double error = err_signal - output;

#ifdef VSS_TransERR
    // pthread_mutex_lock(&filter_mutex);
    ErrorArray[error_index++ % MAX_ERR_SIZE] = error;
    // pthread_mutex_unlock(&filter_mutex);
#endif

    return error;
}

static void W_Update(StaticDeque *deque, double error)
{
    // pthread_mutex_lock(&filter_mutex);

    DequeNode *current = deque->front;
    for (int i = 0; i < LMS_M && current; i++)
    {
        W[i] += MU_current * error * current->data;
#ifdef VSS_TransX
        InputArray[i] = current->data; // 更新输入缓存
#endif
        current = current->next;
    }

    // pthread_mutex_unlock(&filter_mutex);
}

void W_Reset(void)
{
    // pthread_mutex_lock(&filter_mutex);
    for (int i = 0; i < LMS_M; i++)
    {
        W[i] = 0.0; // 重置滤波器系数
    }
    // pthread_mutex_unlock(&filter_mutex);
}

/* 对外接口 Data1-->REF Data2-->ERR*/
double outputget(double Data1, double Data2)
{
    static StaticDeque REF = {0};
    static double error = 0.0;
    static bool initialized = 0;

    // 初始化检查
    if (!initialized)
    {
        static_deque_init(&REF);
        initialized = 1;
    }

    double sum = 0;
    int i = 0;

    // 更新输入队列
    if (deque_push_front(&REF, Data1) != 0)
    {
        return 0; // 错误处理
    }

    // 计算输出
    // pthread_mutex_lock(&deque_mutex);
    DequeNode *current = REF.front;
    for (i = 0; i < LMS_M && current; i++)
    {
        sum += current->data * W[i];
        current = current->next;
    }
    if (sum < -10.0 || sum > 10.0)
    {
        sum = 0; // 限制输出范围
        W_Reset(); // 重置系数
        return sum; // 返回0表示重置
    }
    // pthread_mutex_unlock(&deque_mutex);

    // 更新系数
    error = Error_Update(Data2, sum);
    W_Update(&REF, error);

    return sum;
}

void FilterInit(void)
{
    // 初始化内存池
    node_index = 0;
    for (int i = 0; i < MAX_DEQUE_SIZE; i++)
    {
        node_pool[i].prev = NULL;
        node_pool[i].next = NULL;
        node_pool[i].data = 0;
    }

    // 初始化滤波器系数
    // pthread_mutex_lock(&filter_mutex);
    for (int i = 0; i < LMS_M; i++)
    {
        W[i] = 0;
    }
    // pthread_mutex_unlock(&filter_mutex);

}