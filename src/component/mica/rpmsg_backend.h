/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 * 	http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 * Create: 2023-11-20
 * Description: openamp backend
 */

#include "openamp/open_amp.h"
#include "openamp_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RPMSG_VIRTIO_CONSOLE_CONFIG        \
    (&(const struct rpmsg_virtio_config) { \
        .h2r_buf_size = 512,  \
        .r2h_buf_size = 512,  \
        .split_shpool = false,\
})

extern int rpmsg_service_init(void);

extern void (*g_rpmsg_ipi_handler)(void);

/**
 * rpmsg_backend_init - register rpmsg-virtio-device.
 *
 * Return: pointer of rpmsg_device(rpdev) on success, NULL on failure.
 */
extern struct rpmsg_device *rpmsg_backend_init(void);

/**
 * rpmsg_backend_remove - remove the backend
 */
extern void rpmsg_backend_remove(void);
/**
 * receive_message - Call it to receive messages from host(linux)
 */
extern int send_message(unsigned char *message, int len);
extern void receive_message(void);

/* 下线时，设置rsc_table的reserved[1]为CPU_OFF_FUNCID，告诉mica侧更新状态 */
extern void rsc_table_set_offline_flag(void);

extern int rcv_data_from_nrtos(void *rcv_data, int *data_len);
extern int send_data_to_nrtos(void *send_data, int data_len);

#ifdef __cplusplus
}
#endif
