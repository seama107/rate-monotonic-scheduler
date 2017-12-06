#include "worker.h"

using namespace std;


void *worker_thread(void *arg) {
  Worker* worker = (Worker*) arg;
  // cout << "This worker is on CPU " << sched_getcpu() << endl;

  pthread_attr_t tattr;
  sched_param param;
  sched_getattr(0, &param, sizeof(sched_param), 0);
  cout << "Thread " << worker->id << " on priority " <<  param.sched_priority << endl;

  while ( !(worker->thread_exit)) {
    sem_wait(worker->sem_id);
    cout << "Worker begin " << worker->id << endl;
    for(int i = 0; i < worker->work_per_job; ++i){
      int** mat = worker->doWork( rand() );
      worker->delete_mat(mat);
    }
    worker->jobs_completed++;
    cout << "Worker end " << worker->id << endl;
  }


  pthread_exit(NULL);
}

Worker::Worker(int i) {
  id = i;
  jobs_completed = 0;
  thread_exit = false;
  work_per_job = jobRate[id];
  string sem_name_str = "worker_sem_" + to_string(id);
  sem_name = sem_name_str.c_str();
  sem_id = sem_open(sem_name, O_CREAT, 0600, 0); //Initializes to 0
}

Worker::~Worker() {
  sem_unlink(sem_name);
}

int Worker::get_id() {
  return id;
}

int Worker::get_completed_jobs() {
  return jobs_completed;
}

void Worker::add_job() {
  sem_post(sem_id);
}

int Worker::get_remaining_jobs() {
  int remaining;
  sem_getvalue(sem_id, &remaining);
  return remaining;
}

bool Worker::is_busy() {
  return (get_remaining_jobs() != 0);
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
