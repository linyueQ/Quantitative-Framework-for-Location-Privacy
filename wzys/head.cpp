#pragma once
#include<head.h>
using namespace std;

TraceSet a1;//保护部分用

TraceSet a21;//度量部分用
TraceSet a22;
vector<vector<double>> tsmetric;
int paintFlag=0;
vector<TracePoint> user1;
vector<TracePoint> user2;

TraceSet a3;//还原轨迹部分用
vector<Key> key;//还原位置扰动
vector<TracePoint> tp;//还原假位置注入、位置隐匿
map<int,int> kv_pair;//还原K匿名

TraceSet a41;//攻击部分要用到
TraceSet a42;

//*********************************************************文件读写函数*********************************************************

//时间戳以小时为单位，传入一个二维vector然后就可以读取到文件里面的数据到这个二维vector里面
//由于有的文件没有isExposed，所以当读取没有isExposed的时候，type为0；如果有isExposed，type为1
//二维vector可以简单地理解为可变长的二维数组，比如a[2][3]就是代表有两个用户， 每个用户的轨迹长度为3
//另外注意一下，如果LPPM采用隐匿，则location为空
bool FileRead(char* filename, TraceSet &tr, int type)
{
    vector<TracePoint> a;//暂时存放用户的一条轨迹
    TracePoint b;
    ull lastUser = -1;//用于记录上一条记录的用户号
    const char *divide = "[],()| ";//分割符有5个
    char temp[50];//读取一行的数据
    char *p;

    ifstream fin(filename, ios::in);
    if (!fin)
    {
        cout << "fin打开文件失败，文件路径为：" << filename << endl << endl;
        return false;
    }

    //先把user、time、location的范围读出来
    fin.getline(temp, 50);
    p = strtok(temp, divide);
    string tt;
    int ss = 0;
    while (p)
    {
        if (ss == 0)
        {
            tt = p;
            tr.minUserStamp = strtoull(tt.c_str(), NULL, 0);
        }
        else if (ss == 1)
        {
            tt = p;
            tr.maxUserStamp = strtoull(tt.c_str(), NULL, 0);
        }
        else if (ss == 2)
        {
            tt = p;
            tr.minTimeStamp = strtoull(tt.c_str(), NULL, 0);
        }
        else if (ss == 3)
        {
            tt = p;
            tr.maxTimeStamp = strtoull(tt.c_str(), NULL, 0);
        }
        else if (ss == 4)
        {
            tt = p;
            tr.minLocationStamp = strtoull(tt.c_str(), NULL, 0);
        }
        else if (ss == 5)
        {
            tt = p;
            tr.maxLocationStamp = strtoull(tt.c_str(), NULL, 0);
        }
        else if (ss == 6)
        {
            tt = p;
            tr.areaRow = strtoull(tt.c_str(), NULL, 0);
        }
        else if (ss == 7)
        {
            tt = p;
            tr.areaCol = strtoull(tt.c_str(), NULL, 0);
        }
        p = strtok(NULL, divide);
        ss++;
    }

    while (fin.getline(temp, 50))
    {
        int s = 0;
        p = strtok(temp, divide);
        string t;
        while (p)//将读到的数据记录到b的相应位置
        {
            if (s == 0)//记录用户号
            {
                t = p;
                b.user = strtoull(t.c_str(), NULL, 0);
            }
            else if (s == 1)//记录时间戳
            {
                t = p;
                b.time = strtoull(t.c_str(), NULL, 0);
            }
            else//把剩余的元素（包括isExposed）全部记录到location向量里面
            {
                t = p;
                b.location.push_back(strtoull(t.c_str(), NULL, 0));
            }
            p = strtok(NULL, divide);
            s++;
        }
        //这里要判断，最后一个元素是不是isExposed
        if (type == 1)
        {
            b.isExposed = b.location.back();//将最后一个元素赋值给isExposed
            b.location.pop_back();//删除最后一个元素
        }
        //在一条记录b处理完毕以后，要开始判断该条记录和上一条记录是否属于同一用户的
        if (b.user != lastUser)//如果不是，那么将轨迹a存到trace中，然后清空a，再记录b
        {
            if(a.size()!=0) tr.trace.push_back(a);
            a.clear();
            a.push_back(b);
        }
        else//如果是，那么直接记录b
        {
            a.push_back(b);
        }
        lastUser = b.user;
        b.location.clear();
    }
    tr.trace.push_back(a);
    fin.close();
    return true;
}

//文件写入，操作类似于文件读取，如果要写入isExposed，则type为1
bool FileWrite(char* filename, TraceSet &tr, int type)
{
    ofstream fout(filename, ios::out);
    if (!fout)
    {
        cout << "fout打开文件失败，文件路径为：" << filename << endl << endl;
        return false;
    }

    fout << "[" << tr.minUserStamp << ", " << tr.maxUserStamp << "] [" << tr.minTimeStamp << ", " << tr.maxTimeStamp << "] ["
         << tr.minLocationStamp << ", " << tr.maxLocationStamp << "] [" << tr.areaRow << ", " << tr.areaCol << "]\n";

    for (int i = 0; i < tr.trace.size(); i++)
    {
        for (int j = 0; j < tr.trace[i].size(); j++)
        {
            fout << tr.trace[i][j].user << ", " << tr.trace[i][j].time << ", (";
            for (int k = 0; k < tr.trace[i][j].location.size(); k++)
            {
                fout << tr.trace[i][j].location[k];
                if (k != tr.trace[i][j].location.size() - 1) fout << ", ";
            }
            fout << ")";
            if (type == 1)//type为1要把isExposed也写进去
            {
                fout << "| " << tr.trace[i][j].isExposed;
            }
            if (i == tr.trace.size() - 1 && j == tr.trace[i].size() - 1) break;
            fout << endl;
        }
    }
    fout.close();
    return true;
}

