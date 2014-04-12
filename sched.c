/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>
#include <libc.h>

union task_union task[NR_TASKS]
  __attribute__((__section__(".data.task")));

struct task_struct *idle_task;
struct task_struct *m_task1;

struct list_head freequeue;
struct list_head readyqueue;

#if 1
struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}
#endif 

extern struct list_head blocked;


/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) 
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) 
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}


int allocate_DIR(struct task_struct *t) 
{
	int pos;

	pos = ((int)t-(int)task)/sizeof(union task_union);

	t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[pos]; 

	return 1;
}

void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");

	while(1){
	/*   printc('i');
	   printc('d');
	   printc('l');
	   printc('e');
	   printc(' ');*/
	}
}


void init_idle (void)
{
  struct list_head * idle_head = list_first( &freequeue );
  idle_task = list_head_to_task_struct(idle_head);
  idle_task->PID = 0;
  idle_task->quantum = QUANTUM;
  idle_task->stats.elapsed_total_ticks = get_ticks();
  idle_task->stats.user_ticks = 0;
  idle_task->stats.system_ticks = 0;
  idle_task->stats.blocked_ticks = 0;
  idle_task->stats.ready_ticks = 0;
  idle_task->stats.total_trans = 0; 
  idle_task->stats.remaining_ticks = 0;
  allocate_DIR( idle_task );
  list_del(idle_head);
  union task_union * this_union = (union task_union *) idle_task;
  this_union->stack[KERNEL_STACK_SIZE-1] = (int)cpu_idle;
  this_union->stack[KERNEL_STACK_SIZE-2] = 0;
  idle_task->kernel_esp = & (this_union->stack[KERNEL_STACK_SIZE-2]);
}

void init_task1(void)
{
/*
1) Assign PID 1 to the process
2) Initialize ﬁeld dir_pages_baseAaddr with a new directory to store the process address space
   using the allocate_DIR routine.
3) Complete the initialization of its address space, by using the function set_user_pages (see ﬁle
   mm.c). This function allocates physical pages to hold the user address space (both code and
   data pages) and adds to the page table the logical-to-physical translation for these pages.
   Remember that the region that supports the kernel address space is already conﬁgure for all
   the possible processes by the function init_mm.
4) Update the TSS to make it point to the new_task system stack.
5) Set its page directory as the current page directory in the system, by using the set_cr3
   function (see ﬁle mm.c).
*/

  struct list_head * task1_head = list_first( &freequeue );
  list_del( task1_head );

  m_task1 = list_head_to_task_struct(task1_head);
  m_task1->PID = 1;
  m_task1->quantum = QUANTUM;
  m_task1->stats.elapsed_total_ticks = get_ticks();
  m_task1->stats.user_ticks = 0;
  m_task1->stats.system_ticks = 0;
  m_task1->stats.blocked_ticks = 0;
  m_task1->stats.ready_ticks = 0;
  m_task1->stats.total_trans = 0; 
  m_task1->stats.remaining_ticks = 0;
  _process_ticks = m_task1->quantum;
  allocate_DIR( m_task1 );
  set_user_pages( m_task1 ); 
  
  //Update TSS
  union task_union * task1_union = (union task_union *) m_task1; 
  tss.esp0 = (int) &task1_union->stack[KERNEL_STACK_SIZE];
  set_cr3(get_DIR(m_task1));
}


void init_sched(){
  INIT_LIST_HEAD( &freequeue );
  int i;
  for (i = 0 ; i < NR_TASKS; ++i)
  {
    list_add_tail( &(task[i].task.list), &freequeue);  
  }

  INIT_LIST_HEAD( &readyqueue );
  _PID = 2; //next task pid
}

struct task_struct* current()
{
  int ret_value;
  
  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}


union task_union * getTask(int pid)
{
  int i;
  for ( i = 0; i < NR_TASKS; ++i){
    if(task[i].task.PID == pid)
      return &task[i];
  }  
  return 0;
}

