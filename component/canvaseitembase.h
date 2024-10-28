//canvasitembase.h
#ifndef _CANVASE_ITEM_BASE_
#define _CANVASE_ITEM_BASE_

#include <QObject>
#include <QGraphicsItem>
#include <QPixmap>
#include <QGraphicsObject>
#include <QPen>
#include <QVector>

/**
 * @brief The CanvaseItemBase class 绘画图元，用于在QGraphicView中自由绘画，支持polygon和矩形
 */
class CanvaseItemBase: public QGraphicsItem
{
public:
    CanvaseItemBase(QGraphicsItem* parentItem = nullptr);

public:
    void setPen(QPen pen);

    //修改图元尺寸
    void setItemSize(qreal width, qreal height);
    void getItemSize(qreal& width, qreal& height);

    void setRectDrawEnable();
    void setFreeDrawEnable();

    bool isRectDrawEnable();
    bool isFreeDrawEnable();

    void setFreeDrawPoints(QVector<QPointF> *points);

    //清除当前状态
    void resetItem();


protected:
    //边界矩形
    QRectF boundingRect() const override;
    //绘制事件
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) final;
    //获取形状
    QPainterPath shape() const override;

//    //鼠标事件
//    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
//    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
//    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
//    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) override;


private:
    //图元的宽和高
    qreal rectWidth = 0;
    qreal rectHeight = 0;

    //多边形图元point
    QPolygonF polygon;

    // 画笔
    QPen pen;

    bool rectDrawEnable = false;
    bool freeDrawEnable = false;
};
#endif

