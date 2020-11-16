# RenodeFunctionQueueScheduler
This scheduler uses both task_queue.h and renode_helpers.h. 

## Main functions

``` SysTick_Handler ```

Gets called everytime the SysTick handler throws an interrupt. It also increments the skipped_cycles variable. This variable is used to keep track of the cycles spent in a certain task function (that is essentially doing its job in time units more than a single systick and hence skipping cycles where the rest of the main program isn't doing its regular dispatching). The reason why we keep track of this is to later use it over the delay queue to decrement the tasks' delays by the amount of the skipped cycles since practically this is the amount of time they've been waiting for during the execution of a running task with a delay. Nevertheless, their delays also get decremented by 1 every time unit.

```queue_task```

This function enqeues a task using its function pointer into the main queue. It essentially calls the ```enqueue``` function which handles the enqueuing of tasks; more on it in the task_queue.h section.

```dispatch```

This function gets called in our while(1) loop every time the SysTick throws an interrupt every 1 time unit. It first decrements all delays of the tasks in the delay queue and uses the skipped cycles value as well if it is non-zero. It then checks if there are any tasks in the delay queue that are now ready to be moved to the main queue. It does so by calling the ```push_all_ready``` function which essentially takes both pointers of delay and main queues to handle the pushing of tasks from one queue to another. It then resets the skipped cycles value to 0 to not over-count the skipped cycles by other tasks. It then checks if the main queue is empty. If so, then it prints "IDLE" through UART2. Else, if the main queue is not empty, it dequeues the first task in the main queue (the task with the highest priority) and executes it through its function pointer because this task is a ready task by definition whose time to be scheduled has come in this dispatch call.

```rerun```

A function that is used by tasks to rerun themselves periodically every specified delay time units. It takes the task function pointer, its priority and its delay. If the delay is 0, then this task is by definition ready and can be enqueued in the main queue. Otherwise, if the delay is non-zero, then this task is by definition is to be enqueued in the delay queue using its priority and delay values. If the delay option is set to 0 by the task within its function then this means that this task is always ready and will always remain in the main queue because it inserts itself everytime its executed in it through the rerun function.

```init```

This function intiates everything. It first intiates the hardware setup/settings to be used in the application, which essentially calls the ```SystemInit()```, ```SystemCoreClockUpdate```, ```gpioInit```, ```uartInit```, ```SysTick_Config``` which enables the SysTick timer to interrupt the system every time unit and the rest of the interrupt controller commands. The ```init``` function then instantiates both ready and delay queues using ```q_init``` function which takes their max_size and their reference to allocate memory of max size to their tasks arrays.

## task_queue.h

### This file implements the basic functionality of a priority queue (based on binary heaps) with O(log(N)) enqueuing and dequeuing

1. Has some useful typedef to be used by the queue functions.
* lf(p): gets the left child node.
* rt(p): get the right child node.
* p(c): gets the parent node of c.
* min(a,b): returns the minimum of a and b.
* fptr: defined as a void function pointer.
    
2. task struct has the following data fields:
* f: function pointer of type fptr corresponding to the task function.
* prio: corresponds to the priority of the task.
* delay: the delay of the task put in the delay queue.
        
3. task_queue struct has:
* cur_sz: how many elements are in the queue.
* max_sz: maximum size.
* tasks: a task struct pointer pointing to the array of tasks the task_queue has.
        
4. Functions that handle _task_queue:

``` struct task_queue q_init(uint16_t size) ```
Takes a max_size value, instantiates a task_queue object, sets its max size to the function's received argument, sets its current size to 0, allocates memory to the the tasks using    malloc.

``` void enqueue(struct task_queue* q, fptr f, uint16_t prio, uint16_t delay) ```

This function is responsible for creating a new task to be queued in the task queue by calling the _enqeue_ function. It takes:
  1. tast_queue pointer
  2. function pointer of the task to be enqueued.
  3. task priorioty
  4. task delay
  
``` void _enqueue(struct task_queue* q, struct task new_task) ```

This function handles the enqueueing into our _task_queue_. It takes a pointer to the queue and the newly created task as arguements. Then it firstly checks if the queue did not exceed its max_size. If yes, prompts the user with Queue is full" message. If not, inserts the task at the end of queue. Then it hierarichally compares the parents tree by swaping elements until the child is placed correctly compared to its parent priority value.

``` void min_heap(struct task_queue* q, int i)```

Applies the max heap algorithm to restrcture the queue after dequeueing

``` void swap(struct task *task1, struct task *task2)``` 

Swap two tasks by reference.

``` struct task dequeue(struct task_queue* q)```

Takes a queue pointer to dequeue the first task in it, and runs min_heap to restructure the queue and returns the task.

``` void decrement_all(struct task_queue* q, uint16_t cnt)```

Decrements all delays of tasks in a certain queue; used over the delayed queue.

``` void push_all_ready(struct task_queue* delayed_q, struct task_queue* main_q)```

Pushes all ready tasks from the delayed queue to the ready queue on condition if the task delay is 0 (which means it's ready to run).
int compare(struct task a, struct task b);

# Unit tests
## Ready Queue Test
### Task Description

This demo consists of 5 tasks. They do not use _rerun_ since it is only meant to test the Ready Queue. Tasks are defined as the following:

| task 	| delay 	| priority 	| Functionality  	|
|------	|-------	|----------	|----------------	|
| 1    	| N/A      	| 1        	| Prints "ONE"   	|
| 2    	| N/A      	| 2        	| Prints "TWO"   	|
| 3    	| N/A      	| 3        	| Prints "THREE" 	|
| 4    	| N/A   	| 4        	| Prints "FOUR"  	|
| 5    	| N/A   	| 5        	| Prints "FIVE"  	|

### Expected Output

![alt text](https://github.com/D-Diaa/RenodeFunctionQueueScheduler/blob/main/unittests/images/ready_queue_truth.png?raw=true)

### Output

![alt text](https://github.com/D-Diaa/RenodeFunctionQueueScheduler/blob/main/unittests/images/ready_queue_test.png?raw=true)

## Delayed Queue Test

### Task Description

This demo consists of 4 tasks. Tasks are defined as the following:

| task 	| delay 	| priority 	| Functionality  	|
|------	|-------	|----------	|----------------	|
| 1    	| 5      	| 1        	| Prints "ONE"   	|
| 2    	| 8      	| 2        	| Prints "TWO"   	|
| 3    	| 12      	| 3        	| Prints "THREE" 	|
| 4    	| 10   	    | 4        	| Prints "FOUR"  	|


### Expected Output

![alt text](https://github.com/D-Diaa/RenodeFunctionQueueScheduler/blob/main/unittests/images/delay_queue_truth.png?raw=true)

### Output

![alt text](https://github.com/D-Diaa/RenodeFunctionQueueScheduler/blob/main/unittests/images/delay_queue_test.png?raw=true)

## Full Functionality Test

| task 	| delay 	| priority 	| Functionality  	                |
|------	|-------	|----------	|---------------------------------- |
| 1    	| 5      	| 4        	| Delays for 5 ticks  	            |
| 2    	| 5      	| 3        	| Delays for 3 ticks                |
| 3    	| 3      	| 2        	| Delays for 1 tick                 |
| 4    	| N/A      	| 1         | Delays for 1 tick                 |

### Expected Output

![alt text](https://github.com/D-Diaa/RenodeFunctionQueueScheduler/blob/main/unittests/images/multi-delay-truth.png?raw=true)

### Output

![alt text](https://github.com/D-Diaa/RenodeFunctionQueueScheduler/blob/main/unittests/images/unittest-multi-delay.png?raw=true)


## Applications

We developed two applications each of which has its source code in its corresponding branch in branchs ```calcApp``` and ```timerApp```. 

### App 1: calcApp

Our calcApp has 3 different tasks collaborating together to make a calculator at the end that evaluates an expression received through UART2 and sends the result back through UART2.

The tasks we have have the following description:

| task 	| delay 	| priority 	| Functionality  	                |
|------	|-------	|----------	|---------------------------------- |
| 1    	| 2      	| 1        	| Receives Expression from UART2  	|
| 2    	| 3      	| 2        	| Evaluates Expression              |
| 3    	| 2      	| 3        	| Sends Result to UART2             |

### Diagram Explaining calcApp

![alt text](https://github.com/D-Diaa/RenodeFunctionQueueScheduler/blob/main/unittests/images/calcAppDiagram.png?raw=true)

### Output

![alt text](https://github.com/D-Diaa/RenodeFunctionQueueScheduler/blob/main/unittests/images/calcApp.png?raw=true)

### App 2: timerApp

This app has 3 different tasks: the seconds counter, the minutes counter and the tasks\ which sends both values to UART2 in a nice format. It essentially implements a timer whose seconds and minutes are coming from two different tasks that increment the values independently. 

| task 	| delay 	| priority 	| Functionality  	                |
|------	|-------	|----------	|---------------------------------- |
| 1    	| 3      	| 3        	| Seconds Incrementer             	|
| 2    	| 3      	| 2        	| Mins Incrementer                  |
| 3    	| 1      	| 3        	| Sends Result to UART2             |

### Diagram Explaining timerApp

![alt text](https://github.com/D-Diaa/RenodeFunctionQueueScheduler/blob/main/unittests/images/timerAppDiagram.png?raw=true)

### Output

![alt text](https://github.com/D-Diaa/RenodeFunctionQueueScheduler/blob/main/unittests/images/timerApp.jpeg?raw=true)
![alt text](https://github.com/D-Diaa/RenodeFunctionQueueScheduler/blob/main/unittests/images/timerApp2.jpeg?raw=true)

