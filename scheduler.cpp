#include "worker.h"
#include <iostream>
#include <pthread.h>
#include <thread>
#include <chrono>
#include <sched.h>

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

//Setting Affinity
#ifdef __linux__
void set_pthread_attr_affinity(pthread_attr_t* tattr_ptr, int cpuid) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(cpuid, &cpuset);
  pthread_attr_setaffinity_np(tattr_ptr, sizeof(cpu_set_t), &cpuset);
  cout << "Affinity set to CPU " <<  cpuid << endl;
}
#else
void set_pthread_attr_affinity(pthread_attr_t* tattr_ptr, int cpuid) { return;}
#endif


void *schedule(void *arg) {
  cout << "Scheduler called." << endl;

  pthread_t worker_threads[N_JOBS];
  int missed_deadlines[N_JOBS] = {};

  for(int i = 0; i < N_JOBS; ++i) {
    //Initializing data and worker threads
    cout << "Creating worker " << i << endl;
    workers[i] = Worker(i);
    pthread_mutex_lock( &(workers[i].run_lock));
    pthread_mutex_lock( &(workers[i].work_lock));

    //Setting priority
    pthread_attr_t tattr;
    int ret;
    sched_param param;
    ret = pthread_attr_init(&tattr);
    ret = pthread_attr_getschedparam(&tattr, &param);
    param.sched_priority += priorities[i];
    ret = pthread_attr_setschedparam(&tattr, &param);

    set_pthread_attr_affinity(&tattr, CPU_ID);

    //Creating thread
    int rc = pthread_create(&worker_threads[i], &tattr, worker_thread, &workers[i]);
    if (rc) {
      cout << "Unable to create worker " << i << endl;
      exit(-1);
    }
  }


  for(int period = 0; period < N_PERIODS; ++period) {
    for(int t = 0; t < MAJOR_PERIOD; ++t) {
    cout << "Scheduling for t = " << period*MAJOR_PERIOD + t << endl;

      for(int i = 0; i < N_JOBS; ++i) {
        if(t % jobRate[i] == 0){
          if(workers[i].is_busy()) {
            missed_deadlines[i]++;
          }

          cout << "Job " << i << " scheduled." << endl;
          if(workers[i].is_busy()) {
            missed_deadlines[i]++;
            cout << "Worker " << i << " still busy: Overrun condition" << endl;
            cout << "Worker " << i << " has " << workers[i].get_jobs_remaining() << " jobs remaining." << endl;
            cout << "Worker " << i << " deadlines missed: " << missed_deadlines[i] << endl;

          }
          cout << "Jobs completed: " << workers[i].get_completed_jobs() << endl;

          workers[i].add_job();
          if(period == 0 && t == 0) {
            //First time, kick off the threads in their wait()
            pthread_mutex_unlock( &(workers[i].run_lock));
          }
          pthread_mutex_unlock(& (workers[i].work_lock));

        }
      }
      sleep(TIME_UNIT);
    }
  }
  //Signalling the exit flag for all threads
  for(int i = 0; i < N_JOBS; ++i) {
    workers[i].set_exit();
  }

  //wait() on threads to finish
  for(int i = 0; i < N_JOBS; ++i) {
    pthread_mutex_lock( &(workers[i].run_lock));
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
