#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <sstream>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

//---------------------------------
// System
//---------------------------------

string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

string LinuxParser::Kernel() {
  string skip;
  string kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> skip >> skip >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Reads and returns the system memory utilization
float LinuxParser::MemoryUtilization() { 
  string skip;
  string temp;
  string line;
  vector<string> memory;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    for (int i = 0; i < 2; ++i) {
      std::getline(stream, line);
      std::istringstream linestream(line);
      linestream >> skip >> temp >> skip;
      memory.push_back(temp);
    }
  }
  float mem_total = std::stof(memory[0]);
  float mem_free = std::stof(memory[1]);
  return (mem_total - mem_free) / mem_total;
}

// Reads and returns the system uptime
long LinuxParser::UpTime() { 
  long uptime = 0.0;
  string temp = "0.0";
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> temp;
  }
  uptime = std::stol(temp);
  return uptime; 
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { 
  vector<string> jiffies = CpuUtilization();
  long t_jiffies = 0;
  for(string jiffie : jiffies) {
    t_jiffies += std::stoi(jiffie);
  }
  return t_jiffies;
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { 
  long a_jiffies = Jiffies() - IdleJiffies();
  return a_jiffies;
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { 
  vector<string> jiffies = CpuUtilization();
  long idle = std::stoi(jiffies[3]);
  long iowait = std::stoi(jiffies[4]);
  long i_jiffies = idle + iowait;
  return i_jiffies;
}

// Reads and returns CPU utilization
vector<string> LinuxParser::CpuUtilization() { 
  vector<string> timers;
  string timer;
  string line;
  string skip;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line); 
    linestream >> skip;
    for(int i = 0; i < 10; ++i) {
      linestream >> timer;
      timers.push_back(timer);
    }
  }
  return timers; 
}

//---------------------------------
// Process
//---------------------------------

// Reads and returns the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) { 
  string utime;
  string stime;
  string line;
  string skip;
  std::ifstream stream(kProcDirectory + std::to_string(pid)+ kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line); 
    for(int i = 1; i < 13; ++i) {
      linestream >> skip;
    }
    linestream >> utime >> stime;
  }
  long a_jiffies = (std::stol(utime) + std::stol(stime));
  return a_jiffies;
}

// Parses the file in the path for a given token as a key to a value
string LinuxParser::SystemProcesses(string token, string path) {
  string processes = "n/a";
  bool search = true;
  string line;
  string temp;
  std::ifstream stream(path);
  if(stream.is_open()) {
    while(search == true && stream.peek() != EOF) {
      std::getline(stream, line);
      std::istringstream linestream(line); 
      linestream >> temp;
      if(temp == token) {
        linestream >> temp;
        processes = temp;
        search = false;
      } // End inner if
    } // End while
  } // End outer if
  return processes; 
}

// Reads and returns the total number of processes
int LinuxParser::TotalProcesses() { 
  string path = kProcDirectory + kStatFilename;
  string result = LinuxParser::SystemProcesses("processes", path);
  return std::stoi(result);
}

// Reads and returns the number of running processes
int LinuxParser::RunningProcesses() { 
  string path = kProcDirectory + kStatFilename;
  string result = LinuxParser::SystemProcesses("procs_running", path);
  return std::stoi(result);
}

// Reads and returns the command associated with a process
string LinuxParser::Command(int pid) { 
  string line = "n/a";
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
  }
  return line; 
}

// Reads and returns the memory used by a process
string LinuxParser::Ram(int pid) { 
  string path = kProcDirectory + "/" + std::to_string(pid) + kStatusFilename;
  return LinuxParser::SystemProcesses("VmSize:", path);
}

// Reads and returns the user ID associated with a process
string LinuxParser::Uid(int pid) { 
  string path = kProcDirectory + "/" + std::to_string(pid) + kStatusFilename;
  return LinuxParser::SystemProcesses("Uid:", path);
}

// Reads and returns the user associated with a process
string LinuxParser::User(int pid) {
  string line;
  string test_user;
  string test_uid;
  string skip;
  string user = "n/a";
  string uid = LinuxParser::Uid(pid);
  std::ifstream stream(kPasswordPath);
  bool search = true;
  if (stream.is_open()) {
    while(search && stream.peek() != EOF ) {
      std::getline(stream, line);
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line); 
      linestream >> test_user >> skip >> test_uid;
      if(uid == test_uid) {
        user = test_user;
        search = false;
      }
    }
  }
  return user; 
}

// Reads and returns the uptime of a process
long LinuxParser::UpTime(int pid) { 
  long ticks = 0;
  string line;
  string skip;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line); 
    for(int i = 1; i < 22; ++i) {
      linestream >> skip;
    }
    linestream >> ticks;
  }
  return ticks / sysconf(_SC_CLK_TCK);
}