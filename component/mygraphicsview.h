#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H

#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsItem>
#include <QDebug>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QVector>
#include <QPainter>
#include <QColor>
#include <QLabel>
#include <QFile>
#include <QFileInfo>
#include <QFileDevice>
#include <QDateTime>
#include <QPushButton>
#include <QHBoxLayout>
#include "canvaseitembase.h"

/**
 * @brief The MyGraphicsView class 继承于QGraphicsView，与自定义canvaseitembase组合，实现图片切换、缩放、拖动、绘画矩形与多边形。
 */
class MyGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    MyGraphicsView();
    MyGraphicsView(QWidget *parent = nullptr);

    void init();
    void setImgByPath(QString path);
    void enableRectDraw();
    void enableFreeDraw();
    void disableDraw();
    bool isRectDrawEnable();
    bool isFreeDrawEnable();

protected:
    void wheelEvent(QWheelEvent *event)override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    int imgShowSize = 500;

    QGraphicsScene *scene = new QGraphicsScene();
    // QWidget *widget = new QWidget(this);
    // QHBoxLayout *layout = new QHBoxLayout();
    QLabel *labelTitle = new QLabel(this);
    // QPushButton *pushButtonToggleShow = new QPushButton("LESS");
    // bool showMore = true;
    QString fileInfomation;

    //绘制辅助图元
    CanvaseItemBase* assistItem;
    QPen pen;

    //显示图片
    QGraphicsPixmapItem *imgScaleItem;

    //鼠标起始点，结束点，变化大小
    QPointF startPoint;
    QPointF endPoint;
    QPointF delta;
    QPointF accumulateDelta;
    bool isMousePressed = false;
    QPoint historyCenter;

    //缩放比例
    qreal maxScaleRatio = 5;
    qreal minScaleRatio = 0.1;
    qreal wheelScaleDelta = 0.1;
    bool zoomInEnable = true, zoomOutEnable = true; //控制缩放比例


    //矩形框选
    QSize imgOriginSize;   //原图尺寸
    QRectF imgOriginRect;
    QRectF boxOriginRect;

    QSize imgScaleSize;    //缩放图尺寸
    QRectF imgScaleRect;
    QRectF boxScaleRect;

    //自由框选路径
    QVector<QPointF> pointSceneVector;
    QVector<QPointF> pointItemVector;

    //矩形框选 自由框选 使能
    bool rectDrawEnable = false;
    bool freeDrawEnable = false;
};

#endif // MYGRAPHICSVIEW_H
