#include <iostream>
#include <string>
#include <vector>
#include <list>
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
int cycles=0;
int stalls=0;
bool forwarding;
int pipeline[5] = {0,-1,-1,-1,-1};
list<string> last2;

int block1_size;
int block2_size;
int set1_size;
int set2_size;
int cache1_latency;
int cache2_latency;
int memory_latency;
int l1_accesses = 0;
int l1_misses = 0;
int l2_accesses = 0;
int l2_misses = 0;



class Block
{
public:

  int tag;
  int lru;

  char data[100];

  Block(int tagPart = -1)
  {
    tag = tagPart;
    // update this value
    if(tagPart == -1)
    lru = -1;
    else lru = 0;
  }

};

class Set
{
public:

  Block **blks;
  int num_blks_set;

  Set(int num_blk)
  {
    this->num_blks_set = num_blk;
    blks = new Block*[num_blk];

    for(int i=0 ; i<num_blk ; i++)
    {
      blks[i] = new Block();
    }
  }

  bool look(int tagPart)
  {
    for(int i=0 ; i<num_blks_set ; i++)
    {
      if(tagPart == blks[i]->tag)
      {
        // update the lru of this block
        updateLRU(i);
        return true;
      }
    }
    return false;
  }

  void updateLRU(int idx)
  {
    for(int i=0 ; i<num_blks_set ; i++)
    {
      if(blks[i]->lru < blks[idx]->lru and blks[i]->lru!=-1)
      {
        blks[i]->lru++;
      }
    }

    blks[idx]->lru = 0;
  }

  void insertion(int tagPart)
  {
    Block *new_blk = new Block(tagPart);

    int idx = 0;

    for(int i=0 ; i<num_blks_set ; i++)
    {
      if(blks[i]->lru == -1)
      {
        idx = i;
        break;
      }
      if(blks[i]->lru >= blks[idx]->lru)
      {
        idx = i;
      }

      blks[i]->lru++;

    }

    blks[idx] = new_blk;
  }
};

class Cache
{
public:

  Set **sets;
  int num_set_cache;

  Cache(int num_set , int num_blk)
  {
    this->num_set_cache = num_set;
    sets = new Set*[num_set];

    for(int i=0 ; i<num_set ; i++)
    {
      sets[i] = new Set(num_blk);
    }
  }

  pair<int , int> extract_elements(int64_t addr , int block_size , int set_size)
  {
    while(block_size != 1)
    {
      block_size >>= 1;
      addr >>= 1;
    }

    string temp = "";

    while(set_size!=1)
    {
      set_size >>= 1;
      if(addr&1)
      {
        temp += '1';
      }
      else
      {
        temp += '0';
      }
      addr >>= 1;
    }
    int p=1;
    int ans = 0;
    int i=0;
    while(i < temp.size())
    {
      if(temp[i] == '1')
      {
        ans = ans + (p);
      }
      p *= 2;
      i++;
    }

    return {ans , addr};
  }

  bool access(int64_t addr , int block_size , int set_size)
  {
    pair<int , int> elements = extract_elements(addr , block_size , set_size);
    int tag = elements.second;
    int set_no = elements.first;
    bool look_blk = sets[set_no]->look(tag);

    return look_blk;
  }

  void insert(int tagPart , int setPart)
  {
    sets[setPart]->insertion(tagPart);
  }

};

Cache *l1cache , *l2cache;

pair<int , int> extract_elements(int64_t addr , int block_size , int set_size)
{
  while(block_size != 1)
  {
    block_size >>= 1;
    addr >>= 1;
  }

  string temp = "";

  while(set_size!=1)
  {
    set_size >>= 1;
    if(addr&1)
    {
      temp += '1';
    }
    else
    {
      temp += '0';
    }
    addr >>= 1;
  }
  int p=1;
  int ans = 0;
  int i=0;
  while(i < temp.size())
  {
    if(temp[i] == '1')
    {
      ans = ans + (p);
    }
    p *= 2;
    i++;
  }

  return {ans , addr};
}

