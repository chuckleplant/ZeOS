#include <libc.h>


char buff[24];

int pid;


long inner(long n)
{
        int i;
        long suma;
        suma = 0;
        for (i=0; i<n; i++) suma = suma + i;
        return suma;
}
long outer(long n)
{
        int i;
        long acum;
        acum = 0;
        for (i=0; i<n; i++) acum = acum + inner(i);
        return acum;
}



int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

/** session 0
	long count, acum;
	count = 75;
	acum = 0;
	acum = outer(count);
 session 0 */

/*session 1
	
	runjp();
	while (1)
	{ 
	}
	return 0;
*/
	
	
	/*
	int pid = fork();
	
	//char c;
	//itoa(_pid, &c);
	//write(1, &c, 1);
	int i = 0;
	if(pid != 0){
		for(; i < 100; i++){
			write(1,"parent",6);
		}
	} else {
	  int pid2 = fork();
	  if(pid2 == 0){
	    for(; i < 100; i++){
	     	write(1,"1",1);
		  }
	  } else {
		  for(; i < 100; i++){
	     	write(1,"2",1);
		  }
		}
	}
	exit();
	*/
	
while(1);

	//runjp_rank(25, 28);
//	runjp();
	
	return 0;
}
