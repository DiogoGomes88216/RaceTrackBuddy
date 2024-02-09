#include "cfilesaver.h"
cFileSaver::cFileSaver()
{

}
cFileSaver::~cFileSaver()
{
    file.close();
}

void cFileSaver::setFile_Path(string time, string date)
{
    string path = "/mnt/usb/Pva/PVA_";
    path.append(date + "_" + time + ".csv");
    file_path = path;
}
bool cFileSaver::WriteCsv(string *buff, int length)
{
    file.open(file_path, ios::out|ios::app);

    for(int i = 0; i < length; i++)
    {
        file << buff[i];

        if(i != length - 1)
        file << ", ";
    }
    file << "\n";
    file.close();
    return true;
}
