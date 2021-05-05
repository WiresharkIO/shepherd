#include "pru_comm.h"
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/delay.h>
#include <linux/mutex.h>

#include "pru_mem_msg_sys.h"

/***************************************************************/
/***************************************************************/

struct RingBuffer msg_ringbuf_from_pru;
struct RingBuffer msg_ringbuf_to_pru;

// TODO: base msg-system on irqs (there are free ones from rpmsg)
// TODO: replace by official kfifo, https://tuxthink.blogspot.com/2020/03/creating-fifo-in-linux-kernel.html
static void ring_init(struct RingBuffer *const buf)
{
    buf->start = 0u;
    buf->end = 0u;
    buf->active = 0u;
    mutex_init(&buf->mutex);
}

void ring_put(struct RingBuffer *const buf, const struct ProtoMsg *const element)
{
    mutex_lock(&buf->mutex);
    buf->ring[buf->end] = *element;

    // special faster version of buf = (buf + 1) % SIZE
    if(++(buf->end) == RING_SIZE) 	buf->end = 0U;

    if (buf->active < RING_SIZE)	buf->active++;
    else
    {
        if(++(buf->start) == RING_SIZE) buf->start = 0U; // fast modulo
    }
    mutex_unlock(&buf->mutex);
}

uint8_t ring_get(struct RingBuffer *const buf, struct ProtoMsg *const element)
{
    if(buf->active == 0) return 0;
    mutex_lock(&buf->mutex);
    *element = buf->ring[buf->start];
    if(++(buf->start) == RING_SIZE) buf->start = 0U; // fast modulo
    buf->active--;
    mutex_unlock(&buf->mutex);
    return 1;
}

void put_msg_to_pru(const struct ProtoMsg *const element)
{
    ring_put(&msg_ringbuf_to_pru, element);
}

uint8_t get_msg_from_pru(struct ProtoMsg *const element)
{
    return ring_get(&msg_ringbuf_from_pru, element);
}

/***************************************************************/
/***************************************************************/

struct hrtimer coordinator_loop_timer;
static enum hrtimer_restart coordinator_callback(struct hrtimer *timer_for_restart);

/* series of halving sleep cycles, sleep less coming slowly near a total of 100ms of sleep */
const static unsigned int coord_timer_steps_ns[] = {
        500000u,   200000u,   100000u,
        50000u,    20000u,    10000u};
const static size_t coord_timer_steps_ns_size = sizeof(coord_timer_steps_ns) / sizeof(coord_timer_steps_ns[0]);


/***************************************************************/
/***************************************************************/

int mem_msg_sys_exit(void)
{
    hrtimer_cancel(&coordinator_loop_timer);
    return 0;
}

int mem_msg_sys_reset(void)
{
    ring_init(&msg_ringbuf_from_pru);
    ring_init(&msg_ringbuf_to_pru);
    return 0;
}

int mem_msg_sys_test(void)
{
    struct ProtoMsg msg1;
    struct CtrlRepMsg msg2;
    printk(KERN_INFO "shprd.k: testing msg-pipelines between kM and PRUs -> triggering messages for pipeline 1-3");
    msg1.msg_type = MSG_TEST;
    msg1.value = 1;
    put_msg_to_pru(&msg1); // message-pipeline pru0
    msg1.value = 2;
    put_msg_to_pru(&msg1); // error-pipeline pru0

    msg2.msg_type = MSG_TEST;
    msg2.buffer_block_period = 3;
    pru_comm_send_ctrl_reply(&msg2); // error-pipeline pru1
}

int mem_msg_sys_init(void)
{
    struct timespec ts_now;
    uint64_t now_ns_system;

    mem_msg_sys_reset();

    hrtimer_init(&coordinator_loop_timer, CLOCK_REALTIME, HRTIMER_MODE_ABS);
    coordinator_loop_timer.function = &coordinator_callback;

    /* Timestamp system clock */
    getnstimeofday(&ts_now);
    now_ns_system = (uint64_t)timespec_to_ns(&ts_now);

    hrtimer_start(&coordinator_loop_timer,
            ns_to_ktime(now_ns_system + coord_timer_steps_ns[0]),
            HRTIMER_MODE_ABS);

    printk(KERN_INFO "shprd.k: msg-system was initialized");
    mem_msg_sys_test();

    return 0;
}

