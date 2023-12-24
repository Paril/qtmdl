#pragma once

#include <QWidget>
#include <QVector2D>
#include <QOpenGLWidget>
#include <QImage>

class QUVPainter : public QWidget
{
public:
    QUVPainter(QWidget *parent = nullptr);

    void focusLost();
    QMatrix4x4 getDragMatrix();

protected:
    virtual void paintEvent(QPaintEvent *event) override;

    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void rectangleSelect(QMouseEvent *e, QRectF rect, QVector2D tcScale);

    bool _dragging = false;
    QPoint _dragPos, _downPos, _dragDelta;
    QVector2D _dragWorldPos;
};
