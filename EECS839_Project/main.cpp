#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <fstream>
#include <algorithm>
#include "helper.cpp"
using namespace std;

int ruleRows, dataRows;
ifstream rfile, dfile;
vector<string> att;
vector<vector<int>> number;
vector<vector<string>> rules, data;
 
void printIntro()
{
    cout << "\n-----------\nEECS 839 PROJECT - Special Data Mining\n   Victor Berger da Silva\n   KUID 2770737\n   victorberger@ku.edu\n-----------\n\n";
}

bool parseLine(string &s, char key)
{
    for(int k = 0; k < s.length(); ++k)
    {
        if(s.at(k) == ' ')
            continue;
        if(key != 'n' && s.at(k) == key)
            return 1;
        if(key == 'n' && isdigit(s.at(k)))
            return 1;
        return  0;
    }
}

vector<string> parseRule(string &str)
{
    int start;
    string subs;
    vector<string> v;
    for(int k = 0; k < str.length(); ++k)
    {
        if(str.at(k) == ' ')
            continue;
        if(str.at(k) == '(')
            start = k;
        else if(str.at(k) == ')')
            v.push_back(str.substr(start + 1, k - start - 1));      
    }
    return v;
}

void parseAttributes(string &str)
{
    string s;
    int start = 0;
    for(int k = 0; k < str.length(); ++k)
    {
        if(str.at(k) == ' ')
        {
            s = str.substr(start + 1, k - start - 1); 
            s.erase(remove(s.begin(), s.end(), ' '), s.end());
            if(!s.empty())
                att.push_back(s);
            start = k;
        }
    }
}

vector<string> parseData(string &str)
{
    string buf;
    vector<string> v;    
    stringstream ss(str);
    
    while(ss >> buf)
    {
        buf.erase(remove(buf.begin(), buf.end(), ' '), buf.end());
        if(!buf.empty())
            v.push_back(buf);
    }
    return v;
}

bool checkNumbers(string &str)
{
    string subs;
    int start = 0;
    vector<int> v;
    
    for(int k = 0; k < str.length(); ++k)
    {
        if(str.at(k) == ',')
        {
            subs = str.substr(start, k - start);
            if(subs.empty())
                return 1;
            for(int l = 0; l < subs.length(); ++l)
                if(!isdigit(subs[l]))
                    return 1;
            v.push_back(stoi(subs));
            start = k + 1;
        }       
    }
    
    subs = str.substr(start, str.length() - 1);
    if(subs.empty())
        return 1; 
    v.push_back(stoi(subs));
    number.push_back(v);

    if(v.size() != 3)
        return 1;    
    
    return 0;
}

int parseRulesFile(string &filename)
{
    string line;
    bool num = 1;
    vector<string> row;
    while(getline(rfile, line))
    {
        if(line.empty() || parseLine(line, '!'))
            continue;
        
        line.erase(remove(line.begin(), line.end(), ' '), line.end());

        if(parseLine(line, 'n') && num == 1)
        {
            if(checkNumbers(line))
                return 1;
            rules.push_back(row);
            num = 0;
            continue;
        }
        else if(parseLine(line, 'n'))
            return 1;

        if(parseLine(line, '(') && num == 0)
        {
            vector<string> v = parseRule(line);
            rules[ruleRows].insert(rules[ruleRows].end(), v.begin(), v.end());
            ++ruleRows;
            num = 1;
            continue;
        } 
        else if(parseLine(line, '('))
            return 1;
    }        
    return 0;
}

void parseDataFile(string &filename)
{
    string line;
    bool attf = 0, initf = 0;
    while(getline(dfile, line))
    {
        if(line.empty() || parseLine(line, '!'))
            continue;
        if(!initf && parseLine(line, '<'))
        {
            initf = 1;
            continue;
        }
        if(initf && parseLine(line, '['))
        {
            parseAttributes(line);
            attf = 1;
            continue;
        }
        if(attf)
        { 
            data.push_back(parseData(line));
            ++dataRows;
        }
    }    
}

bool checkMatching()
{
    for(int k = 0; k < number.size(); ++k)
        if(number[k].at(2) == 0)
            return 0;
    return 1;
}

