#include "claptimer.h"
#include "math.h"

cLapTimer::~cLapTimer()
{
    LapBuff.clear();
}

void cLapTimer::SetTrack(Track newTrack)
{
    currTrack = newTrack;
}
bool cLapTimer::crossed(Line line, int timediff, int SectorNumber, int *Result)
{
    double distPtp, distPtf, speed;
    coordinates crossingPoint;
    float s1_x, s1_y, s2_x, s2_y;
    Line sector = currTrack.sectors[SectorNumber];

    s1_x = line.q.Lat - line.p.Lat;         s1_y = line.q.Lon - line.p.Lon;
    s2_x = sector.q.Lat - sector.p.Lat;     s2_y = sector.q.Lon - sector.p.Lon;

    float s, t;
    s = (-s1_y * (line.p.Lat - sector.p.Lat) + s1_x * (line.p.Lon - sector.p.Lon)) / (-s2_x * s1_y + s1_x * s2_y);
    t = ( s2_x * (line.p.Lon - sector.p.Lon) - s2_y * (line.p.Lat - sector.p.Lat)) / (-s2_x * s1_y + s1_x * s2_y);

    if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
    {
       crossingPoint.Lat = line.p.Lat + (t * s1_x);
       crossingPoint.Lon = line.p.Lon + (t * s1_y);

       distPtp = distance(line.p, line.q);
       distPtf = distance(crossingPoint, line.p);
       speed = distPtp / timediff;
       *Result = distPtf / speed;
       return true; // Line was crossed
    }
    return false; // Line was not Crossed
}
double toRadians(const long double degree)
{
    double one_deg = (M_PI) / 180;
    return (one_deg * degree);
}
double cLapTimer::distance(coordinates Pos1, coordinates Pos2)
{
    // Convert the latitudes
    // and longitudes
    // from degree to radians.
    Pos1.Lat = toRadians(Pos1.Lat);
    Pos1.Lon = toRadians(Pos1.Lon);
    Pos2.Lat = toRadians(Pos2.Lat);
    Pos2.Lon = toRadians(Pos2.Lon);

    // Haversine Formula
    double dlong = Pos2.Lon - Pos1.Lon;
    double dlat = Pos2.Lat - Pos1.Lat;

    double ans = pow(sin(dlat / 2), 2) + cos(Pos1.Lat) * cos(Pos2.Lat) * pow(sin(dlong / 2), 2);
    ans = 2 * asin(sqrt(ans));

    double R = 6371; // Radius of Earth in Kilometers

    // Calculate the result
    ans = ans * R;
    return ans * 1000; //result in meters
}

