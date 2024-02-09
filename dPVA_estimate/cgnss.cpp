#include "cgnss.h"


char line[80];
char serialport;
FILE* f;
struct sigaction saio;
struct termios termAttr;

cGnss::cGnss()
{
    info = {'V',0,{0,0},0,0};
}
bool cGnss::Start()
{
    serialport = open("/dev/ttyAMA0", O_RDWR |O_NOCTTY| O_NDELAY); //open ttyAMA0, device driver for uart
    if (serialport < 0)
    {
        cout << "Unable to open /dev/ttyAMA0" << endl;
        return false;
    }

    if((f = fdopen(serialport, "r")) == NULL)
    {
        cout << "Unable to fdopen /dev/ttyAMA0" << endl;
        return false;
    }

    tcgetattr(serialport, &termAttr);

    cfsetispeed(&termAttr, B115200); //set the baudrate
    cfsetospeed(&termAttr, B115200); //set the baudrate
    termAttr.c_cflag &= ~(CRTSCTS | PARENB | CSTOPB | CSIZE);
    termAttr.c_cflag |= (CS8 | CLOCAL | CREAD);
    termAttr.c_lflag |= (ICANON);
    termAttr.c_lflag &= ~(ECHO | ECHOE | ISIG );
    termAttr.c_iflag &= ~(IXON | IXOFF | IXANY);
    termAttr.c_oflag &= ~OPOST;

    tcsetattr(serialport, TCSANOW, &termAttr);
    tcflush(serialport,TCIOFLUSH);
    return true;
}
double stringToDouble(string inputString){

    //If string empty, return 0.
    double returnValue = 0;
    std::istringstream istr(inputString);

    istr >> returnValue;

    return (returnValue);
}
float GpsToDecimalDegrees(string nmeaPos, string quadrant)
{
      char latLon[10];
      char deg[4] = {0};
      char *dot, *min;
      int len;
      float dec = 0;
      nmeaPos.copy(latLon,10,0);

      if ((dot = strchr(latLon, '.')))
      {                                         // decimal point was found
        min = dot - 2;                          // mark the start of minutes 2 chars back
        len = min - latLon;                     // find the length of degrees
        strncpy(deg, latLon, len);              // copy the degree string to allow conversion to float
        dec = atof(deg) + atof(min) / 60;       // convert to float
        if (quadrant == "S" || quadrant  == "W")
          dec *= -1;
      }
      return dec;
}
vector<string> splitStringByComma(string input)
{
    vector<string>  returnVector;
    stringstream    ss(input);
    string          element;

    while(std::getline(ss, element, ',')) {
        returnVector.push_back(element);
    }
    return returnVector;
}
void cGnss::setRMC(const string RMCSentence)
{
    vector<std::string> elementVector = splitStringByComma(RMCSentence);

    info.Time               = stringToDouble(elementVector[1]);
    info.Status             = elementVector[2][0];
    info.Position.Lat       = GpsToDecimalDegrees(elementVector[3], elementVector[4]);
    info.Position.Lon       = GpsToDecimalDegrees(elementVector[5], elementVector[6]);
    info.Speed              = stringToDouble(elementVector[7]) * KNOTS_TO_KMH;
    info.Date               = stoi(elementVector[9]);
}
bool isValidRMC(const string RMCSentence)
{
    bool returnBool = true;
    vector<std::string> elementVector = splitStringByComma(RMCSentence);

    if (elementVector[0] != "$GPRMC")               returnBool = false;
//    if (elementVector[2][0] != 'A')                 returnBool = false;
//    if (elementVector[3].length() < MINUTE_LENGTH)  returnBool = false;
//    if (elementVector[5].length() < MINUTE_LENGTH)  returnBool = false;

    return returnBool;
}

void cGnss::UpdateGnss()
{
    if(fgets(line, sizeof line, f) != NULL)
    {
        if(isValidRMC(line))
        {
            setRMC(line);
            fgets(line, sizeof line, f);
        }
    }
}
