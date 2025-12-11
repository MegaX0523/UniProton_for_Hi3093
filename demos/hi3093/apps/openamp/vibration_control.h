#pragma once

#define SENSOR_ARRAY_TYPE_REF 1
#define SENSOR_ARRAY_TYPE_ERR 0

void control_task_entry();
int rec_msg_proc(void *data, int len);

