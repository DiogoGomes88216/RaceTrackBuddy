#ifndef CLAPTIMER_H
#define CLAPTIMER_H
#include<QTime>
#include "/home/diogo/Qt-Projects/dPVA_estimate/cgnss.h"
#include "csvFile.h"

typedef struct
{
    coordinates p,q;
}Line;
typedef struct
{
    Line sectors[3];
}Track;
typedef struct
{
    int sectors[3];
    int LapTime;
}Lap;

class cLapTimer
{
    Track currTrack;
public:
    ~cLapTimer();
    vector <Lap> LapBuff;
    int LapNumber;
    Lap BestLap;

    void SetTrack(Track newTrack);
    bool crossed(Line line, int timediff, int SectorNumber, int *Result);
    double distance(coordinates Pos1, coordinates Pos2);
    void saveSession(string sessionDate, int sessionStartTime);
    Lap getAvgLap();
    Lap getOptLap();
    string calculateGap(int lapTime);
};

#endif // CLAPTIMER_H
