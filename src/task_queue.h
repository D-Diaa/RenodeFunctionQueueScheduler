#include  <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Useful defines and typedefs
#define lf(p) (2*p+1)
#define rt(p) (2*p+2)
#define p(c) ((c-1)/2)
typedef void (*fptr)(void);

// Task object
struct task{
		fptr f;
		int prio;
		int delay;
};

// Task queue
struct task_queue{
    int cur_sz;
		int max_sz;
    struct task * tasks;
};

// Function declarations
struct task_queue q_init(uint16_t size);
void enqueue(struct task_queue* q, fptr f, int prio, int delay);
void _enqueue(struct task_queue* q, struct task new_task);
void max_heap(struct task_queue* q, int i);
void swap(struct task *task1, struct task *task2);
struct task dequeue(struct task_queue* q);
void decrement_all(struct task_queue* q, int cnt);
void push_all_ready(struct task_queue* delayed_q, struct task_queue* main_q);
int compare(struct task a, struct task b);

// Helper: less than operator for tasks (treated as pair<delay, prio>)
int compare(struct task a, struct task b)
{
	if(a.delay<b.delay) return 1;
	return (a.delay==b.delay) && (a.prio<b.prio);
}

// Helper: Swap task references
void swap(struct task *task1, struct task *task2) {
		struct task temp = *task1;
		*task1 = *task2;
		*task2 = temp;
}

// Task queue constructor
struct task_queue q_init(uint16_t size)
{
		struct task_queue q;
		q.cur_sz = 0;
		q.max_sz = size;
		q.tasks = malloc(size*sizeof(struct task));
		return q;
}

// Enqueue function
void enqueue(struct task_queue* q, fptr f, int prio, int delay)
{
		// Create new task and call the private enqueue function
		struct task new_task = {f, prio, delay};
		_enqueue(q, new_task);
}

// Enqueue function
void _enqueue(struct task_queue* q, struct task new_task)
{
		// Check for queue limits
		if (q->cur_sz == q->max_sz -1)
		{
				printf("ERROR! - Queue is full");
				return;
		}
		// put at end of the queue
		q->tasks[q->cur_sz++]=new_task;
		
		// Go up the tree till it can't overthrow its parent
		int i = q->cur_sz-1;
		while (i != 0 && compare(q->tasks[i], q->tasks[p(i)]))
		{
				swap( & q->tasks[p(i)], & q->tasks[i]);
				i = p(i);
		}
}

// Max heap algorithm for dequeuing
void max_heap(struct task_queue* q, int i) {
		int small = i;
		int left = lf(i), right = rt(i);
		// get the minimum (left or right or this node)
		if(left<=q->cur_sz && compare(q->tasks[left],q->tasks[small])) small = left;
		if(right<=q->cur_sz && compare(q->tasks[right],q->tasks[small])) small = right;
		// if this node is the maximum then left, right and this subtree are heapified
		if (small==i) return;
		// else put the small on top and heapify the affected tree
		swap(&q->tasks[small], &q->tasks[i]);
		max_heap(q, small);
}

// Dequeue function
struct task dequeue(struct task_queue* q)
{
		struct task ret = {0, -1, 0};
		// Check for queue limits
		if (q->cur_sz ==0)
		{
				printf("ERROR! - Queue is empty");
				return ret;
		}
		// Take task reference and replace its reference with the lasr task in the queue
		ret = q->tasks[0];
		q->tasks[0] = q->tasks[q->cur_sz-1];
		// Decrease queue size
		q->cur_sz--;
		// Put the last task (now at the top of the queue) in its correct place
		max_heap(q, 0);
		// return the desired task
		return ret;
}

// Decrements all the delays in a certain queue (used with the delayed queue)
void decrement_all(struct task_queue* q, int cnt)
{
	uint8_t i=0;
	for(;i<q->cur_sz;i++)q->tasks[i].delay-=cnt;
}

// Pushes all ready tasks from the delayed queue to the main queue
void push_all_ready(struct task_queue* delayed_q, struct task_queue* main_q)
{
	while(delayed_q->tasks[0].delay==0)
		_enqueue(main_q, dequeue(delayed_q));
}
