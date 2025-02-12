# COS417 Spring 2025: Stride Scheduler

In this project, you will implement a stride scheduler in xv6 as well as accompanying system calls that modify/reveal the scheduler's behaviors. The stride scheduler ensures that processes receive CPU time proportional to their assigned tickets, providing deterministic scheduling behavior. By adjusting these tickets, the scheduler can adapt to changing process workloads, priorities, or resource usage.

Learning Objectives:

* Understand and implement a stride scheduling algorithm.
* Gain experience modifying and extending the xv6 operating system.
* Understand how to implement and modify system calls, the scheduler, and process states.

## Getting Started
To get started, login to courselab and copy over the assignemnt onto (anywhere in) your HOME directory:  
```
cp -r /u/cos417/assignments/stride $HOME 
```
Then `cd` to the **parent** of where your new stride assignment is located, and give yourself permissions via:
`chmod -R 700 stride`

For this project, all your code changes will be in the `solution` directory, which contains all xv6 files, and the testing framework is in the `tests` directory.

---

# Project Details

## Overview of Basic Stride Scheduling

As opposed to the existing Round Robin scheduler (in the `scheduler()` function of `proc.c`), the stride scheduler maintains a few additional pieces of information for each process:

- `tickets` -- a value assigned upon process creation.  It can be modified by a system call.  It should default to 8.
- `stride` -- a value that is inversely proportional to a process' tickets. Particularly, `stride = STRIDE1 / tickets` where `STRIDE1` is a constant defined in `proc.c` (for this project: 1<<10 or 1024).
- `pass` -- initially set to `0`.  This value is updated every time the process runs.

When the stride scheduler runs, it selects the runnable process with the lowest `pass` value to run for the next tick.
After the process runs, the scheduler increments its `pass` value by its `stride`: `pass += stride`.
Intuitively, the stride scheduler dedicates more CPU time to processes assigned with more tickets. Since processes with more tickets will have smaller `strides`, their `pass` increases relatively slower than other processes, hence making them chosen to be run more frequently.
These steps ensure that over time, each process receives CPU time proportional to the amount of tickets it has.

## Dynamic Process Participation

The Basic Stride Algorithm does not account for changes to the total number of processes waiting to be scheduled.

Consider the case when we have 2 processes which have already been running and both have a `stride` value of 1 and `pass` value of 100.
A new process, let us say `A` now joins our system with also a `stride` value of 1. Assume that all three processes have the same number of tickets.

What happens in the case of Basic Stride Scheduling?

Because the `pass` value of `A` is so small compared to the other processes, it will now be scheduled for the next 100 ticks before any other process is allowed to run.
This is not the behavior we want. Since each process has the same amount of tickets, we want the CPU to be shared equally among all of the processes including the newly arrived process. In this particular case we would want all processes to take turns, essentially the same way as how a RR scheduler would behave.

#### How do we do that?

Let us maintain aggregate information about the set of all processes that are scheduleable and use that information when a process enters or leaves this state.

- `global_tickets` -- the sum of all RUNNABLE and RUNNING process's tickets.
- `global_stride` -- inversely proportional to the `global_tickets`, specifically `STRIDE1 / global_tickets`
- `global_pass` -- incremented by the **current** `global_stride` at every tick.

Now, when a process is created or re-enters the RUNNABLE state, its `pass` value should begin at `global_pass`. In the case of process `A` above, `A`'s starting `pass` value will now be the same as the other 2 processes. (Hint: you can look around `proc.c` to identify all the places where processes re/enter the RUNNABLE state and whether their passes should be updated!).

The global variables themselves will need to be recalculated in the `update_on_tick()` function since processes may enter or leave the RUNNABLE state after any tick.

## Implementation
We will be using a modular scheduler for our implementation. Take a look at the Makefile and the `SCHED_MACRO` variable; your implementation will support both RR and Stride scheduler based on the flag passed. We will only be implementing the dynamic stride scheduler for a single CPU.

**Task 1** 

Implement the stride scheduler. The main class to look at and modify is `proc.c`. Additionally, add any necessary new fields to the proc struct (and elsewhere) to support the new scheduler; then initialize and maintain these values. The stride scheduler should pick and run the process with the lowest pass value among all RUNNABLE processes.