//从key二进制文件读出加密信息
bool KeyRead(char *filename, vector<Key> &k)
{
    ifstream fin(filename, ios::in | ios::binary);
    if (!fin)
    {
        cout << "fin打开key文件失败，文件不存在" << endl << endl;
        return false;
    }
    int count = -1;
    fin.read((char*)&count, sizeof(count));
    Key temp;
    for (int i = 0; i < count; i++)
    {
        fin.read((char*)&temp, sizeof(Key));
        k.push_back(temp);
    }
    fin.close();
    return true;
}

//将加密信息Key一个个写入到文件key里面
bool KeyWrite(char *filename, vector<Key> &k)
{
    ofstream fout(filename, ios::out | ios::binary);
    if (!fout)
    {
        cout << "fout打开key文件失败，文件不存在" << endl << endl;
        return false;
    }
    int count = k.size();
    fout.write((char*)&count, sizeof(count));
    for (int i = 0; i < count; i++)
    {
        fout.write((char*)&k[i], sizeof(Key));
    }
    fout.close();
    return true;
}

bool TruePosiRead(char *filename, vector<TracePoint> &k)
{
    ifstream fin(filename, ios::in | ios::binary);
    if (!fin)
    {
        cout << "fin打开TruePosition文件失败，文件不存在" << endl << endl;
        return false;
    }
    int count = -1;
    fin.read((char*)&count, sizeof(count));
    TracePoint temp;
    ull tLocation;
    for (int i = 0; i < count; i++)
    {
        fin.read((char*)&temp.user, sizeof(ull));
        fin.read((char*)&temp.time, sizeof(ull));
        fin.read((char*)&tLocation, sizeof(ull));
        temp.location.clear();
        temp.location.push_back(tLocation);
        fin.read((char*)&temp.isExposed, sizeof(ull));
        k.push_back(temp);
    }
    fin.close();
    return true;
}

bool TruePosiWrite(char *filename, vector<TracePoint> &k)
{
    ofstream fout(filename, ios::out | ios::binary);
    if (!fout)
    {
        cout << "fout打开TruePosition文件失败，文件不存在" << endl << endl;
        return false;
    }
    int count = k.size();
    fout.write((char*)&count, sizeof(count));
    for (int i = 0; i < count; i++)
    {
        fout.write((char*)&k[i].user, sizeof(ull));
        fout.write((char*)&k[i].time, sizeof(ull));
        fout.write((char*)&k[i].location[0], sizeof(ull));
        fout.write((char*)&k[i].isExposed, sizeof(ull));
    }
    fout.close();
    return true;
}

//传入一个double类型的三维指针向量，以及文件名，即可将矩阵读出文件中所有的概率转移矩阵
bool KnowledgeRead(char* filename, vector<vector<double>> &a)
{
    ifstream fin(filename, ios::in);
    if (!fin)
    {
        cout << "fin读取Knowledge失败" << endl;
        return false;
    }
    const char *divide = " ";
    char temp[1000];
    while (fin.getline(temp, 1000))
    {
        char *p;
        p = strtok(temp, divide);
        string t;
        vector<double> tp;//存放一行的数据
        while (p)//将读到tp中
        {
            t = p;
            tp.push_back(atof(t.c_str()));
            p = strtok(NULL, divide);
        }
        //处理完一行的数据就压入到tpp中
        a.push_back(tp);
        tp.clear();
    }
    fin.close();
    return true;
}


//传入一个double类型的二维指针向量，以及文件名，即可将矩阵读出文件中所有的概率转移矩阵
bool KnowledgeWrite(char* filename, vector<vector<double>> &a)
{
    ofstream fout(filename, ios::out);
    if (!fout)
    {
        cout << "fout读取Knowledge失败" << endl;
        return false;
    }
    for (int i = 0; i < a.size(); i++)
    {
        for (int j = 0; j < a[i].size(); j++)
        {
            fout << a[i][j] << " ";
        }
        if(i!=a.size()-1) fout << endl;
    }
    fout.close();
    return true;
}

bool KSetRead(char *filename, map<int, int> &s)
{
    ifstream fin(filename, ios::in | ios::binary);
    if (!fin)
    {
        cout << "读取失败" << endl; return false;
    }
    int count;
    //将其存为二进制文件
    fin.read((char*)&count, sizeof(int));
    for (int i = 0; i < count; i++)
    {
        int fir, sec;
        fin.read((char*)&fir, sizeof(int));
        fin.read((char*)&sec, sizeof(int));
        s.insert(pair<int, int>(fir, sec));
    }
    fin.close();
    return true;
}

bool KSetWrite(char *filename, map<int, int> &s)
{
    ofstream fout(filename, ios::out | ios::binary);
    if (!fout)
    {
        cout << "存储失败" << endl; return false;
    }
    int count = s.size();
    //将其存为二进制文件
    fout.write((char*)&count, sizeof(int));
    map<int, int>::iterator it;
    for (it = s.begin(); it != s.end(); it++)
    {
        fout.write((char*)&it->first, sizeof(int));
        fout.write((char*)&it->second, sizeof(int));
    }
    fout.close();
    return true;
}


bool PointProbabilityRead(char* filename, vector<double> &a)
{
    ifstream fin(filename, ios::in);
    if (!fin)
    {
        cout << "读取失败" << endl; return false;
    }
    char temp[1000];
    const char * divide = " ";
    while (fin.getline(temp, 1000))
    {
        char *p;
        p = strtok(temp, divide);
        string t;
        while (p)//将读到tp中
        {
            t = p;
            a.push_back(atof(t.c_str()));
            p = strtok(NULL, divide);
        }
        //处理完一行的数据就压入到tpp中
    }
    fin.close();
    return true;

}

