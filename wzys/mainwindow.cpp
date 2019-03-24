#include "head.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);    // 禁止最大化按钮
    setFixedSize(this->width(),this->height());                     // 禁止拖动窗口大小
    ui->comboBox_3->addItem("Key");
    ui->comboBox_3->addItem("TruePosition");
    ui->comboBox_3->addItem("Kmap");
    ui->comboBox_2->addItem("基于轨迹失真的度量");
    ui->comboBox_2->addItem("基于熵的度量");
    paint=new painter();
    ui->stackedWidget->addWidget(paint);
    paint->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

//********************************************************保护部分********************************************************
//该按钮用于暴露处理时，打开真实轨迹文件
void MainWindow::on_pushButton_9_clicked()
{
    a1.initial();
    ui->tableWidget->clear();
    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setWindowTitle("请选择需要打开的.trace文件");
    fileDialog->setDirectory(".");//设置默认路径
    fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog->setViewMode(QFileDialog::Detail);//详细
    fileDialog->setDefaultSuffix("*.trace");

    if (fileDialog->exec() == QDialog::Accepted)
    {
        QString path = fileDialog->selectedFiles()[0];
        QTextCodec *code = QTextCodec::codecForName("GB2312");
        string tmp = code->fromUnicode(path).data();
        if(tmp.substr(tmp.size() - 6)!=".trace")
        {
            QMessageBox::information(NULL, "提示", "你选择的文件不是.trace文件，请重新选择");
            return;
        }
        char filepath[300];
        strcpy(filepath, tmp.c_str());
        ui->lineEdit_5->setText(path);

        if(FileRead(filepath,a1,0))
        {
            //将数据显示到表格上面
            showTableWidget(ui->tableWidget,a1);
            //下面是tableWidget的格式调整
            ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应列宽
            ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应行高

            ui->tableWidget->verticalHeader()->setVisible(true); //设置垂直头不可见
            ui->tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);  //可多选（Ctrl、Shift、  Ctrl+A都可以）
            ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选择行为时每次选择一行
            ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置不可编辑

            ui->tableWidget->setStyleSheet("selection-background-color:lightblue;"); //设置选中背景色
            //设置水平、垂直滚动条
            ui->tableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
            ui->tableWidget->repaint();

            //此处定义点击表头排序的功能
            QHeaderView *headerGoods = ui->tableWidget->horizontalHeader();
            //SortIndicator为水平标题栏文字旁边的三角指示器
            headerGoods->setSortIndicator(0, Qt::AscendingOrder);
            headerGoods->setSortIndicatorShown(true);
            headerGoods->setSectionsClickable(true);
            connect(headerGoods, SIGNAL(sectionClicked(int)), ui->tableWidget, SLOT(sortByColumn(int)));

        }
        else QMessageBox::information(NULL, "错误", "在打开如下文件的时候发生错误！无法打开文件"+path);
    }
}

//该按钮用于对之前读出来的轨迹进行暴露处理
void MainWindow::on_pushButton_12_clicked()
{
    //对a进行暴露操作，让用户确定要暴露的记录百分比为多少
    double percent;
    percent = QInputDialog::getDouble(this, tr("请输入要暴露的记录百分比"),tr("暴露比(0~1):"), 0);

    if(percent<0||percent>1) {QMessageBox::information(NULL, "错误", "您输入的暴露比参数不合法，暴露操作终止");return;}

    vector<int> t(500, 0);
    Random_Num(t, 500);
    Get_isExposed(t, a1,percent);//随机生成isExposed决定是否暴露

    showTableWidget(ui->tableWidget,a1);
    //下面是tableWidget的格式调整
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应列宽
    ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应行高

    ui->tableWidget->verticalHeader()->setVisible(true); //设置垂直头不可见
    ui->tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);  //可多选（Ctrl、Shift、  Ctrl+A都可以）
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选择行为时每次选择一行
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置不可编辑

    ui->tableWidget->setStyleSheet("selection-background-color:lightblue;"); //设置选中背景色
    //设置水平、垂直滚动条
    ui->tableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->tableWidget->repaint();

    //此处定义点击表头排序的功能
    QHeaderView *headerGoods = ui->tableWidget->horizontalHeader();
    //SortIndicator为水平标题栏文字旁边的三角指示器
    headerGoods->setSortIndicator(0, Qt::AscendingOrder);
    headerGoods->setSortIndicatorShown(true);
    headerGoods->setSectionsClickable(true);
    connect(headerGoods, SIGNAL(sectionClicked(int)), ui->tableWidget, SLOT(sortByColumn(int)));

}

//导出暴露文件
void MainWindow::on_pushButton_20_clicked()
{
    //暴露处理过后，文件就要进行存储
    QMessageBox::information(NULL, "提示", "请选择要存储的文件路径（注意文件名必须以.trace结尾）");

    QString path = QFileDialog::getSaveFileName(this, "请选择存放的文件路径", "./exposed.trace" , tr("trace file (*.trace)"));
    QTextCodec *code = QTextCodec::codecForName("GB2312");
    string tmp = code->fromUnicode(path).data();
    if(tmp.substr(tmp.size() - 6)!=".trace")
    {
        QMessageBox::information(NULL, "提示", "您的文件名不是以.trace，请重新选择");
        return;
    }
    char filepath[300];
    strcpy(filepath, tmp.c_str());
    if(FileWrite(filepath,a1,1))
    {
        QMessageBox::information(NULL, "提示", "已在指定目录保存文件，文件目录为："+path);
    }
    else QMessageBox::information(NULL, "提示", "保存文件失败");
}

//打开暴露轨迹文件
void MainWindow::on_pushButton_13_clicked()
{
    a1.initial();
    ui->tableWidget_3->clear();
    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setWindowTitle("请选择需要打开的.trace文件");
    fileDialog->setDirectory(".");//设置默认路径
    fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog->setViewMode(QFileDialog::Detail);//详细
    fileDialog->setDefaultSuffix("*.trace");

    if (fileDialog->exec() == QDialog::Accepted)
    {
        QString path = fileDialog->selectedFiles()[0];
        QTextCodec *code = QTextCodec::codecForName("GB2312");
        string tmp = code->fromUnicode(path).data();
        if(tmp.substr(tmp.size() - 6)!=".trace")
        {
            QMessageBox::information(NULL, "提示", "你选择的文件不是.trace文件，请重新选择");
            return;
        }
        char filepath[300];
        strcpy(filepath, tmp.c_str());
        ui->lineEdit_6->setText(path);

        if(FileRead(filepath,a1,1))
        {
            //将Tracset的内容显示到TableWidge上
            showTableWidget(ui->tableWidget_3,a1);
            //下面是tableWidget的格式调整
            ui->tableWidget_3->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应列宽
            ui->tableWidget_3->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应行高

            ui->tableWidget_3->verticalHeader()->setVisible(true); //设置垂直头不可见
            ui->tableWidget_3->setSelectionMode(QAbstractItemView::ExtendedSelection);  //可多选（Ctrl、Shift、  Ctrl+A都可以）
            ui->tableWidget_3->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选择行为时每次选择一行
            ui->tableWidget_3->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置不可编辑

            ui->tableWidget_3->setStyleSheet("selection-background-color:lightblue;"); //设置选中背景色
            //设置水平、垂直滚动条
            ui->tableWidget_3->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
            ui->tableWidget_3->repaint();

            //此处定义点击表头排序的功能
            QHeaderView *headerGoods = ui->tableWidget_3->horizontalHeader();
            //SortIndicator为水平标题栏文字旁边的三角指示器
            headerGoods->setSortIndicator(0, Qt::AscendingOrder);
            headerGoods->setSortIndicatorShown(true);
            headerGoods->setSectionsClickable(true);
            connect(headerGoods, SIGNAL(sectionClicked(int)), ui->tableWidget_3, SLOT(sortByColumn(int)));

        }
        else QMessageBox::information(NULL, "错误", "在打开如下文件的时候发生错误！无法打开文件"+path);
    }
}

