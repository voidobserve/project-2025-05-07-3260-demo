#include "ir_handle.h"

volatile u8 ir_data = 0;
volatile bit flag_is_recv_ir_repeat_code = 0;
volatile bit flag_is_recved_data = 0;