bool PointProbabilityWrite(char* filename, vector<double> &a)
{
    ofstream fout(filename, ios::out);
    if (!fout)
    {
        cout << "fout读取Knowledge失败" << endl;
        return false;
    }
    for (int i = 0; i < a.size(); i++)
    {
        fout << a[i] << " ";
    }
    fout.close();
    return true;
}

//*********************************************************表格显示函数*********************************************************

void showTableWidget(QTableWidget *s,TraceSet &a)
{
    //将结果显示到tableWidget里面
    s->clear();//每次使用表格前先清除上次留下来的内容
    s->setColumnCount(4); //设置列数，由于没有isExposed，所以最多三列足矣
    //如果数据导入成功，则展示到表格上面
    QStringList header;//设置表头内容

    //设置标题栏
    header<<"用户号"<<"时间戳"<<"位置戳"<<"暴露情况";
    s->setHorizontalHeaderLabels(header);

    //先统计一共有多少行
    int RowCount=0;
    for(int i=0;i<a.trace.size();i++)
    {
        RowCount+=a.trace[i].size();
    }

    //开始循环设置轨迹列表
    s->setRowCount(RowCount);//设置行数
    int k=0;
    for(int i=0;i<a.trace.size();i++)
    {
        for(int j=0;j<a.trace[i].size();j++,k++)
        {
            s->setItem(k,0,new QTableWidgetItem(QString::number(a.trace[i][j].user)));
            s->setItem(k,1,new QTableWidgetItem(QString::number(a.trace[i][j].time)));
            QString temp="";
            for(int l=0;l<a.trace[i][j].location.size();l++)
            {
                temp+=QString::number(a.trace[i][j].location[l]);
                if(l!=a.trace[i][j].location.size()-1) temp+=",";
            }
            s->setItem(k,2,new QTableWidgetItem(temp));
            if(a.trace[i][j].isExposed==0) s->setItem(k,3,new QTableWidgetItem(QString("未暴露")));
            else if(a.trace[i][j].isExposed==1) s->setItem(k,3,new QTableWidgetItem(QString("暴露")));
        }
    }
}

//*********************************************************坐标处理函数*********************************************************

//row和col分别代表区域划分一共多少行多少列，x和y分别就是要转换的坐标点的横纵坐标
//因为坐标是从(0,0)开始，所以转换公式为：PosiNum = y * col + ( x + 1 );
ull Coordinate_to_PosiNum(ull col, ull x, ull y)
{
    return y * col + x + 1;
}

void PosiNum_to_Coordinate(ull col, ull PosiNum, Coordinate& point)
{
    point.x = (PosiNum - 1) % col;
    point.y = (PosiNum - 1 - point.x) / col;
}

//*********************************************************随机函数与暴露处理函数*********************************************************

//从0到num随机生成数字，并放入ranlist向量的不同位置
void Random_Num(vector<int> &ranlist, int num)
{
    srand(time(NULL));
    int *d = new int[num];

    for (int i = 0; i < num; i++)  //数组d用来置数,数组ranlist用来取不重复的随机数，特别的我想更随机一些，所以将5的倍数和6的倍数的数字再生成随机数
    {
        d[i] = i;
    }
    for (int i = 0; i < num; i++)
    {
        int index = rand() % (num - i);
        ranlist[i] = d[index];
        d[index] = d[num - 1 - i];
        if (i % 5 == 0 || i % 6 == 0) ranlist[i] = rand() % (num * 5);
    }
}

//通过ranlist中的数取模，判断是否暴露，暴露取1，没暴露取0
void Get_isExposed(vector<int> ranlist, TraceSet &a,double percent)
{
    srand(time(NULL));
    int target=a.trace[0].size()*percent;
    //先全部置0
    for (int i = 0; i < a.trace.size(); i++)
    {
        for (int j = 0; j < a.trace[i].size(); j++)
        {
            a.trace[i][j].isExposed = 0;
        }
    }
    //然后统计isExposed为1的记录数
    for (int i = 0; i < a.trace.size(); i++)
    {
        int count=0;
        for (int j = 0; count<target; j++)//确保每个用户的暴露记录数一定
        {
            if (ranlist[rand() % ranlist.size()] % 3 == 0 || ranlist[rand() % ranlist.size()] % 5 == 0)
            {
                if(a.trace[i][j%a.trace[i].size()].isExposed==0) count++;
                a.trace[i][j%a.trace[i].size()].isExposed = 1;
            }
        }
    }
}

//*********************************************************位置保护函数*********************************************************


//这里我暂时写一个简单的加密过程，通过key进行加密，这样的话攻击方即使知道了
//加密算法，不知道该记录相应的Key的话，也没有办法获取到原来的位置
void encrypt(TraceSet &a, ull &n, Key &kk)
{
    n ^= kk.key;//异或一下
    while (n < a.minLocationStamp || n > a.maxLocationStamp)
    {
        if (n > a.maxLocationStamp)
        {
            n -= a.maxLocationStamp;
            kk.key_minus++;
        }
        else if (n < a.minLocationStamp)
        {
            n += a.minLocationStamp;
            kk.key_plus++;
        }
    }
}

//解密算法，c，d表示要解密trace中第c个用户的时刻d记录的location
void decrypt(TraceSet &a, vector<Key> &k, int c, int d)
{
    //先找到a中该用户该时刻的Keys
    int i;
    for (i = 0; i < k.size(); i++)
    {
        if (k[i].user == a.trace[c][d].user && k[i].time == a.trace[c][d].time) break;
    }

    for (int j = 0; j < k[i].key_minus; j++)
    {
        a.trace[c][d].location[0] += a.maxLocationStamp;
    }
    for (int j = 0; j < k[i].key_plus; j++)
    {
        a.trace[c][d].location[0] -= a.minLocationStamp;
    }

    a.trace[c][d].location[0] ^= k[i].key;
}