void task_switch(union task_union*t)
{
  /*
1) saves the registers ESI, EDI and EBX , 
2) calls the inner_task_switch routine, and
3) restores the previously saved registers.
*/
  __asm__ __volatile__(
    "pushl %%esi\n"
    "pushl %%edi\n"
    "pushl %%ebx"
    :
  );
  inner_task_switch(t);
    __asm__ __volatile__(
    "popl %%ebx\n"
    "popl %%edi\n"
    "popl %%esi\n"
    :
  );
}

void inner_task_switch(union task_union * t)
{
/*
1) Update the TSS to make it point to the new_task system stack.
2) Change the user address space by updating the current page directory: use the set_cr3
   funtion to set the cr3 register to point to the page directory of the new_task.
3) Store the current value of the EBP register in the current PCB. EBP has the address of the current
   system stack where the inner_task_switch routine begins (the dynamic link).
4) Change the current system stack by setting ESP register to point to the stored value in the
   new PCB.
5) Restore the EBP register from the stack.
6) Return to the routine that called this one using the instruction RET (usually task_switch,
   but. . . ).
*/

  tss.esp0 = (int) &t->stack[KERNEL_STACK_SIZE]; /// se usa la proxima vez que se entre en kernel

  struct task_struct * t_struct = (struct task_struct *) t;
  set_cr3(get_DIR(t_struct));

  struct task_struct * current_struct = current();
  unsigned long * ebp;
  __asm__ __volatile__(
    "movl %%ebp, %0\n"
    : "=g" (ebp)
  );

  current_struct->kernel_esp = ebp;
  unsigned long * new_esp = t_struct->kernel_esp;
  __asm__ __volatile__(
    "movl %0, %%esp\n" 
    "popl %%ebp\n"
    "ret"
    : 
    : "g" (new_esp) 
  );
}


int isInList(struct task_struct * task, struct list_head * list)
{
  struct list_head * iterator;
  list_for_each(iterator, list){
    if(&task->list == iterator)
      return 1;
  }
  return 0;
}


void update_sched_data_rr()
{
	_process_ticks--;
	if(needs_sched_rr()){
		sched_next_rr();
	}
}


int needs_sched_rr(){
	if( _process_ticks == 0){
		if( current()->PID != 0 && list_empty(&readyqueue)){
			_process_ticks = current()->quantum;
			return 0;
		} else {
			if( current()->PID == 0){
				if(list_empty(&readyqueue)){
					_process_ticks = idle_task->quantum;
					return 0;
				}
			}
			return 1;
		}
	}
	return 0;
}


//dst_queue: queue according to the new state of the process
void update_current_state_rr (struct list_head * dst_queue)
{
  if(dst_queue == &readyqueue)
    update_system_to_ready(current());
	list_add_tail(&current()->list, dst_queue);
}




void sched_next_rr (void)
{
	struct task_struct * next_process;
	
	// if no process is ready go to idle_task
	if( list_empty(&readyqueue))
		next_process = idle_task;
	else
	{
		if(current()->PID != 0){ //not idle : update the readyqueue by inserting the current process at the end of the readyqueue;
			struct list_head * free_elem;
			int is_free = 0;
			list_for_each(free_elem, &freequeue){
				if(list_head_to_task_struct( free_elem ) == current() ){
					is_free = 1;
				} 
			}
			if(!is_free)
			  update_current_state_rr(&readyqueue);
			
		}
	
		// extract the first process of the ready queue
    
		next_process = list_head_to_task_struct(list_first(&readyqueue));
		list_del(&next_process->list);
		update_ready_to_system(next_process);
	}
	
	
		
	// perform a context switch to this selected process. Remember that when a process returns to the execution stage after
	// a context switch, its quantum ticks must be restored.
	
  _process_ticks = next_process->quantum;

	update_system_to_user(next_process);
	task_switch((union task_union *)next_process);
}


int get_quantum (struct task_struct *t)
{
	return t->quantum;
}

void set_quantum (struct task_struct *t, int new_quantum)
{
	t->quantum = new_quantum;
}


