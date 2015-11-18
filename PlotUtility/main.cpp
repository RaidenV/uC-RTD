#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <iomanip>
#include <stdlib.h>
#include <ctime>
#include <sstream>
#include "DataScript.h"

using namespace std;

void GetData(string, unsigned int, vector<double>&, vector<double>&, vector<double>&);
void clearFile(string);
bool GenerateScriptFile(DataScript, vector<double>&, int, int);
bool GenerateDualScript(string, vector<double>&);
bool GenerateDualPlots(DataScript, DataScript);

const unsigned int DataLodeSize = 600;

int main(int argc, char** argv)
{
    string gnuScriptCmd = "gnuplot /home/raidenv/PIDPlot/GNUscript.sh", //The following are the locations wherein the script files lie;
           gnuScriptName = "/home/raidenv/PIDPlot/GNUscript.sh",
           bashFileName = "/home/raidenv/PIDPlot/PIDBash.sh",
           reBashFileName = "/home/raidenv/PIDPlot/Rerun.sh",
           recordName = "/home/raidenv/PIDPlot/MINICOMrecord.txt",
           dualPlotScriptName = "/home/raidenv/PIDPlot/MINICOMnvscript.txt";

    unsigned char choice;
    unsigned char cont;

    system(bashFileName.c_str()); //Run the first bash script;

    vector<double>* firstLooptime = new vector<double>; //Generate the necessary varibles for collecting the loop data;
    vector<double>* firstAngle = new vector<double>;
    vector<double>* firstVars = new vector<double>;

    GetData(recordName, DataLodeSize, *firstLooptime, *firstAngle, *firstVars); //Extract the data from the record;

    clearFile(recordName); //Very important: clear the minicom log file;

    DataScript* PlotDataFirst = new DataScript; //Create a new DataScript object;
    PlotDataFirst->AddTime(*firstLooptime); //Move the time samples from the extracted data to the object;
    PlotDataFirst->AddData(*firstAngle); //Move the angle samples from the extracted data to the object;
    PlotDataFirst->AddVar(*firstVars); //Move the kp, ki, and kd variables to the object;
    PlotDataFirst->Generate(); //Generate the data plot;

    if(GenerateScriptFile(*PlotDataFirst, *firstVars , PlotDataFirst->getSmallest(), PlotDataFirst->getLargest())) //Generate the GNUplot script file;
        cout << "GNU script generated successfully!" << endl;
    else
        cout << "GNU script generation failed.";

    delete firstLooptime; //Delete the unnecessary data;
    delete firstAngle;
    delete firstVars;

    cout << "Opening Plot..." << endl;

    system(gnuScriptCmd.c_str());  //Run the generated GNU script;

    cout << endl << "Run again using new values? [Y/N]";

    cin >> choice;

    if(toupper(choice) == 'Y')
    {
        do
        {
            DataScript* PlotDataSecond = new DataScript;
            vector<double>* secondVars = new vector<double>;
            double* tempVariable = new double;

            clearFile(gnuScriptName);

            cout << endl << "Enter the Kp value: ";
            cin >> *tempVariable;
            secondVars->push_back(*tempVariable);
            cout << endl << "Enter the Ki value: ";
            cin >> *tempVariable;
            secondVars->push_back(*tempVariable);
            cout << endl << "Enter the Kd value: ";
            cin >> *tempVariable;
            secondVars->push_back(*tempVariable);

            delete tempVariable;

            cout << endl << "Script written successfully!" << endl;

            GenerateDualScript(dualPlotScriptName, *secondVars);

            system(reBashFileName.c_str());

            vector<double>* secondLooptime = new vector<double>;
            vector<double>* secondAngle = new vector<double>;

            GetData(recordName, DataLodeSize, *secondLooptime, *secondAngle, *secondVars);

            clearFile(recordName);

            PlotDataSecond->AddTime(*secondLooptime);
            PlotDataSecond->AddData(*secondAngle);
            PlotDataSecond->AddVar(*secondVars);
            PlotDataSecond->Generate();

            if(GenerateDualPlots(*PlotDataFirst, *PlotDataSecond))
                cout << "Script generated successfully!" << endl;
            system(gnuScriptCmd.c_str());

            *PlotDataFirst = *PlotDataSecond;
            delete PlotDataSecond;
            delete secondLooptime;
            delete secondAngle;
            delete secondVars;

            cout << "Would you like to continue?(Y/N)" << endl;
            cin >> cont;
        }
        while(toupper(cont) == 'Y');
        return 0;
    }

    else
    {
        delete PlotDataFirst;
        return 0;
    }

}