//位置扰动保护
void MainWindow::on_pushButton_14_clicked()
{
    //先判断a的location数据是否合法
    if(a1.trace.size()==0) {QMessageBox::information(NULL, "提示", "轨迹文件不合法，记录的位置戳为空或数目多于1个，不能进行位置扰动保护");return;}
    for(int i=0;i<a1.trace.size();i++)
    {
        for(int j=0;j<a1.trace[i].size();j++)
        {
            if(a1.trace[i][j].location.size()==0||a1.trace[i][j].location.size()>1)
            {
                QMessageBox::information(NULL, "提示", "轨迹文件不合法，记录的位置戳为空或数目多于1个，不能进行位置扰动保护");return;
            }
        }
    }

    //先选择生成的observed文件要存储的位置
    QMessageBox::information(NULL, "提示", "请选择生成的可观察轨迹要存放的文件路径");

    QString path = QFileDialog::getSaveFileName(this, "请选择生成的可观察轨迹要存放的文件路径", "./protect.trace" , tr("trace File (*.trace)"));
    QTextCodec *code = QTextCodec::codecForName("GB2312");
    string tmp = code->fromUnicode(path).data();

    char filepath[300];
    strcpy(filepath, tmp.c_str());

    QMessageBox::information(NULL, "提示", "已记录可观察轨迹文件的路径，下面请选择位置保护密钥文件要存放的文件路径");

    //然后选择Key要存储的路径
    QString path2 = QFileDialog::getSaveFileName(this, "请选择位置保护密钥文件要存放的文件路径", "./Key" , tr(""));
    QTextCodec *code2 = QTextCodec::codecForName("GB2312");
    string tmp2 = code2->fromUnicode(path2).data();

    char filepath2[300];
    strcpy(filepath2, tmp2.c_str());

    //对a进行保护操作
    vector<Key> kk;
    Perturbation(filepath2,a1,kk);

    //把保护好的轨迹存到filepath所指的路径中
    if(!FileWrite(filepath,a1,1)) {QMessageBox::information(NULL, "提示", "文件写入失败");return;}
    else QMessageBox::information(NULL, "提示", "已将可观察轨迹文件protect.trace写入相应目录位置");

    showTableWidget(ui->tableWidget_3,a1);
    //下面是tableWidget的格式调整
    ui->tableWidget_3->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应列宽
    ui->tableWidget_3->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应行高

    ui->tableWidget_3->verticalHeader()->setVisible(true); //设置垂直头不可见
    ui->tableWidget_3->setSelectionMode(QAbstractItemView::ExtendedSelection);  //可多选（Ctrl、Shift、  Ctrl+A都可以）
    ui->tableWidget_3->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选择行为时每次选择一行
    ui->tableWidget_3->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置不可编辑

    ui->tableWidget_3->setStyleSheet("selection-background-color:lightblue;"); //设置选中背景色
    //设置水平、垂直滚动条
    ui->tableWidget_3->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->tableWidget_3->repaint();

    //此处定义点击表头排序的功能
    QHeaderView *headerGoods = ui->tableWidget_3->horizontalHeader();
    //SortIndicator为水平标题栏文字旁边的三角指示器
    headerGoods->setSortIndicator(0, Qt::AscendingOrder);
    headerGoods->setSortIndicatorShown(true);
    headerGoods->setSectionsClickable(true);
    connect(headerGoods, SIGNAL(sectionClicked(int)), ui->tableWidget_3, SLOT(sortByColumn(int)));

}

//假位置注入保护（降低精确度）
void MainWindow::on_pushButton_15_clicked()
{
    //先判断a的location数据是否合法
    if(a1.trace.size()==0) {QMessageBox::information(NULL, "提示", "轨迹文件不合法，记录的位置戳为空或数目多于1个，不能进行位置扰动保护");return;}
    for(int i=0;i<a1.trace.size();i++)
    {
        for(int j=0;j<a1.trace[i].size();j++)
        {
            if(a1.trace[i][j].location.size()==0||a1.trace[i][j].location.size()>1)
            {
                QMessageBox::information(NULL, "提示", "轨迹文件不合法，记录的位置戳为空或数目多于1个，不能进行假位置注入保护");return;
            }
        }
    }

    //先获取降低精确度的等级
    ll Grade;
    Grade = QInputDialog::getInt(this, tr("请输入降低精确度的等级（半径）"),tr("半径:"), 0);

    if(Grade<0) {QMessageBox::information(NULL, "错误", "您输入的等级参数不合法，保护操作终止");return;}

    //然后获取可观察轨迹文件的存储路径
    QMessageBox::information(NULL, "提示", "请选择位置保护生成的可观察轨迹文件要存储的文件路径");

    QString path = QFileDialog::getSaveFileName(this, "请选择存放的文件路径", "./protect.trace" , tr("trace File (*.trace)"));
    QTextCodec *code = QTextCodec::codecForName("GB2312");
    string tmp = code->fromUnicode(path).data();

    char filepath[300];
    strcpy(filepath, tmp.c_str());

    //再选择位置保护生成的文件TruePosition要存储的路径，然后对a进行保护操作
    QMessageBox::information(NULL, "提示", "已记录可观察轨迹文件的路径，下面请选择位置保护密钥文件TruePosition要存放的文件路径");

    //然后选择TruePosition要存储的路径
    QString path2 = QFileDialog::getSaveFileName(this, "请选择位置保护密钥文件TruePosition要存放的文件路径", "./TruePosition" , tr(""));
    QTextCodec *code2 = QTextCodec::codecForName("GB2312");
    string tmp2 = code2->fromUnicode(path2).data();

    char filepath2[300];
    strcpy(filepath2, tmp2.c_str());

    PrecisionReduce(filepath2,a1,ull(Grade));

    //把保护好的轨迹存到filepath所指的路径中
    if(!FileWrite(filepath,a1,1)) {QMessageBox::information(NULL, "提示", "文件写入失败");return;}
    else QMessageBox::information(NULL, "提示", "已将可观察轨迹文件protect.trace写入相应目录位置");

    //将Tracset的内容显示到TableWidge上
    showTableWidget(ui->tableWidget_3,a1);
    //下面是tableWidget的格式调整
    ui->tableWidget_3->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应列宽
    ui->tableWidget_3->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应行高

    ui->tableWidget_3->verticalHeader()->setVisible(true); //设置垂直头不可见
    ui->tableWidget_3->setSelectionMode(QAbstractItemView::ExtendedSelection);  //可多选（Ctrl、Shift、  Ctrl+A都可以）
    ui->tableWidget_3->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选择行为时每次选择一行
    ui->tableWidget_3->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置不可编辑

    ui->tableWidget_3->setStyleSheet("selection-background-color:lightblue;"); //设置选中背景色
    //设置水平、垂直滚动条
    ui->tableWidget_3->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->tableWidget_3->repaint();

    //此处定义点击表头排序的功能
    QHeaderView *headerGoods = ui->tableWidget_3->horizontalHeader();
    //SortIndicator为水平标题栏文字旁边的三角指示器
    headerGoods->setSortIndicator(0, Qt::AscendingOrder);
    headerGoods->setSortIndicatorShown(true);
    headerGoods->setSectionsClickable(true);
    connect(headerGoods, SIGNAL(sectionClicked(int)), ui->tableWidget_3, SLOT(sortByColumn(int)));
}

