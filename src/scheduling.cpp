
#include <scheduling.h>
#include <fstream>
#include <iostream>
#include <list>
#include <deque>
#include <string>
#include <string.h>
#include <map>

using namespace std;

pqueue_arrival read_workload(string filename) {
  pqueue_arrival workload;

  ifstream MyReadFile(filename);                // open workload file
  string myText;
  while (getline (MyReadFile, myText)) {        // for each line in file, create a process
    char *p;
    char *cstr = new char[myText.length() + 1];
    strcpy(cstr, myText.c_str());

    Process item;

    p = strtok(cstr, " ");
    int x;
    sscanf(p, "%d", &x);
    item.arrival = x;                         // initialize struct fields of process
    item.first_run = -1;
    item.completion = -1;
    item.amount_run = 0;
    item.time_allotment_at_level = 0;         // set time process spent on specific level to 0
                                              // add interactivity map to each process
    int count = 1;                            // set count to 0 (will use this to add in duration, key and value pairs to map)
    int key;
    int value;

    while(p != NULL){
      p = strtok(NULL, " ");
      if(p != NULL && count == 1){            // if reading number after first_run, set this number as the process's duration 
        sscanf(p, "%d", &x);
        item.duration = x;
      } else if(p != NULL && count % 2 == 0){     // if reading key, add key into interactivity map, intialize value to temporary value -1 
        sscanf(p, "%d", &key);
        item.interactivity[key] = -1;
      } else if(p != NULL && count % 2 != 0){     // if reading value, set key's value to value from workload
        sscanf(p, "%d", &value);
        item.interactivity[key] = value;
      } 
      count++;                                // increment count for if there are more key value pairs to store in interactivity map
    }

    workload.push(item);                    // push process into pqueue
  }

  MyReadFile.close();                       // close workload file

  show_workload(workload);

  return workload;
}

void show_workload(pqueue_arrival workload) {     // given function in starter code
  pqueue_arrival xs = workload;
  cout << "Workload:" << endl;
  while (!xs.empty()) {
    Process p = xs.top();
    cout << '\t' << p.arrival << ' ' << p.duration << endl;
    xs.pop();
  }
}

void show_processes(list<Process> processes) {    // given function in starter code
  list<Process> xs = processes;
  cout << "Processes:" << endl;
  while (!xs.empty()) {
    Process p = xs.front();
    cout << "\tarrival=" << p.arrival << ", duration=" << p.duration
         << ", first_run=" << p.first_run << ", completion=" << p.completion
         << endl;
    xs.pop_front();
  }
}

list<Process> fifo(pqueue_arrival workload) {     // scheduling with first in first out 
  list<Process> ordered;
  int timer = 0;          // initialize timer to 0

  while(true){
    if(workload.size() == 0){         // if there are no more jobs to run, break out of while loop
      break;
    }

    Process cur = workload.top();    // since there is at least 1 job still left to run, take the next job from top of pqueue_arrival workload
    workload.pop();

    if(cur.arrival <= timer){         // if job arrived before current time, first_run of process is current time
      cur.first_run = timer;
    } else{
      cur.first_run = cur.arrival;    // if job arrives in the future, first_run of process is arrival time and fast forward timer to arrival time
      timer = cur.arrival;
    }

    timer += cur.duration;            // increment timer by duration of job
    cur.completion = timer;           // set job's completion time to timer

    ordered.push_back(cur);           // add completed job to end of ordered list of jobs
  }

  return ordered;     // return list of completed processes ordered by completion time
}

list<Process> sjf(pqueue_arrival workload) {        // scheduling with shortest job first
  list<Process> ordered;
  int timer = 0;                    // initialize timer to 0
  pqueue_duration subgroup;

  while(true){
    if(workload.size() == 0 && subgroup.size() == 0){       // if there are no jobs to run in workload and subgroup, break out of while loop
      break;
    }

    while(workload.size() != 0 && workload.top().arrival <= timer){     // take all jobs from workload that have arrived by current time and push themm into subgroup
      subgroup.push(workload.top());                                    // since subgroup is of type pqueue_duration, jobs in subgroup ordered by shortest duration time
      workload.pop();                                                    
    }

    if(subgroup.size() == 0){                                // if subgroup has no jobs (aka no jobs to run that have arrived already), increment timer and check if jobs arrived again
      timer++;  
      continue;
    }

    Process cur = subgroup.top();           // if job that is not completed yet has already arrived, take it out of subgroup
    subgroup.pop();

    cur.first_run = timer;                  // set first_run of process to current time

    timer += cur.duration;                  // increment timer by length of job

    cur.completion = timer;                 // set completion time of job as current time

    ordered.push_back(cur);                 //  add completed job to end of ordered list of jobs that have completed running
  }

  return ordered;       // return list of completed processes ordered by completion time
}


