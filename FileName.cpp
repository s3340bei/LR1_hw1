#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <queue>
#include <set>
using namespace std;

// space allocated
vector<string> terminals;
vector<string> nonterminals;
vector<pair<string, string>> rules;
vector<vector<char*>> rules_token;
vector<string> testdata;
vector<set<string>> firsts;
vector<set<string>> follows;
struct der		// d1->d2,follows
{
	string d1;
	string d2;
	int closure_pos = 0;//-1 denotes end position
	int rNum;// rules numr
	vector<string> follows;
};
struct sta
{
	vector<der> ders;
	map<string, string> actions;
	map<string, int> GoTo;
};
vector<sta> states;
int stateNum = 0;

// tmp variable
string s;
string ter;
pair<string, string> pa;
int n;
const vector<char*> empty_v;
bool b;
queue<pair<pair<string, int>, vector<string>>> q;//(closure NT, closure_position), follows

// functions
string GetClosureToken(int i, int j);

int main()
{
	// read testdata file
	ifstream ifst("2_testdata.txt", ios::in);
	if (!ifst.is_open())
	{
		cout << "Failed to open file.\n";
		return 1; // EXIT_FAILURE
	}
	while (getline(ifst, s))testdata.push_back(s);
	// close file
	ifst.close();

	// read grammer file
	ifstream ifs("2_grammar.txt", ios::in);
	if (!ifs.is_open())
	{
		cout << "Failed to open file.\n";
		return 1; // EXIT_FAILURE
	}
	// terminals
	getline(ifs, s);
	for (int j = 10; j < s.length(); j++)
	{
		if (s[j] != ' ' && s[j] != ',')ter += s[j];
		else if (s[j] == ',')
			terminals.push_back(ter), ter = "";
	}
	if (ter != "")terminals.push_back(ter), ter = "";
	// nonterminals
	getline(ifs, s);
	for (int j = 13; j < s.length(); j++)
	{
		if (s[j] != ' ' && s[j] != ',')ter += s[j];
		else if (s[j] == ',')
		{
			if (nonterminals.empty())// push starting
				nonterminals.push_back(ter + "'");
			nonterminals.push_back(ter), ter = "";
		}
	}
	if (ter != "")nonterminals.push_back(ter), ter = "";
	// rules
	while (getline(ifs, s))
	{
		for (int i = 0; i < s.length() - 1; i++)
		{
			if (s[i] == '-' && s[i + 1] == '>')
			{
				if (rules.empty())//inital S' derivation
					pa.first = ter + "'", pa.second = ter,
					rules.push_back(pa);
				pa.first = ter, ter = "";//nonterminal
				for (int j = i + 2; j < s.length(); j++)
				{
					if (s[j] != '|')ter += s[j];
					else pa.second = ter, ter = "",
						rules.push_back(pa);
				}
				if (ter != "")pa.second = ter, ter = "",
					rules.push_back(pa);
				break;
			}
			else ter += s[i];//nonterminal
		}
	}
	// close file
	ifs.close();
	//  rules token
	n = min(terminals.size(), nonterminals.size());
	for (int i = 0; i < rules.size(); i++)
	{
		s = rules[i].second;
		rules_token.push_back(empty_v);
		for (int j = 0; j < s.length(); )//j: index of rules[i]
		{
			int k;
			b = false;
			for (k = 0; k < n; k++)
			{
				if (s[j] == terminals[k][0])
				{
					rules_token[i].push_back(&terminals[k][0]);
					//fill the index 1 and following with 0
					for (int l = 0; l < terminals[k].size() - 1; l++)
					{
						rules_token[i].push_back(nullptr);
					}
					// turn to next token
					j += terminals[k].size();
					// avoid line 128, 139 do the loop
					b = true;
					break;
				}
				else if (s[j] == nonterminals[k][0])
				{
					rules_token[i].push_back(&nonterminals[k][0]);
					for (int l = 0; l < nonterminals[k].size() - 1; l++)
						rules_token[i].push_back(nullptr);
					j += nonterminals[k].size();
					b = true;
					break;
				}
			}
			if (!b)for (int m = k; m < terminals.size(); m++)
			{
				if (s[j] == terminals[m][0])
				{
					rules_token[i].push_back(&terminals[m][0]);
					for (int l = 0; l < terminals[m].size() - 1; l++)
						rules_token[i].push_back(nullptr);
					j += terminals[m].size();
					break;
				}
			}
			if (!b)for (int m = k; m < nonterminals.size(); m++)
			{
				if (s[j] == nonterminals[m][0])
				{
					rules_token[i].push_back(&nonterminals[m][0]);
					for (int l = 0; l < nonterminals[m].size() - 1; l++)
						rules_token[i].push_back(nullptr);
					j += nonterminals[m].size();
					break;
				}
			}
		}
	}



	// find firsts and follows
	queue<pair<int, int>> ruleNumQ;
	firsts.assign(nonterminals.size(), set<string>());
	follows.assign(nonterminals.size(), set<string>());
	// terminals add into firsts
	firsts[0].insert("$");
	for (int i = 1; i < rules.size(); i++)
	{
		for (int j = 1; j < nonterminals.size(); j++)
		{
			// get the needed rules
			if (rules[i].first == nonterminals[j])
			{
				s = GetClosureToken(i, 0);
				firsts[j].insert(s);
			}
		}
	}
	// nonterminals add into firsts
	for (int i = 1; i < firsts.size(); i++)
	{
		vector<string> v;
		for (auto& j : firsts[i])
		{
			if (j[0] >= 'A' && j[0] <= 'Z')
			{
				for (int k = 1; k < nonterminals.size(); k++)
				{
					if (j == nonterminals[k])
					{
						for (auto& l : firsts[k])
						{
							firsts[i].insert(l);
						}
						break;
					}
				}
				v.push_back(j);
			}
		}
		// remove nonterminal first
		for (auto& x : v)
			firsts[i].erase(firsts[i].find(x));
	}




	// FA && parsing table
	// line 1
	cout << "[\"" <<
		rules[0].first << "->." << rules[0].second << "\"";// S'->.S
	for (auto& i : rules)
	{
		if (i != rules[0])
			cout << ", \'" << i.first << "->" << i.second << "\'";
	}
	cout << "]\n";
	// line 2
	cout << "///////////////// state ////////////////////\n";

	// State 0
	sta sta_t;
	der der_t;
	der_t.d1 = rules[0].first;
	der_t.d2 = rules[0].second;
	der_t.follows.push_back("$");
	der_t.rNum = 0;
	// S->.AA
	if (*rules_token[0][0] >= 'A' && *rules_token[0][0] <= 'Z')
	{
		s = GetClosureToken(0, 0);
		vector<string> vs;
		vs.push_back("$");
		q.push(make_pair(make_pair(s, 1), vs));// (closure NT, closure_position), follows
	}
	sta_t.ders.push_back(der_t);

	// closure
	while (!q.empty())
	{
		ter = q.front().first.first;
		for (int i = 0; i < rules.size(); i++)
		{
			if (rules[i].first == ter)
			{
				der dr;
				dr.d1 = rules[i].first;
				dr.d2 = rules[i].second;
				dr.rNum = i;
				s = GetClosureToken(i, 0);
				// check if closure end
				if (s[0] >= 'A' && s[0] <= 'Z')
				{

					q.push(make_pair(make_pair(s, 1), q.front().second));// (closure NT, closure_position), follows
				}
				sta_t.ders.push_back(dr);
			}
		}
	}
}
// rules number, closure pos
string GetClosureToken(int i, int j)
{
	if (j < 0)return "";// undefined
	if (j > 0)
	{
		n = 1;
		int itg = 1;
		while (itg < j)
		{
			while (rules_token[i][n] == nullptr)n++;
			itg++;
		}
		j = n;
	}
	s = *(rules_token[i][j]);
	n = 1;
	while (n < rules_token[i].size()
		&& (rules_token[i][j] + n) != nullptr)
		s += *(rules_token[i][j] + n++);
	return s;
}