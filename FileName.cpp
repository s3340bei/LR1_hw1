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
//vector<set<string>> follows;
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

// tmp variable
string s;
string ter;
pair<string, string> pa;
int n;
const vector<char*> empty_v;
bool b;
// State 0
sta sta_0;
der der_t;

// functions

// rules number, closure pos
string GetClosureToken(int i, int j)
{
	if (j < 0)return "";// undefined
	if (j > 0)
	{
		n = 1;
		if (n > rules_token[i].size() - 1)return "";
		int itg = 1;
		while (itg < j)
		{
			while (rules_token[i][n] == nullptr)
			{
				if (n > rules_token[i].size() - 1)return "";// end
				n++;
			}
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
vector<string> GetFirst(string nont)
{
	// O(n), n is the number of nonterminals
	vector<string> v;
	for (int i = 0; i < nonterminals.size(); i++)
	{
		if (nonterminals[i] == nont)
		{
			for (auto& k : firsts[i])
				v.push_back(k);
			return v;
		}
	}
	return v;
}
void OutputState(sta sta, int sNum)
{
	cout << sNum << " : [[\"" << sta.ders[0].d1 << "->";
}
void Closure(int stateNum)
{
	string st = GetClosureToken(stateNum, der_t.closure_pos);
	queue<int> qi;
	for (int i = 0; i < rules.size(); i++)
	{
		if (rules[i].first == st)//find the derivations
		{
			der dr;
			dr.d1 = rules[i].first;
			dr.d2 = rules[i].second;
			dr.rNum = i;
			s = GetClosureToken(stateNum, n + 1);
			if (s != "")
			{
				dr.follows = GetFirst(s);
				if (s[0] >= 'A' && s[0] <= 'Z')
					qi.push(sta_0.ders.size());
			}
			else
			{
				dr.follows = der_t.follows;
				dr.closure_pos = -1;
			}
			sta_0.ders.push_back(dr);
		}
	}
	while (!qi.empty())
	{
		n = qi.front();
		qi.pop();
		der_t = sta_0.ders[n];
		st = GetClosureToken(der_t.rNum, der_t.closure_pos);
		for (int i = 0; i < rules.size(); i++)
		{
			if (rules[i].first == st)//find the derivations
			{
				der dr;
				dr.d1 = rules[i].first;
				dr.d2 = rules[i].second;
				dr.rNum = i;
				dr.closure_pos = der_t.closure_pos + 1;
				s = GetClosureToken(n, dr.closure_pos);
				if (s != "")
				{
					dr.follows = GetFirst(s);
					if (s[0] >= 'A' && s[0] <= 'Z')
						qi.push(sta_0.ders.size());
				}
				else
				{
					dr.follows = der_t.follows;
					dr.closure_pos = -1;
				}
				sta_0.ders.push_back(dr);
			}
		}
	}
	states.push_back(sta_0);
	OutputState(sta_0, stateNum);
}
void FindFirst()
{
	firsts.assign(nonterminals.size(), set<string>());
	//follows.assign(nonterminals.size(), set<string>());
	vector<bool> vb;
	// terminals add into firsts
	for (int i = 0; i < rules.size(); i++)
	{
		b = true;
		for (int j = 0; j < nonterminals.size(); j++)
		{
			// get the needed rules
			if (rules[i].first == nonterminals[j])
			{
				s = GetClosureToken(i, 0);
				firsts[j].insert(s);
				if (s[0] >= 'A' && s[0] <= 'Z')
					b = false;
				break;
			}
		}
		vb.push_back(b);
	}
	// nonterminals add into firsts
	for (int i = 0; i < firsts.size(); i++)
	{
		for (auto&j:firsts[i])
		{
			if (j[0] >= 'A' && j[0] <= 'Z')//first[i][j] is nonterminals.
			{
				for (int k = 0; k < nonterminals.size(); k++)
				{
					if (nonterminals[k] == j)
					{
						if (vb[k])// firsts[k] are all terminals.
						{
							firsts[i].erase(firsts[i].find(j));
							for (auto& l : firsts[k])
								firsts[i].insert(l);
						}
						else// firsts[k] has nonterminal.
						{

						}
					}
				}
			}
		}
	}
}

int main()
{
	// read testdata file
	ifstream ifst("1_testdata.txt", ios::in);
	if (!ifst.is_open())
	{
		cout << "Failed to open file.\n";
		return 1; // EXIT_FAILURE
	}
	while (getline(ifst, s))testdata.push_back(s);
	// close file
	ifst.close();

	// read grammer file
	ifstream ifs("1_grammar.txt", ios::in);
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
					bool bb = false;
					for (int l = j+1; l < nonterminals[k].size(); l++)
						if (s[l] != nonterminals[k][l - j])bb = true;
					if (bb)continue;
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
					bool bb = false;
					for (int l = j + 1; l < nonterminals[m].size(); l++)
						if (s[l] != nonterminals[m][l - j])bb = true;
					if (bb)continue;
					rules_token[i].push_back(&nonterminals[m][0]);
					for (int l = 0; l < nonterminals[m].size() - 1; l++)
						rules_token[i].push_back(nullptr);
					j += nonterminals[m].size();
					break;
				}
			}
		}
	}

	// find firsts.
	FindFirst();


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
	
	// E'->E
	der_t.d1 = rules[0].first;
	der_t.d2 = rules[0].second;
	der_t.follows.push_back("$");
	der_t.rNum = 0;
	sta_0.ders.push_back(der_t);
	//Closure(0);
	// output state 0

}