For tie breaking (when two+ processes have the same `pass`), use their `rtime` ("total runtime", as described in task 3), i.e. the process that has ran for a lower number of ticks will be scheduled first. If both `pass` and `rtime` are equal, use the proccess' pids. The process with the smaller pid will go first. 
Hence the tiebreak order is: `pass` -> `rtime` -> `pid`.

After a process finishes running, the scheduler should increase its `pass` accordingly.

**Task 2**

Create a new system call that allows a process to set its own number of tickets. 

```
int settickets(int n);
```

The max tickets allowed to be set for a process is `MAX_TICKETS` (1<<5 or 32). The minimum tickets allowed is 1. 
If a process sets a value lower than 1, set the number of tickets to `DEFAULT_TICKETS` (8). This is also the number of tickets a new process in your system should have. 

Remember to also recompute the `stride` and `pass` values. The new `stride` is computed as usual, inversely proportional to tickets, while `pass` should be reset to what the current `global_pass` is.

**Task 3**

Create a new system call that retrieves scheduling information for all processes.
```
int getpinfo(struct pstat*);
```

All the information to retrieve is detailed in `pstat.h`:
```
struct pstat {
  // each property below contain a list of:
  int pid[NPROC];        // PIDs of each process
  int inuse[NPROC];      // whether this process (slot of the process table) is in use (1 or 0)

  int tickets[NPROC];    // number of tickets held by each process
  int stride[NPROC];     // stride values of each process
  int pass[NPROC];       // pass values of each process
  int rtime[NPROC];      // total running times of each process
};
```

`rtime` contains the total amount of time (in terms of ticks) each process has ran; remember to calculate and maintain this value for each process as well.

For both tasks 2 and 3, you should look through and modify the following additional related files to implement the new syscalls: 
`defs.h`, `syscall.c`, `syscall.h`, `sysproc.c`, `user.h`, `usys.S`.

Some helpful resources here:

Intro to xv6 syscalls: https://www.youtube.com/watch?v=vR6z2QGcoo8&ab_channel=RemziArpaci-Dusseau

**Verification (optional; no deliverables needed)**

The bulk of your implementation is done! To verify the difference in behavior, you can test both the RR and Stride scheduler on a CPU intensive workload, and use getpinfo to retrieve the scheduling information for them. There is a `workload.c` file in the initial xv6 implementation that we'll use; make sure to append `_workload\` to your `UPROGS` in the Makefile first.

Run the RR scheduler first (without modifying the scheduler flag) by running the following in the `solution` directory:
```
make qemu-nox 
```

Now in xv6 run the workload with the following command:
```
workload &
```

Notice the `&` here, the workload runs for a long time so make sure to run it in the background or you may need to wait a substantial time before you can see your results.

You should now see periodic snapshots of info on all the processes in the system.

Once you see "Done Measuring" printed to the output, 
run 
```
cat rr_process_stats.csv
```
You can copy the results to a file and store it away for easier access later.

Now we'll repeat the process for the stride scheduler, but first quit out xv6 and run `make clean` in your `solution` directory.

Run xv6 with the following:
```
make qemu-nox SCHEDULER=STRIDE  
```

...and run the workload again. The final results this time will be visible by running: 
```
cat stride_process_stats.csv
```

You can compare the results from `stride_process_stats.csv` and `rr_process_stats.csv`. Notice how the processes running on the stride scheduler will have runtimes proportional to the number of tickets they have, while for RR, every process has approximately the same runtime, regardless of how many tickets they have!

-----
## Testing

Please refer to the readme file in the `tests` directory for the specs of our testing framework, including how to add your own tests, run only one test, run all tests even after one fails, and more!

## Submission/Deliverables

You'll be uploading a .zip file of the `solution` directory (which should contain all your code changes) to TigerFile under the `stride` assignment - all test cases will be run whenever you click on 'check submitted files'.

### Related Readings
- [OSTEP Chapter 9: Scheduling: Proportional Share.](https://pages.cs.wisc.edu/~remzi/OSTEP/cpu-sched-lottery.pdf)
Discusses concepts related to proportional-share scheduling algorithms.
