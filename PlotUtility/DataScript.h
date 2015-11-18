#ifndef DATASCRIPT_H_INCLUDED
#define DATASCRIPT_H_INCLUDED

#include <cstring>
#include <string>
#include <vector>
#include <ctime>
#include <sstream>
#include <fstream>


class DataScript
{
private:
    std::string fullname;
    std::string timename;
    std::vector<double> data;
    std::vector<double> timedata;
    std::vector<double> vardata;

public:
    DataScript()
    {
        time_t t = time(0);   // get time now
        struct tm* now = localtime( & t );

        std::string hour = DataScript::IntToString(now->tm_hour);
        std::string minute = IntToString(now->tm_min);
        std::string second = IntToString(now->tm_sec);
        std::string month = IntToString(now->tm_mon + 1);
        std::string day = IntToString(now->tm_mday);
        std::string year = IntToString(now->tm_year + 1900);

        fullname = "/home/raidenv/PIDPlot/Plots/Plot " + hour + "." + minute + "." + second + " " + month + "-" + day + "-" + year + ".dat";
        timename = "Plot " + hour + "." + minute + "." + second + " " + month + "-" + day + "-" + year;
    }

    std::string getFullName(void)
    {
        return fullname;
    }

    std::string getTimeName(void)
    {
        return timename;
    }

    double getVar(unsigned int x)
    {
        return vardata[x];
    }

    ~DataScript();

    void AddData(std::vector<double>&);
    void AddTime(std::vector<double>&);
    void AddVar(std::vector<double>&);
    void Generate(void);
    int getLargest(void);
    int getSmallest(void);


    std::string IntToString(int a)
    {
        std::ostringstream ss;
        ss << a;
        return ss.str();
    }
};


DataScript::~DataScript()
{}

void DataScript::AddData(std::vector<double>& newData)
{
    for(size_t x = 0; x < newData.size(); x++)
    {
        DataScript::data.push_back(newData[x]);
    }
}


void DataScript::AddTime(std::vector<double>& newTime)
{
    for(size_t x = 0; x < newTime.size(); x++)
    {
        DataScript::timedata.push_back(newTime[x]);
    }
}

void DataScript::AddVar(std::vector<double>& newVar)
{
    for(size_t x = 0; x < newVar.size(); x++)
    {
        DataScript::vardata.push_back(newVar[x]);
    }
}

void DataScript::Generate(void)
{
    std::ofstream dataFile;
    dataFile.open(DataScript::fullname.c_str());
    dataFile << "#" << DataScript::timename << std::endl;
    dataFile << "# PID loop" << std::endl;
    dataFile << "# Kp: " << DataScript::vardata[0] << std::endl;
    dataFile << "# Ki: " << DataScript::vardata[1] << std::endl;
    dataFile << "# Kd: " << DataScript::vardata[2] << std::endl;

    for(size_t z = 0; z < DataScript::data.size(); z++)
    {
        dataFile << DataScript::timedata[z] << "    " << DataScript::data[z] << std::endl;
    }

    dataFile.close();
}

int DataScript::getLargest(void)
{
    int temp = 0;
    for(size_t x = 0; x < DataScript::data.size(); x++)
    {
        if(DataScript::data[x] > temp)
            temp = DataScript::data[x];
    }

    return (temp + 10);
}

int DataScript::getSmallest(void)
{
    int temp = 0;
    for(size_t x = 0; x < DataScript::data.size(); x++)
    {
        if(DataScript::data[x] < temp)
            temp = DataScript::data[x];
    }

    return (temp - 10);
}

#endif // DATASCRIPT_H_INCLUDED