list<Process> stcf(pqueue_arrival workload) {   // scheduling with shortest time to completion first
  list<Process> ordered;
  int timer = 0;                                 // initialize timer to 0
  pqueue_duration subgroup;

  while(true){
    if(workload.size() == 0 && subgroup.size() == 0){          // if there are no jobs to run in workload and subgroup, break out of while loop
      break;
    }

    while(workload.size() != 0 && workload.top().arrival <= timer){      // take all jobs from workload that have arrived by current time and push themm into subgroup
      subgroup.push(workload.top());                                    // since subgroup is of type pqueue_duration, jobs in subgroup ordered by shortest duration time
      workload.pop();
    }

    if(subgroup.size() == 0){           // if subgroup has no jobs (aka no jobs to run that have arrived already), increment timer and check if jobs arrived again
      timer++;
      continue;
    }

    Process cur = subgroup.top();       // if job that is not completed yet has already arrived, take it out of subgroup
    subgroup.pop();

    if(cur.first_run == -1){            // if job has not yet been run before, set first_run time to current time
      cur.first_run = timer;
    }

    cur.duration -= 1;                 // decrement job duration by 1, increment timer by 1
    timer++;

    if(cur.duration == 0){                      // if job has completed running, set completion time to current time
      cur.completion = timer;
      ordered.push_back(cur);          //  add completed job to end of ordered list of jobs that have completed running
    } else{
      subgroup.push(cur);              // if job has not finished running, push back to subgroup of jobs that can be run next job
    }
  }

  return ordered;       // return list of completed processes ordered by completion time
}

list<Process> rr(pqueue_arrival workload) {     // scheduling with round robin
  queue<Process> subgroup;    
  list<Process> ordered;
  int timer = 0;                  // initialize timer to 0

  if(workload.size() != 0){               // set timer to arrival time of first job to arrive
    timer = workload.top().arrival;
  }

  while(workload.size() != 0 && workload.top().arrival <= timer){       // take all jobs from workload that have arrived by current time and push themm into subgroup
    subgroup.push(workload.top());                        
    workload.pop();
  }

  while(workload.size() != 0 || subgroup.size() != 0){          // while there is a job that has not completed running yet, keep running algorithm
    if(subgroup.size() == 0){                                                    // if there are no jobs that have already arrived by current time that have yet to complete running
      timer = workload.top().arrival;                                            // set timer to arrival time of next job to arrive
      while(workload.size() != 0 && workload.top().arrival <= timer){  // take all jobs from workload that have arrived by current time and push themm into subgroup
        subgroup.push(workload.top());
        workload.pop();
      }
    }

    Process cur = subgroup.front();     // take next job from queue of jobs that are ready to run
    subgroup.pop();

    if(cur.first_run == -1){        // if job has not yet been run before, set first_run time to current time
      cur.first_run = timer;
    }

    timer++;                        
    cur.duration -= 1;                // decrement job duration by 1, increment timer by 1

    if(cur.duration == 0){                  // if job has completed running, set completion time to current time
      cur.completion = timer;
      ordered.push_back(cur);               //  add completed job to end of ordered list of jobs that have completed running
    } else{
      subgroup.push(cur);              // if job has not finished running, push back to subgroup of jobs that can be run next job
    }

    while(workload.size() != 0 && workload.top().arrival <= timer){     // take all jobs from workload that have arrived by current time and push themm into subgroup
        subgroup.push(workload.top());
        workload.pop();
    }
  }

  return ordered;       // return list of completed processes ordered by completion time
}

/* ----------------------------------------------- BEGINNING OF MLFQ IMPLEMENTATION  ----------------------------------------------------------
   Created helper functions for scalability purposes
   Main function for MLFQ is named mlfq
*/

