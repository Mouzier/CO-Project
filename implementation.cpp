#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <algorithm>
using namespace std;

int registers[32] = {0};
int memory[1024] = {0};
unordered_map<string , int> mapping;
unordered_map<string,int> labels;
string Base = "0x10010000";
int curIns;
int si = 0;


bool isNumber(string s)
{
  if(s == " " or s == " " or s=="")
    return false;
    if(s[0] == ' ')
    return false;
  int i=0;
  if(s[i] == '+' or s[i] == '-')
    i++;
	for ( ; i < s.length(); i++)
		if (s[i]<48 or s[i]>57)
			return false;

	return true;
}

void Exit()
{
  cout << "Syntax error in line " << curIns << endl;
  exit(0);
}

void findLabels(vector<string> &instructions)
{
  for(int i=0 ; i<instructions.size() ; i++)
  {
    string curLine = instructions[i];
    string word="";
    int j;
    for(j=0 ; j<curLine.length() ; j++)
    {
      if(curLine[j] == ' ' and word.length() == 0)
        continue;

      if(word.length() > 0 and curLine[j] == ' ')
        break;
      word += curLine[j];

    }

    //cout << word << endl;
    int len = word.length();
    if(len > 1 and word[len-1] == ':')
    {
      word.pop_back();
      labels[word] = i;
    }
    else if(word == ".word")
    {
      //cout << "s\n";
      string number = "";
        while(j<curLine.length())
        {
          if(curLine[j] == ' ' or curLine[j] == ',' )
          {
            if(isNumber(number))
            {
              memory[si++] = stoi(number);
            }
            number = "";
          }
          else
          {
            number += curLine[j];
          }
          j++;
        }

          if(isNumber(number))
          {
            memory[si++] = stoi(number);
            number = "";
          }

    }
  }

}

void Add(string &line , int i)
{
  string word = "";
  vector<string> reg;
  for( ; i<line.length() ; i++)
  {
    if(reg.size() == 3)
    {
      break;
    }
    if(line[i] == ' '  or line[i] == ',')
    {
      if(word == "")
        continue;

      else
      {
        reg.push_back(word);
        word = "";
      }
    }
    else
    {
      word += line[i];
    }
  }

  reg.push_back(word);

  // should check for any syntax errors
  if(reg.size() < 3)
  {
    Exit();
  }
  if(mapping.count(reg[0])==0 or mapping.count(reg[1])==0 or mapping.count(reg[2])==0)
  {
      Exit();
  }

  registers[mapping[reg[0]]] = registers[mapping[reg[1]]] + registers[mapping[reg[2]]];
}

void Addi(string &line , int i)
{
  string word = "";
  vector<string> reg;
  for( ; i<line.length() ; i++)
  {
    if(reg.size() == 3)
    {
      break;
    }
    if(line[i] == ' ' or line[i] == ',')
    {
      if(word == "")
        continue;

      else
      {
        reg.push_back(word);
        word = "";
      }
    }
    else
    {
      word += line[i];
    }
  }

  reg.push_back(word);

  // should check for any syntax errors
  if(reg.size() < 3)
  {
    Exit();
  }
  if(mapping.count(reg[0])==0 or mapping.count(reg[1])==0 or !isNumber(reg[2]))
  {
    Exit();
  }

  registers[mapping[reg[0]]] = registers[mapping[reg[1]]] + stoi(reg[2]);
}

void Sub(string &line , int i)
{
  string word = "";
  vector<string> reg;
  for( ; i<line.length() ; i++)
  {
    if(reg.size() == 3)
    {
      break;
    }
    if(line[i] == ' '  or line[i] == ',')
    {
      if(word == "")
        continue;

      else
      {
        reg.push_back(word);
        word = "";
      }
    }
    else
    {
      word += line[i];
    }
  }

  reg.push_back(word);

  // should check for any syntax errors
  if(reg.size() < 3)
  {
    Exit();
  }
  if(mapping.count(reg[0])==0 or mapping.count(reg[1])==0 or mapping.count(reg[2])==0)
  {
    Exit();
  }

  registers[mapping[reg[0]]] = registers[mapping[reg[1]]] - registers[mapping[reg[2]]];
}

