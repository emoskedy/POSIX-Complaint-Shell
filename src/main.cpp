#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <sstream>
#include <filesystem>
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
    
    if (input.find("exit") == 0) {
      break;
    } 
    else if (input.find("echo") == 0) {
      std::cout << input.substr(5) << "\n";
    }
    else if (input.find("type") == 0) {
      std::string commandType = input.substr(5);
      std::set<std::string> st = {"echo", "exit", "type"};
      if (st.find(commandType) != st.end()) {
        std::cout << commandType << " is a shell builtin\n";
      }
      else {
        std::string path = getPath(commandType);
        if (path != "") {
          std::cout << commandType << " is " << path << "\n";
        }
        else {
          std::cout << commandType << ": not found\n";
        }
      }
    }
    else {
      std::cout << input << ": command not found\n";
    }
    // for (std::string a : tokens) std::cout << a << "\n";
  }
  return 0; 
}
