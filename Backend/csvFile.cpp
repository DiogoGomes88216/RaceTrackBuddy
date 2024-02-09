#include "csvFile.h"

std::fstream& GotoLine(std::fstream& file, unsigned int num)
{
    file.seekg(std::ios::beg);
    for(uint i=0; i < num - 1; ++i){
        file.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
    }
    return file;
}
vector<string> listFiles(string path )
{
   DIR* dirFile = opendir(path.c_str());
   vector<string> returnVector;
   if ( dirFile )
   {
      struct dirent* hFile;
      while (( hFile = readdir( dirFile )) != NULL )
      {
         if ( strstr( hFile->d_name, ".csv" ))
            returnVector.push_back(path + hFile->d_name);
      }
      closedir( dirFile );
   }
   return returnVector;
}
vector<string> read_header(string path)
{
    // File pointer
    fstream fin;
    // Open an existing file
    fin.open(path,ios::in);
    if(fin.fail())
    {
        cout << "Input file opening failed.\n";
        exit(1);
    }
    vector<string> headerInfo;

    string line, temp;

    while (fin >> temp)
    {
        fin >>line;
        headerInfo.push_back(line);
    }
    fin.close();
    return headerInfo;
}
vector<string> read_laps(string path)
{
    vector<string> returnVector;
    fstream fin;
    fin.open(path,ios::in);
    if(fin.fail())
    {
        cout << "Input file opening failed.\n";
        exit(1);
    }
    string line,word;
    GotoLine(fin,7);

    while (getline(fin, line))
    {
       stringstream s(line);

       while (getline(s, word, ','))
       {
           returnVector.push_back(word);
       }
    }
    fin.close();
    return returnVector;
}
vector<string> read_tracks(string path)
{
    vector<string> returnVector;
    fstream fin;
    fin.open(path,ios::in);
    if(fin.fail())
    {
        cout << "Input file opening failed.\n";
        exit(1);
    }
    string line,word;
    GotoLine(fin,2);

    while (getline(fin, line))
    {
       stringstream s(line);

       while (getline(s, word, ','))
       {
           returnVector.push_back(word);
       }
    }
    fin.close();
    return returnVector;
}
string makeFile_Path(string path, string time, string date)
{
    path.append("Session_");
    path.append(date + "_" + time + ".csv");
    return path;
}
bool WriteCsv(string file_path, string *buff, int length)
{
    fstream file;
    file.open(file_path, ios::out|ios::app);
    if(file.fail())
    {
        cout<<"Couldn't open File"<<endl;
    }
    for(int i = 0; i < length; i++)
    {
        file << buff[i];

        if(i != length - 1)
        file << ",";
    }
    file << "\n";
    file.close();
    return true;
}
