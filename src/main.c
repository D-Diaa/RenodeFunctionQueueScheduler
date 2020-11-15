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

uint8_t rq[] = {0, 0, 0, 0};
uint8_t sq[] = {0, 0, 0 ,0};
uint8_t c = '0';
uint8_t nl = '\n';

//void SysTick_Handler(void);

void SysTick_Handler(void)
{
	timerFlag = 1;
	skipped_cycles++;
}

void USART2_IRQHandler(void)
{
	c = receiveUART();
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

	static uint8_t msg[] = "Queue Full...Slow down!!\n";
	static uint8_t qSize;
	if (uartFlag) {
		if (rq[3] == 0xFF) {
			sendUART(msg, sizeof(msg));
		} else {
			rq[qSize++] = c;
			sendUART(&c, 1);
		}
		if (qSize == 3) {
			qSize = 0;
			rq[3] = 0xFF;
		}
		uartFlag = 0;
	}
	rerun(&f1, 1, 2);
}
static void f2() //evaluate task
{
	#ifdef debug
	uint8_t tst[] = "TWO!!! \n";
	sendUART(tst, sizeof(tst));
	#endif
	
	int temp;
	if (rq[3] == 0xFF) {
		uint8_t q1 = rq[0];
		uint8_t q2 = rq[2];
		uint8_t op = rq[1];
		if (op == '+') //TODO: if needed support multi-digit results
			temp = (q1 - '0') + (q2 - '0');
		else
			temp = q1 - q2;
		sprintf((char *)sq, "%d", temp);
		sq[3] = 0xFF;
		rq[3] = 0;
	}
	rerun(&f2, 2, 3);
}
static void f3()
{
	#ifdef debug
	uint8_t tst[] = "THREE!!! \n";
	sendUART(tst, sizeof(tst));
	#endif

	if (sq[3] == 0xFF) {
		sendUART(&nl, 1);
		sq[3] = 0;
		sendUART(sq, sizeof(sq));
		sendUART(&nl, 1);
	}
	rerun(&f3, 3, 2);
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
	queue_task(&f1, 1);
	queue_task(&f3, 3);

	while(1) {
		if(timerFlag && !stopFlag) {
			dispatch();
			timerFlag = 0;
		}
	}
}