//位置扰动：通过一系列稍微复杂的数学运算，对已暴露的某条记录的位置信息进行修改
//攻击方如果不知道加密的公式则无法获取到正确的路径
void Perturbation(char *filename, TraceSet &a, vector<Key> &k)
{
    for (int i = 0; i < a.trace.size(); i++)
    {
        for (int j = 0; j < a.trace[i].size(); j++)
        {
            //注意这时候location应该还是只有一个数（a.trace[i][j].location[0]）（因为未经过降低精度之类的处理）
            if (a.trace[i][j].isExposed == 1)
            {
                Key temp(a.trace[i][j].time);//将key置为j
                temp.user = a.trace[i][j].user;
                temp.time = a.trace[i][j].time;
                encrypt(a, a.trace[i][j].location[0], temp);
                k.push_back(temp);
            }
        }
    }
    if(KeyWrite(filename,k))
    {
        QMessageBox::information(NULL, "提示", "已在指定目录保存文件，文件目录为："+QString(filename));
    }
    else QMessageBox::information(NULL, "提示", "保存文件失败");
}

void AntiPerturbation(TraceSet &a, vector<Key>& k)
{
    for (int i = 0; i < a.trace.size(); i++)
    {
        for (int j = 0; j < a.trace[i].size(); j++)
        {
            //注意这时候location应该还是只有一个数（因为未经过降低精度之类的处理）
            if (a.trace[i][j].isExposed == 1)
            {
                decrypt(a, k, i, j);
            }
        }
    }
}

//降低精确度（虚假位置注入）：距离原坐标横向纵向坐标值在Grade之内的坐标点都会加进已经暴露的记录的location里面
//比如原位置为(1,5)，Grade为1，则(0,4)(0,5)(0,6)等(1,5)周围的8个点都会加进location里面，当然要考虑到边界点的情况
bool PrecisionReduce(char *filename, TraceSet &a, ull Grade)
{
    vector<TracePoint> k;
    //首先和位置扰动一样，先找到已经暴露的记录
    int mo=1;
    for (int i = 0; i < a.trace.size(); i++)
    {
        for (int j = 0; j < a.trace[i].size(); j++)
        {
            if (a.trace[i][j].isExposed == 1)
            {
                //先把原来的信息记录下来，等一下会输出成TruePosi文件，以便以后对数据进行还原
                TracePoint tempp;
                tempp.assignment(a.trace[i][j].user, a.trace[i][j].time, a.trace[i][j].location, a.trace[i][j].isExposed);
                k.push_back(tempp);

                //首先进行坐标转换，获取原位置的坐标
                Coordinate oldLocation;
                PosiNum_to_Coordinate(a.areaCol, a.trace[i][j].location[0], oldLocation);
                if (a.trace[i][j].location[0] == 3) cout << "(" << oldLocation.x << ", " << oldLocation.y << ")\n";
                a.trace[i][j].location.clear();//将原来的位置点读出后清空掉location这个向量

                //现在构造该条记录location周围的点，为了方便构造我们以(x-Grade,y+Grade)即方阵的左上角
                //为起点，构造出一个 2*Grade + 1 的方阵，这些点就是我们要添加的假位置
                //如果遇到边界点，我们就不要把它超出区域范围的点存入vector里面
                //比如(0,2)，周围只有5个点左边3个点(-1,3)(-1,2)(-1,1)是不合法的
                vector<Coordinate> PointAroundLocation;
                int bound = 2 * Grade + 1;
                Coordinate LeftTopPoint(oldLocation.x - Grade, oldLocation.y + Grade);
                Coordinate temp;

                for (int m = 0; m < bound; m++)
                {
                    for (int n = 0; n < bound; n++)
                    {
                        //首先要注入的虚假位置需要在位置范围之内
                        if (LeftTopPoint.x + m >= 0 && LeftTopPoint.x + m < a.areaCol
                                && LeftTopPoint.y - n >= 0 && LeftTopPoint.y - n < a.areaRow)
                        {
                            temp.x = LeftTopPoint.x + m;
                            temp.y = LeftTopPoint.y - n;
                            PointAroundLocation.push_back(temp);
                        }
                    }
                }

                vector<ull> tempPosiNum;
                //获取到周围的有效坐标位置以后，我们要把他们转换成PosiNum，并且存入到
                //相应记录的location向量里面，但是全部如果每个点都全部存入，会让攻击方
                //推算到哪些被保护的点的位置是相同的，所以我们随机取出部分点存进去
                while (tempPosiNum.size() == 0)
                {
                    for (int m = 0; m < PointAroundLocation.size(); m++)
                    {
                        srand(time(NULL) * mo + mo);
                        if (rand() % 3 == 0)
                        {
                            tempPosiNum.push_back(Coordinate_to_PosiNum(a.areaCol, PointAroundLocation[m].x, PointAroundLocation[m].y));
                            mo = mo * 3 % 200;
                        }
                        mo += 11;
                    }
                }

                sort(tempPosiNum.begin(), tempPosiNum.end());
                a.trace[i][j].location = tempPosiNum;
            }
        }
    }
    if (TruePosiWrite(filename, k)) QMessageBox::information(NULL, "提示", "已在指定目录保存文件，文件目录为："+QString(filename));
    else {QMessageBox::information(NULL, "提示", "保存文件失败");return false;}
    return true;
}

