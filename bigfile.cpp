#include <iostream>
#include <fstream>

using namespace std;

int main(void){
    cout << "Content-Type: image/jpeg\n\n";
    ifstream in("bigfile.jpeg");
    cerr << "FILE IS OPEN: " << in.is_open() << endl;
    cout << in.rdbuf();
    cout.flush();
    return 0;
}