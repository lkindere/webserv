#include <stdlib.h>

#include <iostream>

#include "Webserv.hpp"

int main(int argc, char *argv[]) {
  if (argc != 2) return 1;

  ConfigData conf(parseConfig(argv[1]));
  std::cout << "Parsing succesful: " << conf.status.success << '\n';
  std::cout << "Parsing errorline: " << conf.status.error_line << '\n';
  std::cout << "Parsing error msg: " << conf.status.error_msg << std::endl;
  if (conf.status.success == 0)
    exit(1);
  Webserv webserv(conf);
  if (webserv.init() != 0) exit(1);
  while (1) {
    if (webserv.accept() != 0) exit(1);
    if (webserv.process() != 0) exit(1);
  }
  return 0;
}