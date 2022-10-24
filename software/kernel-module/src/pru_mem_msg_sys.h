#ifndef SRC_MEM_MSG_PRU_H
#define SRC_MEM_MSG_PRU_H

#include "commons.h"
#include <linux/mutex.h>

struct RingBuffer
{
    struct ProtoMsg ring[MSG_FIFO_SIZE];
    uint32_t        start; // TODO: these can be smaller (like u8), at least in documentation
    uint32_t        end;
    uint32_t        active;
    struct mutex    mutex;
};

void    put_msg_to_pru(const struct ProtoMsg *const element);
uint8_t get_msg_from_pru(struct ProtoMsg *const element);

void    mem_msg_sys_exit(void);
void    mem_msg_sys_reset(void);
void    mem_msg_sys_init(void);
void    mem_msg_sys_test(void);

void    mem_msg_sys_pause(void);
void    mem_msg_sys_start(void);

#endif //SRC_MEM_MSG_PRU_H
