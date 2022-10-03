#include "Config.hpp"

#include <iostream>

int main(void){
    ConfigData ret(parseConfig("new.conf"));
    std::cout << "Parsing succesful: " << ret.status.success << '\n';
    std::cout << "Parsing errorline: " << ret.status.error_line << '\n';
    std::cout << "Parsing error msg: " << ret.status.error_msg << std::endl;

    return 0;
}