//位置隐匿保护
void MainWindow::on_pushButton_16_clicked()
{
    //先判断a的location数据是否合法
    if(a1.trace.size()==0) {QMessageBox::information(NULL, "提示", "轨迹文件不合法，记录的位置戳为空或数目多于1个，不能进行位置扰动保护");return;}
    for(int i=0;i<a1.trace.size();i++)
    {
        for(int j=0;j<a1.trace[i].size();j++)
        {
            if(a1.trace[i][j].location.size()==0||a1.trace[i][j].location.size()>1)
            {
                QMessageBox::information(NULL, "提示", "轨迹文件不合法，记录的位置戳为空或数目多于1个，不能进行假位置注入保护");return;
            }
        }
    }

    //然后获取可观察轨迹文件的存储路径
    QMessageBox::information(NULL, "提示", "请选择位置保护生成的可观察轨迹文件要存储的文件路径");

    QString path = QFileDialog::getSaveFileName(this, "请选择存放的文件路径", "./protect.trace" , tr("trace File (*.trace)"));
    QTextCodec *code = QTextCodec::codecForName("GB2312");
    string tmp = code->fromUnicode(path).data();

    char filepath[300];
    strcpy(filepath, tmp.c_str());

    //再选择位置保护生成的文件TruePosition要存储的路径，然后对a进行保护操作
    QMessageBox::information(NULL, "提示", "已记录可观察轨迹文件的路径，下面请选择位置保护密钥文件TruePosition要存放的文件路径");

    //然后选择TruePosition要存储的路径
    QString path2 = QFileDialog::getSaveFileName(this, "请选择位置保护密钥文件TruePosition要存放的文件路径", "./TruePosition" , tr(""));
    QTextCodec *code2 = QTextCodec::codecForName("GB2312");
    string tmp2 = code2->fromUnicode(path2).data();

    char filepath2[300];
    strcpy(filepath2, tmp2.c_str());

    //然后调用保护函数
    PositionHidden(filepath2,a1);

    //把保护好的轨迹存到filepath所指的路径中
    if(!FileWrite(filepath,a1,1)) {QMessageBox::information(NULL, "提示", "文件写入失败");return;}
    else QMessageBox::information(NULL, "提示", "已将可观察轨迹文件protect.trace写入相应目录位置");


    //将Tracset的内容显示到TableWidge上
    showTableWidget(ui->tableWidget_3,a1);
    //下面是tableWidget的格式调整
    ui->tableWidget_3->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应列宽
    ui->tableWidget_3->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应行高

    ui->tableWidget_3->verticalHeader()->setVisible(true); //设置垂直头不可见
    ui->tableWidget_3->setSelectionMode(QAbstractItemView::ExtendedSelection);  //可多选（Ctrl、Shift、  Ctrl+A都可以）
    ui->tableWidget_3->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选择行为时每次选择一行
    ui->tableWidget_3->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置不可编辑

    ui->tableWidget_3->setStyleSheet("selection-background-color:lightblue;"); //设置选中背景色
    //设置水平、垂直滚动条
    ui->tableWidget_3->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->tableWidget_3->repaint();

    //此处定义点击表头排序的功能
    QHeaderView *headerGoods = ui->tableWidget_3->horizontalHeader();
    //SortIndicator为水平标题栏文字旁边的三角指示器
    headerGoods->setSortIndicator(0, Qt::AscendingOrder);
    headerGoods->setSortIndicatorShown(true);
    headerGoods->setSectionsClickable(true);
    connect(headerGoods, SIGNAL(sectionClicked(int)), ui->tableWidget_3, SLOT(sortByColumn(int)));

}

//K匿名保护
void MainWindow::on_pushButton_17_clicked()
{
    //先判断a的location数据是否合法
    if(a1.trace.size()==0) {QMessageBox::information(NULL, "提示", "轨迹文件不合法，记录的位置戳为空或数目多于1个，不能进行位置扰动保护");return;}
    for(int i=0;i<a1.trace.size();i++)
    {
        for(int j=0;j<a1.trace[i].size();j++)
        {
            if(a1.trace[i][j].location.size()==0||a1.trace[i][j].location.size()>1)
            {
                QMessageBox::information(NULL, "提示", "轨迹文件不合法，记录的位置戳为空或数目多于1个，不能进行位置扰动保护");return;
            }
        }
    }

    //接下来获取K匿名的等级
    int Grade;
    Grade = QInputDialog::getInt(this, tr("请输入K匿名的等级"),tr("等级:"), 0);

    if(Grade<a1.trace.size()||Grade>a1.maxUserStamp-a1.minUserStamp+1)
    {
        QMessageBox::information(NULL, "错误", "您输入的K匿名等级不合法，退出保护操作");return;
    }

    //然后选择生成的observed文件要存储的位置
    QMessageBox::information(NULL, "提示", "请选择生成的可观察轨迹要存放的文件路径");

    QString path = QFileDialog::getSaveFileName(this, "请选择生成的可观察轨迹要存放的文件路径", "./protect.trace" , tr("trace File (*.trace)"));
    QTextCodec *code = QTextCodec::codecForName("GB2312");
    string tmp = code->fromUnicode(path).data();

    char filepath[300];
    strcpy(filepath, tmp.c_str());

    QMessageBox::information(NULL, "提示", "已记录可观察轨迹文件的路径，下面请选择位置保护密钥文件要存放的文件路径");

    //然后选择Kmap要存储的路径
    QString path2 = QFileDialog::getSaveFileName(this, "请选择位置保护密钥文件要存放的文件路径", "./Kmap" , tr(""));
    QTextCodec *code2 = QTextCodec::codecForName("GB2312");
    string tmp2 = code2->fromUnicode(path2).data();

    char filepath2[300];
    strcpy(filepath2, tmp2.c_str());

    if(!K_Anonymization(filepath2,a1,Grade)) return;

    //把保护好的轨迹存到filepath所指的路径中
    if(!FileWrite(filepath,a1,1)) {QMessageBox::information(NULL, "提示", "文件写入失败");return;}
    else QMessageBox::information(NULL, "提示", "已将可观察轨迹文件protect.trace写入相应目录位置");

    //将Tracset的内容显示到TableWidge上
    showTableWidget(ui->tableWidget_3,a1);
    //下面是tableWidget的格式调整
    ui->tableWidget_3->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应列宽
    ui->tableWidget_3->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应行高

    ui->tableWidget_3->verticalHeader()->setVisible(true); //设置垂直头不可见
    ui->tableWidget_3->setSelectionMode(QAbstractItemView::ExtendedSelection);  //可多选（Ctrl、Shift、  Ctrl+A都可以）
    ui->tableWidget_3->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选择行为时每次选择一行
    ui->tableWidget_3->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置不可编辑

    ui->tableWidget_3->setStyleSheet("selection-background-color:lightblue;"); //设置选中背景色
    //设置水平、垂直滚动条
    ui->tableWidget_3->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->tableWidget_3->repaint();

    //此处定义点击表头排序的功能
    QHeaderView *headerGoods = ui->tableWidget_3->horizontalHeader();
    //SortIndicator为水平标题栏文字旁边的三角指示器
    headerGoods->setSortIndicator(0, Qt::AscendingOrder);
    headerGoods->setSortIndicatorShown(true);
    headerGoods->setSectionsClickable(true);
    connect(headerGoods, SIGNAL(sectionClicked(int)), ui->tableWidget_3, SLOT(sortByColumn(int)));
}

