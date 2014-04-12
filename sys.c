/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>
#include <errno.h>
#include <interrupt.h>

#include <limits.h>

#define LECTURA 0
#define ESCRIPTURA 1

int sys_get_stats(int pid, struct stats *st);

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -9; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -13; /*EACCES*/
  return 0;
}

// A
void update_user_to_system()
{
  struct task_struct * c = current();
  c->stats.user_ticks += get_ticks() - c->stats.elapsed_total_ticks;
  c->stats.elapsed_total_ticks = get_ticks();
}

void update_system_to_user(struct task_struct * c)
{
  c->stats.system_ticks += get_ticks() - c->stats.elapsed_total_ticks;
  c->stats.elapsed_total_ticks = get_ticks();
}

void update_system_to_ready(struct task_struct * c)
{
  c->stats.system_ticks += get_ticks() - c->stats.elapsed_total_ticks;
  c->stats.elapsed_total_ticks = get_ticks();
}

void update_ready_to_system(struct task_struct * c)
{
  c->stats.ready_ticks += get_ticks() - c->stats.elapsed_total_ticks;
  c->stats.elapsed_total_ticks = get_ticks();
}


int sys_ni_syscall()
{
  update_user_to_system();
  update_system_to_user(current());
	return -38; /*ENOSYS*/
}

int sys_getpid()
{
  update_user_to_system();
  update_system_to_user(current());
	return current()->PID;
}

int ret_from_fork()
{
  update_system_to_user(current());
  return 0;
}



int sys_fork()
{
  update_user_to_system();
  int PID=-1;
  int i;
  int j;
  // creates the child process
/*
 a) Get a free task_struct for the process. If there is no space for a new process, an error
   will be returned.
b) Inherit system data: copy the parent’s task_union to the child. Determine whether it is necessary to modify the page table of the parent to access the child’s system data. The
   copy_data function can be used to copy.
c) Initialize ﬁeld dir_pages_baseAddr with a new directory to store the process address
   space using the allocate_DIR routine.
d) Search physical pages in which to map logical pages for data+stack of the child process
   (using the alloc_frames function). If there is no enough free pages, an error will be
   return.
*/

//a
  if(list_empty( &freequeue )){
    update_system_to_user(current());
    return -ENOMEM;
  }
  struct list_head * freequeue_head = list_first( &freequeue );
  struct task_struct * child_struct  = list_head_to_task_struct(freequeue_head);
  list_del(freequeue_head); // not in freequeue anymore
  
//b 
  struct task_struct * current_struct = current();
  union task_union * current_union = (union task_union *) current_struct;
  union task_union * child_union = (union task_union *) child_struct;
  copy_data(current_union, child_union, sizeof(union task_union));
  // TODO determine  whether it is necessary to modify the page table of the parent to access the child’s system data

//c 
  allocate_DIR(child_struct);

//d
  int physical_pages[NUM_PAG_DATA];
  for(i = 0; i < NUM_PAG_DATA; ++i)
  {
    physical_pages[i] = alloc_frame();
    if( physical_pages[i] < 0){
      for(j = i-1; j >= 0; j--)
      {
        free_frame((unsigned int)j);
      }
      update_system_to_user(current());
      return -EAGAIN;
    }
  }


/*
e) Inherit user data:
    i) Create new address space: Access page table of the child process through the direc-
       tory ﬁeld in the task_struct to initialize it (get_PT routine can be used):
       A) Page table entries for the system code and data and for the user code can be a
          copy of the page table entries of the parent process (they will be shared)
*/
  page_table_entry * child_pt = get_PT(child_struct);
  page_table_entry * parent_pt = get_PT(current_struct);
  int child_logical_address;
  for(child_logical_address = 0; child_logical_address < NUM_PAG_KERNEL + NUM_PAG_CODE; child_logical_address++)
  {
    int physical_parent_frame = get_frame(parent_pt, child_logical_address);
    set_ss_pag( child_pt, child_logical_address, physical_parent_frame);
  }
/*     B) Page table entries for the user data+stack have to point to new allocated pages
          which hold this region*/

  for(; child_logical_address < NUM_PAG_KERNEL + NUM_PAG_CODE + NUM_PAG_DATA; child_logical_address++)
  {
    set_ss_pag(child_pt, child_logical_address, physical_pages[child_logical_address - (NUM_PAG_KERNEL + NUM_PAG_CODE)]);
  }



/*
   ii) Copy the user data+stack pages from the parent process to the child process. The
       child’s physical pages cannot be directly accessed because they are not mapped in
       the parent’s page table. In addition, they cannot be mapped directly because the
       logical parent process pages are the same. They must therefore be mapped in new
       entries of the page table temporally (only for the copy). Thus, both pages can be
       accessed simultaneously as follows:
       A) Use temporal free entries on the page table of the parent. Use the set_ss_pag and
          del_ss_pag functions.
       B) Copy data+stack pages.
       C) Free temporal entries in the page table and ﬂush the TLB to really disable the
          parent process to access the child pages.
*/

  // direccion logica del systemcode+systemdata+usercode = direccion logica datos user
  



  
  int parent_log = NUM_PAG_KERNEL + NUM_PAG_CODE;  
  for(i = 0; i < NUM_PAG_DATA; ++i){
  	set_ss_pag(parent_pt, parent_log + NUM_PAG_DATA + i, physical_pages[i]);
  	copy_data( (void*) ((parent_log + i) * PAGE_SIZE), (void*) ((parent_log + NUM_PAG_DATA + i) * PAGE_SIZE), PAGE_SIZE );
	del_ss_pag(parent_pt, parent_log + NUM_PAG_DATA + i);
  }
  
  
  
  //set_cr3(parent_pt); // flush tlb fallaba, por que? TODO
  set_cr3(get_DIR(current_struct));

/*
f) Assign a new PID to the process. The PID must be different from its position in the
    task_array table.
g) Initialize the ﬁelds of the task_struct that are not common to the child.
     i) Think about the register or registers that will not be common in the returning of the
        child process and modify its content in the system stack so that each one receive its
        values when the context is restored.
h) Prepare the child stack emulating a call to context_switch and be able to restore its
    context in a known position. The stack of the new process must be forged so it can be
    restored at some point in the future by a task_switch. In fact the new process has to
    restore its hardware context and continue the execution of the user process, so you can
    create a routine ret_from_fork which does exactly this. And use it as the restore point
    like in the idle process initialization 4.4.
 i) Insert the new process into the ready list: readyqueue. This list will contain all processes
    that are ready to execute but there is no processor to run them.
 j) Return the pid of the child process.
*/
  if(_PID == INT_MAX){
    update_system_to_user(current());
    return -1; // please restart
  }
  child_struct->PID = _PID++;
  PID = child_struct->PID;

/* 
* 0		-19
* ret_from_fork	-18
* sys_call_handler // -17
* SAVEALL // -16	
* iret    // -5
*/	
 
  
  child_struct->kernel_esp = (unsigned long *)&child_union->stack[KERNEL_STACK_SIZE-19];
  child_union->stack[KERNEL_STACK_SIZE-19] = 0;
  child_union->stack[KERNEL_STACK_SIZE-18] = (int)ret_from_fork;

  list_add_tail(&child_struct->list, &readyqueue);
  child_struct->stats.elapsed_total_ticks = get_ticks();
  child_struct->stats.user_ticks = 0;
  child_struct->stats.system_ticks = 0;
  child_struct->stats.blocked_ticks = 0;
  child_struct->stats.ready_ticks = 0;
  child_struct->stats.total_trans = 0; 
  child_struct->stats.remaining_ticks = 0;
//  last_forked_child = child_struct = 0;
  update_system_to_user(current());
  return PID; 
}


