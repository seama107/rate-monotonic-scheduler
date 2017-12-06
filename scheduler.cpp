#include "worker.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <sched.h>
#include <pthread.h>

#define TIME_UNIT 20
#define N_PERIODS 10
#define MAJOR_PERIOD 16
#define N_JOBS 4
#define CPU_ID 0

using namespace std;

const int jobRate[N_JOBS] = {1, 2, 4, 16};
const int MAX_PRIORITY = sched_get_priority_max(SCHED_FIFO);
const int MIN_PRIORITY = sched_get_priority_min(SCHED_FIFO);
Worker workers[N_JOBS];

void sleep(unsigned ms) {
  this_thread::sleep_for(chrono::nanoseconds(ms));
}

void create_thread(pthread_t thread, void *(*start_routine) (void *), void *arg, int priority){
  // Setting priority
  pthread_attr_t tattr;
  sched_param param;
  pthread_attr_init(&tattr);
  pthread_attr_setschedpolicy(&tattr, SCHED_FIFO);
  int pol;
  pthread_attr_getschedpolicy(&tattr, &pol);
  cout << "policy " << pol << endl;
  cout << "FIFO = " << SCHED_FIFO << " RR = " << SCHED_RR << endl;

  pthread_attr_getschedparam(&tattr, &param);
  cout << "Def. priority " << param.sched_priority << endl;
  cout << "Setting to " << priority << endl;
  param.sched_priority = priority;
  int err = pthread_attr_setschedparam(&tattr, &param);

  if(err == EINVAL) {
    cout << "Invalid Priority: " << priority << endl;
  }
  else if(err == EPERM)
    cout << "Permissions not sufficient to set priority" << endl;


  // Setting affinity
  #ifdef __linux__
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(CPU_ID, &cpuset);
  pthread_attr_setaffinity_np(&tattr, sizeof(cpu_set_t), &cpuset);
  cout << "Affinity set to CPU " <<  CPU_ID << endl;
  #endif

  pthread_create(&thread, &tattr, start_routine, arg);
  //pthread_setschedparam(&thread, SCHED_FIFO, )


}

void *schedule(void *arg) {
  cout << "Scheduler called." << endl;

  int missed_deadlines[N_JOBS] = {};

  pthread_t worker_threads[N_JOBS];

  for(int i = 0; i < N_JOBS; ++i) {
    //Initializing data and worker threads
    cout << "Creating worker " << i << endl;
    workers[i] = Worker(i);
    int priority = MAX_PRIORITY - i - 1; //RMS step
    create_thread(worker_threads[i], worker_thread, &workers[i], priority);
  }

  int t_final = N_PERIODS * MAJOR_PERIOD - 1;

  for(int period = 0; period < N_PERIODS; ++period) {
    for(int t = 0; t < MAJOR_PERIOD; ++t) {
    // cout << "Scheduling for t = " << period*MAJOR_PERIOD + t << endl;

      for(int i = 0; i < N_JOBS; ++i) {
        if(t % jobRate[i] == 0){
          //Job i will be scheduled at this value of t
          // cout << "Job " << i << " scheduled. Jobs Completed: " << workers[i].get_completed_jobs() << endl;
          if(workers[i].is_busy()) {
            missed_deadlines[i]++;
            // cout << "Worker " << i << " still busy: Overrun condition" << endl;
            // cout << "Worker " << i << " has " << workers[i].get_remaining_jobs() << " jobs remaining." << endl;
            // cout << "Worker " << i << " deadlines missed: " << missed_deadlines[i] << endl;
          }
          workers[i].add_job();
        }
      }
      if(period*MAJOR_PERIOD + t == t_final){
        cout << "Final time step." << endl;
        //Signalling the exit flag for all threads
        for(int i = 0; i < N_JOBS; ++i) {
          workers[i].set_exit();
        }
      }
      sleep(TIME_UNIT);
    }
  }

  //Printing results
  for(int i = 0; i < N_JOBS; ++i) {
    cout << "Job " << i << " completed " << workers[i].get_completed_jobs() << " times." << endl;
    cout << "Deadline missed " << missed_deadlines[i] << " times." << endl;
  }

  pthread_exit(NULL);
}

int main(int argc, char const *argv[]) {
  cout << MAX_PRIORITY << endl;
  cout << MIN_PRIORITY << endl;

  pthread_t scheduler;
  create_thread(scheduler, schedule, NULL, MAX_PRIORITY);

  pthread_exit(NULL);
  return 0;
}
