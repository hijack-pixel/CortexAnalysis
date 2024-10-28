//canvasitembase.cpp
#include "canvaseitembase.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QVector2D>
#include <QVector3D>

CanvaseItemBase::CanvaseItemBase(QGraphicsItem* parentItem): QGraphicsItem(parentItem)
{
    pen.setColor(QColor(15, 134, 211));
    pen.setStyle(Qt::DashLine);
    pen.setWidth(2);
}

void CanvaseItemBase::setPen(QPen p)
{
    pen.swap(p);  //This operation is very fast and never fails.
}

void CanvaseItemBase::setItemSize(qreal width, qreal height)
{
    if(rectWidth != width || rectHeight != height)
    {
        //If you want to change the item's bounding rectangle, you must first call prepareGeometryChange().
        prepareGeometryChange();
    }
    rectWidth = width;
    rectHeight = height;
}

void CanvaseItemBase::getItemSize(qreal &width, qreal &height)
{
    width = rectWidth;
    height = rectHeight;
}

void CanvaseItemBase::setRectDrawEnable()
{
    rectDrawEnable = true;
    freeDrawEnable = false;
}

void CanvaseItemBase::setFreeDrawEnable()
{
    freeDrawEnable = true;
    rectDrawEnable = false;
}

bool CanvaseItemBase::isRectDrawEnable()
{
    return rectDrawEnable;
}
bool CanvaseItemBase::isFreeDrawEnable()
{
    return freeDrawEnable;
}

void CanvaseItemBase::setFreeDrawPoints(QVector<QPointF> *points)
{
    polygon.clear();
    polygon << *points;
}

void CanvaseItemBase::resetItem()
{
    polygon.clear();
    setItemSize(0, 0);
    rectDrawEnable = false;
    freeDrawEnable = false;
}

//QGraphicsView uses this to determine whether the item requires redrawing.
QRectF CanvaseItemBase::boundingRect() const
{
    return QRectF(0, 0, rectWidth, rectHeight);
}

void CanvaseItemBase::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter->setRenderHint(QPainter::TextAntialiasing, true);
    painter->setPen(pen);

    // 自定义绘制
    QRectF outLintRect = QRectF(0, 0, rectWidth, rectHeight);
    if((rectWidth == 0) ||(rectWidth == 0))
    {
        painter->setCompositionMode(QPainter::CompositionMode_Clear);
        painter->eraseRect(outLintRect);
        return;
    }
    else
    {
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
        if(rectDrawEnable)
        {
            painter->drawRect(QRectF(0, 0, rectWidth, rectHeight));
        }
        else if(freeDrawEnable)
        {
            painter->drawPolygon(polygon);
        }
    }
}

//可以返回item形状用于碰撞检测等功能，比如返回这个item所画的形状
QPainterPath CanvaseItemBase::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

//void CanvaseItemBase::mousePressEvent(QGraphicsSceneMouseEvent *event)
//{
//    return QGraphicsItem::mousePressEvent(event);
//}

//void CanvaseItemBase::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
//{
//    return QGraphicsItem::mouseMoveEvent(event);
//}

//void CanvaseItemBase::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
//{
//    return QGraphicsItem::mouseReleaseEvent(event);
//}

// notify custom items that some part of the item's state changes
//change is the parameter of the item that is changing. value is the new value; the type of the value depends on change.
//QVariant CanvaseItemBase::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
//{
//    //If you want to change the item's bounding rectangle, you must first call prepareGeometryChange().
//    //If the item is presently selected, it will become unselected, and vice verca.
//    if(change == QGraphicsItem::ItemSelectedChange)
//        prepareGeometryChange();

//    return QGraphicsItem::itemChange(change, value);
//}