// Helper function for MLFQ scheduling function, Q1
// Supports one iteration of round robin 
void rr_iteration_deque_to_deque(deque<Process> &cur_queue, map<int, queue<Process>> &cur_relieving_jobs, deque<Process> &lower_queue, int lower_queue_timeslot, int &timer, list<Process> &complete){
      Process cur_process = cur_queue.front();              // Get next process in queue to run
      cur_queue.pop_front();

      if(cur_process.first_run == -1){                      // Set first_run if job hasn't been run yet
        cur_process.first_run = timer;
      }

      timer++;                                              // Increment timer by one, update job's time allotment at current level, duration and amount_run
      cur_process.time_allotment_at_level -= 1;
      cur_process.duration -= 1;
      cur_process.amount_run++;
                                                            // Check if 1. job is now finished or 2. job has used up time slot at current level or 3. job is relieving the CPU at this time
      if(cur_process.duration == 0){                        // If job is now completed, set completion time to current time and add completed job to end of ordered list of jobs that have completed running
        cur_process.completion = timer;
        complete.push_back(cur_process);
      } else if(cur_process.interactivity.count(cur_process.amount_run)){                                   // If job now relieves CPU, add job to cur_relieving_jobs map with key = time job ready to run again, value = queue containing the process itself
        if(cur_relieving_jobs[cur_process.interactivity[cur_process.amount_run] + timer].size() == 0){        
          queue<Process> val;
          val.push(cur_process);
          cur_relieving_jobs[cur_process.interactivity[cur_process.amount_run] + timer] = val;
        } else{                                                                                                    // If multiple jobs already use key = time job ready to run again, push job to end of value queue at that key
          cur_relieving_jobs[cur_process.interactivity[cur_process.amount_run] + timer].push(cur_process);
        }
      } else if(cur_process.duration != 0 && cur_process.time_allotment_at_level == 0){                   // If job not complete, but used up timeslot at current level, set timeslot as lower_queue's timeslot and push job to lowerqueue 
        cur_process.time_allotment_at_level = lower_queue_timeslot;                                   
        lower_queue.push_back(cur_process);
      } else if(cur_process.duration != 0 && cur_process.time_allotment_at_level != 0){                    // If job not complete, but did not use up timeslot, push job back to front of curqueue since timeslot not used up yet
        cur_queue.push_front(cur_process);
      }
}

// Helper function for MLFQ scheduling function
// Supports boosting jobs from Q2 to Q1 
void boost_from_lower_deque(deque<Process> &lowerq, deque<Process> &higherq, int higherq_timeslot){
  while(lowerq.size() != 0){                                            // While lower queue has jobs, take jobs and set jobs time_allotment_at_level to Q1's timeslot and push job to Q1
    Process item = lowerq.front();
    item.time_allotment_at_level = higherq_timeslot;
    higherq.push_back(item);
    lowerq.pop_front();
  }
}

// Helper function for MLFQ scheduling function
// Supports boosting jobs from Q3 and Q4 to Q1
void boost_from_lower_pqueue(pqueue_duration &lowerq, deque<Process> &higherq, int higherq_timeslot){
  while(lowerq.size() != 0){                                            // While lower queue has jobs, take jobs and set jobs time_allotment_at_level to Q1's timeslot and push job to Q1
    Process item = lowerq.top();
    item.time_allotment_at_level = higherq_timeslot;
    higherq.push_back(item);
    lowerq.pop();
  }
}

// Helper function for MLFQ scheduling function
// Supports pushing jobs that can be added back to a level queue after relieving CPU for some time to be added back to their queue
void add_jobs_ready_to_return_after_relieving_CPU_deque(map<int, queue<Process>> &relieving_jobs_list, int timer, deque<Process> &level_queue){
  if(relieving_jobs_list.count(timer)){
    queue<Process> processes_to_add_back = relieving_jobs_list[timer];
    relieving_jobs_list.erase(timer);
    while (!processes_to_add_back.empty()) {
      level_queue.push_back(processes_to_add_back.front());
      processes_to_add_back.pop();
    }
  }
}

// Helper function for MLFQ scheduling function
// Supports pushing jobs that can be added back to a level queue after relieving CPU for some time to be added back to their queue
void add_jobs_ready_to_return_after_relieving_CPU_pqueue(map<int, queue<Process>> &relieving_jobs_list, int timer, pqueue_duration &level_queue){
  if(relieving_jobs_list.count(timer)){
    queue<Process> processes_to_add_back = relieving_jobs_list[timer];
    relieving_jobs_list.erase(timer);
    while (!processes_to_add_back.empty()) {
      level_queue.push_back(processes_to_add_back.front());
      processes_to_add_back.pop();
    }
  }
}
  