/***************************************************************/
/***************************************************************/

static enum hrtimer_restart coordinator_callback(struct hrtimer *timer_for_restart)
{
    struct ProtoMsg pru_msg;
    struct timespec ts_now;
    static unsigned int step_pos = 0;
    uint8_t had_work = 0;

    /* Timestamp system clock */
    getnstimeofday(&ts_now);

    do
    {
        if (pru0_comm_receive_msg(&pru_msg)) had_work = 2;
        else if (pru0_comm_receive_error(&pru_msg)) had_work = 4;
        else if (pru1_comm_receive_error(&pru_msg)) had_work = 5;
        else
        {
            had_work = 0;
            continue;
        }

        if (pru_msg.msg_type<0xC0)
        {
            ring_put(&msg_ringbuf_from_pru, &pru_msg);
        }
        else
        {
            switch (pru_msg.msg_type) // TODO: move over to py, just keep ringbuffer-overflow here
            {
            case MSG_STATUS_RESTARTING_ROUTINE:
                printk(KERN_INFO
                "shprd.pru%u: (re)starting main-routine (val=%u)", had_work & 1u, pru_msg.value);
                break;
            case MSG_ERROR:
                printk(KERN_ERR
                "shprd.pru%u: general error (val=%u)", had_work & 1u, pru_msg.value);
                break;
            case MSG_ERR_MEMCORRUPTION:
                printk(KERN_ERR
                "shprd.pru%u: msg.id from kernel is faulty -> mem corruption? (val=%u)", had_work & 1u, pru_msg.value);
                break;
            case MSG_ERR_BACKPRESSURE:
                printk(KERN_ERR
                "shprd.pru%u: msg-buffer to kernel was still full -> backpressure (val=%u)", had_work & 1u, pru_msg.value);
                break;
            case MSG_ERR_INCMPLT:
                printk(KERN_ERR
                "shprd.pru%u: sample-buffer not full (fill=%u)", had_work & 1u, pru_msg.value);
                break;
            case MSG_ERR_INVLDCMD:
                printk(KERN_ERR
                "shprd.pru%u: received invalid command / msg-type (%u)", had_work & 1u, pru_msg.value);
                break;
            case MSG_ERR_NOFREEBUF:
                printk(KERN_ERR
                "shprd.pru%u: ringbuffer is depleted - no free buffer (val=%u)", had_work & 1u, pru_msg.value);
                break;
            case MSG_ERR_TIMESTAMP:
                printk(KERN_ERR
                "shprd.pru%u: received timestamp is faulty (val=%u)", had_work & 1u, pru_msg.value);
                break;
            case MSG_ERR_SYNC_STATE_NOT_IDLE:
                printk(KERN_ERR
                "shprd.pru%u: Sync not idle at host interrupt (val=%u)", had_work & 1u, pru_msg.value);
                break;
            case MSG_TEST:
                printk(KERN_INFO
                "shprd.k: [test] received answer from pru%u / pipeline %u", had_work & 1u, pru_msg.value);
                break;
            default:
                /* these are all handled in userspace and will be passed by sys-fs */
                printk(KERN_ERR
                "shprd.k: received invalid command / msg-type (%u) from pru%u", pru_msg.msg_type, had_work & 1u);
            }
        }

        /* resetting to shortest sleep period */
        step_pos = coord_timer_steps_ns_size - 1;
    }
    while (had_work > 0);

    if (pru0_comm_check_send_status() && ring_get(&msg_ringbuf_to_pru, &pru_msg))
    {
        // TODO: a routine for backpressure-detection would be nice to have
        pru0_comm_send_msg(&pru_msg);
        /* resetting to shortest sleep period */
        step_pos = coord_timer_steps_ns_size - 1;
    }

    hrtimer_forward(timer_for_restart, timespec_to_ktime(ts_now),
            ns_to_ktime(coord_timer_steps_ns[step_pos])); /* variable sleep cycle */

    if (step_pos > 0) step_pos--;

    return HRTIMER_RESTART;
}
