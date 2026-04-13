#include "filter.h"

#include <math.h>

#define FxLMS
#define ALPHA 1.0
#define TYPE_LMS 0
#define TYPE_SEP 1
#define TYPE_ERR 2
#define TYPE_INP 3

static void static_deque_init(StaticDeque* deque);
static DequeNode* static_node_alloc(int type, double data);
static void deque_push_front(int type, StaticDeque* deque, double data);
static int lms_node_index = 0;
static int inp_node_index = 0;
static int sep_node_index = 0;
static int err_node_index = 0;
static double weight[LMS_M] = {0};        // 滤波器系数
static double weight_sep[SEC_M] = {0};    // 次级通道参数
double mu_control = MU_MAX;               // 步长参数
static double mu_identify = MU_MAX / 20;  // 步长参数
static double ref_now = 0.0; 

static DequeNode lms_node_pool[MAX_INPUT_SIZE];
static DequeNode inp_node_pool[MAX_INPUT_SIZE];
static DequeNode sep_node_pool[MAX_DEQUE_SIZE];
static DequeNode err_node_pool[MAX_ERROR_SIZE];
static StaticDeque lms_deque = {0};
static StaticDeque inp_deque = {0};
static StaticDeque sep_deque = {0};
static StaticDeque err_deque = {0};

// ==================== 次级通道建模参数（你辨识出的结果）====================
#define p       1.7756f
#define alpha   4.0301f
#define beta    0.60058f
#define gamma   -0.20386f
#define a1      0.72038f
#define a2      0.080728f
#define b1      -0.17853f
#define b2      0.23379f

// 采样时间 Δt（与MATLAB辨识时一致，默认1即可）
#define delta_t 1.0f

// ==================== 次级通道滤波状态结构体（必须静态保存）====================
typedef struct {
    // 迟滞环节 h(n) 状态
    float u_prev;    // 上一时刻输入 u(n-1)
    float h_prev;    // 上一时刻迟滞输出 h(n-1)
    
    // ARX 线性环节状态
    float y1;        // y(n-1)
    float y2;        // y(n-2)
    float x1;        // x(n-1)
    float x2;        // x(n-2)
} SecondaryPathState;

#define BETA_PX 0.99  // 参考信号功率平滑系数（接近1，功率估计更平稳）
#define EPSILON 1e-6  // 防除零小常数

