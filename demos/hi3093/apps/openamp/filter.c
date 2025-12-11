#include "filter.h"

#include <math.h>

#define FxLMS
#define ALPHA 1.0
#define TYPE_LMS 0
#define TYPE_SEP 1
#define TYPE_ERR 2

static void static_deque_init(StaticDeque* deque);
static DequeNode* static_node_alloc(int type, double data);
static void deque_push_front(int type, StaticDeque* deque, double data);
static int lms_node_index = 0;
static int sep_node_index = 0;
static int err_node_index = 0;
static double weight[LMS_M] = {0};        // 滤波器系数
static double weight_sep[LMS_M] = {0};    // 次级通道参数
double mu_control = MU_MAX;               // 步长参数
static double mu_identify = MU_MAX / 20;  // 步长参数

static DequeNode lms_node_pool[MAX_INPUT_SIZE];
static DequeNode sep_node_pool[MAX_DEQUE_SIZE];
static DequeNode err_node_pool[MAX_ERROR_SIZE];
static StaticDeque lms_deque = {0};
static StaticDeque sep_deque = {0};
static StaticDeque err_deque = {0};

/* 静态队列操作函数 */
static void static_deque_init(StaticDeque* deque) {
    deque->front = NULL;
    deque->rear = NULL;
    deque->size = 0;
}

static DequeNode* static_node_alloc(int type, double data) {
    DequeNode* node = NULL;
    if (type == TYPE_LMS) {
        if (lms_node_index >= MAX_INPUT_SIZE) {
            lms_node_index = 0;
        }
        node = &lms_node_pool[lms_node_index++];
    } else if (type == TYPE_ERR) {
        if (err_node_index >= MAX_ERROR_SIZE) {
            err_node_index = 0;
        }
        node = &err_node_pool[err_node_index++];
    } else if (type == TYPE_SEP) {
        if (sep_node_index >= MAX_DEQUE_SIZE) {
            sep_node_index = 0;
        }
        node = &sep_node_pool[sep_node_index++];
    }

    node->data = data;
    node->prev = NULL;
    node->next = NULL;
    return node;
}

static void deque_push_front(int type, StaticDeque* deque, double data) {
    DequeNode* node = static_node_alloc(type, data);
    if (!node) return;  // 检查节点分配是否成功

    if (deque->size >= LMS_M) {
        if (deque->rear) {
            DequeNode* old_rear = deque->rear;
            if (old_rear->prev) {
                deque->rear = old_rear->prev;
                deque->rear->next = NULL;
            } else {
                deque->front = NULL;
                deque->rear = NULL;
            }
            deque->size--;
        }
    }

    // 插入新节点到头部
    if (deque->size == 0) {
        deque->front = node;
        deque->rear = node;
    } else {
        node->next = deque->front;
        deque->front->prev = node;
        deque->front = node;
    }
    deque->size++;
}

double calculate_vss_step_size() {
    double error_norm = 0.0;  // 误差信号范数
    int count = 0;            // 实际计算的误差信号数量
    DequeNode* current = err_deque.front;

    // 计算误差信号的二范数（最近16个信号）
    while (current != NULL && count < LMS_M) {
        error_norm += current->data * current->data;
        current = current->next;
        count++;
    }

    // 计算平方根得到范数
    error_norm = sqrt(error_norm);

    // 计算步长：μ(n) = μ_max * log(α + ||e(n)||)
    double step_size = MU_MAX * log(ALPHA + error_norm);

    // 步长约束处理
    if (step_size > MU_MAX) {
        step_size = MU_MAX;  // 超过上限则取最大值
    } else if (step_size < 0) {
        step_size = 0.0;  // 保证步长为非负
    }

    return step_size;
}

void weight_update(double err_signal) {
    deque_push_front(TYPE_ERR, &err_deque, err_signal);
    mu_control = calculate_vss_step_size();

    DequeNode* current = lms_deque.front;
    for (int i = 0; i < LMS_M && current; i++) {
        weight[i] += mu_control * err_signal * current->data;
        if (weight[i] > 5.0)
            weight[i] = 5.0;
        else if (weight[i] < -5.0)
            weight[i] = -5.0;
        current = current->next;
    }
}

void weight_sep_update(double err_signal) {
    // double norm = 1e-8;
    double output = 0.0;
    double err = 0.0;
    DequeNode* current = sep_deque.front;

    for (int i = 0; i < LMS_M && current; i++) {
        output += current->data * weight_sep[i];
        current = current->next;
    }

    err = err_signal - output;

    current = sep_deque.front;
    for (int i = 0; i < LMS_M && current; i++) {
        // weight_sep[i] += (mu / norm) * err_signal * current->data;
        weight_sep[i] += mu_identify * err * current->data;
        if (weight_sep[i] > 10.0)
            weight_sep[i] = 10.0;
        else if (weight_sep[i] < -10.0)
            weight_sep[i] = -10.0;
        current = current->next;
    }
}