//session 1 implement sys_write
int sys_write(int fd, char * buffer, int size)
{
    update_user_to_system();
    
    // check parameters
    int _error = 0;
    if(check_fd(fd, ESCRIPTURA) < 0) _error = check_fd(fd, ESCRIPTURA);
    else if(buffer == NULL) _error = -EFAULT;
    else if(size < 0) _error = -EINVAL;
    
    if(_error){
      update_system_to_user(current());
      return _error;
    }
    // copy data from user space if needed   
    
    // implement requested service, device dependant
    sys_write_console(buffer, size);
    update_system_to_user(current());
    return size;
}

int sys_clock()
{
  update_user_to_system();
  update_system_to_user(current());
  return zeos_ticks;
}

void sys_exit()
{  
/*
a) Free the data structures and resources of this process (physical memory, task_struct,
   and so). It uses the free_frame function to free physical pages.
b) Use the scheduler interface to select a new process to be executed and make a context
   switch.*/
  update_user_to_system();   
  free_user_pages(current());  
	update_current_state_rr(&freequeue);
	sched_next_rr();                                 
}


int sys_get_stats(int pid, struct stats *st)
{
  update_user_to_system();
  if(st == 0 || !access_ok(VERIFY_WRITE, st, sizeof(struct stats))){
    update_system_to_user(current());
    return -EFAULT;
  }
  
  if(pid < 0){
    update_system_to_user(current());
    return -EINVAL;
  }
  
  int i;
  for(i = 0; i < NR_TASKS; ++i){
    if(task[i].task.PID == pid){
      copy_to_user(&task[i].task.stats, st, sizeof(struct stats));
      update_system_to_user(current());
      return 0;
    }
  }
  
  update_system_to_user(current());
  return -ESRCH;
  
}
