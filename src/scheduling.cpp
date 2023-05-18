
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
  // open filename
  // for each line in file, create a process
  // now push into pqueue
  pqueue_arrival workload;

  ifstream MyReadFile(filename);
  string myText;
  while (getline (MyReadFile, myText)) {
    char *p;
    char *cstr = new char[myText.length() + 1];
    strcpy(cstr, myText.c_str());

    Process item;

    p = strtok(cstr, " ");
    int x;
    sscanf(p, "%d", &x);
    item.arrival = x;
    item.first_run = -1;
    item.completion = -1;
    item.amount_run = 0;
    item.time_allotment_at_level = 0;

    // add interactivity map to each process
    int count = 1;
    int key;
    int value;

    while(p != NULL){
      p = strtok(NULL, " ");
      if(p != NULL && count == 1){
        sscanf(p, "%d", &x);
        item.duration = x;
      } else if(p != NULL && count % 2 == 0){
        sscanf(p, "%d", &key);
        item.interactivity[key] = -1;
      } else if(p != NULL && count % 2 != 0){
        sscanf(p, "%d", &value);
        item.interactivity[key] = value;
      }
      count++;
    }

    workload.push(item);
  }

  MyReadFile.close();

  show_workload(workload);

  return workload;
}

void show_workload(pqueue_arrival workload) {
  pqueue_arrival xs = workload;
  cout << "Workload:" << endl;
  while (!xs.empty()) {
    Process p = xs.top();
    cout << '\t' << p.arrival << ' ' << p.duration << endl;
    xs.pop();
  }
}