void AntiPrecisionReduce(TraceSet& a, vector<TracePoint> &k)
{
    for (int i = 0; i < a.trace.size(); i++)
    {
        for (int j = 0; j < a.trace[i].size(); j++)
        {
            if (a.trace[i][j].isExposed == 1)
            {
                for (int s = 0; s < k.size(); s++)
                {
                    if (a.trace[i][j].user == k[s].user&&a.trace[i][j].time == k[s].time)
                    {
                        a.trace[i][j].location = k[s].location;
                        break;
                    }
                }
            }
        }
    }
}

//位置隐匿:将暴露出来的信息全部存起来，然后清空相应用户的location向量
//然后将存储隐匿的信息的输出成文件
bool PositionHidden(char *filename, TraceSet& a)
{
    vector<TracePoint> temp;
    TracePoint t;
    for (int i = 0; i < a.trace.size(); i++)
    {
        for (int j = 0; j < a.trace[i].size(); j++)
        {
            if (a.trace[i][j].isExposed == 1)
            {
                t.assignment(a.trace[i][j].user, a.trace[i][j].time, a.trace[i][j].location, a.trace[i][j].isExposed);
                temp.push_back(t);
                a.trace[i][j].location.clear();
            }
        }
    }
    if (TruePosiWrite(filename, temp)) QMessageBox::information(NULL, "提示", "已在指定目录保存文件，文件目录为："+QString(filename));
    else {QMessageBox::information(NULL, "提示", "保存文件失败");return false;}
    return true;
}

//根据原来轨迹的数据，还原轨迹
void AntiPositionHidden(TraceSet &a, vector<TracePoint> &k)
{
    for (int i = 0; i < a.trace.size(); i++)
    {
        for (int j = 0; j < a.trace[i].size(); j++)
        {
            if (a.trace[i][j].isExposed == 1)
            {
                for (int s = 0; s < k.size(); s++)
                {
                    if (a.trace[i][j].user == k[s].user&&a.trace[i][j].time == k[s].time)
                    {
                        a.trace[i][j].location = k[s].location;
                        break;
                    }
                }
            }
        }
    }
}

//K-匿名保护算法，伪造出k条和原记录相似的轨迹出来并且加入到文件里面
//然后混淆用户名，将用户放到其他人的轨迹下面，由于我们的轨迹文件比较
//简单，所以没有什么可以作为准标识符的，我对其进行一些改进，直接构造出
//虚假的轨迹，这样，攻击方对不同用户的攻击难度就会增加上升
//解密文件格式：总数+多个<user,数组下标>键值对(map<int,int>)
bool K_Anonymization(char *filename, TraceSet& a,int k)
{
    if(a.trace.size()==0) {QMessageBox::information(NULL, "提示", "轨迹为空或参数错误，保护操作终止");return false;}

    //先记录用户对应哪条轨迹（等下生成map对会用到）
    TraceSet tempp;
    tempp.trace = a.trace;
    //然后作标记，将原来a的轨迹的第一个值改成一个不可能出现的位置戳的值（这里我用a.maxLocationStamp+user作为该条记录的标记)
    for (int i = 0; i < a.trace.size(); i++)
    {
        a.trace[i][0].location[0] = a.maxLocationStamp + a.trace[i][0].user;
    }

    //先找出暴露轨迹文件中所有的用户有哪些
    vector<ull> exist_user;//只存放用户号
    for (int i = 0; i < a.trace.size(); i++)
    {
        exist_user.push_back(a.trace[i][0].user);
    }
    int length = a.trace[0].size();//读出单条轨迹的长度
    int mo = 1;//随机数种子

    //然后判断真实的用户轨迹够不够k条，如果不够就开始构造虚假轨迹
    //如果足够，就直接混淆不同轨迹的用户名
    if (exist_user.size() < k)
    {
        for (int i = 0; i < k - exist_user.size(); i++)
        {
            vector<TracePoint> fake_trace(length);//申请固定长度的向量
            srand(time(NULL) * mo + mo);
            mo = mo * 3 % 200;
            int initial = 0;
            //因为想要伪造的轨迹想要真实一点，有很多时候用户会定在一个位置不动
            int s = rand() % (length / 2);//s用来记录一条轨迹里面一段时间内用户会有多久定着不动
            for (int j = 0; j < a.trace[0].size(); j++)
            {
                if (s == 0)
                {
                    s = rand() % (length / 2);
                    fake_trace[j].location.push_back(rand() % a.maxLocationStamp);
                }
                else
                {
                    if (initial == 0) { fake_trace[j].location.push_back(rand() % a.maxLocationStamp); initial = 1; }//最开始的时候函数没有i-1
                    else fake_trace[j].location.push_back(fake_trace[j - 1].location[0]);
                    s--;
                }
            }
            a.trace.push_back(fake_trace);
        }
        random_shuffle(a.trace.begin(), a.trace.end());

        //因为轨迹不够k条，所以用户号也不够，要进行随机生成
        vector<ull> t(a.maxUserStamp-a.minUserStamp+1, 0);
        int tt=a.minUserStamp;
        for (int i = 0; i < t.size(); i++)
        {
            t[i] = tt;
            tt++;
        }

        random_shuffle(t.begin(), t.end());//打乱向量顺序
        int userlength = exist_user.size();

        int p = 0;
        for (int i = 0; i < k-userlength; i++,p++)
        {
            int flag = 0;
            //查重
            for (int j = 0; j < exist_user.size(); j++)
            {
                if (exist_user[j] == t[p]) { flag = 1; break; }
            }
            if (flag == 1) { i--; continue; }
            else
            {
                exist_user.push_back(t[p]);
            }
        }
    }
    srand(time(NULL) * (mo+1) + mo);
    random_shuffle(exist_user.begin(), exist_user.end());

    //下面开始记录用户和其原轨迹所在位置的键值对
    map<int, int> s;
    for (int i = 0; i < tempp.trace.size(); i++)
    {
        for (int j = 0; j < a.trace.size(); j++)
        {
            if (tempp.trace[i][0].user == a.trace[j][0].location[0]-a.maxLocationStamp)
            {
                a.trace[j][0].location[0] = tempp.trace[i][0].location[0];//还原回去
                s.insert(pair<int, int>(tempp.trace[i][0].user,j));//构造键值对
                break;
            }
        }
    }

    //生成好以后依次赋值就可以了，这里要注意，我们假设每条轨迹都必须是一样大小的
    for (int i = 0; i < a.trace.size(); i++)
    {
        for (int j = 0; j < a.trace[i].size(); j++)
        {
            a.trace[i][j].user = exist_user[i];
            a.trace[i][j].time = tempp.trace[0][j].time;
            if(a.trace[i][j].isExposed==2) a.trace[i][j].isExposed = rand() % 2;
        }
    }

    //将生成好的Kmap存储到用户指定的位置
    if (KSetWrite(filename,s)) QMessageBox::information(NULL, "提示", "已在指定目录保存Kmap文件，文件目录为："+QString(filename));
    else {QMessageBox::information(NULL, "提示", "保存文件失败");return false;}

    return true;
}

