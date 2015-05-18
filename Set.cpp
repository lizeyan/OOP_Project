//:Set.cpp
#include "Set.h"
#include "Word.h"
#include "Dictionary.h"
#include <iostream>
#include <algorithm>
using namespace std;
int Set::Find(string targetWord) const
{
    //返回秩不大于目标的最后一个位置
    int lo = 0, hi = words.size();
    while (lo < hi)
    {
        int mi = (lo + hi) >> 1;
        (targetWord < words[mi]->GetSpell()) ? hi = mi: lo = mi + 1;
    }
    return --lo;
}
bool Set::WordExist(string targetWord)
{
    int pos = Find (targetWord);
    if (pos < 0 || pos >= words.size())
        return false;
    return words[pos]->GetSpell() == targetWord;
}
Word& Set::operator[] (int rank)
{
    return *((Word*)words[rank]);
}
int Set::Insert(Word& targetWord)
{
    int pos = Find(targetWord.GetSpell()) +1;
    words.insert (words.begin() + pos, &targetWord);
    return pos;
}
void Set::ReadAndAdd(std::istream& load)
{
    Dictionary* dic = Dictionary::GetInstance();
    string word;
    while (load >> word)
    {
        if (!WordExist (word))
        {
            if (dic->WordExist (word))
            {
                int pos = dic->FindWord (word);
                Insert ((*dic)[pos]);
            }
        }
    }
}
void Set::Read(std::istream& load)
{
    words.clear();
    ReadAndAdd (load);
}
Set::Set ()	{}
void Set::Test()
{
    for (int i = 0; i < words.size(); ++i)
    {
        words[i]->Test();
    }
}
ostream& operator<< (ostream& out, Set& s)
{
    out << s.setName << ":" << endl;
    for (int i = 0; i < s.words.size(); ++i)
    {
        out << s.words[i]->GetSpell() << endl;
    }
    return out;
};