//********************************************************攻击部分********************************************************

//打开学习轨迹文件
void MainWindow::on_pushButton_clicked()
{
    a41.initial();
    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setWindowTitle("请选择需要打开的.trace文件");
    fileDialog->setDirectory(".");//设置默认路径
    fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog->setViewMode(QFileDialog::Detail);//详细
    fileDialog->setDefaultSuffix("*.trace");

    if (fileDialog->exec() == QDialog::Accepted)
    {
        QString path = fileDialog->selectedFiles()[0];
        QTextCodec *code = QTextCodec::codecForName("GB2312");
        string tmp = code->fromUnicode(path).data();
        if(tmp.substr(tmp.size() - 6)!=".trace")
        {
            QMessageBox::information(NULL, "提示", "你选择的文件不是.trace文件，请重新选择");return;
        }
        char filepath[300];
        strcpy(filepath, tmp.c_str());
        ui->lineEdit->setText(path);

        if(!FileRead(filepath,a41,0)) {QMessageBox::information(NULL, "提示", "打开文件失败，请选择正确的文件");}
        else
        {
            showTableWidget(ui->tableWidget_4,a41);
            //下面是tableWidget的格式调整
            ui->tableWidget_4->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应列宽
            ui->tableWidget_4->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应行高

            ui->tableWidget_4->verticalHeader()->setVisible(true); //设置垂直头不可见
            ui->tableWidget_4->setSelectionMode(QAbstractItemView::ExtendedSelection);  //可多选（Ctrl、Shift、  Ctrl+A都可以）
            ui->tableWidget_4->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选择行为时每次选择一行
            ui->tableWidget_4->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置不可编辑

            ui->tableWidget_4->setStyleSheet("selection-background-color:lightblue;"); //设置选中背景色
            //设置水平、垂直滚动条
            ui->tableWidget_4->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
            ui->tableWidget_4->repaint();

            //此处定义点击表头排序的功能
            QHeaderView *headerGoods = ui->tableWidget_4->horizontalHeader();
            //SortIndicator为水平标题栏文字旁边的三角指示器
            headerGoods->setSortIndicator(0, Qt::AscendingOrder);
            headerGoods->setSortIndicatorShown(true);
            headerGoods->setSectionsClickable(true);
            connect(headerGoods, SIGNAL(sectionClicked(int)), ui->tableWidget_4, SLOT(sortByColumn(int)));
        }
    }
}

//开始知识构建
void MainWindow::on_pushButton_3_clicked()
{
    //先检测数据合法性
    if(a41.trace.size()==0) {QMessageBox::information(NULL, "提示", "轨迹文件不合法，记录的位置戳为空或数目多于1个，不能进行位置扰动保护");return;}
    for(int i=0;i<a41.trace.size();i++)
    {
        for(int j=0;j<a41.trace[i].size();j++)
        {
            if(a41.trace[i][j].location.size()==0||a41.trace[i][j].location.size()>1)
            {
                QMessageBox::information(NULL, "提示", "轨迹文件不合法，记录的位置戳为空或数目多于1个，不能进行位置扰动保护");return;
            }
        }
    }

    //先选择生成的Knowledge文件以及PointProbability文件要存储的位置
    QMessageBox::information(NULL, "提示", "请选择生成Knowledge文件要存放的文件路径");

    QString path = QFileDialog::getSaveFileName(this, "请选择生成Knowledge文件要存放的文件路径", "./Knowledge" , tr("Knowledge File (Knowledge)"));
    QTextCodec *code = QTextCodec::codecForName("GB2312");
    string tmp = code->fromUnicode(path).data();

    char filepath[300];
    strcpy(filepath, tmp.c_str());
    vector<vector<double>> temp;

    QMessageBox::information(NULL, "提示", "请选择生成点概率向量文件PointProbability的存放路径");

    QString path2 = QFileDialog::getSaveFileName(this, "请选择生成点概率向量文件PointProbability的存放路径", "./PointProbability" , tr("PointPro File (PointPro)"));
    QTextCodec *code2 = QTextCodec::codecForName("GB2312");
    string tmp2 = code2->fromUnicode(path2).data();

    char filepath2[300];
    strcpy(filepath2, tmp2.c_str());
    vector<double> temp2;

    //知识构建函数
    glzyjz(a41,temp);
    diangailv(a41,temp2);

    //创建Knowledge文件并写入到用户指定的位置
    if(!KnowledgeWrite(filepath,temp)) {QMessageBox::information(NULL, "错误", "写入Knowledge文件失败，知识构建操作终止");return;}
    if(!PointProbabilityWrite(filepath2,temp2)) {QMessageBox::information(NULL, "错误", "写入PointProbability文件失败，知识构建操作终止");return;}
}

//打开暴露轨迹文件
void MainWindow::on_pushButton_2_clicked()
{
    a42.initial();
    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setWindowTitle("请选择需要打开的.trace文件");
    fileDialog->setDirectory(".");//设置默认路径
    fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog->setViewMode(QFileDialog::Detail);//详细
    fileDialog->setDefaultSuffix("*.trace");

    if (fileDialog->exec() == QDialog::Accepted)
    {
        QString path = fileDialog->selectedFiles()[0];
        QTextCodec *code = QTextCodec::codecForName("GB2312");
        string tmp = code->fromUnicode(path).data();
        if(tmp.substr(tmp.size() - 6)!=".trace")
        {
            QMessageBox::information(NULL, "提示", "你选择的文件不是.trace文件，请重新选择");return;
        }
        char filepath[300];
        strcpy(filepath, tmp.c_str());
        ui->lineEdit->setText(path);

        if(!FileRead(filepath,a42,1)) {QMessageBox::information(NULL, "提示", "打开文件失败，请选择正确的文件");}
        else
        {
            showTableWidget(ui->tableWidget_5,a42);
            //下面是tableWidget的格式调整
            ui->tableWidget_5->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应列宽
            ui->tableWidget_5->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应行高

            ui->tableWidget_5->verticalHeader()->setVisible(true); //设置垂直头不可见
            ui->tableWidget_5->setSelectionMode(QAbstractItemView::ExtendedSelection);  //可多选（Ctrl、Shift、  Ctrl+A都可以）
            ui->tableWidget_5->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选择行为时每次选择一行
            ui->tableWidget_5->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置不可编辑

            ui->tableWidget_5->setStyleSheet("selection-background-color:lightblue;"); //设置选中背景色
            //设置水平、垂直滚动条
            ui->tableWidget_5->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
            ui->tableWidget_5->repaint();

            //此处定义点击表头排序的功能
            QHeaderView *headerGoods = ui->tableWidget_5->horizontalHeader();
            //SortIndicator为水平标题栏文字旁边的三角指示器
            headerGoods->setSortIndicator(0, Qt::AscendingOrder);
            headerGoods->setSortIndicatorShown(true);
            headerGoods->setSectionsClickable(true);
            connect(headerGoods, SIGNAL(sectionClicked(int)), ui->tableWidget_5, SLOT(sortByColumn(int)));
        }
    }
}