// 全局/静态状态变量（保证连续调用时状态不丢失）
static SecondaryPathState sp_state = {0};

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
    } else if (type == TYPE_INP) {
        if (inp_node_index >= MAX_INPUT_SIZE) {
            inp_node_index = 0;
        }
        node = &inp_node_pool[inp_node_index++];
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

/* double calculate_vss_step_size_log(double err_signal) {
    double error_norm = 0.0;  // 误差信号范数
    int count = 0;            // 实际计算的误差信号数量

    double step_size = -5 * MU_MAX * log(1 / (1 + 0.5 * err_signal * err_signal));

    // 步长约束处理
    if (step_size > MU_MAX) {
        step_size = MU_MAX;  // 超过上限则取最大值
    } else if (step_size < 0.02) {
        step_size = 0.02;  // 保证步长为非负
    }

    return step_size;
} */

double calculate_vss_step_size_log(double err_signal) {
    static double px_prev = 0.0;  // 上一时刻参考信号功率 Px(n-1)（新增）

    double ref_signal = ref_now;
    // 1. 新增：估计参考信号功率 Px(n)（移动平均）
    double x_sq = ref_signal * ref_signal;
    double px_current = BETA_PX * px_prev + (1 - BETA_PX) * x_sq;

    // 2. 原核心公式，仅对误差平方项做参考信号功率归一化
    // 仅修改此处：误差平方项除以 (EPSILON + px_current) 归一化
    double step_size = -1 * MU_MAX * log(1 / (1 + 0.5 * err_signal * err_signal/ (EPSILON + px_current) ));

    // 3. 原步长约束逻辑保持不变
    if (step_size > MU_MAX) {
        step_size = MU_MAX;  // 超过上限则取最大值
    } else if (step_size < 0.001) {
        step_size = 0.001;  // 保证步长为非负
    }

    // 4. 新增：更新参考信号功率历史值
    px_prev = px_current;

    return step_size;
}

double calculate_vss_step_size_adv(double err_signal) {
    double error_norm = 0.0;  // 误差信号范数
    int count = 0;            // 实际计算的误差信号数量

    static double px_prev = 0.0;
    double ref_signal = ref_now;
    // 1. 新增：估计参考信号功率 Px(n)（移动平均）
    double x_sq = ref_signal * ref_signal;
    double px_current = BETA_PX * px_prev + (1 - BETA_PX) * x_sq;

    DequeNode* current = err_deque.front;
    // 计算误差信号的二范数（最近16个信号）
    while (current != NULL && count < LMS_M) {
        error_norm += current->data * current->data;
        current = current->next;
        count++;
    }

    // 计算平方根得到范数
    error_norm = sqrt(error_norm);

    double alpha_vss = 2 + 9 * (1 - exp(-0.05 * error_norm));

    // 计算步长：μ(n) = μ_max * log(α + ||e(n)||)
    double step_size = MU_MAX * log(alpha_vss + error_norm)  ;

    // 步长约束处理
    if (step_size > MU_MAX) {
        step_size = MU_MAX;  // 超过上限则取最大值
    } else if (step_size < 0) {
        step_size = 0.0;  // 保证步长为非负
    }

    return step_size;
}

/* double calculate_vss_step_size_pst(double err_signal) {
    static double mu_prev = MU_MAX;  // 上一时刻步长 μ(n-1)
    static double e_prev1 = 0.0;  // 上一时刻误差 e(n-1)
    static double e_prev2 = 0.0;  // 上两时刻误差 e(n-2)

    // 核心公式计算当前步长 μ(n)
    double e1_sq = e_prev1 * e_prev1;  // e²(n-1)
    double e2_sq = e_prev2 * e_prev2;  // e²(n-2)
    double mu_current = 0.995 * mu_prev + 0.05 * e1_sq * e2_sq;

    // 步长限幅，保证算法稳定性
    if (mu_current > MU_MAX) {
        mu_current = MU_MAX;
    }else if (mu_current < 0.02) {
        mu_current = 0.02;
    }

    // 更新历史变量，为下一次迭代做准备
    e_prev2 = e_prev1;
    e_prev1 = err_signal;
    mu_prev = mu_current;

    // 返回当前时刻的步长 μ(n)
    return mu_current;
} */


// 仅新增参考信号输入 ref_signal，其余核心逻辑保持不变
double calculate_vss_step_size_pst(double err_signal) {
    static double mu_prev = MU_MAX;  // 上一时刻步长 μ(n-1)
    static double e_prev1 = 0.0;     // 上一时刻误差 e(n-1)
    static double e_prev2 = 0.0;     // 上两时刻误差 e(n-2)
    static double px_prev = 0.0;     // 上一时刻参考信号功率 Px(n-1)（新增）

    double ref_signal = ref_now;

    // 1. 新增：估计参考信号功率 Px(n)（移动平均）
    double x_sq = ref_signal * ref_signal;
    double px_current = BETA_PX * px_prev + (1 - BETA_PX) * x_sq;

    // 2. 原核心公式，仅对误差项做参考信号功率归一化
    double e1_sq = e_prev1 * e_prev1;  // e²(n-1)
    double e2_sq = e_prev2 * e_prev2;  // e²(n-2)
    // 仅修改此处：误差项除以 (EPSILON + px_current) 归一化
    double mu_current = 0.995 * mu_prev + 0.05 * e1_sq * e2_sq / (EPSILON + px_current);

    // 3. 原步长限幅逻辑保持不变
    if (mu_current > MU_MAX) {
        mu_current = MU_MAX;
    } else if (mu_current < 0.005) {
        mu_current = 0.005;
    }

    // 4. 更新历史变量（新增 px_prev 的更新）
    e_prev2 = e_prev1;
    e_prev1 = err_signal;
    mu_prev = mu_current;
    px_prev = px_current;  // 新增：更新参考信号功率历史值

    // 返回当前时刻的步长 μ(n)
    return mu_current;
}

void weight_update(double err_signal) {
    deque_push_front(TYPE_ERR, &err_deque, err_signal);
    mu_control = calculate_vss_step_size_log(err_signal);
    // mu_control = MU_MAX;  // 固定步长

    DequeNode* current = lms_deque.front;
    for (int i = 0; i < LMS_M && current; i++) {
        weight[i] += mu_control * err_signal * current->data;
        if (weight[i] > 10.0)
            weight[i] = 10.0;
        else if (weight[i] < -10.0)
            weight[i] = -10.0;
        current = current->next;
    }
}

void weight_sep_update(double err_signal) {
    // double norm = 1e-8;
    double output = 0.0;
    double err = 0.0;
    DequeNode* current = sep_deque.front;

    for (int i = 0; i < SEC_M && current; i++) {
        output += current->data * weight_sep[i];
        current = current->next;
    }

    err = err_signal - output;

    current = sep_deque.front;
    for (int i = 0; i < SEC_M && current; i++) {
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
    DequeNode* current = NULL;

    static int count = 50;
    if (count > 1) {
        count--;
        return 0;
    }
    
    double ref_filtered_signal = 0;
    deque_push_front(TYPE_INP, &inp_deque, ref_signal);
    current = inp_deque.front;
    for (int i = 0; i < SEC_M && current; i++) {
        ref_filtered_signal += weight_sep[i] * current->data;
        current = current->next;
    }
    deque_push_front(TYPE_LMS, &lms_deque, ref_filtered_signal);

/*         // ====================== 1. 计算迟滞环节 h(n) ======================
    float u = ref_signal;
    float delta_u = u - sp_state.u_prev;
    float du_dt = delta_u / delta_t;  // (u(n)-u(n-1))/Δt

    // 公式：h(n) = α*du_dt - β*|du_dt|*h_prev - γ*du_dt*|h_prev| + h_prev
    float term1 = alpha * du_dt;
    float term2 = beta * fabsf(du_dt) * sp_state.h_prev;
    float term3 = gamma * du_dt * fabsf(sp_state.h_prev);
    float h = term1 - term2 - term3 + sp_state.h_prev;

    // ====================== 2. 计算非线性输出 x(n) ======================
    // 公式：x(n) = p*u(n) + h(n)
    float x = p * u + h;

    // ====================== 3. ARX 线性环节输出 y(n) ======================
    // 公式：y(n) = a1*y(n-1) + a2*y(n-2) + b1*x(n-1) + b2*x(n-2)
    float y = a1 * sp_state.y1 + a2 * sp_state.y2
            + b1 * sp_state.x1 + b2 * sp_state.x2;

    // ====================== 4. 更新所有状态（为下一时刻准备）======================
    // 更新迟滞状态
    sp_state.u_prev = u;
    sp_state.h_prev = h;

    // 更新 ARX 历史状态（移位存储）
    sp_state.y2 = sp_state.y1;
    sp_state.y1 = y;
    sp_state.x2 = sp_state.x1;
    sp_state.x1 = x;

    // 返回滤波后的信号（FxLMS 用）
    deque_push_front(TYPE_LMS, &lms_deque, y); */

    current = inp_deque.front;
    for (int i = 0; i < LMS_M && current; i++) {
        output += current->data * weight[i];
        current = current->next;
    }
    ref_now = ref_signal;
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
    for (int i = 0; i < MAX_INPUT_SIZE; i++) {
        inp_node_pool[i].prev = NULL;
        inp_node_pool[i].next = NULL;
        inp_node_pool[i].data = 0;
    }

    static_deque_init(&lms_deque);
    static_deque_init(&inp_deque);
    static_deque_init(&sep_deque);
    static_deque_init(&err_deque);

    for (int i = 0; i < LMS_M; i++) {
        weight[i] = 0;
    }

    // 次级通道初始参数
    weight_sep[0] = 1.6422;
    weight_sep[1] = 0.4851;
    weight_sep[2] = 0.4651;
    weight_sep[3] = 0.1436;
    weight_sep[4] = -0.4230;
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
    inp_node_index = 0;
    for (int i = 0; i < MAX_INPUT_SIZE; i++) {
        inp_node_pool[i].prev = NULL;
        inp_node_pool[i].next = NULL;
        inp_node_pool[i].data = 0;
    }

    static_deque_init(&lms_deque);
    static_deque_init(&inp_deque);
    static_deque_init(&sep_deque);
    static_deque_init(&err_deque);

    for (int i = 0; i < LMS_M; i++) {
        weight[i] = 0;
    }
}
