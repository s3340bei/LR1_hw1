#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <queue>
#include <map>
#include <set>
using namespace std;

// space allocated
vector<string> terminals;
vector<string> nonterminals;
vector<pair<string, string>> rules;
vector<vector<char*>> rules_token;
vector<string> testdata;
vector<set<string>> firsts;
struct der
{
	int closure_pos = 0;//-1 denotes end position
	int rNum;// rules numr
	set<string> follows;
	bool operator==(der d)
	{
		if (closure_pos != d.closure_pos)return false;
		if (rNum != d.rNum)return false;
		if (follows != d.follows)return false;
		return true;
	}
	bool operator!=(der d)
	{
		if (closure_pos != d.closure_pos)return true;
		if (rNum != d.rNum)return true;
		if (follows != d.follows)return true;
		return false;
	}
};
struct sta
{
	vector<der> ders;
	// false: reduce, true: shift
	map<string, pair<bool, int>> actions;
	map<string, int> GoTo;
	bool operator==(sta sta1)
	{
		if (ders != sta1.ders)return false;
		return true;
	}
};
vector<sta> states;
int stateNum = 0;
vector<vector<string>> rulesToken_departed;


// tmp variable
string s;
string ter;
pair<string, string> pa;
int n;
const vector<char*> empty_v;
bool b;
der der_t;
queue<int> qi;// store the un_closure derNum in state i

// functions

