# os-final-project - P3 MLFQ scheduling extension

# documentation + design decisions

For my final project, I decided to extend Project 3: CPU Scheduling by implementing MLFQ scheduling.

My implementation of MLFQ scheduling has 4 levels of queues. Q1 has higher priority than Q2, Q2 has higher priority than Q3, and Q3 has higher priority than Q4.

Here is the structure of my queues:
- Q1: Uses round robin scheduling, q1_timeslot = 3
- Q2: Uses round robin scheduling, q2_timeslot = 6
- Q3: Uses shortest job first scheduling, q3_timeslot = 10
- Q4: Uses shortest job first scheduling, q4_timeslot = 20

For boosting time slot, I have set variable boost_timeslot = 100

To extend P3 to incorporate MLFQ, I made the design decision to modify struct Process.
In addition to original fields: arrival, first_run, duration and completion, I added the following fields:
- map interactivity
    - Stores interactivity of a process, key is time process relieves CPU and value is amount of time process relieves CPU for
- int amount_run
    - Stores amount time process has been run
- int time_allotment_at_level
    - Stores time a process has used up at specific level

I also made design decisions to add helper functions for tasks like a round robin iteration, shortest job first iteration, boosting from a lower queue to highest priority queue and pushing jobs that finished relieving CPU to be pushed back onto their level queue.

I decided to create these helper functions so that it is easier for others to understand my program by breaking it down into chunks, make it easier to debug, allow for scalability in the case where we want to add more layers (we can just call my helper functions with updated fields).

I also decided to help scalability by not hard-coding my timeslot and boosting time slot values every time, but instead by setting them in variables at the start of my main MLFQ function so that it is easy to alter and experiment.

Lastly, I made the design decision to create Q1 and Q2 which use round robin scheduling to be of type deque because in the case the user sets Q1 or Q2 timeslot > 1, we need to be able to push the job to the front of the line of jobs because it has not been run for that timeslot amount of time yet.

Meanwhile, I decided to create Q3 and Q4 which use shortest job first scheduling to be of type pqueue_duration because for SJF, we want to run the shortest job out of remaining jobs next. 

For readability, I made sure to add documentation comments to all functions so that it is clear of each function's purpose and logic behind them.


# 10 minute video presentation (slides + demo)
- presents 3 important aspects of code
