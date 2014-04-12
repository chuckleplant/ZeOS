/*
 * libc.c 
 */

#include <libc.h>

#include <types.h>

int errno;

void itoa(int a, char *b)
{
  int i, i1;
  char c;
  
  if (a==0) { b[0]='0'; b[1]=0; return ;}
  
  i=0;
  while (a>0)
  {
    b[i]=(a%10)+'0';
    a=a/10;
    i++;
  }
  
  for (i1=0; i1<i/2; i1++)
  {
    c=b[i1];
    b[i1]=b[i-i1-1];
    b[i-i1-1]=c;
  }
  b[i]=0;
}

int strlen(char *a)
{
  int i;
  
  i=0;
  
  while (a[i]!=0) i++;
  
  return i;
}

int write(int fd, char *buffer, int size)
{
/*
The parameters of the stack must be
copied to the registers ebx, ecx, edx, esi, edi. The correct order is the ﬁrst parameter (on the
left) in ebx, the second parameter in ecx, etc.

"a" (ax/eax), "b" (bx/ebx), "c" (cx/ecx), "d"
(dx/edx), "D" (di/edi), "S" (si/esi), etc.

*/
	int value = 0;
	  __asm__ __volatile__ (
	    "movl $4, %%eax\n\t"
	    "int $0x80\n\t"
	    "movl %%eax, %0\n\t"
	    : "=a" (value)
	    : "b" (fd), "c" (buffer), "d" (size));

	if(value < 0){
		errno = -value;
		return -1;
	}
	else return value;
}

void perror()
{
	switch(errno){
		case 0: write(1, "No error", 8); break;
		case ENOSYS: write(1, "Function not implemented", 24); break;
		case EINVAL: write(1, "Invalid argument", 16); break;
		case EBADFD: write(1, "File descriptor in bad state", 28); break;
		default: write(1, "Unhandled error", 15); break;
	}
}
	
int gettime()
{
	int value = 0;
	  __asm__ __volatile__ (
	    "movl $10, %%eax\n\t"
	    "int $0x80\n\t"
	    "movl %%eax, %0\n\t"
	    : "=a" (value)
	    : );

	if(value < 0){
		errno = -value;
		return -1;
	}
	else return value;
}

int getpid()
{
	/*  It must return the pid of the process that called it. The identiﬁcation of the getpid call is 20.
  Since this is a very simple system call, its implementation is straightforward.
  The wrapper routine in libc.c moves to eax the identiﬁer of the getpid syscall (20), traps the
kernel and returns the correspoding PID also in the eax register. The service routine returns the
PID in the task_struct of the current process.
*/
	int value = -1;
	  __asm__ __volatile__ (
	    "movl $20, %%eax\n\t"
	    "int $0x80\n\t"
	    "movl %%eax, %0\n\t"
	    : "=g" (value)
	    : 
	    : "ax"
	);
	if(value < 0){
		errno = -value;
		return -1;
	}
	else return value;
}

int fork()
{
    int value = -1;

    __asm__ __volatile__ (
        "movl $2, %%eax\n"
        "int $0x80\n"
        "movl %%eax, %0\n"
        : "=g" (value)
        :
        : "ax"
    );
    
  if(value < 0){
		errno = -value;
		return -1;
	}
	else return value;
}


void exit()
{
	__asm__ __volatile__ (
        "movl $1, %%eax\n"
        "int $0x80\n"
        :
        :
        : "ax"
    );
}

int get_stats(int pid, struct stats * st)
{
    int value = -1;

    __asm__ __volatile__ (
        "movl $35, %%eax\n"
        "movl  %1, %%ebx\n"
        "movl %2, %%ecx\n"
        "int $0x80\n"
        "movl %%eax, %0\n"
        : "=g" (value)
        : "g" (pid), "g" (st)
        : "ax", "bx", "cx"
    );
    
    if(value < 0){
		  errno = -value;
		  return -1;
	  }
	  else return value;
}

