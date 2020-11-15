#include "task_queue.h"
#include "renode_helpers.h"

//#define debug

static const uint16_t MAX_SIZE = 20;
static uint8_t msg[] = "IDLE !!\n";
static struct task_queue main_queue;
static struct task_queue delayed_queue;
static volatile uint16_t skipped_cycles = 0;
static volatile char timerFlag = 0;
static volatile uint8_t stopFlag = 0;
static volatile char uartFlag = 0;

uint8_t s = 0;
uint16_t m = 0;
uint8_t nl = '\n';

//void SysTick_Handler(void);

void SysTick_Handler(void)
{
	timerFlag = 1;
	skipped_cycles++;
}

void USART2_IRQHandler(void)
{
	uartFlag = 1;
	
	#ifdef debug
	uint8_t tst[] = "in UART2 handler!!! \n";
	sendUART(tst, sizeof(tst));
	#endif
}

// Enqueue task in the main queue with a certain priorty
static void queue_task(fptr f, uint16_t prio)
{
	enqueue(&main_queue, f, prio, 0);
}

// Dispatch a task and run it if available, otherwise display 'idle'
static void dispatch()
{
	// handle tasks in delayed queue
	if (delayed_queue.cur_sz > 0) {
		// decrement delays
		decrement_all(&delayed_queue, skipped_cycles);
		// pop those ones that are ready from the delayed to the main
		push_all_ready(&delayed_queue, &main_queue);
	}
	// New dispacthed --> no skipped cycles for the rest
	skipped_cycles = 0;
	// dispatch tasks if available, else display ("idle")
	if(main_queue.cur_sz > 0) dequeue(&main_queue).f();
	else sendUART(msg, sizeof(msg));
}

// Allows rerunning a task (acts mainly as a conditional enqueuer)
static void rerun (fptr f, uint16_t prio, uint16_t delay)
{
	// Enqueue to the main if the task is not delayed, otherwise put in the delayed queue
	if(delay == 0) enqueue(&main_queue, f, prio, delay);
	else enqueue(&delayed_queue, f, prio, delay);
}

// Initialize peripherals and queues
static void init()
{
	hardware_init();
	main_queue = q_init(MAX_SIZE);
	delayed_queue = q_init(MAX_SIZE);	
}

// Example tasks
static void f1() //recieve task
{
	#ifdef debug
	uint8_t tst[] = "ONE!!! \n";
	sendUART(tst, sizeof(tst));
	#endif

	static uint8_t timer;
	timer++;
	s = (timer == 60)? (timer = 0): timer;
	rerun(&f1, 3, 3);
}
static void f2() //evaluate task
{
	#ifdef debug
	uint8_t tst[] = "TWO!!! \n";
	sendUART(tst, sizeof(tst));
	#endif
	
	static uint16_t timer;
	if (s == 59)
		timer++;
	m = (timer == 999)? (timer = 0): timer;
	rerun(&f2, 2, 3);
}
static void f3()
{
	#ifdef debug
	uint8_t tst[] = "THREE!!! \n";
	sendUART(tst, sizeof(tst));
	#endif

	static uint8_t out[] = {0, 0, 0, ':', 0, 0, 0};
	sprintf((char *)out + 4, "%d", s);
	sprintf((char *)out, "%d", m);
	sendUART(out, sizeof(out));
	sendUART(&nl, 1);
	rerun(&f3, 1, 3);
}

// Main program
int main()
{
	init();
	#ifdef debug
	uint8_t tst[] = "Running main \n";
	sendUART(tst, sizeof(tst));
	#endif
	queue_task(&f2, 2);
	queue_task(&f1, 3);
	queue_task(&f3, 1);

	while(1) {
		if(timerFlag && !stopFlag) {
			dispatch();
			timerFlag = 0;
		}
	}
}