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
const int priorities[N_JOBS] = {1, 2, 3, 4};
Worker workers[N_JOBS];

void sleep(unsigned ms) {
  this_thread::sleep_for(chrono::milliseconds(ms));
}

void create_thread(pthread_t* worker_threads, int i) {
  //Setting priority
  pthread_attr_t tattr;
  int ret;
  sched_param param;
  ret = pthread_attr_init(&tattr);
  ret = pthread_attr_getschedparam(&tattr, &param);
  param.sched_priority += priorities[i];
  cout << "Thread priority set to " << param.sched_priority << endl;
  cout << "Min priority " << sched_get_priority_min(int policy) << endl;
  cout << "Max priority " << sched_get_priority_max(int policy) << endl;
  ret = pthread_attr_setschedparam(&tattr, &param);

  //Setting Affinity
  #ifdef __linux__
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(CPU_ID, &cpuset);
  pthread_attr_setaffinity_np(&tattr, sizeof(cpu_set_t), &cpuset);
  cout << "Affinity set to CPU " <<  CPU_ID << endl;
  #endif

  //Creating thread
  int rc = pthread_create(&worker_threads[i], &tattr, worker_thread, &workers[i]);
  if (rc) {
    cout << "Unable to create worker " << i << endl;
    exit(-1);
  }


}

void *schedule(void *arg) {
  cout << "Scheduler called." << endl;

  #ifdef __linux__
  cout << "Scheduler affinity set to CPU " <<  CPU_ID << endl;
  //Setting self Affinity
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(CPU_ID, &mask);
  sched_setaffinity(0, sizeof(mask), &mask);
  #endif


  int missed_deadlines[N_JOBS] = {};

  pthread_t worker_threads[N_JOBS];

  for(int i = 0; i < N_JOBS; ++i) {
    //Initializing data and worker threads
    cout << "Creating worker " << i << endl;
    workers[i] = Worker(i);
    create_thread(worker_threads, i);
  }

  int t_final = N_PERIODS * MAJOR_PERIOD - 1;

  for(int period = 0; period < N_PERIODS; ++period) {
    for(int t = 0; t < MAJOR_PERIOD; ++t) {
    cout << "Scheduling for t = " << period*MAJOR_PERIOD + t << endl;

      for(int i = 0; i < N_JOBS; ++i) {
        if(t % jobRate[i] == 0){
          //Job i will be scheduled at this value of t

          cout << "Job " << i << " scheduled. Jobs Completed: " << workers[i].get_completed_jobs() << endl;
          if(workers[i].is_busy()) {
            missed_deadlines[i]++;
            cout << "Worker " << i << " still busy: Overrun condition" << endl;
            cout << "Worker " << i << " has " << workers[i].get_remaining_jobs() << " jobs remaining." << endl;
            cout << "Worker " << i << " deadlines missed: " << missed_deadlines[i] << endl;
            //exit(-1);

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

  pthread_t scheduler;
  int rc = pthread_create(&scheduler, NULL, schedule, NULL);
  if (rc) {
    cout << "Unable to create scheduler" << endl;
    exit(-1);
  }
  pthread_exit(NULL);
  return 0;
}