//a是空的，b是服务端要进行解密的TraceSet，s是密钥
void AntiK_Anonymization(TraceSet& a,TraceSet &b, map<int, int> &s)
{
    //先构造a的基本信息
    a.minUserStamp = b.minUserStamp;
    a.maxUserStamp = b.maxUserStamp;
    a.minTimeStamp = b.minTimeStamp;
    a.maxTimeStamp = b.maxTimeStamp;
    a.minLocationStamp = b.minLocationStamp;
    a.maxLocationStamp = b.maxLocationStamp;
    a.areaCol = b.areaCol;
    a.areaRow = b.areaRow;

    map<int, int>::iterator it;
    //根据a和map进行解密
    for (it = s.begin(); it != s.end(); it++)
    {
        vector<TracePoint> temp;
        for (int i = 0; i < b.trace[it->second].size(); i++)
        {
            TracePoint tempp;
            tempp.user = it->first;
            tempp.time = b.trace[it->second][i].time;
            tempp.location = b.trace[it->second][i].location;
            tempp.isExposed = b.trace[it->second][i].isExposed;
            temp.push_back(tempp);
        }
        a.trace.push_back(temp);
    }

}
//*********************************************************度量函数*********************************************************

//针对跟踪攻击和相遇披露攻击，基于误差的度量，具体来说就是以一定的误差距离，判断两条轨迹之间是否
//有一致的点，然后计算两条轨迹一致的程度，其中a是用户的实际轨迹，b是攻击方的轨迹，u是要查询
//的用户，devition是误差允许范围（比如说2，那么攻击方推算出该点半径为2的范围内的点都算攻击成功）
//timeBegin~timeEnd是要估算的时间范围。这里我设计了两个衡量标准，一个是吻合的位置的数目，另一个
//是对相应的攻击轨迹和实际轨迹的点求距离差，然后再取平均值，由这两者共同来衡量隐私保护程度
bool MetricForTracking(TraceSet &a, TraceSet &b, ull timeBegin, ull timeEnd, ull u, ull deviation, TrackingEvalutate &eva)
{
    //我们默认TraceSet内轨迹大小都是一样的
    vector<Coordinate> actualTrace;
    vector<Coordinate> attackTrace;
    int exitFlag = 0;
    if (timeEnd <= timeBegin) return false;

    //首先要将位置戳转换为坐标点，并存起来
    for (int i = 0; i < a.trace[i].size(); i++)
    {
        if (a.trace[i][0].user == u)
        {
            if (timeBegin<a.trace[i][0].time || timeEnd>a.trace[i][a.trace[i].size() - 1].time)
            {
                exitFlag = 1; break;
            }
            for (int j = 0; j < a.trace[i].size(); j++)
            {
                if (a.trace[i][j].time >= timeBegin && a.trace[i][j].time <= timeEnd)
                {
                    Coordinate temp;
                    PosiNum_to_Coordinate(a.areaCol, a.trace[i][j].location[0], temp);
                    actualTrace.push_back(temp);
                }
            }
            break;
        }
    }
    if (exitFlag == 1) return false;

    //由于攻击方的文件的用户顺序和实际轨迹文件的用户顺序可能不一样，这里只能分开两个循环来写了
    for (int i = 0; i < b.trace[i].size(); i++)
    {
        if (b.trace[i][0].user == u)
        {
            if (timeBegin<b.trace[i][0].time || timeEnd>b.trace[i][b.trace[i].size() - 1].time)
            {
                exitFlag = 1; break;
            }
            for (int j = 0; j < b.trace[i].size(); j++)
            {
                if (b.trace[i][j].time >= timeBegin && b.trace[i][j].time <= timeEnd)
                {
                    Coordinate temp;
                    PosiNum_to_Coordinate(b.areaCol, b.trace[i][j].location[0], temp);
                    attackTrace.push_back(temp);
                }
            }
            break;
        }
    }
    if (exitFlag == 1) return false;

    vector<double> distance;
    double match = 0;
    //下面开始进行两条轨迹的吻合度判断，这里我想了一种模型，将每条记录的位置戳的距离记录下来
    //最后求和再取均值，这样能够作为衡量两条曲线吻合度的一种不太准确的衡量标准
    for (int i = 0; i < actualTrace.size(); i++)
    {
        //计算某条记录的距离差
        long double powx, powy;
        powx = pow(abs(double(actualTrace[i].x - attackTrace[i].x)), 2);
        powy = pow(abs(double(actualTrace[i].y - attackTrace[i].y)), 2);
        distance.push_back(sqrt(powx + powy));
        if (distance[distance.size() - 1] <= deviation)
        {
            match++;
        }
    }
    eva.coincidence = match / actualTrace.size();
    eva.AverageDistance = 0;
    for (int i = 0; i < distance.size(); i++)
    {
        eva.AverageDistance += distance[i];
    }
    eva.AverageDistance = eva.AverageDistance / distance.size();
    return true;
}


