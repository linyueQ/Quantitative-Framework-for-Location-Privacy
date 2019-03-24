#ifndef PAINTER_H
#define PAINTER_H

#include "head.h"
#include <QWidget>

namespace Ui {
class painter;
}

class painter : public QWidget
{
    Q_OBJECT

public:
    explicit painter(QWidget *parent = 0);
    ~painter();
    void paintEvent(QPaintEvent *e);

private:
    Ui::painter *ui;
};

#endif // PAINTER_H