// Main function for MLFQ scheduling
// Calls on helper functions
list<Process> mlfq(pqueue_arrival workload) {           // scheduling with Multilevel Feedback Queue (mlfq)
  list<Process> complete;
  int timer = 0;                                        // initialize timer to 0
  
  deque<Process> q1;                                    // current mlfq implementation has 4 levels, first 2 levels use round-robin (RR) scheduling. last 2 levels use shortest job first (SJF) scheduling.
  deque<Process> q2;
  pqueue_duration q3;
  pqueue_duration q4;

  int q1_timeslot = 3;                                  // set timeslots for each level queue here
  int q2_timeslot = 6;
  int q3_timeslot = 10;
  int q4_timeslot = 20;
  int boost_timeslot = 100;

  map<int, queue<Process>> q1_relieving_jobs;           // create maps to hold jobs relieving CPU for each level queue 
  map<int, queue<Process>> q2_relieving_jobs;
  map<int, queue<Process>> q3_relieving_jobs;
  map<int, queue<Process>> q4_relieving_jobs;

  bool Q3_has_job_to_continue = false;                  // initialize edge case vars for Q3 and Q4 to hold their current job if in mid-run
  bool Q4_has_job_to_continue = false;
  Process cur_job_Q3;
  Process cur_job_Q4;

                                                                              // Keep running MLFQ while loop till all jobs finished running
  while(workload.size() != 0 || 
        q1.size() != 0 || q2.size() != 0 || q3.size() != 0 || q4.size() != 0 || 
        Q3_has_job_to_continue || Q4_has_job_to_continue || 
        !q1_relieving_jobs.empty() || !q2_relieving_jobs.empty() || !q3_relieving_jobs.empty() || !q4_relieving_jobs.empty()
  ){
    while(workload.size() != 0 && workload.top().arrival == timer){                   // Add jobs from workload (not yet on a level queue) that have arrived based on current time to Q1
      Process item = workload.top();
      item.time_allotment_at_level = q1_timeslot;                                     // Set these jobs time_allotment_at_level to Q1's timeslot
      q1.push_back(item);
      workload.pop();
    }

    // Run one job based on priority of queues (i.e. if Q1 not empty, run a job from Q1. if Q1 empty, but Q2 not empty, run a job from Q2)
    if(q1.size() != 0){                                                                                  // If Q1 not empty, run a job from Q1 with RR                             
      rr_iteration_deque_to_deque(q1, q1_relieving_jobs, q2, q2_timeslot, timer, complete);
    } else if(q2.size() != 0){                                                                              // If Q2 not empty, run a job from Q2 with RR
      rr_iteration_deque_to_pqueue_duration(q2, q2_relieving_jobs, q3, q3_timeslot, timer, complete);
    } else if(q3.size() != 0 || Q3_has_job_to_continue){                                                 // If Q3 not empty, run a job from Q3 with SJF
      // 3a. Get top process in queue to run
      if(!Q3_has_job_to_continue){
        cur_job_Q3 = q3.top();
        q3.pop();
      }

      // 3b. Set first_run if applicable
      if(cur_job_Q3.first_run == -1){
        cur_job_Q3.first_run = timer;
      }

      // 3c. Update timer, time allotment at level, duration and amount_run
      timer++;
      cur_job_Q3.time_allotment_at_level -= 1;
      cur_job_Q3.duration -= 1;
      cur_job_Q3.amount_run++;

      // 3d. Check if job finished / used up time slot / relieves CPU
      if(cur_job_Q3.duration == 0){                                                                       // job finished
        cur_job_Q3.completion = timer;
        complete.push_back(cur_job_Q3);
        Q3_has_job_to_continue = false;
      } else if(cur_job_Q3.interactivity.count(cur_job_Q3.amount_run)){                                   // job relieves CPU now
        if(q3_relieving_jobs[cur_job_Q3.interactivity[cur_job_Q3.amount_run] + timer].size() == 0){
          queue<Process> val;
          val.push(cur_job_Q3);
          q3_relieving_jobs[cur_job_Q3.interactivity[cur_job_Q3.amount_run] + timer] = val;
        } else{
          q3_relieving_jobs[cur_job_Q3.interactivity[cur_job_Q3.amount_run] + timer].push(cur_job_Q3);
        }
        Q3_has_job_to_continue = false;
      } else if(cur_job_Q3.duration != 0 && cur_job_Q3.time_allotment_at_level == 0){                   // job not complete, but used up timeslot
        cur_job_Q3.time_allotment_at_level = q4_timeslot;
        q4.push(cur_job_Q3);
        Q3_has_job_to_continue = false;
      } else if(cur_job_Q3.duration != 0 && cur_job_Q3.time_allotment_at_level != 0){                  // job not complete, but did not use up timeslot
        Q3_has_job_to_continue = true;
      }
    } else if(q4.size() != 0 || Q4_has_job_to_continue){
      // 3a. Get top process in queue to run
      if(!Q4_has_job_to_continue){
        cur_job_Q4 = q4.top();
        q4.pop();
      }
      
      // 3b. Set first_run if applicable
      if(cur_job_Q4.first_run == -1){
        cur_job_Q4.first_run = timer;
      }

      // 3c. Update timer, time allotment at level, duration and amount_run
      timer++;
      cur_job_Q4.time_allotment_at_level -= 1;
      cur_job_Q4.duration -= 1;
      cur_job_Q4.amount_run++;

      // 3d. Check if job finished / used up time slot / relieves CPU
      if(cur_job_Q4.duration == 0){                                                                       // job finished
        cur_job_Q4.completion = timer;
        complete.push_back(cur_job_Q4);
        Q4_has_job_to_continue = false;
      } else if(cur_job_Q4.interactivity.count(cur_job_Q4.amount_run)){                                   // job relieves CPU now
        if(q4_relieving_jobs[cur_job_Q4.interactivity[cur_job_Q4.amount_run] + timer].size() == 0){
          queue<Process> val;
          val.push(cur_job_Q4);
          q4_relieving_jobs[cur_job_Q4.interactivity[cur_job_Q4.amount_run] + timer] = val;
        } else{
          q4_relieving_jobs[cur_job_Q4.interactivity[cur_job_Q4.amount_run] + timer].push(cur_job_Q4);
        }
        Q4_has_job_to_continue = false;
      } else if(cur_job_Q4.duration != 0){                                                                // job not complete
        Q4_has_job_to_continue = true;
      }
    } else{
      timer++;
    }

    add_jobs_ready_to_return_after_relieving_CPU_deque(q1_relieving_jobs, timer, q1);             // Add any processes that can be added back after finishing relieving CPU back to level queue
    add_jobs_ready_to_return_after_relieving_CPU_deque(q2_relieving_jobs, timer, q2);
    add_jobs_ready_to_return_after_relieving_CPU_pqueue(q3_relieving_jobs, timer, q3);
    add_jobs_ready_to_return_after_relieving_CPU_pqueue(q4_relieving_jobs, timer, q4);

    if(timer % boost_timeslot == 0){                            // Check if it is time to boost all jobs in queues to Q1
      boost_from_lower_deque(q2, q1, q1_timeslot);              // Boost all jobs from Q2 to Q1
      boost_from_lower_pqueue(q3, q1, q1_timeslot);             // Boost all jobs from Q3 to Q1
      boost_from_lower_pqueue(q4, q1, q1_timeslot);             // Boost all jobs from Q4 to Q1

      if(Q3_has_job_to_continue){                               // If Q3 has job that still needs to run, but was popped off Q3, boost this job to Q1 too
        q1.push_back(cur_job_Q3);
      }
      if(Q4_has_job_to_continue){                               // If Q4 has job that still needs to run, but was popped off Q4, add this job to Q1 too
        q1.push_back(cur_job_Q4);
      }
      Q3_has_job_to_continue = false;
      Q4_has_job_to_continue = false;
    }
  }

  return complete;                        // Return list of completed processes ordered by completion time
}


float avg_turnaround(list<Process> processes) {     // return avg turnaround time given list of completed processes
  int sum = 0;
  int div = 0;
  for(auto i: processes){
    Process c = i;
    sum += c.completion - c.arrival;
    div += 1;
  }
  return float(sum) / float(div);
}

float avg_response(list<Process> processes) {     // returns avg response time given list of completed processes
  int sum = 0;
  int div = 0;
  for(auto i: processes){
    Process c = i;
    sum += c.first_run - c.arrival;
    div += 1;
  }
  return float(sum) / float(div);
}

void show_metrics(list<Process> processes) {      // given function in starter code
  float avg_t = avg_turnaround(processes);
  float avg_r = avg_response(processes);
  show_processes(processes);
  cout << '\n';
  cout << "Average Turnaround Time: " << avg_t << endl;
  cout << "Average Response Time:   " << avg_r << endl;
}