#ifndef CSVFILE_H
#define CSVFILE_H
#include <iostream>
#include <filesystem>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <string>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <limits>
using namespace std;

std::fstream& GotoLine(std::fstream& file, unsigned int num);
vector<string> listFiles(string path );
vector<string> read_header(string path);
vector<string> read_laps(string path);
vector<string> read_tracks(string path);
string makeFile_Path(string path, string time, string date);
bool WriteCsv(string file_path, string *buff, int length);

#endif // CSVFILE_H
