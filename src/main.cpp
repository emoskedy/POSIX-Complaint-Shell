#include <iostream>
#include <vector>
#include <stack>
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

void cd_builtin(std::string input) {
  int i = 2, n = input.size(); // Mark i as the start of space character
  // Trying to reach the first character
  while (isspace(input[i])) {
    i++;
  }
  
  // Getting the current path
  char buf[PATH_MAX];
  std::string currPath = getcwd(buf, sizeof(buf));

  // Dealing with absolute paths
  if (input[i] == '/') {
    if (chdir(input.substr(i).c_str()) != 0) {
      chdir(currPath.c_str());
      std::string error = "cd: " + input.substr(3);
      perror(error.c_str());
    }
  }
  else if (input[i] == '~') {
    std::string path = getenv("HOME");
    chdir(path.c_str());
  }
  else { // Dealing with relative paths
    std::string dir; 
    std::stringstream ss;
    ss.str(input.substr(i));
    std::vector<std::string> directories;

    // Storing directories into a vector
    while (getline(ss, dir, '/')) {
      directories.push_back(dir);
    }
    
    int d = directories.size();
    std::vector<std::string> newP;
    ss.clear();
    ss.str(currPath.substr(1));

    while (getline(ss, dir, '/')) {
      newP.push_back(dir);
    }

    // Dealing with changing to parent or current directory
    for (int i = 0; i < d; i++) {
      if (directories[i] == "..") {
        newP.pop_back();
      }
      else {
        newP.push_back(directories[i]);
      }
    }

    // Putting all directories back into a string
    std::string newPath = "/";
    for (int i = 0; i < newP.size(); i++) newPath += newP[i] + "/";

    if (chdir(newPath.c_str()) != 0) {
      chdir(currPath.c_str());
      std::string error = "cd: " + newPath;
      perror(error.c_str());
    }
  }
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
    else if (input.find("pwd") == 0) { // pwd command
      char buf[PATH_MAX];
      std::cout << getcwd(buf, sizeof(buf)) << "\n";
    }
    else if (input.find("cd") == 0) { // cd command
      cd_builtin(input);
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