void Subi(string &line , int i)
{
  string word = "";
  vector<string> reg;
  for( ; i<line.length() ; i++)
  {
    if(reg.size() == 3)
    {
      break;
    }
    if(line[i] == ' ' or line[i] == ',')
    {
      if(word == "")
        continue;

      else
      {
        reg.push_back(word);
        word = "";
      }
    }
    else
    {
      word += line[i];
    }
  }

  reg.push_back(word);

  // should check for any syntax errors
  if(reg.size() < 3)
  {
    Exit();
  }
  if(mapping.count(reg[0])==0 or mapping.count(reg[1])==0 or !isNumber(reg[2]))
  {
      Exit();
  }

  registers[mapping[reg[0]]] = registers[mapping[reg[1]]] - stoi(reg[2]);
}

pair<bool , int> BNE(string &line , int i)
{
  string word = "";
  vector<string> reg;
  for( ; i<line.length() ; i++)
  {
    if(reg.size() == 3)
    {
      break;
    }
    if(line[i] == ' ' or line[i] == ',')
    {
      if(word == "")
        continue;

      else
      {
        reg.push_back(word);
        word = "";
      }
    }
    else
    {
      word += line[i];
    }
  }

  reg.push_back(word);

  if(reg.size() < 3)
  {
    Exit();
  }

  if(mapping.count(reg[0])==0 or mapping.count(reg[1])==0)
  {
    Exit();
  }
  if(labels.count(reg[2])==0)
  {
    cout << "No such label exists... on line " << curIns << endl;
    exit(0);
  }
  pair<bool , int> p;
  if(registers[mapping[reg[0]]] != registers[mapping[reg[1]]])
  {
    p.first = true;
    p.second = labels[reg[2]];

  }
  else
  {
    p.first = false;
    p.second = -1;
  }

  return p;
}

pair<bool , int> BLE(string &line , int i)
{
  string word = "";
  vector<string> reg;
  for( ; i<line.length() ; i++)
  {
    if(reg.size() == 3)
    {
      break;
    }
    if(line[i] == ' ' or line[i] == ',')
    {
      if(word == "")
        continue;

      else
      {
        reg.push_back(word);
        word = "";
      }
    }
    else
    {
      word += line[i];
    }
  }

  reg.push_back(word);

  if(reg.size() < 3)
  {
    Exit();
  }

  if(mapping.count(reg[0])==0 or mapping.count(reg[1])==0)
  {
    cout << line << endl;
    Exit();
  }
  if(labels.count(reg[2])==0)
  {
    cout << "No such label exists... on line " << curIns << endl;
    exit(0);
  }

  pair<bool , int> p;
  if(registers[mapping[reg[0]]] <= registers[mapping[reg[1]]])
  {
    p.first = true;
    p.second = labels[reg[2]];

  }
  else
  {
    p.first = false;
    p.second = -1;
  }

  return p;
}

pair<bool , int> BEQ(string &line , int i)
{
  string word = "";
  vector<string> reg;
  for( ; i<line.length() ; i++)
  {
    if(reg.size() == 3)
    {
      break;
    }
    if(line[i] == ' ' or line[i] == ',')
    {
      if(word == "")
        continue;

      else
      {
        reg.push_back(word);
        word = "";
      }
    }
    else
    {
      word += line[i];
    }
  }

  reg.push_back(word);

  if(reg.size() < 3)
  {
    Exit();
  }
  if(mapping.count(reg[0])==0 or mapping.count(reg[1])==0)
  {
    cout << line << endl;
    Exit();
  }
  if(labels.count(reg[2])==0)
  {
    cout << "No such label exists... on line " << curIns << endl;
    exit(0);
  }

  pair<bool , int> p;
  if(registers[mapping[reg[0]]] == registers[mapping[reg[1]]])
  {
    p.first = true;
    p.second = labels[reg[2]];

  }
  else
  {
    p.first = false;
    p.second = -1;
  }

  return p;
}

