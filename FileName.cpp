#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <queue>
using namespace std;
int main()
{
	// space allocated
	vector<string> terminals;
	vector<string> nonterminals;
	vector<pair<string, string>> rules;
	vector<vector<char*>> rules_token;
	vector<string> testdata;
	struct der
	{
		// d1->d2,follows
		string d1;
		string d2;
		int closure_pos = 0;//-1 denotes end position
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
	queue<pair<string, vector<string>>> q;//closure NT, follows



	// read testdata file
	ifstream ifst("3_testdata.txt", ios::in);
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
			nonterminals.push_back(ter), ter = "";
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
	// deals with rules
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
	// S->.AA
	if (*rules_token[stateNum][0] >= 'A' && *rules_token[stateNum][0] <= 'Z')
	{
		s = *rules_token[stateNum][0];
		n = 1;
		while (n < rules_token[stateNum].size())
			s += *(rules_token[stateNum][0] + n++);
		vector<string> vs;
		vs.push_back("$");
		q.push(make_pair(s, vs));
	}
	der_t.follows.push_back("$");
	sta_t.ders.push_back(der_t);


	// closure
	while (!q.empty())
	{
		ter = q.front().first;
		for (int i = 0; i < rules.size(); i++)
		{
			if (rules[i].first == ter)
			{
				der dr;
				dr.d1 = rules[i].first;
				dr.d2 = rules[i].second;
				if (*rules_token[i][0] >= 'A' && *rules_token[i][0] <= 'Z')
				{
					s = *rules_token[i][0];
					n = 1;
					while (n < rules_token[i].size())
						s += *(rules_token[i][0] + n++);
					vector<string> vs;
					vs.push_back("$");
					q.push(make_pair(s, vs));
				}
				dr.follows.push_back("$");
				sta_t.ders.push_back(dr);
			}
		}
	}
}