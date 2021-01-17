#ifndef __SHEPHERD_CONFIG_H_
#define __SHEPHERD_CONFIG_H_

#define ADC_SAMPLES_PER_BUFFER  (10000U)
#define BUFFER_PERIOD_NS    	(100000000U)

/* The IEP is clocked with 200 MHz -> 5 nanoseconds per tick */
#define TIMER_TICK_NS       	(5U)
#define TIMER_BASE_PERIOD   	(BUFFER_PERIOD_NS / TIMER_TICK_NS)
#define SAMPLE_INTERVAL_NS  	(BUFFER_PERIOD_NS / ADC_SAMPLES_PER_BUFFER)

#endif /* __SHEPHERD_CONFIG_H_ */
