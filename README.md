# RenodeFunctionQueueScheduler
This scheduler uses both task_queue.h and renode_helpers.h.

# task_queue.h
1. Has some useful typedef to be used by the queue functions.
    lf(p): gets the left child node
    rt(p): get the right child node
    p(c): gets the parent node of c
    min(a,b): returns the minimum of a and b
    fptr: defined as a void function pointer.
2. task struct has the following data fields:
        a. 'f': function pointer of type fptr corresponding to the task function.
        b. 'prio': corresponds to the priority of the task.
        c. 'delay':
3. task_queue struct has:
        a. cur_sz: how many elements are in the queue.
        b. max_sz: maximum size.
        c. tasks: a task struct pointer pointing to the array of tasks the task_queue has.
4. Functions that handle _task_queue:
# # struct task_queue q_init(uint16_t size)
Takes a max_size value, instantiates a task_queue object, sets its max size to the function's received argument, sets its current size to 0, allocates memory to the the tasks using    malloc.

# # void enqueue(struct task_queue* q, fptr f, uint16_t prio, uint16_t delay)
This function is responsible for creating a new task to be queued in the task queue by calling the _enqeue_ function. It takes:
  1. tast_queue pointer
  2. function pointer of the task to be enqueued.
  3. task priorioty
  4. task delay
# # void \_enqueue(struct task_queue* q, struct task new_task)
This function handles the enqueueing into our _task_queue_. It takes a pointer to the queue and the newly created task as arguements. Then it firstly checks if the queue did not exceed its max_size. If yes, prompts the user with Queue is full" message. If not, inserts the task at the end of queue. Then it hierarichally compares the parents tree by swaping elements until the child is placed correctly compared to its parent priority value.

# # void min_heap(struct task_queue* q, int i)
applies the max heap algorithm to restrcture the queue after dequeueing

# # void swap(struct task *task1, struct task *task2)
Swap two tasks by reference

# # struct task dequeue(struct task_queue* q)
Takes a queue pointer to dequeue the first element in it, returns it and runs min_heap to restructure the queue.

# # void decrement_all(struct task_queue* q, uint16_t cnt)
Decrements all delays of tasks in a certain queue; used over the delayed queue.

# void push_all_ready(struct task_queue* delayed_q, struct task_queue* main_q);
pushes all ready tasks from the delayed queue to the ready queue on condition if the task delay is 0 (which means it's ready to run).
int compare(struct task a, struct task b);

