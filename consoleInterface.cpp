//:consoleInterface.cpp
#include "Interface.h"
#include "consoleInterface.h"
#include "recite.h"
#include "WordFactory.h"
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
using namespace std;
bool judge(int n);
consoleInterface* consoleInterface::instance = 0;
consoleInterface* consoleInterface::GetInstance()
{
    if (instance == 0)
        instance = new consoleInterface;
    return instance;
}
bool consoleInterface::End(string a, int pos)
{
    return a[pos] == 32 || a[pos] ==10 || a[pos] == 13 || a[pos] == '\t';
}
void consoleInterface::TakeArg(string a, string b, string& r)
{
    r = "";
    int begin = kmp(a,b);
    if (begin < 0 || begin >= b.size())
    {
        return;
    }
    int e1 = begin;
    while (++e1 < b.size() && !End(b,e1));
    int e2 = e1;
    while (++e2 < b.size() && !End(b,e2));
    if (e1 >= b.size() || e2 > b.size())
        return;
    for (int i = e1 + 1; i < e2; ++i)
        r += b[i];
    return;
}
void consoleInterface::TakeArg(string a, string b, vector<string>& r)
{
    r.clear();
    int p = kmp(a, b);
    if (p < 0 || p >= b.size())
        return;
    int q = p;
    while (p <= b.size() && q <= b.size())
    {
        while (p < b.size() && !End(b, p))
            p++;
        q = p + 1;
        while (q <= b.size() && !End(b, q))
            q++;
        string tmp;
        for (int i = p + 1; i < q; ++i)
            tmp += b[i];
        r.push_back(tmp);
        p = q + 1;
    }
}
consoleInterface::consoleInterface()
{
    dic = Dictionary::GetInstance();
    mode = 0;
    user = NULL;
    op = NULL;
    pos = -1;
    level=0;
    testType=0;
    modified = false;
}
void consoleInterface::ini()
{
    string source = "StandardSource.txt";
    #ifdef _WIN32
    source = "StandardSourceWin.txt";
    #endif
    srand((unsigned int)time(NULL));
    int beginTime = clock();
    cout << "loading......" << endl;
    ifstream fin(source.c_str());
    dic->Read (fin);
    fin.close();
    fin.open("RAT.ini");
    string tmp1;
    string tmp2;
    string tmp3;
    while(getline(fin, tmp1) && getline(fin, tmp2) && getline(fin, tmp3))
    {
        users.push_back(new User(tmp1, tmp2, tmp3));
    }
    fin.close();
    cout << "succefullly load." << endl
    << "time expend: "
    << (double)(clock() - beginTime) / CLOCKS_PER_SEC
    << "seconds" << endl;
    User* cuser = NULL;
    do
    {
        cout << "user name: ";
        string userName;
        getline(cin, userName);
        for (int i = 0; i < users.size(); ++i)
        {
            if (users[i]->GetName() == userName)
            {
                cuser = users[i];
                break;
            }
        }
    }
    while (!Login(cuser));
    user = cuser;
    load();
    for(int i=0; i<user->GetSize(); i++){//初始化set中的一些东西
        user->GetSet(i)->lastRecite[0]=0;
        for(int j=0; j<user->GetSet(i)->GetSize(); j++){
            if(user->GetSet(i)->GetWord(j)->succeessReview>=8 && !user->GetSet(i)->GetWord(j)->kill){
                user->GetSet(i)->GetWord(j)->haveRecited=false;
                user->GetSet(i)->GetWord(j)->kill=true;//如果累计成功背诵8次以上就相当于kill了
            }
            if(user->GetSet(i)->GetWord(j)->haveRecited){//初始化recite
                user->GetSet(i)->recited.push_back(user->GetSet(i)->GetWord(j));
            }
            if(user->GetSet(i)->GetWord(j)->kill){//初始化kill
                user->GetSet(i)->killed.push_back(user->GetSet(i)->GetWord(j));
            }
        }
    }
    for(int i=0; i<user->GetSize(); i++){
        Remind(user->GetSet(i));
    }
}
void consoleInterface::Remind(Set* m){
    cout<<m->GetName()<<" :"<<endl;
    int today[2];
    time_t t = time(0);
    char tmp[5],tmp1[8];
    strftime( tmp1, sizeof(tmp), "%Y",localtime(&t) );
    today[0]=atoi(tmp1);
    strftime( tmp, sizeof(tmp), "%j",localtime(&t) );
    today[1]=atoi(tmp);
    int plus=0;//因为年数差异产生的天数差异
    for(int i=m->GetBeginDay()[0]; i<today[0]; i++){
        plus+=365;
        if(judge(i)){
            plus++;
        }
    }
    int dayleft=m->GetUseDay()-today[1]+m->GetBeginDay()[1]+plus;//还剩下多少天可以背单词
    
    if(dayleft<=0){
        std::cout<<"No Day Left! Please add days."<<std::endl;
    }else{
        int wordleft=m->GetSize()-m->GetRecitedSize()-m->killed.size();//还有多少单词没有背
        int reciteToday=wordleft/dayleft-m->reciteToday+m->plusDay;//今天需要背多少单词
        cout<<"\t"<<reciteToday<<" words need to be recited today."<<endl;
        int reviewToday=min(m->GetRecitedSize(), reciteToday)-m->reviewToday;
        cout<<"\t"<<reviewToday<<" words need to be reviewed today."<<endl;
    }
}
void consoleInterface::load()
{
    cout << "loading user's information......" << endl;
    string s="";
    s+=user->GetName();
    s+="/source.txt";
    ifstream fin(s.c_str());
    if(fin)
    {
        string ans="";
        getline(fin, ans);
        while(ans!="*")
        {
            string lin[12];
            int i;
            for(i=0; ; i++){
                if(ans[i]!=' '){
                    lin[0]+=ans[i];
                }else{
                    break;
                }
            }
            int num=1;
            i++;
            for(;i<ans.size(); i++){
                if(ans[i]==' '){
                    num++;
                    continue;
                }else{
                    lin[num]+=ans[i];
                }
            }
            int po=user->FindSet(lin[0]);
            Set* set=user->GetSet(po);
            set->useDay=atoi(lin[1].c_str());
            set->beginDay[0]=atoi(lin[2].c_str());
            set->beginDay[1]=atoi(lin[3].c_str());
            time_t t = time(0);
            char tmp[5],tmp1[8];
            strftime( tmp1, sizeof(tmp), "%Y",localtime(&t) );
            int year=atoi(tmp1);
            strftime( tmp, sizeof(tmp), "%j",localtime(&t) );
            int day=atoi(tmp);
            set->lastRecite[0]=atoi(lin[4].c_str());
            set->lastRecite[1]=atoi(lin[5].c_str());
            if(year == set->lastRecite[0] && day == set->lastRecite[1]){
                set->reciteToday=atoi(lin[6].c_str());
                set->reviewToday=atoi(lin[7].c_str());
            }else{
                set->reciteToday=0;
                set->reviewToday=0;
            }
            set->plusDay=atoi(lin[8].c_str());
            getline(fin, ans);
        }//set的两个数据都读进来了
        for(int i=0; i<dic->GetSize(); i++){
            string haved="";
            getline(fin,haved);
            if(haved=="0"){
                getline(fin,haved);
                continue;
            }
            string lin1[12];
            int num=0;
            getline(fin, ans);
            for(int j=0; j<ans.size(); j++){
                if(ans[j]==' '){
                    num++;
                    continue;
                }else{
                    lin1[num]+=ans[j];
                }
            }
            Test::staticRightNum0=atoi(lin1[0].c_str());
            Test::staticRightNum1=atoi(lin1[1].c_str());
            Test::staticRightNum2=atoi(lin1[2].c_str());
            Test::staticWrongNum0=atoi(lin1[3].c_str());
            Test::staticWrongNum1=atoi(lin1[4].c_str());
            Test::staticWrongNum2=atoi(lin1[5].c_str());
            Test::rightRate0=atof(lin1[6].c_str());
            Test::rightRate1=atof(lin1[7].c_str());
            Test::rightRate2=atof(lin1[8].c_str());//各种正确率读进来
            
            string lin2[14];
            num=0;
            getline(fin, ans);
            for(int j=0; j<ans.size(); j++){
                if(ans[j]==' '){
                    num++;
                    continue;
                }else{
                    lin2[num]+=ans[j];
                }
            }
            (*dic)[i].quan0=atoi(lin2[0].c_str());
            (*dic)[i].quan1=atoi(lin2[1].c_str());
            (*dic)[i].quan2=atoi(lin2[2].c_str());
            (*dic)[i].quanReview=atoi(lin2[3].c_str());
            (*dic)[i].quanSelect[0]=atoi(lin2[4].c_str());
            (*dic)[i].quanSelect[1]=rand()%100000;
            (*dic)[i].haveRecited=atoi(lin2[5].c_str());
            (*dic)[i].kill=atoi(lin2[6].c_str());
            (*dic)[i].right=atoi(lin2[7].c_str());
            (*dic)[i].wrong=atoi(lin2[8].c_str());
            (*dic)[i].reviewDay=atoi(lin2[9].c_str());
            (*dic)[i].zu=atoi(lin2[10].c_str());
            (*dic)[i].huiHe=atoi(lin2[11].c_str());
            
            getline(fin, ans);
            int size=0;
            string number;
            int j;
            for(j=0; j<ans.size(); j++){
                if(ans[j]==' '){
                    break;
                }else{
                    number+=ans[j];
                }
            }
            size=atoi(number.c_str());
            string lin3[2000];
            num=0;
            for(; j<ans.size(); j++){
                if(ans[j]==' '){
                    num++;
                    continue;
                }else{
                    lin3[num]+=ans[j];
                }
            }
            (*dic)[i].reciteTime.clear();
            int nu=-1;
            for(int k=0; k<num; k++){
                if(k%5==0){
                    vector<int> linshi;
                    for(int l=0; l<6; l++) linshi.push_back(1);
                    (*dic)[i].reciteTime.push_back(linshi);
                    nu++;
                }
                (*dic)[i].reciteTime[nu][k%5]=atoi(lin3[k].c_str());
            }
            
            getline(fin, ans);
            size=0;
            number="";
            for(j=0; j<ans.size(); j++){
                if(ans[j]==' '){
                    break;
                }else{
                    number+=ans[j];
                }
            }
            size=atoi(number.c_str());
            string lin4[2000];
            num=0;
            for(; j<ans.size(); j++){
                if(ans[j]==' '){
                    num++;
                    continue;
                }else{
                    lin4[num]+=ans[j];
                }
            }
            nu=-1;
            (*dic)[i].reviewTime.clear();
            for(int k=0; k<num; k++){
                if(k%5==0 ){
                    vector<int> linshi;
                    for(int l=0; l<6; l++) linshi.push_back(1);
                    (*dic)[i].reviewTime.push_back(linshi);
                    nu++;
                }
                (*dic)[i].reviewTime[nu][k%5]=atoi(lin4[k].c_str());
            }
            
            getline(fin,ans);
            string lin5[2500];
            num=0;
            for(int j=0; j<ans.size(); j++){
                if(ans[j]==' '){
                    num++;
                    continue;
                }else{
                    lin5[num]+=ans[j];
                }
            }
            (*dic)[i].check.clear();
            for(int k=1; k<=atoi(lin5[0].c_str()); k++){
                int linshi=atoi(lin5[k].c_str());
                (*dic)[i].check.push_back(linshi);
            }
            
            for(int k=0; k<(*dic)[i].EntrySize(); k++){
                getline(fin,ans);
                string lin6[12];
                num=0;
                for(int q=0; q<ans.size(); q++){
                    if(ans[q]==' '){
                        num++;
                        continue;
                    }else{
                        lin6[num]+=ans[q];
                    }
                }
                (*dic)[i].GetEntry(k)->test->rightNum0=atoi(lin6[0].c_str());
                (*dic)[i].GetEntry(k)->test->rightNum1=atoi(lin6[1].c_str());
                (*dic)[i].GetEntry(k)->test->rightNum2=atoi(lin6[2].c_str());
                (*dic)[i].GetEntry(k)->test->wrongNum0=atoi(lin6[3].c_str());
                (*dic)[i].GetEntry(k)->test->wrongNum1=atoi(lin6[4].c_str());
                (*dic)[i].GetEntry(k)->test->wrongNum2=atoi(lin6[5].c_str());
                (*dic)[i].GetEntry(k)->test->quan0=atoi(lin6[6].c_str());
                (*dic)[i].GetEntry(k)->test->quan1=atoi(lin6[7].c_str());
                (*dic)[i].GetEntry(k)->test->quan2=atoi(lin6[8].c_str());
            }
            
            getline(fin, ans);
        }
    }
    fin.close();
    cout << "successfully load user's information." << endl;
    cout << "current user is " << user->GetName() << endl;
}
void consoleInterface::operation()
{
    string command;
    while (getline (cin, command))
    {
        while(command == "") getline (cin, command);
        if (mode == 1)
        {
            quiryModeAnalyse(command);
        }
        else
        {
            normalAnalyse(command);
        }
    }
}
void consoleInterface::FindWordExact(string targetWord)
{
    if (targetWord == "")
    {
        cout << "please input word." << endl;
        return;
    }
    int pos = dic->FindWord(targetWord);
    if (pos < 0 || pos >= dic->GetSize() || !dic->WordExist(targetWord))
    {
        cout << targetWord << " not found" << endl;
    }
    else
    {
        cout << (*dic)[pos];
    }
}
void consoleInterface::FindSimilarWord(string targetWord)
{
    if (targetWord == "")
    {
        cout << "please input word." << endl;
        return;
    }
    vector<Word*> result;
    dic->FindWordFuzzy(targetWord, result);
    if (result.size() == 0)
    {
        cout << "no similar word in current dictionary found" << endl;
    }
    else
    {
        cout << "similar words:" << endl;
        for (int i = 0 ; i < result.size(); ++i)
        {
            cout << "\t" << i << ": " << result[i]->GetSpell() << endl;
        }
    }
}
void consoleInterface::FindWordFuzzy(string targetWord)
{
    if (targetWord == "")
    {
        cout << "please input word." << endl;
        return;
    }
    vector<Word*> result;
    dic->FindWordFuzzy(targetWord, result);
    if (result.size() == 0)
    {
        cout << "no matched word in current dictionary found" << endl;
    }
    else
    {
        for (int i = 0 ; i < result.size(); ++i)
        {
            cout << i << ": " << endl << *(result[i]);
        }
    }
}
void consoleInterface::quiryModeAnalyse(string command)
{
    string targetWord;
    if (kmp("-e", command) == 0)
    {
        TakeArg("-e", command, targetWord);
        FindWordExact(targetWord);
    }
    else if (kmp("-s", command) == 0)
    {
        TakeArg("-s", command, targetWord);
        FindSimilarWord(targetWord);
    }
    else if (kmp("-f", command) == 0)
    {
        TakeArg("-f", command, targetWord);
        FindWordFuzzy(targetWord);
    }
    else if (command == "mode 0" || command == "m 0")
    {
        mode = 0;
    }
    else if (command == "mode 1" || command == "m 1")
    {
        mode = 1;
    }
    else
    {
        FindWordExact(command);
    }
}
void consoleInterface::Exam()
{
    if (user->GetLevel() == 3)
    {
        cout << "You are the highest level now. "
        << "Maybe we will open more level in the furure" << endl;
        return;
    }
    cout << "You will pass the exam if you test at least 20 times and keep you rate:\n"
    << "type0: 0.95\ttype1: 0.80\ttype2: 0.75" << endl
    << "The type of exam is random. The level of type 0 or type 1 is 4.\nQuit anytime with \"mode 0\"" << endl;
    string examFileName[4] = {"", "level1.txt", "level2.txt", "level3.txt"};
    Set* examSet = new Set;
    ifstream fin(examFileName[user->GetLevel() + 1].c_str());
    examSet->Read(fin);
    fin.close();
    int level = 4;
    int testType = rand() % 3;
    TestDo(examSet, level, testType);
    if (Pass(testType))
    {
        modified = 1;
        user->LevelUp();
        cout << "Good! You pass the exam. Now your level is " << user->GetLevel() << "." << endl;
    }
    else
    {
        cout << "Maybe you need to learn more." << endl;
    }
    delete examSet;
}
void consoleInterface::Save()
{
    ofstream fout;
    if (!modified)
        return;
    string command;
    cout << "save the modification? Y or N" <<endl;
    while (cin >> command)
    {
        if (command == "Y")
        {
            cout << "saving...." << endl;
            //rewrite user
            cout << "rewrite RAT.ini" << endl;
            fout.open("RAT.ini");
            for (int i = 0; i < users.size(); ++i)
            {
                fout << users[i]->GetName() << endl << users[i]->GetPassword()
                << endl << users[i]->GetLevel();
                if (!(i == users.size() - 1))
                    fout << endl;
            }
            fout.close();
            //rewrite .set
            cout << "rewrite " << (user->GetName() + "/" + user->GetName() + ".set").c_str() << endl;
            fout.open((user->GetName() + "/" + user->GetName() + ".set").c_str());
            if (!fout)
            {
                system(("mkdir " + user->GetName()).c_str());
                fout.open((user->GetName() + "/" + user->GetName() + ".set").c_str());
            }
            for (int i = 0; i < user->GetSize(); ++i)
            {
                fout << user->GetSet(i)->GetName();
                if (!(i == user->GetSize() - 1))
                    fout << endl;
            }
            fout.close();
            //rewrite set
            for (int i = 0; i < user->GetSize(); ++i)
            {
                fout.open((user->GetName() + "/" + user->GetSet(i)->GetName() + ".txt").c_str());
                if (!fout)
                {
                    system(("mkdir " + user->GetName()).c_str());
                    fout.open((user->GetName() + "/" + user->GetSet(i)->GetName() + ".txt").c_str());
                }
                for (int j = 0; j < user->GetSet(i)->GetSize(); ++j)
                {
                    fout << (*(user->GetSet(i)))[j].GetSpell();
                    if (j != user->GetSet(i)->GetSize() - 1)
                        fout << endl;
                }
                fout.close();
            }
            output();
            cout << "saved" << endl;
            break;
        }
        else if (command == "N")
        {
            break;
        }
    }
    modified = false;
}
void consoleInterface::Info()
{
    cout << "------------------------------------------------------------------------------" << endl;
    cout << "current user: " << user->GetName() << endl;
    cout << "level: " << user->GetLevel() << endl;
    cout << "set information: " <<  endl;
    if (user != NULL)
        for (int i = 0; i < user->GetSize(); ++i)
        {
            cout << "\t" << i + 1 << ": " << user->GetSet(i)->GetName()<<endl;
            cout<<"\t"<<"   "<<user->GetSet(i)->GetSize()<< " size" << endl;
            Look(user->GetSet(i));
        }
    cout << user->GetSize() << " sets in all." << endl;
    cout << "------------------------------------------------------------------------------" << endl;
}
void consoleInterface::Exit()
{
    Save();
    exit(0);
}
void consoleInterface::normalAnalyse(string command)
{
    if (command == "help" || command == "h")
    {
        outHelp();
    }
    else if (command == "version" || command == "v")
    {
        outVersion();
    }
    else if (command == "exit" || command == "e" || command == "quit" || command == "q")
    {
        Exit();
    }
    else if (command == "mode 0" || command == "m 0")
    {
        mode = 0;
    }
    else if (command == "mode 1" || command == "m 1")
    {
        mode = 1;
    }
    else if (command == "info" || command == "i")
    {
        Info();
    }
    else if (kmp("info ", command) == 0 || kmp("i ", command) == 0)
    {
        string setName;
        TakeArg ("-t", command, setName);
        int p = kmp("-d", command);
        bool d = (p >= 0 && p < command.size());
        List(setName, d);
    }
    else if (kmp("test", command) == 0 || kmp("t ", command) == 0)
    {
        string setName, testTypeString;
        TakeArg("-t", command, setName);
        TakeArg("-l", command, testTypeString);
        Test(setName, testTypeString);
    }
    else if(kmp("recite", command)==0 || kmp("r ", command)==0)
    {
        string setName;
        int file = kmp ("-t", command);
        if (file >= 0 && file < command.size())
        {
            TakeArg("-t", command, setName);
            Recite(setName);
        }
        else{
            file = kmp ("-c", command);
            if(file >= 0 && file < command.size()){
                TakeArg("-c", command, setName);
                string targetDay;
                TakeArg("-d", command, targetDay);
                Change(setName, targetDay);
            }else{
                file = kmp ("-p", command);
                if(file >= 0 && file < command.size()){
                    TakeArg("-p", command, setName);
                    string targetDay;
                    TakeArg("-d", command, targetDay);
                    Plus(setName, targetDay);
                }
            }
        }
    }
    else if (kmp("add", command) == 0)
    {
        string setName;
        TakeArg("-t", command, setName);
        int file = kmp ("-f", command);
        if (file >= 0 && file < command.size())
        {
            string fileName;
            TakeArg("-f", command, fileName);
            AddFile (setName, fileName);
        }
        else
        {
            vector<string> words;
            TakeArg("-w", command, words);
            AddWord (setName, words);
        }
    }
    else if (kmp("touch", command) == 0)
    {
        int a = kmp ("-t", command);
        int b = kmp ("-u", command);
        string name;
        if (b < 0 || b >= command.size())
        {
            TakeArg("-t", command, name);
            TouchSet(name);
        }
        else if (a < 0 || a >= command.size())
        {
            TakeArg("-u", command, name);
            string password;
            TakeArg("-p", command, password);
            TouchUser(name,password);
         }
        else if (a < b)
        {
            TakeArg("-t", command, name);
            TouchSet(name);
        }
        else
        {
            TakeArg("-u", command, name);
            string password;
            TakeArg("-p", command, password);
            TouchUser(name,password);
         }
    }
    else if (command == "rm cuser")
    {
        RemoveUser();
    }
    else if (kmp("convert", command) == 0)
    {
        string source, target;
        TakeArg("-s", command, source);
        TakeArg("-t", command, target);
        convert(source, target);
    }
    else if (kmp("rm", command) == 0)
    {
        string setName;
        TakeArg("-t", command, setName);
        int w = kmp ("-w", command);
        if (w < 0 || w >= command.size())
            RemoveSet(setName);
        else
        {
            vector<string> words;
            TakeArg("-w", command, words);
            RemoveWord(setName, words);
        }
    }
    else if (kmp ("switch", command) == 0 || kmp ("s ",command) == 0)
    {
        string userName;
        TakeArg("-u", command, userName);
        Switch(userName);
    }
    else if (kmp("exam", command) == 0)
    {
        Exam();
    }
    else if (command == "");
    else
    {
        cout << "no command of " << command << ". Try \"help\"" << endl;
    }
}
void consoleInterface::List(string setName, bool detailed)
{
    if (user == NULL)
    {
        cout << "please login" << endl;
        return;
    }
    if (setName == "")
    {
        cout << "please input word." << endl;
        return;
    }
    int pos = user->FindSet(setName);
    if (pos < 0 || pos >= user->GetSize() || setName != user->GetSet(pos)->GetName())
    {
        cout << "set not exist: " << setName << endl;
    }
    else
    {
        cout << setiosflags(ios::left);
        Set* s = user->GetSet(pos);
        for (int i = 0; i < s->GetSize(); ++i)
        {
            if (detailed)
                cout << "******************************************************************************" << endl;
            cout <<setw(5) <<  i  + 1<< ": " << setw(25) << (*s)[i].GetSpell() << (*s)[i].sort() << endl;
            if (detailed)
            {
                for (int j = 0; j < (*s)[i].EntrySize(); ++j)
                {
                    cout << "------------------------------------------------------------------------------" << endl;
                    cout << *((*s)[i].GetEntry(j)) << endl;
                }
            }
        }//~for
    }//~else
}
void consoleInterface::Write(ostream& fout, const Word* w)
{
    if (w == NULL)
        return;
    fout << w->GetSpell() << endl;
    for (int i = 0; i < w->EntrySize(); ++i)
    {
        w->GetEntry(i)->OutputEleSource(fout);
        w->GetEntry(i)->OutputSource(fout);
        fout << "#" << endl;
    }
    fout << "*" << endl;
}
void consoleInterface::convert(string source, string target)
{
    ifstream fin(source.c_str());
    if (!fin)
    {
        cout << "fail to open the file: " << source << endl;
        return;
    }
    if (source == "")
    {
        cout << "please input source file." << endl;
        return;
    }
    if (target == "")
    {
        cout << "please input target file." << endl;
        return;
    }
    ofstream fout(target.c_str(), ios::app);
    string tmp;
    vector<string> unit;
    WordFactory* fac = new WordFactory;
    while (getline(fin, tmp))
    {
        if (tmp == "")
        {
            if (unit.size() == 0)
                continue;
            Word* w = fac->youdaoCreate(unit);
            Write(fout, w);
            delete w;
            unit.clear();
        }
        else
            unit.push_back(tmp);
    }
    delete fac;
    fout.close();
    fin.close();
}
void consoleInterface::RemoveUser()
{
    if (user->GetName() == "guest")
    {
        cout << "You can't remove guest user." << endl;
        return;
    }
    cout << "confirm password....." << endl;
    if (Login(user))
    {
        User* oldUser = user;
        User* guest;
        for (int i = 0; i < users.size(); ++i)
        {
            if (users[i]->GetName() == "guest")
            {
                guest = users[i];
                break;
            }
        }
        for (int i = 0; i < users.size(); ++i)
        {
            if (users[i] == oldUser)
            {
                users.erase(users.begin() + i);
                break;
            }
        }
        modified = 1;
        Save();
        user = guest;
    }
    else
    {
        cout << "failed to confirm" << endl;
    }
}
void consoleInterface::RemoveSet(string setName)
{
    if (user == NULL)
    {
        cout << "please login" << endl;
        return;
    }
    if (setName == "")
    {
        cout << "please input word." << endl;
        return;
    }
    int pos = user->FindSet(setName);
    if (pos < 0 || pos >= user->GetSize() || setName != user->GetSet(pos)->GetName())
    {
        cout << "this set not exist: " << setName << endl;
    }
    else
    {
        user->RemoveSet(pos);
        modified = 1;
    }
}
void consoleInterface::RemoveWord(string setName,vector<string>& words)
{
    if (user == NULL)
    {
        cout << "please login" << endl;
        return;
    }
    if (setName == "")
    {
        cout << "please input word." << endl;
        return;
    }
    if (words.size() == 0)
    {
        cout << "please input words." << endl;
        return;
    }
    int pos = user->FindSet(setName);
    if (pos < 0 || pos >= user->GetSize())
    {
        cout << "wrong set: " << setName << endl;
        return;
    }
    Set* s = user->GetSet(pos);
    for (int i = 0; i < words.size(); ++i)
    {
        int p = s->Find(words[i]);
        if (p < 0 || p >= s->GetSize())
            continue;
        s->removeWord(p);
    }
}
void consoleInterface::Recite(std::string setName)
{
    if (user == NULL)
    {
        cout << "please login" << endl;
        return;
    }
    if (setName == "")
    {
        cout << "please input word." << endl;
        return;
    }
    int linpos = user->FindSet(setName);
    if (linpos < 0 || linpos >= user->GetSize())
    {
        cout << "no such set. Try \"touch\"" << endl;
    }
    else
    {
        modified=true;
        cout<<"In recite/review word mode, press q to quit recite, press k to kill this word, press enter to go on."<<endl;
        recite* Recite = new recite(user->GetSet(linpos));
        Recite->ReciteControl(cout, cin);
        if(Recite->GetDone()){
            cout<<"Congratulations! You have finished today's task!"<<endl;
        }
        user->GetSet(linpos)->recited.clear();
        for(int i=0; i<user->GetSet(linpos)->words.size(); i++){
            if(user->GetSet(linpos)->words[i]->haveRecited){
                user->GetSet(linpos)->recited.push_back(user->GetSet(linpos)->words[i]);
            }
        }
        for(int i=0; i<user->GetSet(linpos)->recited.size(); i++){
            if(user->GetSet(linpos)->recited[i]->kill){
                user->GetSet(linpos)->recited.erase(user->GetSet(linpos)->recited.begin()+i);
                i--;
            }
        }
    }
}
void consoleInterface::Change(std::string setName, string targetDay)
{
    if (user == NULL)
    {
        cout << "please login" << endl;
        return;
    }
    if (setName == "")
    {
        cout << "please input word." << endl;
        return;
    }
    int linpos = user->FindSet(setName);
    if (linpos < 0 || linpos >= user->GetSize())
    {
        cout << "no such set. Try \"touch\"" << endl;
    }
    else
    {
        if(targetDay == ""){
            cout<<"please type in day."<<endl;
            return;
        }
        if(atoi(targetDay.c_str())<0){
            cout<<"day input error, try again."<<endl;
            return;
        }
        int today[2];
        time_t t = time(0);
        char tmp[5],tmp1[8];
        strftime( tmp1, sizeof(tmp), "%Y",localtime(&t) );
        today[0]=atoi(tmp1);
        strftime( tmp, sizeof(tmp), "%j",localtime(&t) );
        today[1]=atoi(tmp);
        int plus=0;//因为年数差异产生的天数差异
        for(int i=user->GetSet(linpos)->GetBeginDay()[0]; i<today[0]; i++){
            plus+=365;
            if(judge(i)){
                plus++;
            }
        }
        int dayuse=today[1]-user->GetSet(linpos)->GetBeginDay()[1]+plus;
        user->GetSet(linpos)->useDay=atoi(targetDay.c_str())+dayuse;
        cout<<"success!"<<endl;
        modified=true;
    }
}
void consoleInterface::Plus(std::string setName, string targetDay)
{
    if (user == NULL)
    {
        cout << "please login" << endl;
        return;
    }
    if (setName == "")
    {
        cout << "please input word." << endl;
        return;
    }
    int linpos = user->FindSet(setName);
    if (linpos < 0 || linpos >= user->GetSize())
    {
        cout << "no such set. Try \"touch\"" << endl;
    }
    else
    {
        if(targetDay == ""){
            cout<<"please type in day."<<endl;
            return;
        }
        if(atoi(targetDay.c_str())<0){
            cout<<"day input error, try again."<<endl;
            return;
        }
        user->GetSet(linpos)->plusDay=atoi(targetDay.c_str());
        cout<<"success!"<<endl;
    }
}
void consoleInterface::Look(Set* m)
{
    cout<<"\t"<<"   "<<m->killed.size()<<" words killed."<<endl;
    cout<<"\t"<<"   "<<m->recited.size()<<" words recited."<<endl;
    cout<<"\t"<<"   "<<m->words.size()-m->recited.size()-m->killed.size()<<" words left."<<endl;
    int today[2];
    time_t t = time(0);
    char tmp[5],tmp1[8];
    strftime( tmp1, sizeof(tmp), "%Y",localtime(&t) );
    today[0]=atoi(tmp1);
    strftime( tmp, sizeof(tmp), "%j",localtime(&t) );
    today[1]=atoi(tmp);
    int plus=0;
    for(int i=m->GetBeginDay()[0]; i<today[0]; i++){
        plus+=365;
        if(judge(i)){
            plus++;
        }
    }
    int dayleft=m->GetUseDay()-today[1]+m->GetBeginDay()[1]+plus;
    cout<<"\t"<<"   "<<dayleft<<" days left"<<endl;
}
void consoleInterface::Test(string setName, string testTypeString)
{
    if (user == NULL)
    {
        cout << "please login" << endl;
        return;
    }
    if (setName == "")
    {
        cout << "please input word." << endl;
        return;
    }
    if (testTypeString == "")
    {
        cout << "please input type." << endl;
        return;
    }
    stringstream ss;
    ss << testTypeString;
    ss >> testType;
    if(testType>2 || testType < 0){
        cout<<"no this testType, please input again!"<<endl;
        return;
    }
    pos = user->FindSet(setName);
    if (pos < 0 || pos >= user->GetSize())
    {
        cout << "no such set. Try \"touch\"" << endl;
    }
    else
    {
        if(testType!=2){
            cout<<"plesae choose level (2~8)"<<endl;
            cin>>level;
            while(level>8||level<2){
                cout<<"the level is illegal, please input again!"<<endl;
                cin>>level;
            }
            string s;
            getline (cin, s);
            cout<<endl;
        }
        else{
            level=-1;
        }
        TestDo();
    }
}
void consoleInterface::TestDo()
{
    modified=true;
    while(1)
    {
        op=new opera(user->GetSet(pos), level, testType, false);
        op->ope(cout);
        string answer;
        getline(cin,answer);
        while(answer==""){
            getline(cin,answer);
        }
        if(answer=="mode 0" || answer == "m 0")
            return;
        else if (answer == "mode 1" || answer == "m 1")
        {
            mode = 1;
            return;
        }
        else if (answer == "exit")
        {
            Exit();
        }
        op->first(answer, cout);
    }
}
void consoleInterface::TestDo(Set* s, int le, int ttype)
{
    modified=true;
    if (ttype == 2)
        le = -1;
    Test::Clear();
    while(1)
    {
        op=new opera(s, le, ttype, true);
        op->ope(cout);
        string answer;
        getline(cin,answer);
        while (answer == "")
            cin >> answer;
        if(answer=="mode 0" || answer == "m 0")
            return;
        else if (answer == "mode 1" || answer == "m 1")
        {
            mode = 1;
            return;
        }
        else if (answer == "exit")
        {
            Exit();
        }
        op->first(answer, cout);
    }
}
void consoleInterface::AddWord(string setName, vector<string>& words)
{
    if (user == NULL)
    {
        cout << "please login" << endl;
        return;
    }
    if (setName == "")
    {
        cout << "please input word." << endl;
        return;
    }
    if (words.size() == 0)
    {
        cout << "please input words." << endl;
        return;
    }
    int pos = user->FindSet(setName);
    if (pos < 0 || pos >= user->GetSize())
    {
        cout << "wrong set: " << setName << endl;
        return;
    }
    Set* s = user->GetSet(pos);
    for (int i = 0; i < words.size(); ++i)
    {
        if (s->WordExist(words[i]))
            continue;
        int p = dic->FindWordExact(words[i]);
        if (p < 0 || p > dic->GetSize())
            continue;
        Word* w = &(*dic)[p];
        s->Insert(*w);
        cout << "insert " << words[i] << " into " << setName << endl;
    }
}
void consoleInterface::AddFile(string setName, string fileName)
{
    if (user == NULL)
    {
        cout << "please login" << endl;
        return;
    }
    if (setName == "")
    {
        cout << "please input word." << endl;
        return;
    }
    if (fileName == "")
    {
        cout << "please input file name." << endl;
        return;
    }
    int pos = user->FindSet(setName);
    if (pos < 0 || pos >= user->GetSize())
    {
        cout << "no this set: " << setName << endl;
        return;
    }
    ifstream fin(fileName.c_str());
    if (!fin)
    {
        cout << "fail to open the file" << endl;
        return;
    }
    AnalyseFile(fin, user->GetSet(pos));
    fin.close();
    cout << "Finished" << endl;
}
bool consoleInterface::FamiliarWord(string word, Set* levelSet)
{
    int pos = dic->FindWord(word);
    if (pos < 0 || pos >= dic->GetSize())
        return false;
    if (levelSet != NULL && levelSet->WordExist(word))
    {
        return true;
    }
    return false;
}
void consoleInterface::AnalyseFile(ifstream& fin, Set* s)
{
    string examFileName[4] = {"", "level1.txt", "level2.txt", "level3.txt"};
    Set* examSet = new Set;
    if (user->GetLevel() > 0)
    {
        ifstream finSet(examFileName[user->GetLevel()].c_str());
        examSet->Read(finSet);
        finSet.close();
    }
    string file;
    string tmp;
    while (fin >> tmp)
    {
        file += tmp;
    }
    int p = 0, q = 0;
    while (q < file.size() && p < file.size())
    {
        while (!IsLetter(file[p]) && p < file.size())
            p++;
        q = p;
        while (IsLetter(file[q]) && q < file.size())
            q++;
        string tmpWord;
        for (int i = p; i < q; ++i)
        {
            tmpWord += file[i];
        }
        if (!FamiliarWord(tmpWord, examSet))
        {
            if (!s->WordExist(tmpWord) && dic->WordExist(tmpWord))
            {
                modified = 1;
                s->Insert((*dic)[dic->FindWord(tmpWord)]);
            }
        }
        p = q + 1;
    }
    delete examSet;
}
void consoleInterface::TouchSet(string setName)
{
    if (user == NULL)
    {
        cout << "please login" << endl;
        return;
    }
    if (setName == "")
    {
        cout << "please input word." << endl;
        return;
    }
    int pos = user->FindSet(setName);
    if (pos < 0 || pos >= user->GetSize() || setName != user->GetSet(pos)->GetName())
    {
        Set* set = new Set(setName);
        user->InsertSet (*set);
        modified = true;
        cout<<"please input how many days you want to use to recite the word of this set."<<endl;
        string dayUse;
        cin>>dayUse;
        while(atoi(dayUse.c_str())<=0){
            cout<<"please input corret day."<<endl;
            cin>>dayUse;
        }
        set->useDay=atoi(dayUse.c_str());
        time_t t = time(0);
        char tmp[5],tmp1[8];
        strftime( tmp1, sizeof(tmp), "%Y",localtime(&t) );
        int year=atoi(tmp1);
        strftime( tmp, sizeof(tmp), "%j",localtime(&t) );
        int day=atoi(tmp);
        set->beginDay[0]=year;
        set->beginDay[1]=day;
        set->lastRecite[0]=year;
        set->lastRecite[1]=day;
        set->reciteToday=0;
        set->reviewToday=0;
        set->recited.clear();
        set->killed.clear();
        set->words.clear();
        set->plusDay=0;
        ifstream fin((user->GetName() + "/" + setName + ".txt").c_str());
        set->Read(fin);
        fin.close();
        cout << "successfully touch" << endl;
    }
    else
    {
        cout << "this set exist." << endl;
    }
}
void consoleInterface::TouchUser(string userName, string password)
{
    if (userName == "")
    {
        cout << "please input user name." << endl;
        return;
    }
    if (password == "")
    {
        cout << "please input password." << endl;
        return;
    }
    User* newUser =new User(userName, password, "0");
    users.push_back(newUser);
    modified = true;
    cout << "successfully touch user:" << userName << endl;
    //
    ifstream exist((newUser->GetName()+"/"+newUser->GetName() + ".set").c_str());
    if (!exist)
    {
        system(("mkdir " + newUser->GetName()).c_str());
    }
    exist.close();
}
void consoleInterface::Switch(string userName)
{
    if (userName == "")
    {
        cout << "please input user name." << endl;
        return;
    }
    for (int i = 0; i < users.size(); ++i)
    {
        if (userName == users[i]->GetName())
        {
            if (Login (users[i]))
            {
                Save();
                user = users[i];
            }
            return;
        }
    }
    cout << "wrong userName:" << userName << " not found" << endl;
}
bool consoleInterface::Login(User* u)
{
    if (u == NULL)
    {
        cout << "wrong user name" << endl;
        return false;
    }
    string tmp;
    char p;
    cout << "password:";
    getline (cin, tmp);
    if (tmp == u->GetPassword())
    {
        return true;
    }
    else
    {
        return false;
    }
}
void consoleInterface::outHelp()
{
    cout << "------------------------------------------------------------------------------" << endl;
    cout << "List of commands" << endl << endl
    << "m[ode] 1 --step into quiry mode" << endl
    << "m[ode] 0 --step into normal mode" << endl
    << "in mode 1:" << endl
    << "\t[-e] wordname --list all about this word,type -i to include all idioms" << endl
    << "\t-s [-i] wordname --list similar word" << endl
    << "\t-f [-i] string -- list all words details contain this string,type -i \n\tto include all idioms" << endl
    << "i[nfo] --info about the current user" << endl
    << "i[nfo] -t setname [-d] -- list all the word in the set, -d to list all details" << endl
    << "test -t setname -l testType --test words in this set" << endl
    << "in test:"<<endl
    << "\ttestType can only be 0,1 or 2"<<endl
    << "\tinput mode0 to return normal mode"<<endl
    << "\tinput exit to exit, you can't simply use e here" << endl
    << "r[ecite] -t setname --recite words in this set, type q to quit."<<endl
    << "r[ecite] -c setname daynumber --Add days to recite this set."<<endl
    << "r[ecite] -p setname wordsnumber --Add extra days to recite this set today."<<endl
    << "rm -t setname -- remove a set" << endl
    << "rm cuser -- remove current user, then switch to guest user" << endl
    << "rm -t setname -w word's number -- remove word form set" << endl
    << "in order to remove user or set completely, you need to remove the file \n\tfrom your mass memory." << endl
    << "add -t setname -w word -- add word to a set, and you can add many word to one\n\t set in one command." << endl
    << "add -t setname -f filename --add new words from a text file, \n\tRAT will recognize your unfamiliar word" << endl
    << "\tIn order to do this, end each word with \'.\'" << endl
    << "exam --exam for one higher level" << endl
    << "convert -s source -t target -- convert form youdao to RAT" << endl
    << "touch -t setname --new set, the name must be a constant string." << endl
    << "touch -u userName -p password-- new user" << endl
    << "s[witch] -u userName --switch user" << endl
    << "e[xit] --exit this program" << endl
    << "q[uit] --exit this program" << endl
    << "h[elp] --view help document" << endl
    << "v[ersion] --view information about this program" << endl;
    cout << "------------------------------------------------------------------------------" << endl;
}
void consoleInterface::outVersion()
{
    cout << "------------------------------------------------------------------------------" << endl;
    cout << "RAT remember and test 1.0" << endl
    << "Author: Li Zeyan	Lv Xin" << endl;
    cout << "------------------------------------------------------------------------------" << endl;
}
void consoleInterface::output(){
    string s="";
    s+=user->GetName();
    s+="/source.txt";
    ofstream fout(s.c_str());
    for(int i=0; i<user->GetSize(); i++){
        fout<<user->GetSet(i)->GetName()<<' '<<user->GetSet(i)->GetUseDay()<<' '<<user->GetSet(i)->GetBeginDay()[0]<<' '<<user->GetSet(i)->GetBeginDay()[1]<<' '<<user->GetSet(i)->lastRecite[0]<<' '<<user->GetSet(i)->lastRecite[1]<<' '<<user->GetSet(i)->reciteToday<<' '<<user->GetSet(i)->reviewToday<<' '<<user->GetSet(i)->plusDay<<endl;
    }
    fout<<"*"<<endl;
    for(int i=0; i<dic->GetSize(); i++){
        if((*dic)[i].haveRecited || (*dic)[i].kill){
            fout<<1<<endl;
        }else{
            fout<<0<<endl;
        }
        if(!(*dic)[i].haveRecited && !(*dic)[i].kill){
            fout<<"*"<<endl;
            continue;
        }
        
        fout<<(*dic)[i].GetEntry(0)->test->staticRightNum0<<' '
        <<(*dic)[i].GetEntry(0)->test->staticRightNum1<<' '
        <<(*dic)[i].GetEntry(0)->test->staticRightNum2<<' '
        <<(*dic)[i].GetEntry(0)->test->staticWrongNum0<<' '
        <<(*dic)[i].GetEntry(0)->test->staticWrongNum1<<' '
        <<(*dic)[i].GetEntry(0)->test->staticWrongNum2<<' '
        <<(*dic)[i].GetEntry(0)->test->rightRate0<<' '
        <<(*dic)[i].GetEntry(0)->test->rightRate1<<' '
        <<(*dic)[i].GetEntry(0)->test->rightRate2<<endl;//一行.............
        
        fout<<(*dic)[i].quan0<<' '<<(*dic)[i].quan1<<' '<<(*dic)[i].quan2<<' '
        <<(*dic)[i].quanReview<<' '<<(*dic)[i].quanSelect[0]<<' '
        <<(*dic)[i].haveRecited<<' '<<(*dic)[i].kill<<' '
        <<(*dic)[i].right<<' '<<(*dic)[i].wrong<<' '<<(*dic)[i].reviewDay<<' '
        <<(*dic)[i].zu<<' '<<(*dic)[i].huiHe<<endl;//一行..............
        fout<<(*dic)[i].reciteTime.size()<<' ';
        for(int k=0; k<(*dic)[i].reciteTime.size(); k++){
            for(int j=0; j<5; j++){
                fout<<(*dic)[i].reciteTime[k][j]<<' ';
            }
        }
        fout<<endl;//一行............
        fout<<(*dic)[i].reviewTime.size()<<' ';
        for(int k=0; k<(*dic)[i].reviewTime.size(); k++){
            for(int j=0; j<5; j++){
                fout<<(*dic)[i].reviewTime[k][j]<<' ';
            }
        }
        fout<<endl;//一行............
        fout<<(*dic)[i].check.size()<<' ';
        for(int j=0; j<(*dic)[i].check.size(); j++){
            fout<<(*dic)[i].check[j]<<' ';
        }
        fout<<endl;//一行............
        
        for(int j=0; j<(*dic)[i].EntrySize(); j++){
            fout<<(*dic)[i].GetEntry(j)->test->rightNum0<<' '
            <<(*dic)[i].GetEntry(j)->test->rightNum1<<' '
            <<(*dic)[i].GetEntry(j)->test->rightNum2<<' '
            <<(*dic)[i].GetEntry(j)->test->wrongNum0<<' '
            <<(*dic)[i].GetEntry(j)->test->wrongNum1<<' '
            <<(*dic)[i].GetEntry(j)->test->wrongNum2<<' '
            <<(*dic)[i].GetEntry(j)->test->quan0<<' '
            <<(*dic)[i].GetEntry(j)->test->quan1<<' '
            <<(*dic)[i].GetEntry(j)->test->quan2<<endl;
        }
        fout<<"*"<<endl;//一行............
    }
    fout.close();
}