int caching(int addr)
{
  //cout << "s ";

  l1_accesses++;

  bool l1_check = l1cache->access(addr , block1_size , set1_size);
  if(l1_check)
  {

    cycles = cycles + (cache1_latency - 1);
    stalls = stalls + (cache1_latency-1);
    return 1;
  }

  l1_misses++;
  l2_accesses++;

  bool l2_check = l2cache->access(addr , block2_size , set2_size);
  if(l2_check)
  {
    cycles = cycles + (cache1_latency + cache2_latency - 1);
    stalls = stalls + (cache1_latency + cache2_latency -1);

    pair<int , int> temp = extract_elements(addr , block1_size , set1_size);

    // have to insert this block in the l1 cache
    l1cache->insert(temp.second , temp.first);

    return 2;
  }

  l2_misses++;

  cycles = cycles + (cache1_latency + cache2_latency + memory_latency - 1);
  stalls = stalls + (cache1_latency + cache2_latency + memory_latency - 1);

  // have to insert this block in both the caches
  pair<int , int> temp = extract_elements(addr , block1_size , set1_size);
  l1cache->insert(temp.second , temp.first);

  temp = extract_elements(addr , block2_size , set2_size);
  l2cache->insert(temp.second , temp.first);

  return 3;
}


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

  cout << caching(registers[mapping[temp]]) << " # ";

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
  cout << caching(registers[mapping[temp]]) << " # ";

  //cout <<  idx << " " << memory[idx] << " ";
  memory[idx] = registers[mapping[reg[0]]];
  registers[mapping[temp]] -= offSet;
//  cout << memory[idx] << endl;


}

void run(string line , int lineNo)
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

bool pending()
{
  if(pipeline[0]!=-1 or pipeline[1]!=-1 or pipeline[2]!=-1 or pipeline[3]!=-1 or pipeline[4]!=-1)
    return true;
  return false;
}