int Jump(string &line , int i)
{
  string word = "";
  vector<string> reg;
  for( ; i<line.length() ; i++)
  {
    if(reg.size() == 1)
    {
      break;
    }
    if(line[i] == ' ' or line[i] == ',')
    {
      if(word == "")
        continue;

      else
      {
        reg.push_back(word);
        word = "";
      }
    }
    else
    {
      word += line[i];
    }
  }

  reg.push_back(word);

  if(reg.size() < 1)
  {
    Exit();
  }

  if(labels.count(reg[0]) == 0)
  {
    cout << "No such label exists... on line No " << curIns << endl;
    exit(0);
  }

  return labels[reg[0]];
}

void Li(string &line , int i)
{
  string word = "";
  vector<string> reg;
  for( ; i<line.length() ; i++)
  {
    if(reg.size() == 2)
    {
      break;
    }
    if(line[i] == ' '  or line[i] == ',')
    {
      if(word == "")
        continue;

      else
      {
        reg.push_back(word);
        word = "";
      }
    }
    else
    {
      word += line[i];
    }
  }

  reg.push_back(word);

  if(reg.size() < 2)
  {
    Exit();
  }
  if(mapping.count(reg[0]) == 0)
  {
    Exit();
  }
  string temp = reg[1];
  //cout << temp << endl;
  string type = "";
  type += temp[0];
  if(temp.length()>1)
  {
    type += temp[1];
  }

  int val=0;
  if(type == "0x")
  {
    val = stoi(temp , 0 , 16);
  }
  else
  val = stoi(temp);

  registers[mapping[reg[0]]] = val;

}

void Load(string &line , int i)
{
  string word = "";
  vector<string> reg;
  for( ; i<line.length() ; i++)
  {
    if(reg.size() == 2)
    {
      break;
    }
    if(line[i] == ' '  or line[i] == ',')
    {
      if(word == "")
        continue;

      else
      {
        reg.push_back(word);
        word = "";
      }
    }
    else
    {
      word += line[i];
    }
  }

  reg.push_back(word);

  if(reg.size() < 2)
  {
    Exit();
  }
  if(mapping.count(reg[0]) == 0)
  {
    Exit();
  }

  string temp="";
  string effAdd = reg[1];
  //bool check = false;
  int offSet = 0;
  int base = stoi(Base , 0 , 16);
  for(int j=0 ; j<effAdd.length() ; j++)
  {
    if(effAdd[j] == '(')
    {
      if(!isNumber(temp))
      {
        Exit();
      }
      offSet = stoi(temp);
      temp = "";
    }
    else if(effAdd[j] == ')')
    {
      if(mapping.count(temp) == 0)
      {
        Exit();
      }
      registers[mapping[temp]] += offSet;
      break;
    }
    else
    {
      temp += effAdd[j];
    }
  }

  int idx = (registers[mapping[temp]]-base)/4;

//cout << idx << " " << memory[idx] << " ";
  registers[mapping[reg[0]]] = memory[idx];
  registers[mapping[temp]] -= offSet;
//  cout << memory[idx] << endl;
}

void Store(string &line , int i)
{
  string word = "";
  vector<string> reg;
  for( ; i<line.length() ; i++)
  {
    if(reg.size() == 2)
    {
      break;
    }
    if(line[i] == ' '  or line[i] == ',')
    {
      if(word == "")
        continue;

      else
      {
        reg.push_back(word);
        word = "";
      }
    }
    else
    {
      word += line[i];
    }
  }

  reg.push_back(word);

  if(reg.size() < 2)
  {
    Exit();
  }
  if(mapping.count(reg[0]) == 0)
  {
    Exit();
  }

  string temp="";
  string effAdd = reg[1];
  //bool check = false;
  int offSet = 0;
  int base = stoi(Base , 0 , 16);
  for(int j=0 ; j<effAdd.length() ; j++)
  {
    if(effAdd[j] == '(')
    {
      if(!isNumber(temp))
      {
        Exit();
      }
      offSet = stoi(temp);
      temp = "";
    }
    else if(effAdd[j] == ')')
    {
      if(mapping.count(temp) == 0)
      {
        Exit();
      }
      registers[mapping[temp]] += offSet;
      break;
    }
    else
    {
      temp += effAdd[j];
    }
  }

  int idx = (registers[mapping[temp]]-base)/4;
  //cout <<  idx << " " << memory[idx] << " ";
  memory[idx] = registers[mapping[reg[0]]];
  registers[mapping[temp]] -= offSet;
//  cout << memory[idx] << endl;
}

