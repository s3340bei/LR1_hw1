#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <queue>
#include <map>
#include <set>
#include <stack>
using namespace std;

// space allocated
vector<string> terminals;
vector<string> nonterminals;
vector<pair<string, string>> rules;
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
vector<vector<string>> parsingTable;
vector<string> testToken_departed;


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
				for (int i = 1; i < rules.size(); i++)
				{
					if (rules[i].first != st)continue;//find the closure derivations
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
		// test if the state is exits
		b = true;
		for (int i = 0; i < stateNum; i++)
		{
			if (states[i] == states[stateNum])
			{
				// point to deleting state : point to the existing same state
				// point to after the deleting state : +1
				for (int j = 0; j < states.size(); j++)
				{
					if (j == stateNum)continue;
					for (auto& k : states[j].actions)
					{
						if (k.second.first == true && k.second.second == stateNum)
							k.second.second = i;
						else if (k.second.first == true && k.second.second > stateNum)
							k.second.second -= 1;
					}
					for (auto& k : states[j].GoTo)
					{
						if (k.second == stateNum)
							k.second = i;
						else if (k.second > stateNum)
							k.second -= 1;
					}
				}
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
void Output_grid(char cBig, char cSmall, string s, int maxSize)
{
	int x1 = (maxSize - s.size()) / 2;
	cout << cBig;
	for (int i = 0; i < x1; i++)cout << cBig;
	cout << s;
	for (int i = 0; i < (maxSize - s.size()) - x1; i++)cout << cBig;
	cout << cBig << cSmall;
}
int getTnumber(int x)
{
	if (x >= testToken_departed.size())return 0;
	else s = testToken_departed[x];
	for (int i = 0; i < terminals.size(); i++)
		if (terminals[i] == s)return i + 1;
	return 0;
}
void Output_stack(char cBig, char cSmall, string s, int maxSize)
{
	cout << cBig << s;
	for (int i = 0; i < (maxSize - s.size()); i++)cout << cBig;
	cout << cBig << cSmall;
}
void Output_input(char cBig, char cSmall, string s, int maxSize)
{
	cout << cBig;
	for (int i = 0; i < (maxSize - s.size()); i++)cout << cBig;
	cout << s << cBig << cSmall;
}

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
	rulesToken_departed.assign(rules.size(), vector<string>());
	for (int i = 0; i < rules.size(); i++)
	{
		string tes = rules[i].second;
		for (int j = 0; j < tes.length(); )//j: index of rules[i]
		{
			for (auto& k : terminals)
			{
				if (tes[j] != k[0])continue;
				bool bo1 = true;
				for (int l = j + 1; l < k.length(); l++)
				{
					if (tes[l] != k[l - j])
					{
						bo1 = false; break;
					}
				}
				if (bo1 == false)continue;
				rulesToken_departed[i].push_back(k);
				j += k.size();
				break;
			}
			for (auto& k : nonterminals)
			{
				if (tes[j] != k[0])continue;
				bool bo1 = true;
				for (int l = j + 1; l < k.length(); l++)
				{
					if (tes[l] != k[l - j])
					{
						bo1 = false; break;
					}
				}
				if (bo1 == false)continue;
				rulesToken_departed[i].push_back(k);
				j += k.size();
				break;
			}
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
	// delete the nonterminal S'
	nonterminals.erase(nonterminals.begin());


	// output parsing table
	cout << "///////////////// parsing table ////////////////////\n+";
	// add into parsingTable
	parsingTable.assign(states.size(), vector<string>());
	for (int i = 0; i < states.size(); i++)
	{
		// $
		if (states[i].actions.find("$") == states[i].actions.end())// empty
			parsingTable[i].push_back("");
		else
		{
			if (states[i].actions["$"].second == 0 && states[i].actions["$"].first == false)
				parsingTable[i].push_back("Acc");// r0 is Accept.
			else
			{
				if (states[i].actions["$"].first == false)s = "r";//reduce					
				else s = "s";// shift
				s += to_string(states[i].actions["$"].second);
				parsingTable[i].push_back(s);
			}
		}
		// terminals
		for (auto& j : terminals)
		{
			if (states[i].actions.find(j) == states[i].actions.end())// empty
				parsingTable[i].push_back("");
			else
			{
				if (states[i].actions[j].first == false)s = "r";//reduce					
				else s = "s";// shift
				s += to_string(states[i].actions[j].second);
				parsingTable[i].push_back(s);
			}
		}
		// nonterminals
		for (auto& j : nonterminals)
		{
			if (states[i].GoTo.find(j) == states[i].GoTo.end())// empty
				parsingTable[i].push_back("");
			else
			{
				s = to_string(states[i].GoTo[j]);
				parsingTable[i].push_back(s);
			}
		}
	}
	// find the vertical max size
	vector<int> maxSizeV;
	maxSizeV.push_back(to_string(states.size() - 1).size());
	int x = 0;
	for (int j = 0; j < parsingTable[0].size(); j++)
	{
		n = 0;
		if (j > 0 && j <= terminals.size() && n < terminals[j - 1].size())
			n = terminals[j - 1].size();
		else if (j > terminals.size() && n < nonterminals[j - terminals.size() - 1].size())
			n = nonterminals[j - terminals.size() - 1].size();
		for (int i = 0; i < parsingTable.size(); i++)
		{
			if (parsingTable[i][j].size() > n)
				n = parsingTable[i][j].size();
		}
		maxSizeV.push_back(n);
	}
	// output line 1 : +---+-----+---+
	for (int i = 0; i < maxSizeV.size(); i++)
		Output_grid('-', '+', "", maxSizeV[i]);
	// output line 2 : |  | $ | a | S |
	n = 0;
	cout << "\n|";
	Output_grid(' ', '|', "", maxSizeV[n++]);
	Output_grid(' ', '|', "$", maxSizeV[n++]);
	for (auto& j : terminals)
		Output_grid(' ', '|', j, maxSizeV[n++]);
	for (auto& j : nonterminals)
		Output_grid(' ', '|', j, maxSizeV[n++]);
	// output line 3 : +---+-----+---+
	cout << "\n+";
	for (int i = 0; i < maxSizeV.size(); i++)
		Output_grid('-', '+', "", maxSizeV[i]);
	// output line 4 : states, actions, and GoTo
	for (int i = 0; i < states.size(); i++)
	{
		n = 0;
		cout << "\n|";
		Output_grid(' ', '|', to_string(i), maxSizeV[n++]);
		for (int j = 0; j < parsingTable[0].size(); j++)
			Output_grid(' ', '|', parsingTable[i][j], maxSizeV[n++]);
	}
	// output parsing table end
	cout << "\n+";
	for (int i = 0; i < maxSizeV.size(); i++)
		Output_grid('-', '+', "", maxSizeV[i]);




	// test testdata
	for (auto& tes : testdata)
	{
		// testdata token
		testToken_departed.clear();
		for (int j = 0; j < tes.length(); )//j: index of testdata[i]
		{
			b = false;
			for (auto& k : terminals)
			{
				if (tes[j] != k[0])continue;
				bool bo1 = true;
				for (int l = j + 1; l < k.length(); l++)
				{
					if (tes[l] != k[l - j])
					{
						bo1 = false; break;
					}
				}
				if (bo1 == false)continue;
				testToken_departed.push_back(k);
				j += k.size();
				b = true;
				break;
			}
			if (b == false)break;
		}
		// output testdata line 1 : parsing: ab
		cout << "\nparsing: " << tes << "\n";
		// invalid character
		if (b == false)
		{
			cout << "Invalid character exist!\nresult: Invalid!\n";
			continue;
		}
		// stack, input and actions
		stack<string> stack1;
		stack1.push("0");
		vector<string> output_stack;
		vector<string> output_input;
		vector<string> output_action;
		int in_pos = 0;
		output_stack.push_back("0");
		output_input.push_back(tes);
		string act = parsingTable[0][getTnumber(in_pos)];
		if (act == "")act = "X";
		output_action.push_back(act);
		while (act != "Acc")
		{
			if (act == "X")break;
			// stack_num
			int ac_num = 0;// the last action number
			for (int j = 1; j < act.size(); j++)ac_num = ac_num * 10 + (act[j] - '0');
			if (act[0] == 's')// shift
			{
				// if in_pos > testdata
				if (in_pos > testToken_departed.size())
				{
					act = "X"; break;
				}
				// stack
				stack1.push(testToken_departed[in_pos++]);
				s = output_stack[output_stack.size() - 1];
				output_stack.push_back(s + stack1.top() + to_string(ac_num));
				stack1.push(to_string(ac_num));
				// input
				s = "";
				for (int j = in_pos; j < testToken_departed.size(); j++)
					s += testToken_departed[j];
				output_input.push_back(s + "$");
				// action
				act = parsingTable[ac_num][getTnumber(in_pos)];
				if (act == "")act = "X";
				output_action.push_back(act);
			}
			else// reduce
			{
				// reduce
				string rs = output_stack[output_stack.size() - 1];// rs is the last stack string
				int count = 0;// count reduce string length
				for (int j = rulesToken_departed[ac_num].size() * 2; j > 0; j--)
				{
					count += stack1.top().length();
					stack1.pop();
				}
				rs = rs.substr(0, rs.length() - count);// rs: after reduce
				// stack
				s = stack1.top();
				count = 0;// count the before Goto state number
				for (int j = 0; j < s.length(); j++)count = count * 10 + (s[j] - '0');
				stack1.push(rules[ac_num].first);
				act = to_string(states[count].GoTo[stack1.top()]);// Goto state number
				output_stack.push_back(rs + stack1.top() + act);
				stack1.push(act);
				// input
				output_input.push_back(output_input[output_input.size() - 1]);
				// action
				count = 0;// count the state number
				for (int j = 0; j < act.length(); j++)count = count * 10 + (act[j] - '0');
				act = parsingTable[count][getTnumber(in_pos)];
				if (act == "")act = "X";
				output_action.push_back(act);
			}
		}
		// output result
		if (act == "Acc")cout << "result: Valid!\n+";
		else cout << "result: Invalid!\n+";
		int maxS = 5; int maxI = 5; int maxA = 6;
		for (auto& j : output_stack)if (j.length() > maxS)maxS = j.length();
		for (auto& j : output_input)if (j.length() > maxI)maxI = j.length();
		for (auto& j : output_action)if (j.length() > maxA)maxA = j.length();
		Output_grid('-', '+', "", maxS);
		Output_grid('-', '+', "", maxI);
		Output_grid('-', '+', "", maxA); cout << "\n|";
		Output_grid(' ', '|', "stack", maxS);
		Output_grid(' ', '|', "input", maxI);
		Output_grid(' ', '|', "action", maxA); cout << "\n+";
		Output_grid('-', '+', "", maxS);
		Output_grid('-', '+', "", maxI);
		Output_grid('-', '+', "", maxA);
		for (int j = 0; j < output_stack.size(); j++)
		{
			cout << "\n|";
			Output_stack(' ', '|', output_stack[j], maxS);
			Output_input(' ', '|', output_input[j], maxI);
			Output_grid(' ', '|', output_action[j], maxA);
		}
		cout << "\n+";
		Output_grid('-', '+', "", maxS);
		Output_grid('-', '+', "", maxI);
		Output_grid('-', '+', "", maxA); cout << "\n";
	}
}