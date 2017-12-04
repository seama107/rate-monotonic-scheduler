#ifndef worker_h
#define worker_h

#include <stdlib.h>
#include <iostream>
#include <pthread.h>

#define MAT_ROWS 10
#define MAT_COLS 10
#define N_JOBS 4

const int col_order[] = {0, 5, 1, 6, 2, 7, 3, 8, 4, 9};
extern const int jobRate[N_JOBS];
void *worker_thread(void *arg);

class Worker {
  int id;
  int jobs_remaining;
  int jobs_completed;
  int work_per_job;
  bool busy;
  bool thread_exit;


public:
  Worker() : Worker(0){};
  Worker(int i);
  ~Worker();

  friend void *worker_thread(void *arg);

  bool is_busy();
  int get_id();
  int get_completed_jobs();
  int get_jobs_remaining();
  void add_job();
  void set_exit();

  pthread_mutex_t run_lock;

  int** doWork(int n);
  void delete_mat(int** mat);

};

#endif