void cLapTimer::saveSession(string sessionDate, int sessionStartTime)
{
    string filePath = "/mnt/usb/Sessions/" + sessionDate + "_" + QTime(0,0).addMSecs(sessionStartTime).toString("hhmm").toStdString() + ".csv";

    int date = stoi(sessionDate);
    int day = date/10000;
    date -= (date/10000)*10000;
    int month = date/100;
    date -= (date/100)*100;
    int year = date;
    QDate tempDate(year,month,day);
    string dateLine[2] = {"Date", " " + tempDate.toString("dd/MM/yy").toStdString()};
    WriteCsv(filePath, dateLine, 2);
    string timeLine[2] = {"Session", " " + QTime(0,0).addMSecs(sessionStartTime).toString("hh:mm").toStdString()};
    WriteCsv(filePath, timeLine, 2);
    string laps[2] = {"Laps", " " + to_string(LapNumber -1)};
    WriteCsv(filePath, laps, 2);
    string fstLap[2] ={"Fastest_Lap"," " + QTime(0,0).addMSecs(BestLap.LapTime).toString("mm:ss.zzz").toStdString()};
    WriteCsv(filePath, fstLap, 2);
    string blank[1];
    WriteCsv(filePath, blank,1);
    string lapinfo[6] ={"Lap", " Lap_Time", " S1", " S2", " S3", " Gap"};
    WriteCsv(filePath, lapinfo, 6);

    if(LapNumber <= 1)
        return;
    Lap avg = getAvgLap();
    string AvgLap[6] ={"Avg", QTime(0,0).addMSecs(avg.LapTime).toString("mm:ss.zzz").toStdString(),
                       QTime(0,0).addMSecs(avg.sectors[0]).toString("ss.zzz").toStdString(),
                       QTime(0,0).addMSecs(avg.sectors[1]).toString("ss.zzz").toStdString(),
                       QTime(0,0).addMSecs(avg.sectors[2]).toString("ss.zzz").toStdString(),
                       calculateGap(avg.LapTime)};
    WriteCsv(filePath, AvgLap, 6);

    Lap opt = getOptLap();
    string OptLap[6] ={"Opt", QTime(0,0).addMSecs(opt.LapTime).toString("mm:ss.zzz").toStdString(),
                       QTime(0,0).addMSecs(opt.sectors[0]).toString("ss.zzz").toStdString(),
                       QTime(0,0).addMSecs(opt.sectors[1]).toString("ss.zzz").toStdString(),
                       QTime(0,0).addMSecs(opt.sectors[2]).toString("ss.zzz").toStdString(),
                       calculateGap(opt.LapTime)};
    WriteCsv(filePath, OptLap, 6);

    for(int i = 0 ; i < LapNumber-1; i++)
    {
        string Lap[6] ={to_string(i+1), QTime(0,0).addMSecs(LapBuff[i].LapTime).toString("mm:ss.zzz").toStdString(),
                                      QTime(0,0).addMSecs(LapBuff[i].sectors[0]).toString("ss.zzz").toStdString(),
                                      QTime(0,0).addMSecs(LapBuff[i].sectors[1]).toString("ss.zzz").toStdString(),
                                      QTime(0,0).addMSecs(LapBuff[i].sectors[2]).toString("ss.zzz").toStdString(),
                                      calculateGap(LapBuff[i].LapTime)};
        WriteCsv(filePath, Lap, 6);
    }
}

Lap cLapTimer::getAvgLap()
{
    Lap avg;
    for(int i = 0; i < LapNumber -1; i++)
    {
       avg.sectors[0] += LapBuff[i].sectors[0];
       avg.sectors[1] += LapBuff[i].sectors[1];
       avg.sectors[2] += LapBuff[i].sectors[2];
    }
    avg.sectors[0] = avg.sectors[0] / (LapNumber-1);
    avg.sectors[1] = avg.sectors[1] / (LapNumber-1);
    avg.sectors[2] = avg.sectors[2] / (LapNumber-1);
    avg.LapTime = avg.sectors[0] + avg.sectors[1] + avg.sectors[2];
    return avg;
}

Lap cLapTimer::getOptLap()
{
    Lap opt;
    opt.sectors[0] = LapBuff[0].sectors[0];
    opt.sectors[1] = LapBuff[0].sectors[1];
    opt.sectors[2] = LapBuff[0].sectors[2];

    for(int i = 0; i < LapNumber -1; i++)
    {
        if(LapBuff[i].sectors[0] < opt.sectors[0])
            opt.sectors[0] = LapBuff[i].sectors[0];
        if(LapBuff[i].sectors[1] < opt.sectors[1])
            opt.sectors[1] = LapBuff[i].sectors[1];
        if(LapBuff[i].sectors[2] < opt.sectors[2])
            opt.sectors[2] = LapBuff[i].sectors[2];
    }
    opt.LapTime = opt.sectors[0] + opt.sectors[1] + opt.sectors[2];
    return opt;
}

string cLapTimer::calculateGap(int lapTime)
{
    int gap = lapTime - BestLap.LapTime ;

    if(gap < 0)
       return QTime(0,0).addMSecs(-gap).toString("-ss.zzz").toStdString();
    else
       return QTime(0,0).addMSecs(gap).toString("+ss.zzz").toStdString();
}