//开始攻击
void MainWindow::on_pushButton_4_clicked()
{
    //先检测数据合法性
    if(a42.trace.size()==0) {QMessageBox::information(NULL, "提示", "轨迹文件不合法，记录的位置戳为空或数目多于1个，不能进行位置扰动保护");return;}
    for(int i=0;i<a42.trace.size();i++)
    {
        for(int j=0;j<a42.trace[i].size();j++)
        {
            if(a42.trace[i][j].location.size()==0||a42.trace[i][j].location.size()>1)
            {
                QMessageBox::information(NULL, "提示", "轨迹文件不合法，记录的位置戳为空或数目多于1个，不能进行位置扰动保护");return;
            }
        }
    }

    //将location为空的记录的isExposed改为0
    for(int i=0;i<a42.trace.size();i++)
    {
        for(int j=0;j<a42.trace[i].size();j++)
        {
            if(a42.trace[i][j].location.size()==0) a42.trace[i][j].isExposed=0;
        }
    }

    //然后让用户选择要读取的Knowledge文件和PointProbability
    QMessageBox::information(NULL, "提示", "请选择要读取的Knowledge文件");
    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setWindowTitle("请选择需要打开的Knowledge文件");
    fileDialog->setDirectory(".");//设置默认路径
    fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog->setViewMode(QFileDialog::Detail);//详细

    if (fileDialog->exec() == QDialog::Accepted)
    {
        QString path = fileDialog->selectedFiles()[0];
        QTextCodec *code = QTextCodec::codecForName("GB2312");
        string tmp = code->fromUnicode(path).data();

        char filepath[300];
        strcpy(filepath, tmp.c_str());

        vector<vector<double>> know;
        if(!KnowledgeRead(filepath,know)) {QMessageBox::information(NULL, "提示", "Knowledge文件读取失败，攻击操作终止");return;}
        else
        {

            QMessageBox::information(NULL, "提示", "请选择要读取的PointProbability文件");
            QFileDialog *fileDialog2 = new QFileDialog(this);
            fileDialog2->setWindowTitle("请选择需要打开的PointProbability文件");
            fileDialog2->setDirectory(".");//设置默认路径
            fileDialog2->setAcceptMode(QFileDialog::AcceptOpen);
            fileDialog2->setViewMode(QFileDialog::Detail);//详细

            if (fileDialog2->exec() == QDialog::Accepted)
            {
                QString path2 = fileDialog2->selectedFiles()[0];
                QTextCodec *code2 = QTextCodec::codecForName("GB2312");
                string tmp2 = code2->fromUnicode(path2).data();

                char filepath2[300];
                strcpy(filepath2, tmp2.c_str());

                vector<double> poipro;
                if(!PointProbabilityRead(filepath2,poipro)) {QMessageBox::information(NULL, "提示", "PointProbability文件读取失败，攻击操作终止");return;}
                else
                {

                    //让用户选择攻击轨迹的要存放的地方
                    QMessageBox::information(NULL, "提示", "已记录Knowledge和PointProbability文件的读取位置，下面请选择攻击生成的轨迹attack.trace要生成的位置");
                    QString path3 = QFileDialog::getSaveFileName(this, "请选择生成的可观察轨迹要存放的文件路径", "./attack.trace" , tr("trace File (*.trace)"));
                    QTextCodec *code3 = QTextCodec::codecForName("GB2312");
                    string tmp3 = code3->fromUnicode(path3).data();

                    char filepath3[300];
                    strcpy(filepath3, tmp3.c_str());

                    //开始攻击操作，可能要用a42来存新数据
                    //lujing(a42,know,poipro);

                    QMessageBox::information(NULL, "提示", "已经成功攻击");

                    //攻击完成以后将数据存到用户的指定位置
                    if(!FileWrite(filepath3,a42,1)) {QMessageBox::information(NULL, "提示", "attack.trace文件写入失败，攻击操作终止");return;}

                    showTableWidget(ui->tableWidget_5,a42);
                    //下面是tableWidget的格式调整
                    ui->tableWidget_5->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应列宽
                    ui->tableWidget_5->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应行高

                    ui->tableWidget_5->verticalHeader()->setVisible(true); //设置垂直头不可见
                    ui->tableWidget_5->setSelectionMode(QAbstractItemView::ExtendedSelection);  //可多选（Ctrl、Shift、  Ctrl+A都可以）
                    ui->tableWidget_5->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选择行为时每次选择一行
                    ui->tableWidget_5->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置不可编辑

                    ui->tableWidget_5->setStyleSheet("selection-background-color:lightblue;"); //设置选中背景色
                    //设置水平、垂直滚动条
                    ui->tableWidget_5->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
                    ui->tableWidget_5->repaint();

                    //此处定义点击表头排序的功能
                    QHeaderView *headerGoods = ui->tableWidget_5->horizontalHeader();
                    //SortIndicator为水平标题栏文字旁边的三角指示器
                    headerGoods->setSortIndicator(0, Qt::AscendingOrder);
                    headerGoods->setSortIndicatorShown(true);
                    headerGoods->setSectionsClickable(true);
                    connect(headerGoods, SIGNAL(sectionClicked(int)), ui->tableWidget_5, SLOT(sortByColumn(int)));
                }

            }

        }
    }

}

//********************************************************度量部分********************************************************

//打开实际轨迹文件
void MainWindow::on_pushButton_18_clicked()
{
    a21.initial();
    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setWindowTitle("请选择需要打开的.trace文件");
    fileDialog->setDirectory(".");//设置默认路径
    fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog->setViewMode(QFileDialog::Detail);//详细
    fileDialog->setDefaultSuffix("*.trace");

    if (fileDialog->exec() == QDialog::Accepted)
    {
        QString path = fileDialog->selectedFiles()[0];
        QTextCodec *code = QTextCodec::codecForName("GB2312");
        string tmp = code->fromUnicode(path).data();
        if(tmp.substr(tmp.size() - 6)!=".trace")
        {
            QMessageBox::information(NULL, "提示", "你选择的文件不是.trace文件，请重新选择");
            return;
        }
        char filepath[300];
        strcpy(filepath, tmp.c_str());
        ui->lineEdit_7->setText(path);

        if(!FileRead(filepath,a21,0)) {QMessageBox::information(NULL, "提示", "打开文件失败，请选择正确的文件");}
    }

    //检查是否两个轨迹文件都读出来了，如果都读出来了那么给攻击用户选择下拉框填充用户号
    if(a21.trace.size()!=0&&a22.trace.size()!=0)
    {
        ui->comboBox_4->clear();
        for(int i=0;i<a21.trace.size();i++)
        {
            for(int j=0;j<a22.trace.size();j++)
            {
                if(a21.trace[i][0].user==a22.trace[j][0].user)
                {
                    ui->comboBox_4->addItem(QString::number(a21.trace[i][0].user));
                    break;
                }
            }
        }
    }
}

