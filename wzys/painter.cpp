#include "painter.h"
#include "ui_painter.h"

painter::painter(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::painter)
{
    ui->setupUi(this);
}

painter::~painter()
{
    delete ui;
}

void painter::paintEvent(QPaintEvent *e)
{
    //因为paintEvent是不断执行的一个函数，所以我们要限制绘图条件
    if(a21.trace.size()==0||a22.trace.size()==0||paintFlag==0) {return;}

    QPixmap pixmap(size()); //相当于小汽车，将所画的东西打包塞进小汽车中
    //paintEvent需要画笔工具QPainter
    QPainter p(&pixmap);    //画在小汽车里边，能有效提高效率
    pixmap.fill(Qt::white); //将背景设置为白色

    /*QTransform transform;
    //transform可以代替p的translate，因为他会进行一系列复杂的运算，平移、旋转、缩放等等
    //虽然p也有scale，rotate，translate等函数
    transform.rotate(10);
    transform.scale(0.8, 0.8);  //缩放功能
    //将以下所有的图像旋转10度,该语句后面的所画的图像都要旋转
    p.setTransform(transform);
    //将下方画的所有图形移动相对应的位置,整体图像平移，只影响这句话之后的语句，知道遇到下一个translate语句
    p.translate(100, 100);*/

    //消锯齿，让画出的图形更加美观
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    //设置画笔属性
    p.setPen(QPen(Qt::black, 2, Qt::SolidLine));
    //设置封闭图像的填充颜色,从BrushStyle文件中找，要学会查询函数的使用准则
    //p.setBrush(Qt::CrossPattern);
    //设置字体格式,宋体， 40个像素， 加粗值为100， 斜体
    //p.setFont(QFont("宋体", 40, 100, true));

    /*QTransform transform2;
    transform.scale(0.5, 0.5);
    //这里的false代表是和前面的transform相互独立，效果不叠加
    p.setTransform(transform2, false);*/
    //画字
    //p.drawText(QPoint(200, 100), "Hello, China!");

    //根据实际的行数和列数确定行间隔、列间隔
    int RowInterval=469/a21.areaRow;//y
    int ColInterval=629/a21.areaCol;//x

    //画区域格子
    for(int i=0;i<=a21.areaRow;i++)
    {
        p.drawLine(QPoint(0, i*RowInterval), QPoint(629, i*RowInterval));
    }
    for(int i=0;i<=a21.areaCol;i++)
    {
        p.drawLine(QPoint(i*ColInterval, 0), QPoint(i*ColInterval, 469));
    }

    //将原点设在左下角
    p.translate(0,469);
    //画实际轨迹
    p.setPen(QPen(Qt::blue, 2, Qt::SolidLine));
    for(int i=0;i<user1.size()-1;i++)
    {
        Coordinate start,end;
        PosiNum_to_Coordinate(a21.areaCol,user1[i].location[0],start);
        PosiNum_to_Coordinate(a21.areaCol,user1[i+1].location[0],end);
        p.drawLine(QPoint((start.x+0.5)*ColInterval,-(start.y+0.5)*RowInterval),
                   QPoint((end.x+0.5)*ColInterval,-(end.y+0.5)*RowInterval));
        //标明时间戳
        //p.drawText(QPoint((end.x+0.5)*ColInterval,-(end.y+0.5)*RowInterval), QString::number(i+2));
    }
    //画攻击轨迹，由于和实际轨迹相同时画出来的轨迹会重合，这样就看不清楚了，所以我们攻击轨迹统一y轴方向上往下挪0.1倍间隔，x轴方向往左挪0.1倍间距
    p.setPen(QPen(Qt::red, 2, Qt::DashDotLine));
    for(int i=0;i<user2.size()-1;i++)
    {
        Coordinate start,end;
        PosiNum_to_Coordinate(a22.areaCol,user2[i].location[0],start);
        PosiNum_to_Coordinate(a22.areaCol,user2[i+1].location[0],end);
        p.drawLine(QPoint((start.x+0.4)*ColInterval,-(start.y+0.4)*RowInterval),
                   QPoint((end.x+0.4)*ColInterval,-(end.y+0.4)*RowInterval));
        //标明时间戳
        //p.drawText(QPoint((end.x+0.5)*ColInterval,-(end.y+0.4)*RowInterval), QString::number(i+2));
    }
    /*
    //画一条直线
    p.drawLine(QPoint(0, 0), QPoint(100, 100));
    //画一个椭圆，第一个参数为中心点，第二个第三个分别为x和y的轴长,当二者相等时未圆
    p.drawEllipse(QPoint(100, 100), 50, 50);
    //画出直角矩形
    p.drawRect(QRect(150, 150, 100,60));
    //画出有弧的矩形
    p.drawRoundedRect(QRect(230, 230, 100,60), 20, 20);*/

    //结束画笔
    p.end();

    //开始在窗口上画，画什么呢？
    p.begin(this);
    //前方开始装车的过程，最后一股气打包全部带走，送到窗口
    //北京颜色发生了改变，怎么变回来呢？
    p.drawPixmap(QPoint(0, 0), pixmap);
}
