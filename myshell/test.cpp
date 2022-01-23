#include <thread>
#include <iostream>
using namespace std;

void hello() {
    cout << "Hello concurrent world!" << endl;
}

int main() {
    thread t(hello);
    t.join();

    return 0;
}
