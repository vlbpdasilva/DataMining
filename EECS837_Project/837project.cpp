#include <chrono>
#include <ctime>



#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include "lem.cpp"
using namespace std;

vector<string> attributes, decisions;
vector<vector<string>> data;
string decision;
void defineAttributes(string &a);
int numAttributes, numCols, numRows = 0;

void printIntro()
{
    cout << "\n\n-----------\nEECS 837 PROJECT - Data Mining\n   Victor Berger da Silva\n   KUID 2770737\n-----------\n\n";
}

bool lineIsComment(string &a)
{
    for(int p = 0; p < a.length(); ++p)
        if(a.at(p) == ' ')
            continue;
        else if(a.at(p) == '!')
            return 1;
    return 0;
}

bool lineIsAttributes(string &a)
{
    for(int p = 0; p < a.length(); ++p)
        if(a.at(p) == ' ')
            continue;
        else if(a.at(p) == '[')
        {
            defineAttributes(a);
            return 1;
        }
    return 0;
}

void defineAttributes(string &a)
{
    string word = "";
    for(int k = 0; k < a.length(); ++k)
    {
        if(a[k] == '[' || a[k] == ']')
            continue;
        if(a[k] == ' ' && !word.empty())
        {
            attributes.push_back(word);
            word.clear();
        }
        else if(a[k] != ' ')
            word += a[k];
    }
    if(!word.empty())
        data[numRows].push_back(word);
}

void addData(string &a)
{
    vector<string> row;
    data.push_back(row);
    string word = "";
    for(int k = 0; k < a.length(); ++k)
    {
        if(a[k] == ' ' && !word.empty())
        {
            data[numRows].push_back(word);
            word.clear();
        }
        else if(a[k] != ' ')
            word += a[k];
    }  
    
    if(!word.empty())
        data[numRows].push_back(word);
    
    ++numRows;    
}

int main()
{  
    printIntro();
    int k;
    bool run = 1;
    string inputFileName, inputFileName_parsed, outputFileName, outputFileName_certain, outputFileName_possible, line;
    ifstream infile;     
    
    while(1)
    {
        cout << "\nEnter name of input data file: ";
        getline(cin, inputFileName);
        if(inputFileName.empty() || inputFileName == "")
        {
            cout << "Input file name empty, please try again.";
            continue;
        }
        infile.open(inputFileName);
        if(!infile.is_open() || !infile.good())
        {
            cout << "Input file not found, please try again.";
            continue;
        }
        break;
    }
    
    for(k = inputFileName.length() - 1; k > 0; --k)
        if(inputFileName.at(k) == '.')
            break;
        
    inputFileName_parsed = k > 0 ? inputFileName.substr(0, k) : inputFileName;
    
    cout << "\nEnter name of output  file with rules: ";
    getline(cin, outputFileName);
    
    if(outputFileName.empty())
        cout << "Empty output file name detected. Using default value.\n" << endl;
    
    chrono::steady_clock::time_point start = chrono::steady_clock::now();
    
    outputFileName_certain = outputFileName.empty() ? inputFileName_parsed : outputFileName;
    outputFileName_possible = outputFileName.empty() ? inputFileName_parsed : outputFileName;
    outputFileName_certain.append(".certain.r");
    outputFileName_possible.append(".possible.r");
    
    while(getline(infile, line))
    { 
        if(line.empty() || lineIsComment(line))
            continue;
        if(lineIsComment(line))
            continue;
        for(k = 0; k < line.length(); ++k)
        {
            if(line.at(k) == ' ')
                continue;
            if(line.at(k) == '<')
            {
                run = 0;
                break;
            }
        }
        if(!run)
            break;
    }
    while(getline(infile, line))
    {
        if(line.empty() || lineIsComment(line))
            continue;
        if(lineIsAttributes(line))
            break;
    }

    decision = attributes.back();
    attributes.erase(attributes.end());
    numAttributes = attributes.size();
    
    while(getline(infile, line))
    {
        if(lineIsComment(line) || line.empty() || line.find_first_not_of(' ') == string::npos)
            continue;
        addData(line);
    }

    if(numRows < 1)
    {
        cout << "\nError: input data file contains empty dataset.\n";
        return 1;
    }
    
    numCols = data[0].size();
    
    for(int k = 0; k < numRows; ++k)
    {
        decisions.push_back(data[k][numCols - 1]);
        data[k].erase(data[k].end());
    }
    
    numCols = data[0].size();
    
    lem obj(data, attributes, decisions, decision, numRows, numCols, outputFileName_certain, outputFileName_possible); 
    
    infile.close();
    
    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    
    cout << "\nProgram execution took " << chrono::duration_cast<chrono::milliseconds>(end-start).count() << " miliseconds.\n" << endl;
    
    
    return 0;
    
}