double output_get(double ref_signal) {
    double output = 0;
    double ref_filtered_signal = 0;
    DequeNode* current = NULL;

#ifdef FxLMS
    current = lms_deque.front;
    for (int i = 0; i < LMS_M && current; i++) {
        ref_filtered_signal += weight_sep[i] * current->data;
        current = current->next;
    }
    deque_push_front(TYPE_LMS, &lms_deque, ref_filtered_signal);
#else
    deque_push_front(TYPE_LMS, &lms_deque, ref_signal);
#endif
    current = lms_deque.front;
    for (int i = 0; i < LMS_M && current; i++) {
        output += current->data * weight[i];
        current = current->next;
    }
    return output;
}

double update_input_deque_only(double exc_signal) {
    DequeNode* current = NULL;
    double output = 0;
    deque_push_front(TYPE_SEP, &sep_deque, exc_signal);
    current = sep_deque.front;
    for (int i = 0; i < LMS_M && current; i++) {
        output += current->data * weight[i];
        current = current->next;
    }
    return output;
}

double* get_weight_sep_array(void) { return weight_sep; }

double* get_weight_array(void) { return weight; }

void filter_init(void) {
    for (int i = 0; i < MAX_INPUT_SIZE; i++) {
        lms_node_pool[i].prev = NULL;
        lms_node_pool[i].next = NULL;
        lms_node_pool[i].data = 0;
    }
    sep_node_index = 0;
    for (int i = 0; i < MAX_DEQUE_SIZE; i++) {
        sep_node_pool[i].prev = NULL;
        sep_node_pool[i].next = NULL;
        sep_node_pool[i].data = 0;
    }
    for (int i = 0; i < MAX_ERROR_SIZE; i++) {
        err_node_pool[i].prev = NULL;
        err_node_pool[i].next = NULL;
        err_node_pool[i].data = 0;
    }

    static_deque_init(&lms_deque);
    static_deque_init(&sep_deque);
    static_deque_init(&err_deque);

    for (int i = 0; i < LMS_M; i++) {
        weight[i] = 0;
    }
    // for (int i = 0; i < LMS_M; i++) {
    //     weight_sep[i] = 0;
    // }
    weight_sep[0] = 0.164302;   // 6.7/4.5 * 0.11035291
    weight_sep[1] = 0.142222;   // 6.7/4.5 * 0.09554795
    weight_sep[2] = 0.118937;   // 6.7/4.5 * 0.07990297
    weight_sep[3] = 0.095196;   // 6.7/4.5 * 0.06394793
    weight_sep[4] = 0.072540;   // 6.7/4.5 * 0.04873101
    weight_sep[5] = 0.051771;   // 6.7/4.5 * 0.03477907
    weight_sep[6] = 0.034431;   // 6.7/4.5 * 0.02312974
    weight_sep[7] = 0.021288;   // 6.7/4.5 * 0.01430119
    weight_sep[8] = 0.013876;   // 6.7/4.5 * 0.00932209
    weight_sep[9] = 0.010650;   // 6.7/4.5 * 0.00715488
    weight_sep[10] = 0.011594;  // 6.7/4.5 * 0.00778951
    weight_sep[11] = 0.090266;  // 6.7/4.5 * 0.06063321
    weight_sep[12] = 0.081346;  // 6.7/4.5 * 0.05464950
    weight_sep[13] = 0.074174;  // 6.7/4.5 * 0.04982501
    weight_sep[14] = 0.093216;  // 6.7/4.5 * 0.06262295
    weight_sep[15] = 0.084856;  // 6.7/4.5 * 0.05700105
}

void filter_reinit(void) {
    for (int i = 0; i < MAX_INPUT_SIZE; i++) {
        lms_node_pool[i].prev = NULL;
        lms_node_pool[i].next = NULL;
        lms_node_pool[i].data = 0;
    }
    sep_node_index = 0;
    for (int i = 0; i < MAX_DEQUE_SIZE; i++) {
        sep_node_pool[i].prev = NULL;
        sep_node_pool[i].next = NULL;
        sep_node_pool[i].data = 0;
    }
    for (int i = 0; i < MAX_ERROR_SIZE; i++) {
        err_node_pool[i].prev = NULL;
        err_node_pool[i].next = NULL;
        err_node_pool[i].data = 0;
    }

    static_deque_init(&lms_deque);
    static_deque_init(&sep_deque);
    static_deque_init(&err_deque);

    for (int i = 0; i < LMS_M; i++) {
        weight[i] = 0;
    }
}