#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <algorithm>
using namespace std;

class lem
{
public:

    const string key = "_victor_12345_";
    vector<vector<string>> certainRules;
    vector<vector<string>> possibleRules;    
    vector<vector<int>> resultingDrops;
    vector<vector<int>> certainDroppedAttributes;
    vector<vector<int>> possibleDroppedAttributes;
    
    lem(vector<vector<string>> &data, vector<string> &attributes, vector<string> &decisions, string decision, int numRows, int numCols, string outputFileName_certain, string outputFileName_possible)
{
    bool isNumerical;
    vector<int> numericalColumns;
    vector<vector<int>> Astar, Dstar;
    for(int col = 0; col < numCols; ++col)
    {
        isNumerical = 1;
        for(int row = 0; row < numRows; ++row)
            if(!stringIsNumeric(data[row][col]))
            {
                isNumerical = 0;
                break;
            }
        if(isNumerical)
            numericalColumns.push_back(col);
    }
    
    if(!numericalColumns.empty())
    {   
        discretizeColumn(data, decisions, numericalColumns, numRows, numCols, attributes);
        cleanTable(data, decisions, numericalColumns, numRows, numCols, attributes);
    }

    computeAstar(data, Astar, numRows);
    computeDstar(decisions, Dstar, numRows);
    
    bool setConsistent = 1;
    if(!isConsistent(Astar, numRows, decisions))
        setConsistent = 0;

    control_lem(data, attributes, Astar, Dstar, decisions, numRows, setConsistent);    
    
    if(setConsistent)
    {
        certainRules = droppingConditions(data, certainRules, attributes, certainDroppedAttributes, decisions, resultingDrops);        
//         printRules(certainRules, attributes, decisions, certainDroppedAttributes, decision, resultingDrops);        
        writeRules(certainRules, attributes, decisions, certainDroppedAttributes, decision, resultingDrops, outputFileName_certain);        
        writeEmptyPossible(outputFileName_possible);
    }
    else
    {
        certainRules = droppingConditions(data, certainRules, attributes, certainDroppedAttributes, decisions, resultingDrops);        
//         printRules(certainRules, attributes, decisions, certainDroppedAttributes, decision, resultingDrops); 
        writeRules(certainRules, attributes, decisions, certainDroppedAttributes, decision, resultingDrops, outputFileName_certain); 
        resultingDrops.clear();
        possibleRules = droppingConditions(data, certainRules, attributes, possibleDroppedAttributes, decisions, resultingDrops);        
//         printRules(possibleRules, attributes, decisions, certainDroppedAttributes, decision, resultingDrops);        
        writeRules(possibleRules, attributes, decisions, certainDroppedAttributes, decision, resultingDrops, outputFileName_possible);    
    }
}

vector<vector<string>> droppingConditions(vector<vector<string>> &data, vector<vector<string>> &ruleset, vector<string> &attributes, vector<vector<int>> &droppedAttributes, vector<string> decisions, vector<vector<int>> &resultingDrops)
{
    vector<string> currentRule;
    vector<string> resultingRule;
    vector<vector<string>> resultingRuleset;
    vector<int> droppedConditions;
    vector<int> myRemainingAttributes;
    vector<int> myRemainingAttributes_backup;
    
    int k;
    
    for(int rule = 0; rule < ruleset.size(); ++rule)
    {
        k = 0;
        currentRule.clear();
        resultingRule.clear();
        droppedConditions.clear();
        myRemainingAttributes.clear();
        myRemainingAttributes_backup.clear();
        currentRule = ruleset[rule];
        
        myRemainingAttributes = getRemainingAttributes(ruleset[rule].back(), droppedAttributes, decisions, attributes);

        myRemainingAttributes_backup = myRemainingAttributes;
        
        for(int condition = 0; condition < currentRule.size() - 1; ++condition)
        {
            droppedConditions.push_back(condition);
            myRemainingAttributes_backup = myRemainingAttributes;
            
            if(myRemainingAttributes.size() <= 1)
            {
                resultingRule.push_back(currentRule.at(condition));
                break;
            }
            
            myRemainingAttributes.erase(myRemainingAttributes.begin() + k);

            if(rulesetConsistent(rule, data, myRemainingAttributes, droppedConditions, ruleset, decisions))
            {
                myRemainingAttributes_backup = myRemainingAttributes;
                --k;
            }
            else
            {   
                resultingRule.push_back(currentRule.at(condition));
                myRemainingAttributes = myRemainingAttributes_backup;
                droppedConditions.pop_back();               
            }  
            ++k;
        } 
        
        if(resultingRule.size() > 0)
        {
            resultingRule.push_back(ruleset[rule].back());
            if(!veiv(resultingRuleset, resultingRule))
            {
                resultingRuleset.push_back(resultingRule);
                resultingDrops.push_back(droppedConditions);
            }
        }
    }
    return resultingRuleset;
}

bool veiv(vector<vector<string>> vv, vector<string> v)
{
    for(int k = 0; k < vv.size(); ++k)
        if(v == vv[k])
            return 1;
    return 0;
}

vector<int> getRemainingAttributes(string decision, vector<vector<int>> &droppedAttributes, vector<string> &decisions, vector<string> &attributes)
{
    vector<int> dropAtt, remAtt;
    vector<string> uniqueDecisions = getUniqueDecisions(decisions);
    for(int d = 0; d < uniqueDecisions.size(); ++d)
        if(decision == uniqueDecisions[d])
            for(int o = 0; o < droppedAttributes[d].size(); ++o)
                dropAtt.push_back(droppedAttributes[d][o]);
    for(int k = 0; k < attributes.size(); ++k)
        if(!eiv(dropAtt, k))
            remAtt.push_back(k);
    return remAtt;
}

bool rulesetConsistent(int rowNum, vector<vector<string>> &data,  vector<int> &remainingAttributes, vector<int> &droppedConditions, vector<vector<string>> &ruleset, vector<string> &decisions)
{
    bool equal;
    vector<string> row = ruleset.at(rowNum);    
    int currentCondition, currentAttribute;
    for(int i = 0; i < data.size(); ++i)
    {   
        equal = 1;        
        currentCondition = 0;
        while(eiv(droppedConditions, currentCondition))
            ++currentCondition;
        
        for(int remAtt = 0; remAtt < remainingAttributes.size(); ++remAtt)
        {
            currentAttribute = remainingAttributes.at(remAtt);            
            if(row.at(currentCondition) != data[i].at(currentAttribute))
            {
                equal = 0;  
                break;
            }
            
            ++currentCondition;
            while(eiv(droppedConditions, currentCondition))
                ++currentCondition;
        }
        
        if(equal)
            if(row.back() != decisions.at(i))  
                return 0;
    }
    return 1;
}

void print2D(vector<vector<string>> &v)
{
    for(int k = 0; k < v.size(); ++k)
    {
        for(int l = 0; l < v[k].size(); ++l)
            cout << v[k][l] << " ";
        cout << endl;
    }
}

void print2D(vector<vector<int>> &v)
{
    for(int k = 0; k < v.size(); ++k)
    {
        for(int l = 0; l < v[k].size(); ++l)
            cout << v[k][l] << " ";
        cout << endl;
    }
}

void control_lem(vector<vector<string>> &data, vector<string> &attributes, vector<vector<int>> &Astar, vector<vector<int>> &Dstar, vector<string> decisions, int numRows, const bool setConsistent)
{
    vector<int> droppedAttributes;
    vector<string> currentDecisions;
    vector<string> uniqueDecisions = getUniqueDecisions(decisions);
    
    for(int dec = 0; dec < uniqueDecisions.size(); ++dec)
    {
        currentDecisions.clear();
        droppedAttributes.clear();

        currentDecisions = lowerApp(uniqueDecisions.at(dec), Astar, Dstar, decisions);
        certainDroppedAttributes.push_back(run_lem(data, attributes, Astar, currentDecisions, droppedAttributes, numRows));

        if(!setConsistent)
        {               
            currentDecisions = upperApp(uniqueDecisions.at(dec), Astar, Dstar, decisions);            
            possibleDroppedAttributes.push_back(run_lem(data, attributes, Astar, currentDecisions, droppedAttributes, numRows, 0));
        }        
    }
}

vector<string> getUniqueDecisions(vector<string> v)
{
    vector<string> uniqueDecisions;
    for(int k = 0; k < v.size(); ++k)
        if(!eiv(uniqueDecisions, v.at(k)))
            uniqueDecisions.push_back(v.at(k));
    return uniqueDecisions; 
}

vector<string> lowerApp(string decision, vector<vector<int>> &Astar, vector<vector<int>> &Dstar, vector<string> &decisions)
{
    int index;
    vector<int> lA;
    vector<string> finalColumn;
    for(int k = 0; k < Dstar.size(); ++k)
        if(decision == decisions.at(Dstar[k][0]))
        {
            index = k;
            break;
        }
        
    for(int k = 0; k < Astar.size(); ++k)
    {
        if(isSubset(Astar[k], Dstar[index]))
            for(int x = 0; x < Astar[k].size(); ++x)
                lA.push_back(Astar[k][x]);
    }
    
    for(int k = 0; k < decisions.size(); ++k)
        if(eiv(lA, k))
            finalColumn.push_back(decision);
        else
            finalColumn.push_back(key);

    return finalColumn;
}

vector<string> upperApp(string decision, vector<vector<int>> &Astar, vector<vector<int>> &Dstar, vector<string> &decisions)
{
    int index;
    vector<int> uA;
    vector<string> finalColumn;
    for(int k = 0; k < Dstar.size(); ++k)
        if(decision == decisions.at(Dstar[k][0]))
        {
            index = k;
            break;
        }
    for(int k = 0; k < Astar.size(); ++k)
        if(hasIntersection(Dstar[index], Astar[k]))
            for(int x = 0; x < Astar[k].size(); ++x)
                uA.push_back(Astar[k][x]);          
    
    for(int k = 0; k < decisions.size(); ++k)
        if(eiv(uA, k))
            finalColumn.push_back(decision);
        else
            finalColumn.push_back(key);

    return finalColumn;
}

void print(vector<string> &v)
{
    for(int k = 0; k < v.size(); ++k)
        cout << v.at(k) << " ";
    cout << endl;
}

bool isSubset(vector<int> a, vector<int> b)
{
    sort(a.begin(), a.end());
    sort(b.begin(), b.end());
    
    return (includes(a.begin(), a.end(), b.begin(), b.end()) ||
            includes(b.begin(), b.end(), a.begin(), a.end()));
}

bool hasIntersection(vector<int> &a, vector<int> &b)
{
    vector<int> c;
    set_intersection(a.begin(), a.end(), b.begin(), b.end(), back_inserter(c));
    return !c.empty();
}

void printRules(vector<vector<string>> ruleset, vector<string> &attributes, vector<string> &decisions, vector<vector<int>> &droppedAttributes, string decision, vector<vector<int>> &resultingDrops)
{    
    int x, s = 0;
    for(int r = 0; r < ruleset.size(); ++r)
    {
        x = 0;
        for(int c = 0; c < ruleset[r].size() - 1; ++c)
        {
            while(eiv(droppedAttributes[s], x) || eiv(resultingDrops[r], x))
                ++x;

            cout << "(" << attributes[x] << ", " << ruleset[r][c] << ") ";
            ++x;
            if(c != ruleset[r].size() - 2)
                cout << "& ";
        }
        cout << "-> (" << decision << ", " << ruleset[r].back() << ")" << endl;
        if(r + 1 < ruleset.size())
            if(ruleset[r].back() != ruleset[r + 1].back())
                ++s;
    }
}

void writeRules(vector<vector<string>> ruleset, vector<string> &attributes, vector<string> &decisions, vector<vector<int>> &droppedAttributes, string decision, vector<vector<int>> &resultingDrops, string outputFileName)
{
    ofstream outfile(outputFileName);
    int x, s = 0;
    for(int r = 0; r < ruleset.size(); ++r)
    {
        x = 0;
        for(int c = 0; c < ruleset[r].size() - 1; ++c)
        {
            while(eiv(droppedAttributes[s], x) || eiv(resultingDrops[r], x))
                ++x;

            outfile << "(" << attributes[x] << ", " << ruleset[r][c] << ") ";
            ++x;
            if(c != ruleset[r].size() - 2)
                outfile << "& ";
        }
        outfile << "-> (" << decision << ", " << ruleset[r].back() << ")" << endl;
        if(r + 1 < ruleset.size())
            if(ruleset[r].back() != ruleset[r + 1].back())
                ++s;
    }
    outfile.close();
}

void writeEmptyPossible(string outputFileName)
{
    ofstream outfile(outputFileName);
    outfile << "! Possible rule set is not shown since it is identical with the certain rule set";
    outfile.close();    
}

vector<int> run_lem(vector<vector<string>> &data, vector<string> &attributes, vector<vector<int>> &Astar, vector<string> &decisions, vector<int> &droppedAttributes, int numRows, bool certain = 1)
{
    vector<vector<int>> tempAstar = Astar;
    vector<vector<string>> tempData = data;
    
    bool con;
    int numDropped = 0;

    for(int att = 0; att < attributes.size(); ++att)
    {
        vector<vector<string>> backupData = tempData;

        for(int k = 0; k < numRows; ++k)
            tempData[k].erase(tempData[k].begin() + att - numDropped);

        tempAstar.clear();
        computeAstar(tempData, tempAstar, numRows);
        
        con = isConsistent(tempAstar, numRows, decisions);
        
        if(!con) tempData = backupData;        
        else
        {   
            droppedAttributes.push_back(att);      
            ++numDropped;
        }
    }

    for(int k = 0; k < tempData.size(); ++k)
        tempData[k].push_back(decisions.at(k));

    vector<int> rep;
    for(int k = 0; k < tempData.size() - 1; ++k)
        for(int l = k + 1; l < tempData.size(); ++l)
            if(tempData[l] == tempData[k])
                rep.push_back(k);               
    rep.erase(unique(rep.begin(), rep.end()), rep.end()); 
    numDropped = 0;
    for(int k = 0; k < rep.size(); ++k)
    {
        tempData.erase(tempData.begin() + rep.at(k) - numDropped);
        ++numDropped;
    }
    
    if(certain)
    {
        for(int k = 0; k < tempData.size(); ++k)
            if(tempData[k].back() != key)
                certainRules.push_back(tempData[k]);
    }
    else
    {
        for(int k = 0; k < tempData.size(); ++k)
            if(tempData[k].back() != key)
                possibleRules.push_back(tempData[k]);
    }

    return droppedAttributes;
}

void discretizeColumn(vector<vector<string>> &data, vector<string> &decisions, vector<int> numericalColumns, int numRows, int &numCols, vector<string> &attributes)
{
    int curCol; double val;
    vector<double> curColVec, curColVec_u, cutpoints;
    vector<double>::iterator it;
    for(int counter = 0; counter < numericalColumns.size(); ++counter)
    {       
        curColVec.clear();
        curColVec_u.clear();
        cutpoints.clear();
        curCol = numericalColumns.at(counter); 
        for(int k = 0; k < numRows; ++k)
        {
             curColVec.push_back(atof(data[k][curCol].c_str()));  
             curColVec_u.push_back(atof(data[k][curCol].c_str()));  
        }
        sort(curColVec.begin(), curColVec.end());

        for(int k = 0; k + 1 < numRows; ++k)
        {
                val = (curColVec[k] + curColVec[k + 1])/2;
                if(eiv(curColVec, val) || eiv(cutpoints, val)) 
                    continue;
                cutpoints.push_back(val);          
        }     

        reorganizeTable(data, decisions, cutpoints, curCol, attributes, numRows, numCols, curColVec_u);       
    }
}

void reorganizeTable(vector<vector<string>> &data, vector<string> &decisions, vector<double> cutpoints, int colNum, vector<string> &attributes, int numRows, int &numCols, vector<double> &curColVec_u)
{        
    string val, attributeName;
    for(int a = 0; a < cutpoints.size(); ++a)
    {
        attributeName = attributes.at(colNum);
        attributes.push_back(attributeName.append(dtos(cutpoints.at(a))));      
        for(int b = 0; b < numRows; ++b)
        {
            if(curColVec_u.at(b) < cutpoints.at(a))
            {
                val = dtos(curColVec_u.front());
                val.append("..");
                val.append(dtos(cutpoints.at(a)));
                data[b].push_back(val);
            }
            else
            {
                val = dtos(cutpoints.at(a));
                val.append("..");
                val.append(dtos(curColVec_u.back()));
                data[b].push_back(val);
            }
        }
        ++numCols;
    }
}

string dtos(double d)
{
    ostringstream ss;
    ss << d;
    return ss.str();
}

bool stringIsNumeric(string s)
{
    bool p = 0;
    for(char c : s)
    {
        if(c == '.' && !p)
            p = 1;
        else if(!isdigit(c))
            return 0;
    }
    return 1;
}

void cleanTable(vector<vector<string>> &data, vector<string> &decisions, vector<int> numericalColumns, int numRows, int &numCols, vector<string> &attributes)
{
    for(int k = numericalColumns.size() - 1; k >= 0 ; --k)
    {   
        attributes.erase(attributes.begin() + numericalColumns.at(k));
        for(int r = 0; r < numRows; ++r)
            data[r].erase(data[r].begin() + numericalColumns.at(k));
        --numCols;
    }     
}

void computeAstar(vector<vector<string>> &data, vector<vector<int>> &Astar, int numRows)
{   
    vector<int> skip, inner;
    vector<int>::iterator it;
    for(int r = 0; r < numRows; ++r)
    {
        it = find(skip.begin(), skip.end(), r);
        if(it != skip.end())
            continue;        
        inner.clear();
        inner.push_back(r);

        for(int t = r + 1; t < numRows; ++t)
            if(data[r] == data[t])
            {
                inner.push_back(t);
                skip.push_back(t);
            }
        Astar.push_back(inner);
    }         
}

void computeDstar(vector<string> &decisions, vector<vector<int>> &Dstar, int numRows)
{    
    vector<int> skip, inner;
    vector<int>::iterator it;
    for(int r = 0; r < numRows; ++r)
    {
        it = find(skip.begin(), skip.end(), r);
        if(it != skip.end())
            continue;        
        inner.clear();
        inner.push_back(r);

        for(int t = r + 1; t < numRows; ++t)
            if(decisions[r] == decisions[t])
            {
                inner.push_back(t);
                skip.push_back(t);
            }
        Dstar.push_back(inner);
    }    
}

bool isConsistent(vector<vector<int>> &Astar, int numRows, vector<string> &decisions, bool debug = 0)
{
    for(int a = 0; a < Astar.size(); ++a)
        for(int b = 0; b + 1 < Astar[a].size(); ++b)
            if(decisions[Astar[a][b]] != decisions[Astar[a][b+1]]) 
                return 0;
    return 1;
}

bool eiv(vector<double> v, double i)
{
    vector<double>::iterator it;
    it = find(v.begin(), v.end(), i);
    return (it != v.end());
}

bool eiv(vector<int> v, double i)
{
    vector<int>::iterator it;
    it = find(v.begin(), v.end(), i);
    return (it != v.end());   
}

bool eiv(vector<string> v, string i)
{
    vector<string>::iterator it;
    it = find(v.begin(), v.end(), i);
    return (it != v.end());   
}

};
