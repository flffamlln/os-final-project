#include <gtest/gtest.h>
#include <scheduling.h>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

TEST(SchedulingTest, SchedulingTest1) {
  pqueue_arrival pq = read_workload("workloads/workload_01.txt");
  EXPECT_EQ(pq.size(), 3);
}

TEST(SchedulingTest, SchedulingTest2) {
  pqueue_arrival pq = read_workload("workloads/not_a_file.txt");
  EXPECT_EQ(pq.size(), 0);
}

TEST(SchedulingTest, SchedulingTest3) {
  pqueue_arrival pq = read_workload("workloads/workload_01.txt");

  ASSERT_FALSE(pq.empty());
  Process x1 = pq.top();
  pq.pop();
  EXPECT_EQ(x1.arrival, 0);

  ASSERT_FALSE(pq.empty());
  Process x2 = pq.top();
  pq.pop();
  EXPECT_EQ(x2.arrival, 0);

  ASSERT_FALSE(pq.empty());
  Process x3 = pq.top();
  pq.pop();
  EXPECT_EQ(x3.arrival, 0);
}

TEST(SchedulingTest, SchedulingTest4) {
  pqueue_arrival pq = read_workload("workloads/workload_01.txt");

  ASSERT_FALSE(pq.empty());
  Process x1 = pq.top();
  pq.pop();
  EXPECT_EQ(x1.duration, 10);

  ASSERT_FALSE(pq.empty());
  Process x2 = pq.top();
  pq.pop();
  EXPECT_EQ(x2.duration, 10);

  ASSERT_FALSE(pq.empty());
  Process x3 = pq.top();
  pq.pop();
  EXPECT_EQ(x3.duration, 10);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