int main()
{  
    printIntro();    
    char dummy;
    string str, rulesFileName, dataFileName;
    bool match, strength, spec, supp, concept_stats, cases_stats, repeat = 1; 
    
    while(repeat)
    {
        ruleRows = 0; 
        dataRows = 0;
        att.clear();
        number.clear();
        rules.clear(); 
        data.clear();
        
        while(1)
        {
            if(rfile.is_open())
                rfile.close();
            cout << "Enter name of rules file: ";
            getline(cin, rulesFileName);
            if(rulesFileName.empty())
            {
                cout << "   ERROR: Rules file name empty, please try again." << endl;
                continue;
            }
            rfile.open(rulesFileName);
            if(!rfile.is_open() || !rfile.good())
            {
                cout << "   ERROR: Rules file not found, please try again." << endl;
                continue;
            }
            if(parseRulesFile(rulesFileName))
            {
                cout << "   ERROR: Rule file is not in the proper format." << endl;
                continue;
            }
            break;
        }
        while(1)
        {
            if(dfile.is_open())
                dfile.close();
            cout << "Enter name of data file:  ";
            getline(cin, dataFileName);
            if(dataFileName.empty())
            {
                cout << "   ERROR: data file name empty, please try again." << endl;
                continue;
            }
            dfile.open(dataFileName);
            if(!dfile.is_open() || !dfile.good())
            {
                cout << "   ERROR: data file not found, please try again." << endl;
                continue;
            }
            parseDataFile(dataFileName);
            break;
        }
        
        rfile.close();
        dfile.close();
        cout << flush;
        
        while(1)
        {
            str = "";
            cout << "\nUse matching factor? [y or RETURN]  "; 
            getline(cin, str);
            str.erase(remove(str.begin(), str.end(), ' '), str.end());
            if(str.length() > 1)
            {
                cout << "   ERROR: Response not recognized." << endl;
                continue;
            }
            dummy = str[0];
            
            if(dummy == 'y' || dummy == 'Y')
            {
                match = 1;
                break;
            }
            if(str.empty())
            {
                match = 0;
                break;
            }
            cout << "   ERROR: Response not recognized." << endl;    
        }
        
        while(1)
        {
            str = "";
            cout << "Use strength or conditional probability for strength_factor? [s or p]  "; 
            getline(cin, str);
            str.erase(remove(str.begin(), str.end(), ' '), str.end());
            if(str.length() > 1 || str.empty())
            {
                cout << "   ERROR: Response not recognized." << endl;
                continue;
            }
            dummy = str[0];
            
            if(dummy == 's' || dummy == 'S')
            {
                strength = 1;
                break;
            }
            
            if(dummy == 'p' || dummy == 'P')
            {
                if(checkMatching())
                {   
                    strength = 0;
                    break;
                }
                else
                {
                    cout << "   ERROR: One or more rules contain number of matching cases\n   equal to 0, so conditional probability cannot be executed." << endl; 
                    strength = 1;
                    break;
                }
            }
            cout << "   ERROR: Response not recognized." << endl;
        }
        
        while(1)
        {
            str = "";
            cout << "Use specificity factor? [y or RETURN]  "; 
            getline(cin, str);
            str.erase(remove(str.begin(), str.end(), ' '), str.end());
            if(str.length() > 1)
            {
                cout << "   ERROR: Response not recognized." << endl;
                continue;
            }
            dummy = str[0];
            
            if(dummy == 'y' || dummy == 'Y')
            {
                spec = 1;
                break;
            }
            if(str.empty())
            {
                spec = 0;
                break;
            }
            cout << "   ERROR: Response not recognized." << endl;    
        }
        
        while(1)
        {
            str = "";
            cout << "Use support? [y or RETURN]  "; 
            getline(cin, str);
            str.erase(remove(str.begin(), str.end(), ' '), str.end());
            if(str.length() > 1)
            {
                cout << "   ERROR: Response not recognized." << endl;
                continue;
            }
            dummy = str[0];
            
            if(dummy == 'y' || dummy == 'Y')
            {
                supp = 1;
                break;
            }
            if(str.empty())
            {
                supp = 0;
                break;
            }
            cout << "   ERROR: Response not recognized." << endl;    
        }
        
        while(1)
        {
            str = "";
            cout << "\nDisplay concept statistics? [y or RETURN]  "; 
            getline(cin, str);
            str.erase(remove(str.begin(), str.end(), ' '), str.end());
            if(str.length() > 1)
            {
                cout << "   ERROR: Response not recognized." << endl;
                continue;
            }
            dummy = str[0];
            
            if(dummy == 'y' || dummy == 'Y')
            {
                concept_stats = 1;
                break;
            }
            if(str.empty()) 
            {
                concept_stats = 0;
                break;
            }
            cout << "   ERROR: Response not recognized." << endl;    
        }
            
        while(1)
        {
            str = "";
            cout << "Display how cases associated with concepts were classified? [y or RETURN]  "; 
            getline(cin, str);
            str.erase(remove(str.begin(), str.end(), ' '), str.end());
            if(str.length() > 1)
            {
                cout << "   ERROR: Response not recognized." << endl;
                continue;
            }
            dummy = str[0];
            
            if(dummy == 'y' || dummy == 'Y')
            {
                cases_stats = 1;
                break;
            }
            if(str.empty())
            {
                cases_stats = 0;
                break;
            }
            cout << "   ERROR: Response not recognized." << endl;    
        }
        
        cout << "\n-----------\nThis report was created from: " << rulesFileName << " and from: " << dataFileName << endl;

        helper obj(rules, data, att, number, match, strength, spec, supp, concept_stats, cases_stats);
        
        while(1)
        {
            str = "";
            cout << "\nRun program again? [y or RETURN]  "; 
            getline(cin, str);
            str.erase(remove(str.begin(), str.end(), ' '), str.end());
            if(str.length() > 1)
            {
                cout << "   ERROR: Response not recognized." << endl;
                continue;
            }
            dummy = str[0];
            
            if(dummy == 'y' || dummy == 'Y')
            {
                repeat = 1;
                break;
            }
            if(str.empty())
            {
                cout << "Terminating program.\n" << endl;
                return 0;
            }
            cout << "   ERROR: Response not recognized." << endl; 
        }
    }
    return 0;
}
