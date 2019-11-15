#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  //----------------
  // Accessors
  //----------------
  int Pid();
  std::string User();
  std::string Command();
  float CpuUtilization();
  std::string Ram();
  long int UpTime();
  
  //----------------
  // Mutators
  //----------------
  void Pid(int);
  void User(std::string);
  void Command(std::string);
  void CpuUtilization(float);
  void Ram(std::string);
  void UpTime(long int);
  
  //---------------------
  // Operator overload
  //---------------------
  bool operator<(Process const& a) const;  // TODO: See src/process.cpp
  
 private:
  int pid_{0};
  std::string user_{""};
  std::string command_{""};
  float cpu_utilization_{0.0};
  std::string ram_{""};
  long up_time_{0};
};

#endif