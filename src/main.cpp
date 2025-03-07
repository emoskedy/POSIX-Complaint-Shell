#include <iostream>

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  while (true) {
    std::string input;
    std::cout << "$ ";
    std::getline(std::cin, input);
    if (input.find("exit") == 0) {
      break;
    } 
    else if (input.find("echo") == 0) {
      std::cout << input.substr(5) << "\n";
    }
    else if (input.find("type") == 0) {
      if (input.substr(5) == "echo" || input.substr(5) == "exit" || input.substr(5) == "type") {
        std::cout << input.substr(5) << " is a shell builtin\n";
      }
      else {
        std::cout << input.substr(5) << ": not found\n";
      }
    }
    else std::cout << input << ": command not found\n";
  }
  return 0; 
}