//打开攻击轨迹文件
void MainWindow::on_pushButton_24_clicked()
{
    a22.initial();
    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setWindowTitle("请选择需要打开的.trace文件");
    fileDialog->setDirectory(".");//设置默认路径
    fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog->setViewMode(QFileDialog::Detail);//详细
    fileDialog->setDefaultSuffix("*.trace");

    if (fileDialog->exec() == QDialog::Accepted)
    {
        QString path = fileDialog->selectedFiles()[0];
        QTextCodec *code = QTextCodec::codecForName("GB2312");
        string tmp = code->fromUnicode(path).data();
        if(tmp.substr(tmp.size() - 6)!=".trace")
        {
            QMessageBox::information(NULL, "提示", "你选择的文件不是.trace文件，请重新选择");
            return;
        }
        char filepath[300];
        strcpy(filepath, tmp.c_str());
        ui->lineEdit_8->setText(path);

        if(!FileRead(filepath,a22,1)) {QMessageBox::information(NULL, "提示", "打开文件失败，请选择正确的文件");}
    }

    //检查是否两个轨迹文件都读出来了，如果都读出来了那么给攻击用户选择下拉框填充用户号
    if(a21.trace.size()!=0&&a22.trace.size()!=0)
    {
        ui->comboBox_4->clear();
        for(int i=0;i<a21.trace.size();i++)
        {
            for(int j=0;j<a22.trace.size();j++)
            {
                if(a21.trace[i][0].user==a22.trace[j][0].user)
                {
                    ui->comboBox_4->addItem(QString::number(a21.trace[i][0].user));
                    break;
                }
            }
        }
    }
}

//开始度量
void MainWindow::on_pushButton_21_clicked()
{
    //先判断度量类型
    if(QString::compare(ui->comboBox_2->currentText(),QString("基于轨迹失真的度量"))==0)
    {
        //先获取度量的起始时间、终止时间以及攻击方误差允许范围
        QDialog dialog(this);
        QFormLayout form(&dialog);
        form.addRow(new QLabel("请输入需要度量的起始时间、终止时间和攻击允许误差："));
        //起始时间
        QString timeBegin = QString("起始时间(范围在");
        timeBegin+=QString::number(a21.minTimeStamp);
        timeBegin+="~";timeBegin+=QString::number(a21.maxTimeStamp);
        timeBegin+="之间):";
        QLineEdit *lineedit1 = new QLineEdit(&dialog);
        form.addRow(timeBegin, lineedit1);
        //终止时间
        QString timeEnd = QString("终止时间(范围在");
        timeEnd+=QString::number(a21.minTimeStamp);
        timeEnd+="~";timeEnd+=QString::number(a21.maxTimeStamp);
        timeEnd+="之间):";
        QLineEdit *lineedit2 = new QLineEdit(&dialog);
        form.addRow(timeEnd, lineedit2);
        //攻击允许误差
        QString deviation = QString("攻击允许误差值: ");
        QLineEdit *lineedit3 = new QLineEdit(&dialog);
        form.addRow(deviation, lineedit3);
        // Add Cancel and OK button
        QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
            Qt::Horizontal, &dialog);
        form.addRow(&buttonBox);
        QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
        QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

        ll begintime;
        ll endtime;
        ll deviationn;
        // Process when OK button is clicked
        if (dialog.exec() == QDialog::Accepted)
        {
            //判断三个输入框的内容是否合法
            if(!lineedit1->text().isEmpty()&&!lineedit2->text().isEmpty()&&!lineedit3->text().isEmpty())
            {
                //如果不是empty那么要检查参数值是否合理
                begintime=lineedit1->text().toLongLong();
                endtime=lineedit2->text().toLongLong();
                deviationn=lineedit3->text().toLongLong();
                if(begintime<a21.minTimeStamp||begintime>a21.maxTimeStamp)
                {QMessageBox::information(NULL, "提示", "起始时间值不合法，度量操作终止");return;}
                if(endtime<a21.minTimeStamp||endtime>a21.maxTimeStamp)
                {QMessageBox::information(NULL, "提示", "终止时间值不合法，度量操作终止");return;}
                if(begintime>=endtime)
                {QMessageBox::information(NULL, "提示", "终止时间值小于或等于起始时间值，不合法，度量操作终止");return;}
                if(deviationn<0) {QMessageBox::information(NULL, "提示", "攻击允许误差值不合法，度量操作终止");return;}

                //获取到要度量的用户值
                ull user=ui->comboBox_4->currentText().toULongLong();
                for(int i=0;i<a21.trace.size();i++)
                {
                    if(a21.trace[i][0].user==user)
                    {
                        user1=a21.trace[i];
                    }
                }
                for(int i=0;i<a22.trace.size();i++)
                {
                    if(a22.trace[i][0].user==user)
                    {
                        user2=a22.trace[i];
                    }
                }

                //然后我们就可以开始度量了
                TrackingEvalutate t;
                MetricForTracking(a21,a22,ull(begintime),ull(endtime),user,ull(deviationn),t);

                QString coinci=QString::number(t.coincidence*100);
                QString averdis=QString::number(t.AverageDistance);

                ui->label_13->setText("轨迹吻合度:"+coinci+"%");
                ui->label_15->setText("平均位置距离:"+averdis);

                //下面开始作图
                paintFlag=1;
                paint->repaint();
            }
            else { QMessageBox::information(NULL, "提示", "三个参数均不能为空，度量操作终止");return;}

        }
        else return;
    }
    else if(QString::compare(ui->comboBox_2->currentText(),QString("基于熵的度量"))==0)
    {
        //先读入Knowledge文件
        QFileDialog *fileDialog = new QFileDialog(this);
        fileDialog->setWindowTitle("请选择需要打开的攻击方先验知识文件");
        fileDialog->setDirectory(".");//设置默认路径
        fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
        fileDialog->setViewMode(QFileDialog::Detail);//详细

        if (fileDialog->exec() == QDialog::Accepted)
        {
            QString path = fileDialog->selectedFiles()[0];
            QTextCodec *code = QTextCodec::codecForName("GB2312");
            string tmp = code->fromUnicode(path).data();

            char filepath[300];
            strcpy(filepath, tmp.c_str());

            if(!KnowledgeRead(filepath,tsmetric)) {QMessageBox::information(NULL, "提示", "Knowledge文件读取有误，度量操作停止");return;}
            else
            {
                //都搞定以后开始度量
                double result;
                MetricByEntropy(tsmetric,result);

                //度量后将结果写入到result文件里面
                ui->label_17->setText("轨迹平均熵(范围为0~1之间，熵越高，隐私水平越高)："+QString::number(result));
            }
        }
    }
}
//********************************************************还原轨迹部分********************************************************
//打开被保护的轨迹文件
void MainWindow::on_pushButton_19_clicked()
{
    a3.initial();
    ui->tableWidget_2->clear();
    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setWindowTitle("请选择需要打开的.trace文件");
    fileDialog->setDirectory(".");//设置默认路径
    fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog->setViewMode(QFileDialog::Detail);//详细
    fileDialog->setDefaultSuffix("*.trace");

    if (fileDialog->exec() == QDialog::Accepted)
    {
        QString path = fileDialog->selectedFiles()[0];
        QTextCodec *code = QTextCodec::codecForName("GB2312");
        string tmp = code->fromUnicode(path).data();
        if(tmp.substr(tmp.size() - 6)!=".trace")
        {
            QMessageBox::information(NULL, "提示", "你选择的文件不是.trace文件，请重新选择");
            return;
        }
        char filepath[300];
        strcpy(filepath, tmp.c_str());
        ui->lineEdit_10->setText(path);

        if(FileRead(filepath,a3,1))
        {
            //将Tracset的内容显示到TableWidge上
            showTableWidget(ui->tableWidget_2,a3);
            //下面是tableWidget的格式调整
            ui->tableWidget_2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应列宽
            ui->tableWidget_2->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应行高

            ui->tableWidget_2->verticalHeader()->setVisible(true); //设置垂直头不可见
            ui->tableWidget_2->setSelectionMode(QAbstractItemView::ExtendedSelection);  //可多选（Ctrl、Shift、  Ctrl+A都可以）
            ui->tableWidget_2->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选择行为时每次选择一行
            ui->tableWidget_2->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置不可编辑

            ui->tableWidget_2->setStyleSheet("selection-background-color:lightblue;"); //设置选中背景色
            //ui->tableWidget_2->horizontalHeader()->setStyleSheet("QHeaderView::section{background:skyblue;}"); //设置表头背景色
            //设置水平、垂直滚动条
            ui->tableWidget_2->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
            ui->tableWidget_2->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
            ui->tableWidget_2->repaint();

            //此处定义点击表头排序的功能
            QHeaderView *headerGoods = ui->tableWidget_2->horizontalHeader();
            //SortIndicator为水平标题栏文字旁边的三角指示器
            headerGoods->setSortIndicator(0, Qt::AscendingOrder);
            headerGoods->setSortIndicatorShown(true);
            headerGoods->setSectionsClickable(true);
            connect(headerGoods, SIGNAL(sectionClicked(int)), ui->tableWidget_2, SLOT(sortByColumn(int)));

        }
        else QMessageBox::information(NULL, "错误", "在打开如下文件的时候发生错误！无法打开文件"+path);
    }
}

