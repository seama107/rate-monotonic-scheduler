#include "worker.h"

using namespace std;


void *worker_thread(void *arg) {
  Worker* worker = (Worker*) arg;

  //Wait for inital lock
  pthread_mutex_lock(& (worker->run_lock) );
  while ( !(worker->thread_exit)) {

    pthread_mutex_lock(& (worker->work_lock));
    while (worker->jobs_remaining) {
      //Work loop
      worker->busy = true;
      for(int i = 0; i < worker->work_per_job; ++i){
        int** mat = worker->doWork( rand() );
        worker->delete_mat(mat);
      }
      worker->jobs_remaining--;
      worker->jobs_completed++;
      worker->busy = false;
    }
  }

  //Return lock
  pthread_mutex_unlock(& (worker->run_lock) );

  pthread_exit(NULL);
}

Worker::Worker(int i) {
  id = i;
  jobs_remaining = 0;
  jobs_completed = 0;
  busy = false;
  thread_exit = false;
  work_per_job = jobRate[id];
  pthread_mutex_init(&run_lock, NULL);
}

Worker::~Worker() {
  pthread_mutex_destroy(&run_lock);
}

bool Worker::is_busy() {
  return busy;
}

int Worker::get_id() {
  return id;
}

int Worker::get_completed_jobs() {
  return jobs_completed;
}

int Worker::get_jobs_remaining() {
  return jobs_remaining;
}

void Worker::add_job() {
  jobs_remaining++;
}

void Worker::set_exit() {
  thread_exit = true;
}

int** Worker::doWork(int n) {

  // Allocate memory
  int **mat = new int*[MAT_ROWS];
  for (int i = 0; i < MAT_ROWS; ++i)
    mat[i] = new int[MAT_COLS];

  // Assign values
  for (int i = 0; i < MAT_ROWS; ++i) {
    for (int j = 0; j < MAT_COLS; ++j) {
      mat[i][j] = 1;
    }
  }

  // Multiplication
  for (int i = 0; i < MAT_COLS; ++i) {
    int current_col = col_order[i];
    for (int j = 0; j < MAT_ROWS; ++j) {
      mat[j][current_col] *= n;
    }
  }

  return mat;
}

void Worker::delete_mat(int** mat) {
  // De-Allocate memory to prevent memory leak
  for (int i = 0; i < MAT_ROWS; ++i)
    delete [] mat[i];
  delete [] mat;
}
