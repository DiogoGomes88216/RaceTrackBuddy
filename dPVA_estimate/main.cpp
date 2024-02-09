#include "dpvaestimate.h"

using namespace std;

int main()
{
    dPvaEstimate PvaEstimate;
    cout<<"STARTED"<<endl;
    PvaEstimate.make_daemon();

    if(!PvaEstimate.run())
        cout << "Error" << endl;

    pthread_exit(nullptr);
    return 0;
}
