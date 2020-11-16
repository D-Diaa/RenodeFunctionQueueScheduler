#include "task_queue.h"
#include "renode_helpers.h"

static const uint16_t MAX_SIZE = 16;
static uint8_t msg[] = "IDLE\n";
static uint8_t delay_msg[] = "Manual Delay\n";

static struct task_queue main_queue;
static struct task_queue delayed_queue;

static volatile uint16_t skipped_cycles = 0;
static volatile char timerFlag = 0;
static volatile uint16_t clk = 0;

void SysTick_Handler(void)
{
	timerFlag = 1;
	skipped_cycles++;
	clk++;
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
	// decrement delays
	decrement_all(&delayed_queue, skipped_cycles);
	// pop those ones that are ready from the delayed to the main
	push_all_ready(&delayed_queue, &main_queue);
	// New dispacthed --> no skipped cycles for the rest
	skipped_cycles = 0;
	// dispatch tasks if available, else display ("idle")
	if(main_queue.cur_sz>0) dequeue(&main_queue).f();
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
static inline void init()
{
	// initialize emulator hardware registers
	hardware_init();
	// initialize queues
	q_init(&main_queue,MAX_SIZE);
	q_init(&delayed_queue, MAX_SIZE);	
}

// Example tasks

static void f1()
{
	uint16_t startClk = clk;
	uint8_t tst[] = "ONE\n";
	sendUART(tst, sizeof(tst));
	while (clk - startClk != 4) {};
	rerun(&f1, 4, 5);
}
static void f2()
{
	uint16_t startClk = clk;
	uint8_t tst[] = "TWO\n";
	sendUART(tst, sizeof(tst));
	while (clk - startClk != 2) {};
	rerun(&f2, 3, 5);
}
static void f3()
{
	uint16_t startClk = clk;
	uint8_t tst[] = "THREE\n";
	sendUART(tst, sizeof(tst));
	while (clk - startClk != 0) {};
	rerun(&f3, 2, 3);
}
static void f4()
{
	uint8_t tst[] = "FOUR\n";
	sendUART(tst, sizeof(tst));
}



// Main program
int main()
{	
	init();
	// TEST DELAYED QUEUE ONLY
	queue_task(&f1, 4);
	queue_task(&f2, 3);
	queue_task(&f3, 2);
	queue_task(&f4, 1);

	while(1) {
		if(timerFlag) {
			// dispatch only if manual delay is not invoked
			if(!stopFlag) dispatch();
			// feedback on manual delay
			else sendUART(delay_msg, sizeof(delay_msg)); 		
			timerFlag = 0;  
		}
	}
}