//打开位置保护密钥文件
void MainWindow::on_pushButton_25_clicked()
{
    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setWindowTitle("请选择需要打开的.trace文件");
    fileDialog->setDirectory(".");//设置默认路径
    fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog->setViewMode(QFileDialog::Detail);//详细

    if (fileDialog->exec() == QDialog::Accepted)
    {
        QString path = fileDialog->selectedFiles()[0];
        QTextCodec *code = QTextCodec::codecForName("GB2312");
        string tmp = code->fromUnicode(path).data();

        char filepath[300];
        strcpy(filepath, tmp.c_str());
        ui->lineEdit_9->setText(path);
        //对不同类型的文件有着不同的处理方式
        if(QString::compare(ui->comboBox_3->currentText(),QString("Key"))==0)
        {
            key.clear();
            if(!KeyRead(filepath,key)) { QMessageBox::information(NULL, "错误", "key文件读取有误，读取操作终止");return;}
            else QMessageBox::information(NULL, "提示", "读取key文件成功");
        }
        else if(QString::compare(ui->comboBox_3->currentText(),QString("TruePosition"))==0)
        {
            tp.clear();
            if(!TruePosiRead(filepath,tp)) { QMessageBox::information(NULL, "错误", "TruePosition文件读取有误，读取操作终止");return;}
            else QMessageBox::information(NULL, "提示", "读取TruePosition文件成功");
        }
        else
        {
            kv_pair.clear();
            if(!KSetRead(filepath,kv_pair)) { QMessageBox::information(NULL, "错误", "Kmap文件读取有误，读取操作终止");return;}
            else QMessageBox::information(NULL, "提示", "读取Kmap文件成功");
        }
    }
}

//还原位置扰动保护，会用到a3和key
void MainWindow::on_pushButton_22_clicked()
{
    if(a3.trace.size()==0) {QMessageBox::information(NULL, "错误", "程序尚未读入被保护的轨迹文件，还原操作终止");return;}
    if(key.size()==0) {QMessageBox::information(NULL, "错误", "程序尚未读入密钥文件key，还原操作终止");return;}

    //先进行轨迹还原
    AntiPerturbation(a3,key);

    //然后让用户选择生成出来的还原轨迹要存到哪里
    QMessageBox::information(NULL, "提示", "请选择还原后轨迹的存放路径");
    QString path = QFileDialog::getSaveFileName(this, "请选择还原后的轨迹的存储路径", "./restore.trace" , tr("trace File (*.trace)"));
    QTextCodec *code = QTextCodec::codecForName("GB2312");
    string tmp = code->fromUnicode(path).data();

    char filepath[300];
    strcpy(filepath, tmp.c_str());

    //写入文件
    if(!FileWrite(filepath,a3,1)) {QMessageBox::information(NULL, "提示", "存放路径失败，请检查路径选择是否正确");return;}
    else {QMessageBox::information(NULL, "提示", "保存文件成功");}

    //再将内容显示到表格中
    showTableWidget(ui->tableWidget_2,a3);
    //下面是tableWidget的格式调整
    ui->tableWidget_2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应列宽
    ui->tableWidget_2->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应行高

    ui->tableWidget_2->verticalHeader()->setVisible(true); //设置垂直头不可见
    ui->tableWidget_2->setSelectionMode(QAbstractItemView::ExtendedSelection);  //可多选（Ctrl、Shift、  Ctrl+A都可以）
    ui->tableWidget_2->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选择行为时每次选择一行
    ui->tableWidget_2->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置不可编辑

    ui->tableWidget_2->setStyleSheet("selection-background-color:lightblue;"); //设置选中背景色
    //ui->tableWidget_2->horizontalHeader()->setStyleSheet("QHeaderView::section{background:skyblue;}"); //设置表头背景色
    //设置水平、垂直滚动条
    ui->tableWidget_2->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->tableWidget_2->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->tableWidget_2->repaint();

    //此处定义点击表头排序的功能
    QHeaderView *headerGoods = ui->tableWidget_2->horizontalHeader();
    //SortIndicator为水平标题栏文字旁边的三角指示器
    headerGoods->setSortIndicator(0, Qt::AscendingOrder);
    headerGoods->setSortIndicatorShown(true);
    headerGoods->setSectionsClickable(true);
    connect(headerGoods, SIGNAL(sectionClicked(int)), ui->tableWidget_2, SLOT(sortByColumn(int)));
}

