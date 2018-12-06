#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <algorithm>
using namespace std;

class helper
{
public:
    
int part_inc = 0, part_cor = 0, comp_inc = 0, comp_cor = 0, not_class = 0;
vector<int> cases_not_class, cases_part_inc, cases_part_cor, cases_comp_inc, cases_comp_cor;

helper(vector<vector<string>> rules, vector<vector<string>>data, vector<string> att, vector<vector<int>> number, bool match, bool strength, bool spec, bool supp, bool concept_stats, bool cases_stats)
{            
    int numCond = 0;
    vector<string> conceptNames;
    vector<vector<int>> valConcepts;
    
    for(int k = 0; k < rules.size(); ++k)
        numCond += rules[k].size() - 1;
    
    for(int k = 0; k < data.size(); ++k)
        conceptNames.push_back(data[k].back());
    sort(conceptNames.begin(), conceptNames.end());
    conceptNames.erase(unique(conceptNames.begin(), conceptNames.end()), conceptNames.end());
    
    for(int k = 0; k < conceptNames.size(); ++k)
    {
        vector<int> row = {0, 0, 0, 0, 0};
        valConcepts.push_back(row);
    }

    cout << "The total number of cases:      " << data.size() << endl;
    cout << "The total number of attributes: " << att.size() - 1 << endl;
    cout << "The total number of rules:      " << rules.size() << endl;
    cout << "The total number of conditions: " << numCond << endl;

    valConcepts = completeMatching(rules, number, data, att, conceptNames, valConcepts, match, strength, spec, supp);
    
    cout << "The total number of cases that are not classified: " << not_class << endl;
    
    partial_printer();
    complete_printer();
    
    cout << "       PARTIAL AND COMPLETE MATCHING:" << endl;
    cout << "The total number of cases that are not classified or incorrectly classified: " << not_class + part_inc + comp_inc << endl;

    double er = 100 * (double((not_class + part_inc + comp_inc)) / double(data.size()));
    cout << fixed << setprecision(2) <<  "Error rate: " << er << "%" << endl;

    if(concept_stats)
        printConceptStats(data, conceptNames, valConcepts, att);
    if(cases_stats)
        printCasesStats(data, conceptNames, att, cases_not_class, cases_part_inc, cases_part_cor, cases_comp_inc, cases_comp_cor);
}

vector<vector<int>> completeMatching(vector<vector<string>> rules, vector<vector<int>> number, vector<vector<string>>data, vector<string> att, vector<string> concepts, vector<vector<int>> valConcepts, bool match, bool strength, bool spec, bool supp)
{
    int idx;
    bool _match; 
    string case_str, cond_str;
    
    vector<int> noCompleteMatches;
    vector<vector<int>> completeMatches; 
    
    // Iterate over all cases
    for(int _case = 0; _case < data.size(); ++_case)
    { 
        vector<int> matchingRules;
        completeMatches.push_back(matchingRules);
        
        // For each case, check if each rule is a (complete) match
        // Iterate over all rules
        for(int _rule = 0; _rule < rules.size(); ++_rule)
        { 
            _match = 1;
            
            // Iterate over all conditions of rule, excluding decision
            for(int _cond = 0; _cond < rules[_rule].size() - 1; ++_cond)
            {
               idx = getAttrIdx(getRuleLHS(rules[_rule][_cond]), att);
               case_str = data[_case][idx];
               cond_str = getRuleRHS(rules[_rule][_cond]);

               // Check if condition is a matching numerical interval
               if(condIsInterval(cond_str) && !numberFits(cond_str, case_str))
               {
                   _match = 0;
                   break;
               }
               // Check for "?" missing attribute value
               if(case_str == "?")
               {
                   _match = 0;
                   break;
               }
               // Check if condition string matches case string
               if(cond_str != case_str && case_str != "*" && case_str != "-")
               {
                   _match = 0;
                   break;
               }
            }
            
            // If rule matches case, add rule idx to complete matches
            if(_match)
                completeMatches[_case].push_back(_rule);
        }
    } 

    int concept_idx;
    string left, right; 
    for(int _case = 0; _case < completeMatches.size(); ++_case)
    {        
        // No complete matches found for case (partial matching)
        if(completeMatches[_case].size() < 1)
            noCompleteMatches.push_back(_case);
        // Exactly one match found for case (done)
        else if(completeMatches[_case].size() == 1)
        {
            left = getRuleRHS(rules[completeMatches[_case][0]].back());
            left.erase(remove(left.begin(), left.end(), ' '), left.end());

            right = data[_case].back();
            right.erase(remove(right.begin(), right.end(), ' '), right.end());
            
            concept_idx = findConceptIdx(concepts, right);  
            if(left == right)
            {
                ++comp_cor;
                ++valConcepts[concept_idx][4];
                cases_comp_cor.push_back(_case);
            }
            else
            {
                ++comp_inc;
                ++valConcepts[concept_idx][3];
                cases_comp_inc.push_back(_case);
            }
        }
        // Multiple matches needed for case (need metrics)
        else
        {
            vector<vector<double>> pmf;
            int _idx = 0;
            int rule_idx = metrics(completeMatches[_case], number, match, strength, spec, supp, pmf, _idx, 1);
            
            left = getRuleRHS(rules[rule_idx].back());
            right = data[_case].back();
            right.erase(remove(right.begin(), right.end(), ' '), right.end());

            if(left == right)
                ++part_cor;
            else
                ++part_inc;
        }
    }
    cout << endl; 
    return partialMatching(rules, number, data, att, concepts, valConcepts, noCompleteMatches, match, strength, spec, supp);
}

vector<vector<int>> partialMatching(vector<vector<string>> rules, vector<vector<int>> number, vector<vector<string>>data, vector<string> att, vector<string> concepts, vector<vector<int>> valConcepts, vector<int> noCompleteMatches, bool match, bool strength, bool spec, bool supp)
{ 
    int idx;
    bool _match; 
    string case_str, cond_str;
    vector<vector<double>> pmf;
    vector<int> noPartialMatches;
    vector<vector<int>> partialMatches;
    
    for(int _idx = 0; _idx < data.size(); ++_idx)
    {   
        vector<double> dubs;
        pmf.push_back(dubs);
        for(int _rule = 0; _rule < rules.size(); ++_rule)
            pmf[_idx].push_back(0);
    }   
    // Iterate over all cases that had no complete matches
    for(int _case, _idx = 0; _idx < noCompleteMatches.size(); ++_idx)
    { 
        _case = noCompleteMatches.at(_idx);
        vector<int> matchingRules;
        partialMatches.push_back(matchingRules);
        
        // For each case, check if each rule is a (complete) match
        // Iterate over all rules 
        for(int _rule = 0; _rule < rules.size(); ++_rule)
        {
            _match = 0;
            // Iterate over all conditions of rule, excluding decision
            for(int _cond = 0; _cond < rules[_rule].size() - 1; ++_cond)
            {
               idx = getAttrIdx(getRuleLHS(rules[_rule][_cond]), att);
               case_str = data[_case][idx];
               cond_str = getRuleRHS(rules[_rule][_cond]);
               
               // Check if condition is a numerical interval
               if(condIsInterval(cond_str) && numberFits(cond_str, case_str))
               {
                   _match = 1;
                   ++pmf[_idx][_rule];
               }
               // Check for "*" or "-" missing attribute value
               else if(case_str == "*" || case_str == "-")
               {
                   _match = 1;
                   ++pmf[_idx][_rule];
               }
               // Check if condition string matches case string
               else if(cond_str == case_str && case_str != "?")
               {
                   _match = 1;
                   ++pmf[_idx][_rule];
               }
            }
            
            // If rule matches case, add rule idx to partial matches
            if(_match)
                partialMatches[_idx].push_back(_rule);
        }
    }
    
    for(int k = 0; k < pmf.size(); ++k)
        for(int l = 0; l < pmf[k].size(); ++l)
            pmf[k][l] /= rules.size();
    
    int concept_idx;
    string left, right; 
    for(int _case, _idx = 0; _idx < noCompleteMatches.size(); ++_idx)
    {   
        _case = noCompleteMatches.at(_idx); 
        right = data[_case].back();
        right.erase(remove(right.begin(), right.end(), ' '), right.end());
        concept_idx = findConceptIdx(concepts, right);
        
        // No complete matches found for case (partial matching)
        if(partialMatches[_idx].size() < 1)
        {
            noPartialMatches.push_back(_case);
            ++not_class;
            ++valConcepts[concept_idx][0];
            cases_not_class.push_back(_case);
        }
        // Exactly one match found for case (done)
        else if(partialMatches[_idx].size() == 1)
        {
            
        left = getRuleRHS(rules[partialMatches[_case][0]].back()); 
        left.erase(remove(left.begin(), left.end(), ' '), left.end()); 

            if(left == right)
            {
                ++part_cor;
                ++valConcepts[concept_idx][2];
                cases_part_cor.push_back(_case);
            }
            else
            {
                ++part_inc;
                ++valConcepts[concept_idx][1];
                cases_part_inc.push_back(_case);
            }
        }
        // Multiple matches needed for case (need metrics)
        else
        {
            int rule_idx = metrics(partialMatches[_idx], number, match, strength, spec, supp, pmf, _idx, 0);
            
            left = getRuleRHS(rules[rule_idx].back());
            right = data[_case].back();
            right.erase(remove(right.begin(), right.end(), ' '), right.end());

            if(left == right)
            {
                ++part_cor;
                ++valConcepts[concept_idx][2];
            }
            else
            {
                ++part_inc;
                ++valConcepts[concept_idx][1];
            }
        }
    }

    return valConcepts;
}

int metrics(vector<int> matches, vector<vector<int>> number, bool match, bool strength, bool spec, bool supp, vector<vector<double>> pmf, int _idx, bool complete)
{     
    int max_strength, max_cp, max_spec, max_supp;
    // Metrics: match_factor / strength / condprob / spec / supp    
    vector<double> _row; 
    vector<int> _winners;    
    vector<vector<double>> _metrics;
    for(int k = 0; k < matches.size(); ++k)
    {
        _metrics.push_back(_row);
        for(int fv = 0; fv < 5; ++fv)
            _metrics[k].push_back(0);
    }

    // For each rule, compute values and maximum for each metric
    // Partial Matching Factor
    if(match)
    {
        if(complete)
            for(int i = 0; i < matches.size(); ++i)
                _metrics[i][0] = 1;
        else
        {
            for(int i = 0; i < matches.size(); ++i)
                _metrics[i][0] = pmf[_idx][i];
        }
    }
    // Strength or Conditional Probability
    if(strength)
    {
        max_strength = 0;
        for(int i = 0; i < matches.size(); ++i)
        {
            _metrics[i][1] = number[matches.at(i)][1];
            if(_metrics[i][1] > max_strength)
                max_strength = _metrics[i][1];
        }
    }
    else 
    {
        max_cp = 0;
        for(int i = 0; i < matches.size(); ++i)
        {
            _metrics[i][2] = number[matches.at(i)][1] / number[matches.at(i)][2];
            if(_metrics[i][2] > max_cp)
                max_cp = _metrics[i][2];
        }
        
    }
    // Specificity
    if(spec)
    {   
        max_spec = 0;
        for(int i = 0; i < matches.size(); ++i)
        {
            _metrics[i][3] = number[matches.at(i)][0];
            if(_metrics[i][3] > max_spec)
                max_spec = _metrics[i][3];
        }
    }
    // Support
    if(supp)
    {
        max_supp = 0;
        for(int i = 0; i < matches.size(); ++i)
        {
            if(complete)
                _metrics[i][4] = number[matches.at(i)][1] * number[matches.at(i)][0];
            else
                _metrics[i][4] = number[matches.at(i)][1] * number[matches.at(i)][0] * pmf[_idx][i];
            
            if(_metrics[i][4] > max_cp)
                max_cp = _metrics[i][4];
        }
        
    }

    // Determine which rule "wins" each metric
    for(int metric = 0; metric < _metrics[0].size(); ++metric)
    {
        if(metric == 0 && !match)
            continue;
        if(metric == 1 && !strength)
            continue;
        if(metric == 2 && strength)
            continue;
        if(metric == 3 && !spec)
            continue;
        if(metric == 4 && !supp)
            continue;       
        
        _winners.push_back(0);
        int max = _metrics[0][metric];
        
        for(int r = 0; r < _metrics.size(); ++r) 
            if(_metrics[r][metric] > max)
            {
                max = _metrics[r][metric];
                _winners[metric] = r;
            }
    }   
    
    if(_winners.size() == 1)
        return _winners.at(0);
    
    vector<int> w = _winners;
    w.erase(unique(w.begin(), w.end()), w.end());
    
    if(w.size() == 1)
        return w.at(0);

    int max = 0, mf = _winners[0], counter;
    for(int i = 0;i < _winners.size(); i++)
    {
        counter = (int)count(_winners.begin(), _winners.end(), _winners[i]);
        if(counter > max)
        {       
            max = counter;
            mf = _winners[i];
        }
    }
    
    return mf;
}

int findConceptIdx(vector<string> &concepts, string &s)
{
    for(int k = 0; k < concepts.size(); ++k)
        if(concepts.at(k) == s)
            return k;    
}

void complete_printer()
{
    cout << "       COMPLETE MATCHING:" << endl;
    cout << "   The total number of cases that are incorrectly classified: " << comp_inc << endl;
    cout << "   The total number of cases that are  correctly  classified: " << comp_cor << endl;
}

void partial_printer()
{
    cout << "       PARTIAL MATCHING:" << endl;
    cout << "   The total number of cases that are incorrectly classified: " << part_inc << endl;
    cout << "   The total number of cases that are  correctly  classified: " << part_cor << endl;
}

void printConceptStats(vector<vector<string>>data, vector<string> concepts, vector<vector<int>> valConcepts, vector<string> att)
{
    cout << "\n-----------\nConcept statistics:\n";
    for(int concept_idx = 0; concept_idx < concepts.size(); ++concept_idx)
    {
        int counter = 0;
        for(int _case = 0; _case < data.size(); ++_case)
            if(data[_case].back() == concepts.at(concept_idx))
                ++counter;
        cout << "\nConcept (" << att.back() << ", " << concepts.at(concept_idx) << "): " << endl;
        cout << "The total number of cases that are not classified: " << valConcepts[concept_idx][0] << endl;
        cout << "       PARTIAL MATCHING:" << endl;
        cout << "   The total number of cases that are incorrectly classified: " << valConcepts[concept_idx][1] << endl;
        cout << "   The total number of cases that are  correctly  classified: " << valConcepts[concept_idx][2] << endl;
        cout << "       COMPLETE MATCHING:" << endl;
        cout << "   The total number of cases that are incorrectly classified: " << valConcepts[concept_idx][3] << endl;
        cout << "   The total number of cases that are  correctly  classified: " << valConcepts[concept_idx][4] << endl;
        cout << "The total number of cases in the concept: " << counter << endl;
    }
}

void printCasesStats(vector<vector<string>>data, vector<string> concepts, vector<string> att, vector<int> cases_not_class, vector<int> cases_part_inc, vector<int> cases_part_cor, vector<int> cases_comp_inc, vector<int> cases_comp_cor)
{
    cout << "\n-----------\nHow cases associated with concepts were classified:\n";
    for(int concept_idx = 0; concept_idx < concepts.size(); ++concept_idx)
    {
        cout << "\nConcept (" << att.back() << ", " << concepts.at(concept_idx) << "): " << endl;
        cout << "List of cases that are not classified: " << endl;
        for(int k = 0; k < cases_not_class.size(); ++k)
        {
            int _case = cases_not_class[k];
            for(int l = 0; l < data[k].size(); ++l)
            {
                cout << "(" << att.at(l) << ", " << data[_case][l] << ") ";
                if(l == data[k].size() - 1)
                    break;
                cout << (l != data[k].size() - 2 ? "& " : "-> ");

            }
            cout << endl;
        }
        cout << "       PARTIAL MATCHING:" << endl;
        cout << "List of cases that are incorrectly classified: " << endl;
        for(int k = 0; k < cases_part_inc.size(); ++k)
        {
            int _case = cases_part_inc[k];
            for(int l = 0; l < data[k].size(); ++l)
            {
                cout << "(" << att.at(l) << ", " << data[_case][l] << ") ";
                if(l == data[k].size() - 1)
                    break;
                cout << (l != data[k].size() - 2 ? "& " : "-> ");

            }
            cout << endl;
        }
        cout << "List of cases that are correctly classified: " << endl;
        for(int k = 0; k < cases_part_cor.size(); ++k)
        {
            int _case = cases_part_cor[k];
            for(int l = 0; l < data[k].size(); ++l)
            {
                cout << "(" << att.at(l) << ", " << data[_case][l] << ") ";
                if(l == data[k].size() - 1)
                    break;
                cout << (l != data[k].size() - 2 ? "& " : "-> ");

            }
            cout << endl;
        }
        cout << "       COMPLETE MATCHING:" << endl;
        cout << "List of cases that are incorrectly classified: " << endl;
        for(int k = 0; k < cases_comp_inc.size(); ++k)
        {
            int _case = cases_comp_inc[k];
            for(int l = 0; l < data[k].size(); ++l)
            {
                cout << "(" << att.at(l) << ", " << data[_case][l] << ") ";
                if(l == data[k].size() - 1)
                    break;
                cout << (l != data[k].size() - 2 ? "& " : "-> ");

            }
            cout << endl;
        }
        cout << "List of cases that are correctly classified: " << endl;
        for(int k = 0; k < cases_comp_cor.size(); ++k)
        {
            int _case = cases_comp_cor[k];
            for(int l = 0; l < data[k].size(); ++l)
            {
                cout << "(" << att.at(l) << ", " << data[_case][l] << ") ";
                if(l == data[k].size() - 1)
                    break;
                cout << (l != data[k].size() - 2 ? "& " : "->");

            }
            cout << endl;
        }
    }  
}

bool condIsInterval(string &cond)
{
    if(!isdigit(cond.at(0)))
        return 0;
    size_t f = cond.find("..");
    if(f == string::npos)
        return 0;
    return 1;
}

bool numberFits(string &_int, string &_val)
{
    double _min, _max, _nval;
    size_t f = _int.find("..");
    _min = stod(_int.substr(0, f));
    _max = stod(_int.substr(f + 2, _int.length()));
    _nval = stod(_val);
    
    return (_nval > _min && _nval < _max);
}

string getRuleLHS(string &cond)
{ 
    for(int k = 0; k < cond.length(); ++k)
        if(cond.at(k) == ',')
            return cond.substr(0, k);
}

string getRuleRHS(string &cond)
{   
    for(int k = 0; k < cond.length(); ++k)
        if(cond.at(k) == ',')
            return cond.substr(k + 1, cond.length() - 1);
}

int getAttrIdx(string cond, vector<string> att)
{
    for(int k = 0; k < att.size(); ++k)
        if(att.at(k) == cond)
            return k;
}

};