//基于熵的度量方法，先读取攻击方的Knowledge文件，获取转移概率矩阵，然后计算熵和最大熵值的比值并记录到result里面
void MetricByEntropy(vector<vector<double>> &a, double &result)
{
    //先对矩阵进行百分比处理，让所有概率加起来在1以内
    double sum = 0;
    for (int i = 0; i < a.size(); i++)
    {
        for (int j = 0; j < a[i].size(); j++)
        {
            sum += a[i][j];
        }
    }
    //然后进行归一化
    for (int i = 0; i < a.size(); i++)
    {
        for (int j = 0; j < a[i].size(); j++)
        {
            a[i][j] /= sum;
        }
    }
    double Hmax = -log2(1 / double(a.size()*a[0].size()));
    double H = 0;
    for (int i = 0; i < a.size(); i++)
    {
        for (int j = 0; j < a[i].size(); j++)
        {
            if(a[i][j]!=0) H += -a[i][j] * log2(a[i][j]);
        }

    }
    result = H / Hmax;
}

//*********************************************************知识构造与攻击函数*********************************************************

void glzyjz(TraceSet &a,vector<vector<double>> &zb)//构造概率转移矩阵
{
    ull c[1000];
    for (int i = 0; i < 1000; i++)
        c[i] = 0;
    int d = 0;
    for (int i = 0; i < a.trace.size(); i++)//求有多少个位置点
        for (int j = 0; j < a.trace[i].size(); j++)
        {
            int e = 0;
            for (int k = 0; k <= d; k++)
            {
                if (a.trace[i][j].location[0] == c[k])
                {
                    e++;
                    break;
                }
            }
            if (e == 0)
            {
                c[d] = a.trace[i][j].location[0];
                d++;
            }
        }
    double** zhuanyi = new double *[d];//概率转移矩阵
    for (int i = 0; i < d; i++)
    {
        zhuanyi[i] = new double[d];
    }
    for (int i = 0; i < d; i++)
        for (int j = 0; j < d; j++)
            zhuanyi[i][j] = 0;
    int b;
    b = a.trace.size();
    double*** cun = new double **[b];
    for (int i = 0; i < b; i++)
    {
        cun[i] = new double *[d];
        for (int j = 0; j < d; j++)
        {
            cun[i][j] = new double[d];
        }
    }
    for (int i = 0; i < b; i++)
        for (int j = 0; j < d; j++)
            for (int k = 0; k < d; k++)
                cun[i][j][k] = 0;
    for (int i = 0; i < a.trace.size(); i++)
    {
        for (int j = 0; j < a.trace[i].size() - 1; j++)
        {
            cun[i][a.trace[i][j].location[0] - 1][a.trace[i][j + 1].location[0] - 1]++;
        }
    }
    for (int i = 0; i < b; i++)
    {
        for (int j = 0; j < d; j++)
        {
            double sum = 0;
            for (int k = 0; k < d; k++)
            {
                sum = sum + cun[i][j][k];
            }
            for (int k = 0; k < d; k++)
            {
                if (sum != 0)
                {
                    cun[i][j][k] = cun[i][j][k] / sum;
                }
                else
                {
                    cun[i][j][k] = 1.0 / (double)d;
                }
            }
        }
    }
    for (int i = 0; i < d; i++)
    {
        for (int j = 0; j < d; j++)
        {
            double sum = 0;
            for (int k = 0; k < b; k++)
            {
                sum = sum + cun[k][i][j];
            }
            zhuanyi[i][j] = sum / b;
        }
    }
    for (int i = 0; i < d; i++)
    {
        vector<double> vi;
        for (int j = 0; j < d; j++)
        {
            vi.push_back(zhuanyi[i][j]);
        }
        zb.push_back(vi);
    }
}


void diangailv(TraceSet &a,vector<double> &zb)//构造一个矩阵，表示用户出现在某个点的概率
{
    ull c[1000];
    for (int i = 0; i < 1000; i++)
        c[i] = 0;
    int d = 0;
    for (int i = 0; i < a.trace.size(); i++)//求有多少个位置点
        for (int j = 0; j < a.trace[i].size(); j++)
        {
            int e = 0;
            for (int k = 0; k <= d; k++)
            {
                if (a.trace[i][j].location[0] == c[k])
                {
                    e++;
                    break;
                }
            }
            if (e == 0)
            {
                c[d] = a.trace[i][j].location[0];
                d++;
            }
        }
    int b;
    b = a.trace.size();
    double **gailv = new double *[b];
    for (int i = 0; i < b; i++)
    {
        gailv[i] = new double[d];
    }
    for (int i = 0; i < b; i++)
    {
        for (int j = 0; j < d; j++)
            gailv[i][j] = 0;
    }
    for (int i = 0; i < a.trace.size(); i++)
    {
        for (int j = 0; j < a.trace[i].size(); j++)
        {
            gailv[i][a.trace[i][j].location[0] - 1]++;
        }
    }
    for (int i = 0; i < b; i++)
    {
        for (int j = 0; j < d; j++)
        {
            gailv[i][j] = gailv[i][j] / a.trace[i].size();
        }
    }
    double *shumu = new double[d];
    for (int i = 0; i < d; i++)
    {
        double sum = 0;
        for (int j = 0; j < b; j++)
        {
            sum = sum + gailv[j][i];
        }
        shumu[i] = sum /(double)b;
    }
    for (int i = 0; i < d; i++)
    {
        zb.push_back(shumu[i]);
    }
}