void show_processes(list<Process> processes) {
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

list<Process> fifo(pqueue_arrival workload) {
  list<Process> ordered;
  int timer = 0;

  while(true){
    if(workload.size() == 0){
      break;
    }

    Process cur = workload.top();
    workload.pop();

    if(cur.arrival <= timer){
      cur.first_run = timer;
    } else{
      cur.first_run = cur.arrival;
      timer = cur.arrival;
    }

    timer += cur.duration;
    cur.completion = timer;

    ordered.push_back(cur);
  }

  return ordered;
}

list<Process> sjf(pqueue_arrival workload) {
  list<Process> ordered;
  int timer = 0;
  pqueue_duration subgroup;

  while(true){
    if(workload.size() == 0 && subgroup.size() == 0){
      break;
    }

    while(workload.size() != 0 && workload.top().arrival <= timer){
      subgroup.push(workload.top());
      workload.pop();
    }

    if(subgroup.size() == 0){
      timer++;
      continue;
    }

    Process cur = subgroup.top();
    subgroup.pop();

    cur.first_run = timer;

    timer += cur.duration;

    cur.completion = timer;

    ordered.push_back(cur);
  }

  return ordered;
}


list<Process> stcf(pqueue_arrival workload) {
  list<Process> ordered;
  int timer = 0;
  pqueue_duration subgroup;

  while(true){
    if(workload.size() == 0 && subgroup.size() == 0){
      break;
    }

    while(workload.size() != 0 && workload.top().arrival <= timer){
      subgroup.push(workload.top());
      workload.pop();
    }

    if(subgroup.size() == 0){
      timer++;
      continue;
    }

    Process cur = subgroup.top();
    subgroup.pop();

    if(cur.first_run == -1){
      cur.first_run = timer;
    }

    cur.duration -= 1;
    timer++;

    if(cur.duration <= 0){
      cur.completion = timer + cur.duration;
      cur.duration = 0;
      ordered.push_back(cur);
    } else{
      subgroup.push(cur);
    }
  }

  return ordered;
}

list<Process> rr(pqueue_arrival workload) {
  queue<Process> subgroup;
  list<Process> ordered;
  int timer = 0;

  if(workload.size() != 0){
    timer = workload.top().arrival;
  }

  while(workload.size() != 0 && workload.top().arrival <= timer){
    subgroup.push(workload.top());
    workload.pop();
  }

  while(workload.size() != 0 || subgroup.size() != 0){
    if(subgroup.size() == 0){
      timer = workload.top().arrival;
      while(workload.size() != 0 && workload.top().arrival <= timer){
        subgroup.push(workload.top());
        workload.pop();
      }
    }

    Process cur = subgroup.front();
    subgroup.pop();

    // set first_run
    if(cur.first_run == -1){
      cur.first_run = timer;
    }

    timer++;
    cur.duration -= 1;

    if(cur.duration <= 0){
      cur.completion = timer + cur.duration;
      cur.duration = 0;
      // add to ordered

      ordered.push_back(cur);
    } else{
      subgroup.push(cur);
    }

    while(workload.size() != 0 && workload.top().arrival <= timer){
        subgroup.push(workload.top());
        workload.pop();
    }
  }

  return ordered;
}

void rr_iteration(deque<Process> &cur_queue, map<int, queue<Process>> &cur_relieving_jobs, deque<Process> &lower_queue, int lower_queue_timeslot, int &timer, list<Process> &complete){
      // 3a. Get top process in queue to run
      Process cur_process = cur_queue.front();
      cur_queue.pop_front();

      // 3b. Set first_run if applicable
      if(cur_process.first_run == -1){
        cur_process.first_run = timer;
      }

      // 3c. Update timer, time allotment at level, duration and amount_run
      timer++;
      cur_process.time_allotment_at_level -= 1;
      cur_process.duration -= 1;
      cur_process.amount_run++;

      // 3d. Check if job finished / used up time slot / relieves CPU
      if(cur_process.duration == 0){                                                                    // job finished
        cur_process.completion = timer;
        complete.push_back(cur_process);
      } else if(cur_process.interactivity.count(cur_process.amount_run)){                              // job relieves CPU now
        if(cur_relieving_jobs[cur_process.interactivity[cur_process.amount_run] + timer].size() == 0){
          queue<Process> val;
          val.push(cur_process);
          cur_relieving_jobs[cur_process.interactivity[cur_process.amount_run] + timer] = val;
        } else{
          cur_relieving_jobs[cur_process.interactivity[cur_process.amount_run] + timer].push(cur_process);
        }
      } else if(cur_process.duration != 0 && cur_process.time_allotment_at_level == 0){                   // job not complete, but used up timeslot
        cur_process.time_allotment_at_level = lower_queue_timeslot;
        lower_queue.push_back(cur_process);
      } else if(cur_process.duration != 0 && cur_process.time_allotment_at_level != 0){                    // job not complete, but did not use up timeslot
        cur_queue.push_front(cur_process);
      }
}

void boost_from_lower_deque(deque<Process> &lowerq, deque<Process> &higherq, int higherq_timeslot){
  while(lowerq.size() != 0){
    Process item = lowerq.front();
    item.time_allotment_at_level = higherq_timeslot;
    higherq.push_back(item);
    lowerq.pop_front();
  }
}

void boost_from_lower_pqueue(pqueue_duration &lowerq, deque<Process> &higherq, int higherq_timeslot){
  while(lowerq.size() != 0){
    Process item = lowerq.top();
    item.time_allotment_at_level = higherq_timeslot;
    higherq.push_back(item);
    lowerq.pop();
  }
}

list<Process> mlfq(pqueue_arrival workload) {
  list<Process> complete;
  int timer = 0;
  
  deque<Process> q1;
  deque<Process> q2;
  pqueue_duration q3;
  pqueue_duration q4;

  int q1_timeslot = 3;
  int q2_timeslot = 6;
  int q3_timeslot = 10;
  int q4_timeslot = 20;
  int boost_timeslot = 100;

  map<int, queue<Process>> q1_relieving_jobs;
  map<int, queue<Process>> q2_relieving_jobs;
  map<int, queue<Process>> q3_relieving_jobs;
  map<int, queue<Process>> q4_relieving_jobs;

  bool Q3_has_job_to_continue = false;
  bool Q4_has_job_to_continue = false;
  Process cur_job_Q3;
  Process cur_job_Q4;

  // 1. Keep running MLFQ till all jobs finished running
  while(workload.size() != 0 || 
        q1.size() != 0 || q2.size() != 0 || q3.size() != 0 || q4.size() != 0 || 
        Q3_has_job_to_continue || Q4_has_job_to_continue || 
        !q1_relieving_jobs.empty() || !q2_relieving_jobs.empty() || !q3_relieving_jobs.empty() || !q4_relieving_jobs.empty()
  ){
    // 2. Add jobs from workload that can be played based on arrival to Q1
    while(workload.size() != 0 && workload.top().arrival == timer){
      Process item = workload.top();
      item.time_allotment_at_level = q1_timeslot;
      q1.push_back(item);
      workload.pop();
    }

    // 3. Run job if there is a job that can be run based on priority of queues
    if(q1.size() != 0){
      rr_iteration(q1, q1_relieving_jobs, q2, q2_timeslot, timer, complete);
      /*
      // 3a. Get top process in queue to run
      Process cur_process = q1.front();
      q1.pop_front();

      // 3b. Set first_run if applicable
      if(cur_process.first_run == -1){
        cur_process.first_run = timer;
      }

      // 3c. Update timer, time allotment at level, duration and amount_run
      timer++;
      cur_process.time_allotment_at_level -= 1;
      cur_process.duration -= 1;
      cur_process.amount_run++;

      // 3d. Check if job finished / used up time slot / relieves CPU
      if(cur_process.duration == 0){                                                                    // job finished
        cur_process.completion = timer;
        complete.push_back(cur_process);
      } else if(cur_process.interactivity.count(cur_process.amount_run)){                              // job relieves CPU now
        if(q1_relieving_jobs[cur_process.interactivity[cur_process.amount_run] + timer].size() == 0){
          queue<Process> val;
          val.push(cur_process);
          q1_relieving_jobs[cur_process.interactivity[cur_process.amount_run] + timer] = val;
        } else{
          q1_relieving_jobs[cur_process.interactivity[cur_process.amount_run] + timer].push(cur_process);
        }
      } else if(cur_process.duration != 0 && cur_process.time_allotment_at_level == 0){                   // job not complete, but used up timeslot
        cur_process.time_allotment_at_level = q2_timeslot;
        q2.push_back(cur_process);
      } else if(cur_process.duration != 0 && cur_process.time_allotment_at_level != 0){                    // job not complete, but did not use up timeslot
        q1.push_front(cur_process);
      }
      */
    
    } else if(q2.size() != 0){
      //rr_iteration(q2, q2_relieving_jobs, q3, q3_timeslot, timer, complete);
    } else if(q3.size() != 0 || Q3_has_job_to_continue){
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

    // Check if any processes that relieved CPU can be added back to queues
    if(q1_relieving_jobs.count(timer)){
      queue<Process> processes_to_add_back = q1_relieving_jobs[timer];
      q1_relieving_jobs.erase(timer);
      while (!processes_to_add_back.empty()) {
        q1.push_back(processes_to_add_back.front());
        processes_to_add_back.pop();
      }
    }
    if(q2_relieving_jobs.count(timer)){
      queue<Process> processes_to_add_back = q2_relieving_jobs[timer];
      q2_relieving_jobs.erase(timer);
      while (!processes_to_add_back.empty()) {
        q2.push_back(processes_to_add_back.front());
        processes_to_add_back.pop();
      }
    }
    if(q3_relieving_jobs.count(timer)){
      queue<Process> processes_to_add_back = q3_relieving_jobs[timer];
      q3_relieving_jobs.erase(timer);
      while (!processes_to_add_back.empty()) {
        q3.push(processes_to_add_back.front());
        processes_to_add_back.pop();
      }
    }
    if(q4_relieving_jobs.count(timer)){
      queue<Process> processes_to_add_back = q4_relieving_jobs[timer];
      q4_relieving_jobs.erase(timer);
      while (!processes_to_add_back.empty()) {
        q4.push(processes_to_add_back.front());
        processes_to_add_back.pop();
      }
    }

    // Check if it is time to boost all jobs that can be run to Q1
    if(timer % boost_timeslot == 0){
      // Boost jobs from a queue back to Q1
      boost_from_lower_deque(q2, q1, q1_timeslot);
      /*
      while(q2.size() != 0){
        Process item = q2.front();
        item.time_allotment_at_level = q1_timeslot;
        q1.push_back(item);
        q2.pop_front();
      }
      */

      boost_from_lower_pqueue(q3, q1, q1_timeslot);
      /*
      while(q3.size() != 0){
        Process item = q3.top();
        item.time_allotment_at_level = q1_timeslot;
        q1.push_back(item);
        q3.pop();
      }
      */
      boost_from_lower_pqueue(q4, q1, q1_timeslot);
      /*
      while(q4.size() != 0){
        Process item = q4.top();
        item.time_allotment_at_level = q1_timeslot;
        q1.push_back(item);
        q4.pop();
      }
      */
      if(Q3_has_job_to_continue){
        q1.push_back(cur_job_Q3);
      }
      if(Q4_has_job_to_continue){
        q1.push_back(cur_job_Q4);
      }
      Q3_has_job_to_continue = false;
      Q4_has_job_to_continue = false;
    }
  }

  // Return list of Processes ordered by completion time
  return complete;
}



float avg_turnaround(list<Process> processes) {
  int sum = 0;
  int div = 0;
  for(auto i: processes){
    Process c = i;
    sum += c.completion - c.arrival;
    div += 1;
  }
  return float(sum) / float(div);
}

float avg_response(list<Process> processes) {
  int sum = 0;
  int div = 0;
  for(auto i: processes){
    Process c = i;
    sum += c.first_run - c.arrival;
    div += 1;
  }
  return float(sum) / float(div);
}

void show_metrics(list<Process> processes) {
  float avg_t = avg_turnaround(processes);
  float avg_r = avg_response(processes);
  show_processes(processes);
  cout << '\n';
  cout << "Average Turnaround Time: " << avg_t << endl;
  cout << "Average Response Time:   " << avg_r << endl;
}