#ifndef HEAD_H
#define HEAD_H

#pragma once
/*所有头文件、类的定义、函数声明在这个文件中编写*/

#define _CRT_SECURE_NO_WARNINGS
#include <climits>
#include <stdint.h>
#include <io.h>
#include <direct.h>
#include <time.h>
#include <cstring>
#include <string>
#include <QString>
#include <QMenu>
#include <QFormLayout>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QAction>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QDebug>
#include <QDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QTextCodec>
#include <QWidget>
#include <QTimer>
#include <QList>
#include <QPainter>
#include <QPointF>
#include <QPen>
#include <qmath.h>
#include <QFileDialog>
#include <QByteArray>
#include <QMainWindow>
#include <QPushButton>
#include <QAbstractButton>
#include <QInputDialog>
#include <memory>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <cfloat>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <vector>
#include <map>
#include <list>
#include <algorithm>

typedef uint64_t uint64, ull;//unsigned long long定义简写为ull
typedef int64_t int64, ll;

using namespace std;

//*****************************************************************类定义****************************************************************

//坐标点（和数组下表吻合）
class Coordinate
{
public:
    ll x;
    ll y;

    Coordinate(ull xx = 0, ull yy = 0)
    {
        x = xx; y = yy;
    }
};

//用户的轨迹由多个TracePoint组成
class TracePoint
{
public:
    ull user;//用户号
    ull time;//时间戳（以小时为单位）
    vector<ull> location;//位置戳（存放PosiNum）
    ull isExposed;//表示该条信息是否暴露

    //构造函数
    TracePoint()
    {
        user = 0; time = 0; isExposed = 2;
    }

    //赋值函数
    void assignment(ull u, ull t, vector<ull> &l, ull is)
    {
        user = u; time = t; location = l; isExposed = is;
    }
};

//从文件读出来的数据全部放到该类里面
class TraceSet
{
public:
    ull minUserStamp;
    ull maxUserStamp;
    ull minTimeStamp;
    ull maxTimeStamp;
    ull minLocationStamp;
    ull maxLocationStamp;
    ull areaRow;
    ull areaCol;

    vector<vector<TracePoint>> trace;//包含多个用户的轨迹
    void initial()
    {
        this->minUserStamp = -1;
        this->maxUserStamp = -1;
        this->minTimeStamp = -1;
        this->maxTimeStamp = -1;
        this->minLocationStamp = -1;
        this->maxLocationStamp = -1;
        this->areaRow = -1;
        this->areaCol = -1;
        this->trace.clear();
    }
};

//用于记录key，只要key不泄露，攻击方就无法破解到
class Key
{
public:
    ull user;
    ull time;
    ull key;//用于加密时进行异或
    int key_minus;//表示进行了多少次减maxLocationStamp操作
    int key_plus;

    Key()
    {
        user = 0; key = 15; key_minus = 0; key_plus = 0;
    }

    Key(int j)
    {
        user = 0; key = j; key_minus = 0; key_plus = 0;
    }
};

class TrackingEvalutate
{
public:
    double coincidence;//吻合度
    double AverageDistance;//各个点之间的平均距离
};


//****************************************************************数据定义*******************************+********************************
extern TraceSet a1;//保护部分用

extern TraceSet a21;//度量部分用
extern TraceSet a22;
extern vector<vector<double>> tsmetric;
extern int paintFlag;
extern vector<TracePoint> user1;
extern vector<TracePoint> user2;

extern TraceSet a3;//还原轨迹部分用
extern vector<Key> key;//还原位置扰动
extern vector<TracePoint> tp;//还原假位置注入、位置隐匿
extern map<int,int> kv_pair;//还原K匿名

extern TraceSet a41;//攻击部分要用到
extern TraceSet a42;

//*****************************************************************函数声明*****************************************************************
//Coordinate.h
ull Coordinate_to_PosiNum(ull col, ull x, ull y);
void PosiNum_to_Coordinate(ull col, ull PosiNum, Coordinate& point);

//File.h
bool FileRead(char* filename, TraceSet &tr, int type);
bool FileWrite(char* filename, TraceSet &tr, int type);
bool KeyRead(char *filename, vector<Key> &k);
bool KeyWrite(char *filename, vector<Key> &k);
bool TruePosiRead(char *filename, vector<TracePoint> &k);
bool TruePosiWrite(char *filename, vector<TracePoint> &k);
bool KnowledgeRead(char* filename, vector<vector<double>> &a);
bool KnowledgeWrite(char* filename, vector<vector<double>> &a);
bool KSetRead(char *filename, map<int, int> &s);
bool KSetWrite(char *filename, map<int, int> &s);
bool PointProbabilityRead(char* filename,vector<double> &a);
bool PointProbabilityWrite(char* filename,vector<double> &a);

//showQTableWidget（将TraceSet的内容显示到表格中）
void showTableWidget(QTableWidget *s,TraceSet &a);

//RNG_and_Exposed.h
void Random_Num(vector<int> &ranlist, int num);
void Get_isExposed(vector<int> ranlist, TraceSet &a,double percent);

//LPPM.h
void encrypt(TraceSet &a, ull &n, Key &kk);
void decrypt(TraceSet &a, vector<Key> &k, int c, int d);
void Perturbation(char *filename, TraceSet &a, vector<Key> &k);
void AntiPerturbation(TraceSet &a, vector<Key> &k);
bool PrecisionReduce(char *filename, TraceSet &a, ull Grade);
void AntiPrecisionReduce(TraceSet& a, vector<TracePoint> &k);
bool PositionHidden(char *filename, TraceSet& a);
void AntiPositionHidden(TraceSet &a, vector<TracePoint> &k);
bool K_Anonymization(char *filename, TraceSet& a,int k);
void AntiK_Anonymization(TraceSet& a,TraceSet &b, map<int, int> &s);

//Metrics.h
bool MetricForTracking(TraceSet &a, TraceSet &b, ull timeBegin, ull timeEnd, ull u, ull deviation, TrackingEvalutate &eva);
void MetricByEntropy(vector<vector<double>> &a, double &result);

//attackAndKnowledgeConstruct.h
void glzyjz(TraceSet &a,vector<vector<double>> &zb);//构造概率转移矩阵
void diangailv(TraceSet &a,vector<double> &zb);//构造某个点出现的概率的向量
void dange(TraceSet &a, int e,int b, int c, vector<vector<double>> &d,vector<double> &z);//计算单个最大点
void tuidao(TraceSet &a,int e, int b, int c,vector<vector<double>> &d, vector<double> &z);//局部推测路径
void lujing(TraceSet &a, vector<vector<double>> &d, vector<double> &z);//通过预测概率计算生成完整路径

#endif // HEAD_H