void GetData(string recName, unsigned int dsize, vector<double>& dtime, vector<double>& dangle, vector<double>& dvars)
{
    string str;
    double tempAngle,
           tempTime,
           tempVariable;
    ifstream in;

    in.open(recName.c_str());


    while(str != "-=Begin=-")
    {
        getline(in, str);
        cout << str << endl;
    }
    for(size_t x = 0; x < dsize; x++)
    {
        in >> tempTime >> tempAngle;
        if(tempAngle > 300)
            tempAngle = tempAngle - 360;
        dtime.push_back(tempTime);
        dangle.push_back(tempAngle);
    }

    for(size_t x = 0; x < 3; x++)
    {
        in >> tempVariable;
        dvars.push_back(tempVariable);
    }

    in.close();
}

void clearFile(string filename)
{
    ofstream clearF;
    clearF.open(filename.c_str(), ios_base::trunc);
    clearF.close();
}

bool GenerateScriptFile(DataScript Plot, vector<double>& vars, int minVal, int maxVal)
{
    ofstream scriptFile;
    scriptFile.open("/home/raidenv/PIDPlot/GNUscript.sh", ios_base::trunc);
    scriptFile << "#!/user/local/bin/gnuplot -persist" << endl;
    scriptFile << "set xlabel \"Time (seconds)\"" << endl;
    scriptFile << "set ylabel \"Angle (degrees)\"" << endl;
    scriptFile << "set label 1 at 1, 30 \"Kp: " << vars[0] << "\"" << endl;
    scriptFile << "set label 2 at 1, 20 \"Ki: " << vars[1] << "\"" << endl;
    scriptFile << "set label 3 at 1, 10 \"Kd: " << vars[2] << "\"" << endl;
    scriptFile << "set xrange [0:1.2]" << endl;
    scriptFile << "set yrange [" << minVal << ":" << maxVal <<"]" << endl;
    scriptFile << "set title \"Pid Loop Plot: " << Plot.getTimeName() << "\"" << endl;
    scriptFile << "plot \"" << Plot.getFullName() << "\" using 1:2 with line lw 2 lt rgb \"blue\" notitle" << endl;
    scriptFile << "pause -1";
    scriptFile.close();
    return true;
}

bool GenerateDualScript(string newScriptName, vector<double>& vars)
{
    ofstream newFile;

    newFile.open(newScriptName.c_str(), ios_base::trunc);
    if(newFile.fail())
        return 0;
    else
    {
        newFile << "send \"kp=" << vars[0] << "\"" << endl;
        newFile << "send \"ki=" << vars[1] << "\"" << endl;
        newFile << "send \"kd=" << vars[2] << "\"" << endl;
        newFile << "send \"rec\"" << endl;
        newFile << "expect \"-=Begin=-\"" << endl;
        newFile << "expect \"-=End=-\"" << endl;
        newFile << "! killall minicom";
        newFile.close();
    }

    return true;
}

bool GenerateDualPlots(DataScript First, DataScript Second)
{
    ofstream scriptFile;
    scriptFile.open("/home/raidenv/PIDPlot/GNUscript.sh", ios_base::trunc);
    scriptFile << "#!/user/local/bin/gnuplot -persist" << endl;
    scriptFile << "set xlabel \"Time (seconds)\"" << endl;
    scriptFile << "set ylabel \"Angle (degrees)\"" << endl;
    scriptFile << "set label 1 \"Kp: " << Second.getVar(0) << "\" at .85, 45 tc rgb \"blue\"" << endl;
    scriptFile << "set label 2 \"Ki: " << Second.getVar(1) << "\" at .85, 35 tc rgb \"blue\"" << endl;
    scriptFile << "set label 3 \"Kd: " << Second.getVar(2) << "\" at .85, 25 tc rgb \"blue\"" << endl;
    scriptFile << "set label 4 \"Kp: " << First.getVar(0) << "\" at .85, 15 tc rgb \"red\"" << endl;
    scriptFile << "set label 5 \"Ki: " << First.getVar(1) << "\" at .85, 5 tc rgb \"red\"" << endl;
    scriptFile << "set label 6 \"Kd: " << First.getVar(2) << "\" at .85, -5 tc rgb \"red\"" << endl;
    scriptFile << "set xrange [0:1.2]" << endl;

    int* smallest = new int;
    int* largest = new int;

    if(Second.getSmallest() > First.getSmallest())
        *smallest = First.getSmallest();
    else
        *smallest = Second.getSmallest();

    if(Second.getLargest() < First.getLargest())
        *largest = First.getLargest();
    else
        *largest = Second.getLargest();


    scriptFile << "set yrange [" << *smallest << ":" << *largest <<"]" << endl;
    scriptFile << "set title \"Pid Loop plots: " << Second.getTimeName() << " vs. " << First.getTimeName() << "\"" << endl;
    scriptFile << "plot '" << Second.getFullName() << "' using 1:2 with line lw 2 lt rgb \"blue\" notitle,\\" << endl;
    scriptFile <<  "'" << First.getFullName() << "' using 1:2 with line lw 2 lt rgb \"red\" notitle" << endl;
    scriptFile << "pause -1";
    scriptFile.close();

    delete smallest;
    delete largest;

    return true;
}
