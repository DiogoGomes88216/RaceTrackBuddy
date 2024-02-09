#ifndef CFILESAVER_H
#define CFILESAVER_H

#include <string>
#include <fstream>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>


using namespace std;

class cFileSaver
{
private:
    fstream file;
    string file_path;
public:
    cFileSaver();
    ~cFileSaver();

    void setFile_Path(string time, string date);
    bool WriteCsv(string * buff, int length);
};

#endif // CFILESAVER_H