// rules number, closure pos
//string GetClosureToken(int i, int j)
//{
//	int nt;
//	if (j >= rules_token[i].size())
//		return "";
//	if (ruleToken0.size() > i && j == 0)
//		return ruleToken0[i];
//	if (j < 0)return "";// undefined
//	if (j > 0)
//	{
//		nt = 1;
//		if (nt > rules_token[i].size() - 1)return "";
//		int itg = 1;
//		while (itg < j)
//		{
//			while (rules_token[i][nt] == nullptr)
//			{
//				if (nt > rules_token[i].size() - 1)return "";// end
//			}
//			itg++;
//			nt++;
//		}
//		j = nt;
//	}
//	string str;
//	str = *(rules_token[i][j]);
//	nt = 1;
//	while (nt < rules_token[i].size()
//		&& *(rules_token[i][j] + nt) != '\0')
//		str += *(rules_token[i][j] + nt++);
//	return str;
//}
bool operator==(vector<der> de1, vector<der> de2)
{
	if (de1.size() != de2.size())return false;
	for (int i = 0; i < de1.size(); i++)
		if (de1[i] != de2[i])return false;
	return true;
}
set<string> GetFirst(string nont)
{
	// O(n), n is the number of nonterminals
	for (int i = 0; i < nonterminals.size(); i++)
		if (nonterminals[i] == nont)return firsts[i];
	return firsts[0];
}
void OutputDerivation(der d)
{
	// check is "" or ''
	char c = '\'';
	for (auto& i : rules[d.rNum].first)
	{
		if (i == '\'')
		{
			c = '\"';
			break;
		}
	}
	// output nonterminal
	cout << "[" << c << rules[d.rNum].first << "->";
	// output derivation
	if (d.closure_pos == 0)
		cout << "." << rules[d.rNum].second;
	else if (d.closure_pos == -1)
		cout << rules[d.rNum].second << ".";
	else
	{
		int in = 0;
		while (in < rulesToken_departed[d.rNum].size())
		{
			if (in == d.closure_pos)cout << ".";
			s = rulesToken_departed[d.rNum][in++];
			cout << s;
		}
	}
	// output follow
	auto it = d.follows.begin();
	cout << c << ", [\'" << *it++ << "\'";
	while (it != d.follows.end())
		cout << ", \'" << *it++ << "\'";
	cout << "]]";
}
void OutputState(sta sta, int sNum)
{
	cout << sNum << " : [";
	OutputDerivation(sta.ders[0]);
	for (int i = 1; i < sta.ders.size(); i++)
	{
		cout << ", ";
		OutputDerivation(sta.ders[i]);
	}
	cout << "]\n";
}
void FindFirst()
{
	firsts.assign(nonterminals.size(), set<string>());
	//follows.assign(nonterminals.size(), set<string>());
	vector<bool> vb;
	// terminals add into firsts
	for (int j = 0; j < nonterminals.size(); j++)
	{
		b = true;
		for (int i = 0; i < rules.size(); i++)
		{
			// get the needed rules
			if (rules[i].first == nonterminals[j])
			{
				s = rulesToken_departed[i][0];
				if (s != nonterminals[j])
					firsts[j].insert(s);
				if (s[0] >= 'A' && s[0] <= 'Z')
					b = false;
			}
		}
		vb.push_back(b);
	}
	// nonterminals add into firsts
	b = false;
	queue<string> qs;
	while (b == false)
	{
		b = true;
		for (int i = 0; i < firsts.size(); i++)
		{
			for (auto j : firsts[i])
			{
				if (j[0] >= 'A' && j[0] <= 'Z')//first[i][j] is nonterminals.
				{
					for (int k = 0; k < nonterminals.size(); k++)
					{
						if (nonterminals[k] == j)
						{
							for (auto& l : firsts[k])
							{
								firsts[i].insert(l);
								if (l[0] >= 'A' && l[0] <= 'Z')
									b = false;
							}
							qs.push(j);//delete the old nonterminals.
							break;
						}
					}
				}
			}
			while (!qs.empty())//delete the old nonterminals.
			{
				ter = qs.front();
				qs.pop();
				firsts[i].erase(firsts[i].find(ter));
			}
		}
	}
}
void State_i_Closure()
{
	while (stateNum + 1 < states.size())
	{
		++stateNum;
		// add reduce and push queue the closure derivations
		for (int i = 0; i < states[stateNum].ders.size(); i++)
		{
			if (states[stateNum].ders[i].closure_pos == -1)
				for (auto& j : states[stateNum].ders[i].follows)
					states[stateNum].actions[j] = make_pair(0, states[stateNum].ders[i].rNum);
			else qi.push(i);
		}
		while (!qi.empty())
		{
			n = qi.front();
			while (!qi.empty() && qi.front() == n)qi.pop();
			der_t = states[stateNum].ders[n];
			string st = rulesToken_departed[der_t.rNum][der_t.closure_pos];
			if (st[0] >= 'A' && st[0] <= 'Z')// if the clo_pos is nonterminal
			{
				for (int i = 0; i < rules.size(); i++)
				{
					if (rules[i].first == st)//find the closure derivations
					{
						der dr;
						dr.rNum = i;

						// decied next token of this closure whether need to closure
						s = rulesToken_departed[i][0];
						if (s[0] >= 'A' && s[0] <= 'Z')// next closure 0 is nonterminal
							qi.push(states[stateNum].ders.size());// push the der pos

						// find dr.follow
						if (der_t.closure_pos + 1 >= rulesToken_departed[der_t.rNum].size())// next token of der_t is the end. => follow
							dr.follows = der_t.follows;
						else
						{
							s = rulesToken_departed[der_t.rNum][der_t.closure_pos + 1];
							if (s[0] >= 'A' && s[0] <= 'Z')//next token of der_t is nonterminal
								dr.follows = GetFirst(s);
							else //next token of der_t is terminal
								dr.follows.insert(s);
						}
						// check whether the derivation exits
						b = true;
						for (auto& j : states[stateNum].ders)
						{
							if (j == dr)
							{
								b = false;
								break;
							}
						}
						if (b == true)
							states[stateNum].ders.push_back(dr);
					}
				}
			}
		}
		b = true;
		for (int i = 0; i < stateNum; i++)
		{
			if (states[i] == states[stateNum])
			{
				b = false;
				break;
			}
		}
		if (b == true)OutputState(states[stateNum], stateNum);
		else states.erase(states.begin() + stateNum--);
	}
}
void ClosureDepart(int sNum)
{
	// nonterminals closure
	for (int i = 1; i < nonterminals.size(); i++)
	{
		sta sta_tmp;// state for nonterminals i
		for (int j = 0; j < states[sNum].ders.size(); j++)
		{
			if (states[sNum].ders[j].closure_pos == -1)continue;
			string str3 = rulesToken_departed[states[sNum].ders[j].rNum][states[sNum].ders[j].closure_pos];
			if (str3 == nonterminals[i])// find the nonterminal i 
			{
				der_t = states[sNum].ders[j];
				++der_t.closure_pos;
				if (der_t.closure_pos >= rulesToken_departed[der_t.rNum].size())
					der_t.closure_pos = -1;
				// check whether the derivation exits
				b = true;
				for (auto& j : sta_tmp.ders)
				{
					if (j == der_t)
					{
						b = false;
						break;
					}
				}
				if (b == true)
					sta_tmp.ders.push_back(der_t);
			}
		}
		if (!sta_tmp.ders.empty())
		{
			states.push_back(sta_tmp);
			states[sNum].GoTo[nonterminals[i]] = states.size() - 1;
		}
	}
	// terminal closure
	for (int i = 0; i < terminals.size(); i++)
	{
		sta sta_tmp;// state for terminals i
		for (int j = 0; j < states[sNum].ders.size(); j++)
		{
			if (states[sNum].ders[j].closure_pos == -1)continue;
			string str3 = rulesToken_departed[states[sNum].ders[j].rNum][states[sNum].ders[j].closure_pos];
			if (str3 == terminals[i])// find the terminal i 
			{
				der_t = states[sNum].ders[j];
				++der_t.closure_pos;
				if (der_t.closure_pos >= rulesToken_departed[der_t.rNum].size())
					der_t.closure_pos = -1;
				// check whether the derivation exits
				b = true;
				for (auto& j : sta_tmp.ders)
				{
					if (j == der_t)
					{
						b = false;
						break;
					}
				}
				if (b == true)
					sta_tmp.ders.push_back(der_t);
			}
		}
		if (!sta_tmp.ders.empty())
		{
			states.push_back(sta_tmp);
			states[sNum].actions[terminals[i]] = make_pair(1, states.size() - 1);
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
	ifstream ifs("3_grammar.txt", ios::in);
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
					for (int l = j + 1; l < nonterminals[k].size(); l++)
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
	// store rule token
	rulesToken_departed.assign(rules.size(), vector<string>());
	for (int i = 0; i < rules.size(); i++)
	{
		for (int j = 0; j < rules_token[i].size();)
		{
			string str4;
			str4 = *rules_token[i][j];
			int x = 1;
			while (x < rules_token[i].size()
				&& *(rules_token[i][j] + x) != '\0')
				str4 += *(rules_token[i][j] + x++);
			rulesToken_departed[i].push_back(str4);
			j += x;
		}
	}
	// find firsts.
	FindFirst();





	// output states
	cout << "[\"" << rules[0].first << "->." << rules[0].second << "\"";
	for (auto& i : rules)
	{
		if (i != rules[0])
			cout << ", \'" << i.first << "->" << i.second << "\'";
	}
	cout << "]\n///////////////// state ////////////////////\n";

	//Closure_0();
	// State 0
	sta sta_0;
	// derivations need to be closure
	der_t.follows.clear();
	der_t.follows.insert("$");
	der_t.rNum = 0;
	sta_0.ders.push_back(der_t);
	qi.push(0);
	// closure 0
	while (!qi.empty())
	{
		n = qi.front();
		while (!qi.empty() && qi.front() == n)qi.pop();
		der_t = sta_0.ders[n];
		string st = rulesToken_departed[der_t.rNum][der_t.closure_pos];
		if (st[0] >= 'A' && st[0] <= 'Z')// if the clo_pos is nonterminal
		{
			for (int i = 0; i < rules.size(); i++)
			{
				if (rules[i].first == st)//find the closure derivations
				{
					der dr;
					dr.rNum = i;

					// decied next token of this closure whether need to closure
					s = rulesToken_departed[i][0];
					if (s[0] >= 'A' && s[0] <= 'Z')// next closure 0 is nonterminal
						qi.push(sta_0.ders.size());// push the der pos

					// find dr.follow
					if (der_t.closure_pos + 1 >= rulesToken_departed[der_t.rNum].size())// next token of der_t is the end. => follow
						dr.follows = der_t.follows;
					else
					{
						s = rulesToken_departed[der_t.rNum][der_t.closure_pos + 1];
						if (s[0] >= 'A' && s[0] <= 'Z')//next token of der_t is nonterminal
							dr.follows = GetFirst(s);
						else //next token of der_t is terminal
							dr.follows.insert(s);
					}
					// check whether the derivation exits
					b = true;
					for (auto& j : sta_0.ders)
					{
						if (j == dr)
						{
							b = false;
							break;
						}
					}
					if (b == true)
						sta_0.ders.push_back(dr);
				}
			}
		}
	}
	states.push_back(sta_0);
	OutputState(sta_0, stateNum);
	int sNum_CD = 0;// state i closure depart
	while (sNum_CD < states.size())
	{
		// closure 0 depart
		ClosureDepart(sNum_CD++);
		// state i closure
		State_i_Closure();
	}
}
