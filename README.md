# Rate monotonic scheduler

A four-thread RMS implemented in C++.

## File listing
* worker.cpp - defines doWork() and sem.wait()s
* scheduler.cpp - contains bulk of code, as well as sem.signal()s
* a.out - executable, compiled on CentOS
* output.txt - program output
* design.md - brief design description of the project


## Compile with
```
g++ *.cpp -std=c++11 -pthread
```

### Project Description
Build a Rate Monotonic Scheduler with four threads

* Scheduler details:
    * Threads T0 through T3
    * Thread T0 has a period of 1 unit
    * Thread T1 has a period of 2 units
    * Thread T2 has a period of 4 units
    * Thread T3 has a period of 16 units
    * A unit shall be anywhere from 10-100 ms (depending on operating
system options)
* Each thread will execute the same doWork method but run it a
different amount of times:
    * Thread T0 executes doWork 1 time
    * Thread T1 executes doWork 2 times
    * Thread T2 executes doWork 4 times
    * Thread T3 executes doWork 16 times

* Scheduler shall have a major frame period of 16 units of
time
* Program shall start scheduler and four threads that are to
be scheduled
* Scheduler needs to be woken up by a periodic source
(signal/timer/etc) and it shall schedule the threads
* The program shall run for 10 periods and then terminate, but not
before printing out how many times each thread ran
* Each thread shall increment a dedicated counter each
time it runs
* The scheduler shall be able to identify if a thread has
missed its deadline and keep track of how many times it
happens

#### Hints
* Remember that you have a scheduler that is orchestrating
everything else – separate thread
* Priorities are essential
* Semaphores needed for synchronization
* May need mutex to protect shared data between scheduler and
threads – remember priority inversion
* You need to use processor affinity on all your threads (including
the scheduler)
* For the overrun conditions, you should keep scheduling the
thread that has missed its deadline even after it is in an overrun
condition
* Do not skip scheduling for that execution period
* You can initially use a sleep( ) or similar function to set the
timing on your scheduler until you work out the synchronization
with the other threads and then replace with a timer


### Some thoughts 11/27
* Each worker thread should have 3 shared integers with the scheduler:
  * The amount of jobs left. This should be updated each scheduling (aka jobs+= 16)
  * The amount of jobs completed. This should be updated by the workers
  * an exit flag
* Consider switching to a more OO approach
