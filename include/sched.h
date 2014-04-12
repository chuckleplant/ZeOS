/*
 * sched.h - Estructures i macros pel tractament de processos
 */

#ifndef __SCHED_H__
#define __SCHED_H__

#include <list.h>
#include <types.h>
#include <mm_address.h>
#include <stats.h>
#include <utils.h>

#define NR_TASKS      10
#define KERNEL_STACK_SIZE	1024
#define QUANTUM 200

enum state_t { ST_RUN, ST_READY, ST_BLOCKED };


struct task_struct {
  int PID;
  unsigned long * kernel_esp;
  int quantum;
  struct stats stats;
  page_table_entry * dir_pages_baseAddr;
  struct list_head list;  
};

union task_union {
  struct task_struct task;
  unsigned long stack[KERNEL_STACK_SIZE];    /* pila de sistema, per procés */
};

extern union task_union task[NR_TASKS]; /* Vector de tasques */
extern struct task_struct * idle_task;
extern struct task_struct * m_task1;
struct task_struct * last_forked_child; //debooguing poorposes
extern struct list_head freequeue;
extern struct list_head readyqueue;
int _PID; // global pid counter
int _process_ticks; // ticks since process got assigned the cpu

#define KERNEL_ESP(t)       	(DWord) &(t)->stack[KERNEL_STACK_SIZE]

#define INITIAL_ESP       	KERNEL_ESP(&task[1])

/* Inicialitza les dades del proces inicial */
void init_task1(void);

void init_idle(void);

void init_sched(void);

struct task_struct * current();

void task_switch(union task_union*t);
void inner_task_switch(union task_union * t);

struct task_struct *list_head_to_task_struct(struct list_head *l);

int allocate_DIR(struct task_struct *t);

page_table_entry * get_PT (struct task_struct *t) ;

page_table_entry * get_DIR (struct task_struct *t) ;

/* Headers for the scheduling policy */
void sched_next_rr();
void update_current_state_rr(struct list_head *dest);
int needs_sched_rr();
void update_sched_data_rr();
int get_quantum (struct task_struct *t);
void set_quantum (struct task_struct *t, int new_quantum);
union task_union * getTask(int pid);
int isInList(struct task_struct * task, struct list_head * list);

void update_user_to_system();
void update_system_to_user(struct task_struct * c);
void update_system_to_ready(struct task_struct * c);
void update_ready_to_system(struct task_struct * c);


#endif  /* __SCHED_H__ */
