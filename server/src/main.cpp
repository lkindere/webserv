#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <signal.h>

#include "Webserv.hpp"

using namespace std;

#define MAX_RESTARTS 10

void check_leaks()
{
	system("leaks webserv");
}

void sigint(int i){ //Exit through terminal by typing exit
    (void)i;
}

int main(int argc, char *argv[]) {
	atexit(check_leaks);
    if (argc != 2)
        return 1;
    ConfigData conf(parseConfig(argv[1]));
    if (conf.status.success == 0){
        cout << "Parsing errorline: " << conf.status.error_line << '\n';
        cout << "Parsing error msg: " << conf.status.error_msg << std::endl;
        exit(1);
    }
    size_t restarts = 0;
    signal(SIGINT, sigint);
    while (restarts <= MAX_RESTARTS) {
        cout << "RESTARTS: " << restarts << endl;
        Webserv webserv(conf);
        if (webserv.init() != 0){
            ++restarts;
            sleep(1);
            continue;
        }
        while (1) {
            if (webserv.process() != 0){
                if (++restarts > MAX_RESTARTS){
                    restarts = 0;
                    break;
                }
                sleep(1);
                continue;
            }
            restarts = 0;
        }
    }
}