string rf(string &line , string &r1 , string &r2 , int val)
{

  if(line == "")
  {
    return "";
  }

  vector<string> reg;
  string word = "";
  for(int i=0 ; i<line.length() ; i++)
  {
    if(reg.size() == 4)
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
  reg.push_back("/");
  word = reg[0];
  if(word == "sw" or word == "lw" or word == "beq" or word == "ble" or word=="bne")
  {
      r1 = reg[1];
      string tempo = reg[2];

      string temp = "";
      for(int j=0 ; j<tempo.length() ; j++)
      {
        if(tempo[j] == '(')
        {
            temp = "";
        }
        else if(tempo[j] == ')')
        {
          break;
        }
        else
        {
          temp += tempo[j];
        }
      }

      r2 = temp;

      return word;
  }
  if(val == 1)
  {
    r1 = reg[1];
    r2 = reg[2];
  }
  else
  {
    r1 = reg[2];
    r2 = reg[3];
  }

  return reg[0];
}

int check_dependencies(string &curIns)
{
  string r1 , r2 , r3 , r4, r5;
  r1 = r2 = r3 = r4 = "";
  string temp;
  string cur;
  string temp2;

  cur = rf(curIns , r1 , r2 , 2);
  temp = rf(last2.back() , r3 , r5 , 1);
  temp2 = rf(last2.front() , r4 , r5 , 1);

  //cout << curIns << " --- " << last2.back() << "---" << last2.front() << endl;
  //cout << r1 << "/" << r2 << "---";
  //cout << r3 << "/" << r4 << endl;

  if(cur == "j")
  return 0;
  if(r1==r3 or r2==r3)
  {
    if(temp == "beq" or temp == "bne" or temp == "ble" or temp == "j")
    {
      return 0;
    }

    if(forwarding)
    {
      if(cur == "beq" or cur=="bne" or cur == "ble")
      {
        if(temp == "lw")
        {
          return 2;
        }
        else
        {
          return 1;
        }
      }
      if(temp == "lw")
      {
        return 1;
      }
      return 0;
    }
    else
    {
      if(temp == "sw")
        return 0;

      return 2;
    }
  }

  if(r1==r4 or r2==r4)
  {

    if(temp2 == "beq" or temp2 == "bne" or temp2 == "ble" or temp2 == "j")
    {
      return 0;
    }

    if(forwarding)
    {

      if(cur == "beq" or cur=="bne" or cur == "ble")
      {
        if(temp == "lw")
        {
          return 1;
        }
        return 0;
      }
      return 0;
    }
    else
    {
      if(temp == "sw" or temp2 == "sw")
        return 0;

      return 1;
    }
  }

  return 0;
}

int proceed(string &line)
{
  string word = "";
  for(int i=0 ; i<line.length() ; i++)
  {

      if(line[i] == ' ')
      {

        if(word == "")
          continue;

        else if(word=="add" or word=="addi" or word=="sub" or word=="subi" or word=="li" or word=="lw" or word=="sw")
        {
            return 1;
        }

        else if(word == "bne" or word=="beq" or word=="ble" or word=="j")
        {
            return 2;
        }

        else
        {
          break;
        }
      }
      word += line[i];
    }

    //cout << word << endl;

    if(word=="add" or word=="addi" or word=="sub" or word=="subi" or word=="li" or word=="lw" or word=="sw")
    {
        return 1;
    }

    else if(word == "bne" or word=="beq" or word=="ble" or word=="j")
    {
        return 2;
    }

    return 0;
}

pair<bool , int> point(string &line)
{
  string word = "";
  pair<bool , int> check;
  for(int i=0 ; i<line.length() ; i++)
  {

      if(line[i] == ' ')
      {

        if(word == "")
          continue;

        else if(word == "bne")
        {
            check = BNE(line , i);

            break;
        }
        else if(word == "ble")
        {
          check = BLE(line , i);

          break;
        }
        else if(word == "beq")
        {
          check = BEQ(line , i);

          break;
        }
        else if(word == "j")
        {
          check.first = true;
          check.second = Jump(line , i);
          break;
        }

      }
      word += line[i];

  }

  return check;
}





int main()
{
  string line;
  cout << "Enter the file name\n";
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
/*  while(lineNo < instructions.size())
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
  cout << endl;*/

  cout << endl;

  cout << "Enter 1 to allow forwarding, or else 0\n";
  int temp;
  cin >> temp;
  temp==1? forwarding=1 : forwarding=0;

  cout << endl;
  cycles=0;
  lineNo = 0;
  stalls = 0;
  int insCount=0;
  last2.push_back("");
  last2.push_back("");
  int p=1;

  int cache1_size, cache2_size;
  int associativity1, associativity2;

  cout << "enter l1 cache size (in bytes):   ";
  cin >> cache1_size;
  cout << "enter l2 cache size (in bytes):   ";
  cin >> cache2_size;

  cout << "enter l1 cache's block size (in bytes):   ";
  cin >> block1_size;
  cout << "enter l2 cache's block size (in bytes):   ";
  cin >> block2_size;

  cout << "enter l1 cache's associativity (should be a valid one) :   ";
  cin >> associativity1;
  cout << "enter l2 cache's associativity (should be a valid one) :   ";
  cin >> associativity2;

  cout << "enter l1 cache's latency:   ";
  cin >> cache1_latency;
  cout << "enter l2 cache's latency:   ";
  cin >> cache2_latency;
  cout << "enter memory latency:   ";
  cin >> memory_latency;

  cout << endl;

  cout << "for all the memory accesses, here we are printing 1 if it's a hit in l1 cache, 2 if it's a hit from l2 cache, 3 if it goes to the memory" << endl;


  int no_blks = cache1_size/block1_size;
  int no_sets = no_blks/associativity1;
  set1_size = no_sets;

  l1cache = new Cache(no_sets , no_blks);


  no_blks = cache2_size/block2_size;
  no_sets = no_blks/associativity2;
  set2_size = no_sets;

  l2cache = new Cache(no_sets , no_blks);


  while(pending())
  {
      //cout << cycles << "___";
      /*for(int i=0 ; i<5 ; i++)
      cout << pipeline[i] << " ";
      cout << endl;;*/

      curIns = pipeline[0];
      if(curIns != -1)
      run(instructions[lineNo] , lineNo);

      int type;
      if(pipeline[0] == -1)
      type = 1;
      else
      type = proceed(instructions[pipeline[0]]);
      //cout << "type: " << type << endl;
      if(type == 0)
      {
        lineNo++;
        if(lineNo < instructions.size())
        {
          pipeline[0] = lineNo;
        }
        else
        {
          pipeline[0] = -1;
        }
        continue;
      }
      cycles++;
      if(pipeline[0] != -1)
      {
        insCount++;
      }

      if(forwarding)
      {
        last2.pop_back();
        last2.pop_back();
        if(pipeline[3] == -1)
          last2.push_back("");
        else
          last2.push_back(instructions[pipeline[3]]);

        if(pipeline[2] == -1)
          last2.push_back("");
        else
          last2.push_back(instructions[pipeline[2]]);

        pipeline[4] = pipeline[3];
        pipeline[3] = pipeline[2];
        int stall;

        if(pipeline[1] == -1)
          stall=0;
        else
        stall = check_dependencies(instructions[pipeline[1]]);

        //cout << "stall: " << stall << endl;
        cycles += stall;
        stalls += stall;
        if(stall)
        {
          //cout << stall << " ___ " << instructions[pipeline[1]] << endl;
          pipeline[4] = pipeline[3];
          pipeline[3] = -1;

        }
        pipeline[2] = pipeline[1];
        pipeline[1] = pipeline[0];

      }
      else
      {
        last2.pop_back();
        last2.pop_back();
        if(pipeline[3] == -1)
          last2.push_back("");
        else
          last2.push_back(instructions[pipeline[3]]);

        if(pipeline[2] == -1)
          last2.push_back("");
        else
          last2.push_back(instructions[pipeline[2]]);

          int stall;

          pipeline[4] = pipeline[3];
          pipeline[3] = pipeline[2];

          if(pipeline[1] == -1)
            stall=0;
          else
          stall = check_dependencies(instructions[pipeline[1]]);
          cycles += stall;
          stalls += stall;
          //cout << "stall: " << stall << endl;
          if(stall==2)
          {
           //  cout << stall << " ___ " << instructions[pipeline[1]] << endl;
            pipeline[4] = -1;
            pipeline[3] = -1;
          }
          else if(stall == 1)
          {
          //  cout << stall << " ___ " << instructions[pipeline[1]] << endl;
            pipeline[4] = pipeline[3];
            pipeline[3] = -1;
          }
          pipeline[2] = pipeline[1];
          pipeline[1] = pipeline[0];

      }

      if(type == 2)
      {
        //cycles++;
        pair<bool , int> check = point(instructions[lineNo]);
        //stalls++;
        if(check.first)
        {
            lineNo = check.second;
        }
        else
        {
          lineNo = lineNo;
        }

        pipeline[4] = pipeline[4];
        pipeline[3] = pipeline[3];
        pipeline[2] = pipeline[2];
        pipeline[1] = pipeline[0];
        pipeline[0] = -1;
      /*  if(lineNo < instructions.size())
        {
          pipeline[0] = lineNo;
        }
        else
        {
          pipeline[0] = -1;
        }*/

        continue;
      }


      /*pipeline[4] = pipeline[3];
      pipeline[3] = pipeline[2];
      pipeline[2] = pipeline[1];
      pipeline[1] = pipeline[0];

      if(lineNo < instructions.size())
        pipeline[0] = lineNo;
      else pipeline[0] = -1;
      */
      lineNo++;
      if(lineNo < instructions.size())
      {
        pipeline[0] = lineNo;
      }
      else
      {
        pipeline[0] = -1;
      }

  }

  cout << endl;
  cout << endl;
  cout << "cycles: " << cycles << endl;
  cout << "stalls: " << stalls << endl;
  cout << "total no. of instructions: " << insCount << endl;
  cout << "IPC: " << (float)(insCount)/(float)(cycles) << endl;
  cout << endl;
  cout << "l1 cache miss rate:   " << (float)(l1_misses)/(float)(l1_accesses) << endl;
  cout << "l2 cache miss rate:   " << (float)(l2_misses)/(float)(l2_accesses) << endl;
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
