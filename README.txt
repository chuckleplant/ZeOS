*** This tar file contains the following files:

* libjp2.a: Test suite for the 2nd submission code. It contains the user calls
  'runjp' and 'runjp_rank' to execute the tests plus all the necessary code to
  execute them.  
* README_E2.txt: this file specifies the pre-requisites for each test in libjp2.a.  

*** USAGE:
* You have to create a soft link named libjp.a pointing to the library with the
  tests that you want to run (libjp2.a). Modify the Makefile so the user target
  is linked with the libjp.a file.

* You have to call the 'runjp' routine from the main function of the user.c
  source file. 

* You can execute just a range of tests, replacing  the call to runjp() in the
  main function by a call to runjp_rank(int first, int last), where "first" is
  the identifier of the first test in the range that you want to execute, and
  "last" is the last one.

*** NOTES:
* You should NOT modify the .bochsrc file that we have included in the tar file
  with the zeos base files. For example, if you modify the number of instruction
  per second that the virtual machine executes (ips), then some tests may fail.

* You have to take into account that the tests in a test suite are cumulative.
  That is, if one test fails then the execution of the rest of the tests may be
  inconsistent.

* You have to enable Bochs to write on the console, in order to read all the
  test messages without problems (this is already done in the system image that
  you use in the course laboratories).

* This test may BLOCK your processes, this means that the blocked process will 
  be moved to a special list which is not accesible from your code, and therefore 
  it will not be accessible through the READY list. Remember that the processes 
  are always accessible through the task array.


