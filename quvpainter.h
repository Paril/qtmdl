#pragma once

#include <QWidget>

class QUVPainter : public QWidget
{
public:
    QUVPainter(QWidget *parent = nullptr);

protected:
    virtual void paintEvent(QPaintEvent *event) override;
};
