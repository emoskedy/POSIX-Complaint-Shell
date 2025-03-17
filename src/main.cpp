#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <sstream>
#include <filesystem>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>
namespace fs = std::filesystem;

std::string getPath(std::string command) {
  char* path = getenv("PATH");
  std::stringstream ss(path);
  std::string dir;
  while (!ss.eof()) {
    getline(ss, dir, ':');
    std::string absPath = dir + '/' + command;
    if (fs::exists(absPath)) {
      return absPath;
    }
  }
  return "";
}

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  
  while (true) {
    std::cout << "$ ";
    
    std::string input;
    std::getline(std::cin, input);
    
    if (input.find("exit") == 0) { // exit command
      return 0;
    } 
    else if (input.find("echo") == 0) { // echo command
      std::cout << input.substr(5) << "\n";
    }
    else if (input.find("type") == 0) { // type command
      if (input.size() > 4 && input[5] != ' ') {
        std::string commandType = input.substr(5);
        std::set<std::string> st = {"echo", "exit", "type", "pwd"};
        if (st.find(commandType) != st.end()) { // If it's a builtin command
          std::cout << commandType << " is a shell builtin\n";
        }
        else { // Getting path for executable files
          std::string path = getPath(commandType);
          if (path != "") {
            std::cout << commandType << " is " << path << "\n";
          }
          else {
            std::cout << commandType << ": not found\n";
          }
        }
      }
    }
    else if (input.find("pwd") == 0) {
      char buf[PATH_MAX];
      std::cout << getcwd(buf, sizeof(buf)) << "\n";
    }
    else if (input.find("cd") == 0) {
      chdir("/");
      if (chdir(input.substr(3).c_str()) != 0) {
        std::string error = "cd: " + input.substr(3);
        perror(error.c_str());
      }
    }
    else { // Not builtin command, could be trying to run a program or the command/program is not found
      std::stringstream ss(input);
      std::string program;
      std::vector<std::string> args;
      while (ss >> program) {
        if (program[0] == '"') {
          std::string arg = program;
          while (ss >> program && program[program.size() - 1] != '"') {
            arg += " " + program;
          }
          if (arg != program) arg += " " + program;
          arg = arg.substr(1);
          args.push_back(arg.substr(0, arg.size() - 1));
        }
        else args.push_back(program);
      }
      
      std::vector<char*> argv;
      for (auto &arg : args) { 
        argv.push_back(const_cast<char*>(arg.c_str()));
      }
      argv.push_back(nullptr);
      
      std::string path = getPath(argv[0]);
      // If the program is not found
      if (path == "") {
        std::cout << argv[0] << ": command not found\n";
        continue;
      }

      // If found, create a child process so it won't terminate the program when execute
      pid_t pid = fork();
      if (pid == 0) { // Child process
        execvp(argv[0], argv.data());
      }
      else if (pid > 0) { // Parent process
        int status;
        waitpid(pid, &status, 0);
      }
      else { // cannot fork
        perror("fork failed");
      }
    }
  }
  return 0; 
}