//还原假位置注入(降低精确度)保护
void MainWindow::on_pushButton_23_clicked()
{
    if(a3.trace.size()==0) {QMessageBox::information(NULL, "错误", "程序尚未读入被保护的轨迹文件，还原操作终止");return;}
    if(tp.size()==0) {QMessageBox::information(NULL, "错误", "程序尚未读入密钥文件TruePosition，还原操作终止");return;}

    //先进行轨迹还原
    AntiPrecisionReduce(a3,tp);

    //然后让用户选择生成出来的还原轨迹要存到哪里
    QMessageBox::information(NULL, "提示", "请选择还原后轨迹的存放路径");
    QString path = QFileDialog::getSaveFileName(this, "请选择还原后的轨迹的存储路径", "./restore.trace" , tr("trace File (*.trace)"));
    QTextCodec *code = QTextCodec::codecForName("GB2312");
    string tmp = code->fromUnicode(path).data();

    char filepath[300];
    strcpy(filepath, tmp.c_str());

    //写入文件
    if(!FileWrite(filepath,a3,1)) {QMessageBox::information(NULL, "提示", "存放路径失败，请检查路径选择是否正确");return;}
    else {QMessageBox::information(NULL, "提示", "保存文件成功");}

    //再将内容显示到表格中
    showTableWidget(ui->tableWidget_2,a3);
    //下面是tableWidget的格式调整
    ui->tableWidget_2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应列宽
    ui->tableWidget_2->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应行高

    ui->tableWidget_2->verticalHeader()->setVisible(true); //设置垂直头不可见
    ui->tableWidget_2->setSelectionMode(QAbstractItemView::ExtendedSelection);  //可多选（Ctrl、Shift、  Ctrl+A都可以）
    ui->tableWidget_2->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选择行为时每次选择一行
    ui->tableWidget_2->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置不可编辑

    ui->tableWidget_2->setStyleSheet("selection-background-color:lightblue;"); //设置选中背景色
    //ui->tableWidget_2->horizontalHeader()->setStyleSheet("QHeaderView::section{background:skyblue;}"); //设置表头背景色
    //设置水平、垂直滚动条
    ui->tableWidget_2->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->tableWidget_2->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->tableWidget_2->repaint();

    //此处定义点击表头排序的功能
    QHeaderView *headerGoods = ui->tableWidget_2->horizontalHeader();
    //SortIndicator为水平标题栏文字旁边的三角指示器
    headerGoods->setSortIndicator(0, Qt::AscendingOrder);
    headerGoods->setSortIndicatorShown(true);
    headerGoods->setSectionsClickable(true);
    connect(headerGoods, SIGNAL(sectionClicked(int)), ui->tableWidget_2, SLOT(sortByColumn(int)));
}

//还原位置隐匿保护
void MainWindow::on_pushButton_26_clicked()
{
    if(a3.trace.size()==0) {QMessageBox::information(NULL, "错误", "程序尚未读入被保护的轨迹文件，还原操作终止");return;}
    if(tp.size()==0) {QMessageBox::information(NULL, "错误", "程序尚未读入密钥文件TruePosition，还原操作终止");return;}

    //先进行轨迹还原
    AntiPositionHidden(a3,tp);

    //然后让用户选择生成出来的还原轨迹要存到哪里
    QMessageBox::information(NULL, "提示", "请选择还原后轨迹的存放路径");
    QString path = QFileDialog::getSaveFileName(this, "请选择还原后的轨迹的存储路径", "./restore.trace" , tr("trace File (*.trace)"));
    QTextCodec *code = QTextCodec::codecForName("GB2312");
    string tmp = code->fromUnicode(path).data();

    char filepath[300];
    strcpy(filepath, tmp.c_str());

    //写入文件
    if(!FileWrite(filepath,a3,1)) {QMessageBox::information(NULL, "提示", "存放路径失败，请检查路径选择是否正确");return;}
    else {QMessageBox::information(NULL, "提示", "保存文件成功");}

    //再将内容显示到表格中
    showTableWidget(ui->tableWidget_2,a3);
    //下面是tableWidget的格式调整
    ui->tableWidget_2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应列宽
    ui->tableWidget_2->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应行高

    ui->tableWidget_2->verticalHeader()->setVisible(true); //设置垂直头不可见
    ui->tableWidget_2->setSelectionMode(QAbstractItemView::ExtendedSelection);  //可多选（Ctrl、Shift、  Ctrl+A都可以）
    ui->tableWidget_2->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选择行为时每次选择一行
    ui->tableWidget_2->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置不可编辑

    ui->tableWidget_2->setStyleSheet("selection-background-color:lightblue;"); //设置选中背景色
    //ui->tableWidget_2->horizontalHeader()->setStyleSheet("QHeaderView::section{background:skyblue;}"); //设置表头背景色
    //设置水平、垂直滚动条
    ui->tableWidget_2->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->tableWidget_2->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->tableWidget_2->repaint();

    //此处定义点击表头排序的功能
    QHeaderView *headerGoods = ui->tableWidget_2->horizontalHeader();
    //SortIndicator为水平标题栏文字旁边的三角指示器
    headerGoods->setSortIndicator(0, Qt::AscendingOrder);
    headerGoods->setSortIndicatorShown(true);
    headerGoods->setSectionsClickable(true);
    connect(headerGoods, SIGNAL(sectionClicked(int)), ui->tableWidget_2, SLOT(sortByColumn(int)));
}


void MainWindow::on_pushButton_27_clicked()
{
    if(a3.trace.size()==0) {QMessageBox::information(NULL, "错误", "程序尚未读入被保护的轨迹文件，还原操作终止");return;}
    if(kv_pair.size()==0) {QMessageBox::information(NULL, "错误", "程序尚未读入密钥文件kv_pair，还原操作终止");return;}

    TraceSet restore;
    //先进行轨迹还原
    AntiK_Anonymization(restore,a3,kv_pair);

    //然后让用户选择生成出来的还原轨迹要存到哪里
    QMessageBox::information(NULL, "提示", "请选择还原后轨迹的存放路径");
    QString path = QFileDialog::getSaveFileName(this, "请选择还原后的轨迹的存储路径", "./restore.trace" , tr("trace File (*.trace)"));
    QTextCodec *code = QTextCodec::codecForName("GB2312");
    string tmp = code->fromUnicode(path).data();

    char filepath[300];
    strcpy(filepath, tmp.c_str());

    //写入文件
    if(!FileWrite(filepath,restore,1)) {QMessageBox::information(NULL, "提示", "存放路径失败，请检查路径选择是否正确");return;}
    else {QMessageBox::information(NULL, "提示", "保存文件成功");}

    //再将内容显示到表格中
    showTableWidget(ui->tableWidget_2,restore);
    //下面是tableWidget的格式调整
    ui->tableWidget_2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应列宽
    ui->tableWidget_2->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应行高

    ui->tableWidget_2->verticalHeader()->setVisible(true); //设置垂直头不可见
    ui->tableWidget_2->setSelectionMode(QAbstractItemView::ExtendedSelection);  //可多选（Ctrl、Shift、  Ctrl+A都可以）
    ui->tableWidget_2->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选择行为时每次选择一行
    ui->tableWidget_2->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置不可编辑

    ui->tableWidget_2->setStyleSheet("selection-background-color:lightblue;"); //设置选中背景色
    //ui->tableWidget_2->horizontalHeader()->setStyleSheet("QHeaderView::section{background:skyblue;}"); //设置表头背景色
    //设置水平、垂直滚动条
    ui->tableWidget_2->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->tableWidget_2->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->tableWidget_2->repaint();

    //此处定义点击表头排序的功能
    QHeaderView *headerGoods = ui->tableWidget_2->horizontalHeader();
    //SortIndicator为水平标题栏文字旁边的三角指示器
    headerGoods->setSortIndicator(0, Qt::AscendingOrder);
    headerGoods->setSortIndicatorShown(true);
    headerGoods->setSectionsClickable(true);
    connect(headerGoods, SIGNAL(sectionClicked(int)), ui->tableWidget_2, SLOT(sortByColumn(int)));
}