void run(string line , int &lineNo)
{
  string word = "";
  for(int i=0 ; i<line.length() ; i++)
  {

      if(line[i] == ' ')
      {

        if(word == "")
          continue;

        else if(word == "add")
        {
            lineNo++;
            Add(line , i+1);
            return;
        }
        else if(word == "addi")
        {
            lineNo++;
            Addi(line , i+1);
            return;
        }
        else if(word == "sub")
        {
            lineNo++;
            Sub(line , i+1);
            return;
        }
        else if(word == "subi")
        {
            lineNo++;
            Subi(line , i+1);
            return;
        }
        else if(word == "bne")
        {
            pair<bool , int> check = BNE(line , i);

            if(check.first)
            {
                lineNo = check.second;
            }
            else
            {
              lineNo++;
            }

            return;
        }
        else if(word == "ble")
        {
          pair<bool , int> check = BLE(line , i);

          if(check.first)
          {
              lineNo = check.second;
          }
          else
          {
            lineNo++;
          }

          return;
        }
        else if(word == "beq")
        {
          pair<bool , int> check = BEQ(line , i);

          if(check.first)
          {
              lineNo = check.second;
          }
          else
          {
            lineNo++;
          }

          return;
        }
        else if(word == "j")
        {
            lineNo = Jump(line , i);
            return;
        }
        else if(word == "li")
        {
          lineNo++;
          Li(line , i+1);
          return;
        }
        else if(word == "lw")
        {
            lineNo++;
            Load(line , i+1);
            return;
        }
        else if(word == "sw")
        {
            lineNo++;
            Store(line , i+1);
            return;
        }
        else
        {
          lineNo++;
          return;
        }
      }
      word += line[i];

  }

  lineNo++;
}

void setMapping()
{
  mapping["$zero"] = 0;
  mapping["$at"] = 1;
  mapping["$v0"] = 2;
  mapping["$v1"] = 3;
  mapping["$a0"] = 4;
  mapping["$a1"] = 5;
  mapping["$a2"] = 6;
  mapping["$a3"] = 7;
  mapping["$t0"] = 8;
  mapping["$t1"] = 9;
  mapping["$t2"] = 10;
  mapping["$t3"] = 11;
  mapping["$t4"] = 12;
  mapping["$t5"] = 13;
  mapping["$t6"] = 14;
  mapping["$t7"] = 15;
  mapping["$s0"] = 16;
  mapping["$s1"] = 17;
  mapping["$s2"] = 18;
  mapping["$s3"] = 19;
  mapping["$s4"] = 20;
  mapping["$s5"] = 21;
  mapping["$s6"] = 22;
  mapping["$s7"] = 23;
  mapping["$t8"] = 24;
  mapping["$t9"] = 25;
  mapping["$k0"] = 26;
  mapping["$k1"] = 27;
  mapping["$gp"] = 28;
  mapping["$sp"] = 29;
  mapping["$s8"] = 30;
  mapping["$ra"] = 31;

}


int main()
{
  string line;
  string s;
  cin >> s;
  ifstream file(s);
  if(!file.is_open())
  {
    cout << "error" << endl;
  }

  vector<string> instructions;

  while(getline(file , line))
  {
    string temp = line;
    instructions.push_back(temp);
  }
  file.close();

  findLabels(instructions);
  setMapping();

  int lineNo = 0;
  while(lineNo < instructions.size())
  {
    curIns = lineNo+1;
    run(instructions[lineNo] , lineNo);
  }
  cout << endl;

  cout << "Registers:\n";
  for(int i=0 ; i<32 ; i++)
  {
    cout << registers[i] << " ";
  }
  cout << endl;
  cout << endl;
  cout << "Memory:\n";
  for(int i=0 ; i<1024 ; i++)
  {
    cout << memory[i] << " ";
  }
  cout << endl;
  cout << endl;

}