//一下函数参数含义：a是实例，e是二维数组的行标，表示不同的用户，c，d表示暴露相邻两个点的二维数组的列标，d代表概率转移矩阵，z代表每个位置点的概率矩阵
void dange(TraceSet &a, int e,int b, int c, vector<vector<double>> &d,vector<double> &z)//计算单个最大点
{
    int f = d.size();//概率矩阵行数，也就是位置的个数
    if (a.trace[e][b].isExposed == 1 && a.trace[e][c].isExposed == 1)
    {
        double *g = new double[f];
        double *h = new double[f];
        for (int i = 0; i < f; i++)
        {
            g[i] = 0;//存概率
            h[i] = 0;//跟踪概率
        }
        for (int i = 0; i <f; i++)//初始化第一个
        {
            g[i] = d[a.trace[e][b].location[0]-1][i];
        }
        for (int i = 0; i < f; i++)
        {
            h[i] = g[i];
        }
        for (int i = b+1; i < c-1; i++)
        {
            for (int j = 0; j < f; j++)
            {
                double sum = 0;
                for (int t = 0; t < f; t++)
                {
                    sum=sum+ d[t][j] * h[t];
                }
                g[j] = sum;
            }
            for (int m = 0; m < f; m++)
            {
                h[m] = g[m];
            }
        }
        for (int i = 0; i < f; i++)
        {
            g[i] = h[i] * d[i][a.trace[e][c].location[0] - 1];
        }
        double n = 0;
        int p = 0;
        for (int i = 0; i < f; i++)
        {
            if (g[i] > n)
            {
                n = g[i];
                p = i;
            }
        }
        a.trace[e][c - 1].location[0] = p + 1;
        a.trace[e][c - 1].isExposed = 1;
    }
    if (a.trace[e][b].isExposed == 0 && a.trace[e][c].isExposed == 1)
    {
        double *g = new double[f];
        double *h = new double[f];
        for (int i = 0; i < f; i++)
        {
            g[i] = 0;//存概率
            h[i] = 0;//跟踪概率
        }
        for (int i = 0; i < f; i++)
        {
            g[i] = z[i];
            h[i] = g[i];
        }
        for (int i = b; i < c - 1; i++)
        {
            for (int j = 0; j < f; j++)
            {
                double sum = 0;
                for (int t = 0; t < f; t++)
                {
                    sum = sum + d[t][j] * h[t];
                }
                g[j] = sum;
            }
            for (int m = 0; m < f; m++)
            {
                h[m] = g[m];
            }
        }
        for (int i = 0; i < f; i++)
        {
            g[i] = h[i] * d[i][a.trace[e][c].location[0] - 1];
        }
        int p = 0;
        double n = 0;
        for (int i = 0; i < f; i++)
        {
            if (g[i] > n)
            {
                n = g[i];
                p = i;
            }
        }
        a.trace[e][c - 1].location[0] = p + 1;
        a.trace[e][c - 1].isExposed = 1;
    }
    if (a.trace[e][b].isExposed == 1 && a.trace[e][c].isExposed == 0)
    {
        double *g = new double[f];
        double *h = new double[f];
        for (int i = 0; i < f; i++)
        {
            g[i] = 0;//存概率
            h[i] = 0;//跟踪概率
        }
        for (int i = 0; i < f; i++)
        {
            g[i] = d[a.trace[e][b].location[0]-1][i];
        }
        for (int i = 0; i < f; i++)
        {
            h[i] = g[i];
        }
        for (int i = b + 1; i < c; i++)
        {
            for (int j = 0; j < f; j++)
            {
                double sum = 0;
                for (int t = 0; t < f; t++)
                {
                    sum = sum + d[t][j] * h[t];
                }
                g[j] = sum;
            }
            for (int m = 0; m < f; m++)
            {
                h[m] = g[m];
            }
        }
        int p = 0;
        double n = 0;
        for (int i = 0; i < f; i++)
        {
            if (g[i] > n)
            {
                n = g[i];
                p = i;
            }
        }
        a.trace[e][c].location[0] = p + 1;
        a.trace[e][c].isExposed = 1;
    }
}


void tuidao(TraceSet &a,int e, int b, int c,vector<vector<double>> &d, vector<double> &z)//局部推测路径
{
    if (a.trace[e][b].isExposed == 1 && a.trace[e][c].isExposed == 1)
    {
        int m = b, n = c;
        while (n > m + 1)
        {
            dange(a, e, m, n, d, z);
            n--;
        }
    }
    if (a.trace[e][b].isExposed == 0 && a.trace[e][c].isExposed == 1)
    {
        int m = b, n = c;
        while (n > m)
        {
            dange(a, e, m, n, d, z);
            n--;
        }
    }
    if (a.trace[e][b].isExposed == 1 && a.trace[e][c].isExposed == 0)
    {
        int m = b, n = c;
        while (n > m+1)
        {
            if (a.trace[e][n].isExposed == 0)
            {
                dange(a, e, m, n, d, z);
            }
            if (a.trace[e][n].isExposed == 1)
            {
                dange(a, e, m, n, d, z);
                n--;
            }
        }
    }
}


void lujing(TraceSet &a, vector<vector<double>> &d, vector<double> &z)//通过预测概率计算生成完整路径
{
    for (int i = 0; i < a.trace.size(); i++)
    {
        int m = 0, n = 0;
        for (int j = 1; j < a.trace[i].size(); j++)
        {
            if (a.trace[i][j].isExposed == 1)
            {
                n = j;
                tuidao(a, i, m, n, d, z);
                m = n;
            }
        }
        if (m != a.trace[i].size()-1)
        {
            n = a.trace[i].size()-1;
            tuidao(a, i, m, n, d, z);
        }
    }
}
