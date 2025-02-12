#define NPROC        64  // maximum number of processes

struct pstat {
  // each property below contain a list of:
  int pid[NPROC];        // PIDs of each process
  int inuse[NPROC];      // whether this process (slot of the process table) is in use (1 or 0)

  int tickets[NPROC];    // number of tickets held by each process
  int stride[NPROC];     // stride values of each process
  int pass[NPROC];       // pass values of each process
  int rtime[NPROC];      // total running times of each process
};
