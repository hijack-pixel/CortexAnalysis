#pragma execution_character_set("utf-8")
#include <QMessageBox>
#include <QApplication>
#include <QScreen>
#include <QDialog>

#include "mygraphicsview.h"

extern QString convertImgTitle(QString title);

MyGraphicsView::MyGraphicsView()
{
    init();
}


MyGraphicsView::MyGraphicsView(QWidget *parent): QGraphicsView(parent)
{
    init();
}


MyGraphicsView::~MyGraphicsView()
{
    //  删除框选item、原图item
    QList<QGraphicsItem *> itemList = scene->items();
    foreach(auto item, itemList)
    {
        scene->removeItem(item);
        delete item;
    }

    delete defaultImg;
}

void MyGraphicsView::init()
{
    setMinimumSize(400, 400);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHint(QPainter::Antialiasing);
    // setAttribute(Qt::WA_TranslucentBackground);

    pen.setColor(QColor(255, 203, 59));
    pen.setWidth(2);
    // setTransformationAnchor(QGraphicsView::AnchorViewCenter);    // scene 在 view 的中心点作为锚点

    defaultImg = new QPixmap(50, 50);
    defaultImg->fill();
    setStyleSheet("background-color: white");

    scene = new QGraphicsScene();

    //添加辅助框选图元
    assistItem  = new CanvaseItemBase();
    assistItem->setZValue(1);  //始终显示在上层
    scene->addItem(assistItem);

    imgScaleItem = new QGraphicsPixmapItem();
    imgScaleItem->setCacheMode(QGraphicsItem::ItemCoordinateCache);
    scene->addItem(imgScaleItem);


    labelTitle = new QLabel(this);
    widgetTittle = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout();

    labelTitle->setGeometry(0, 0, 5000, 30); // 设置位置和大小
    // labelTitle->setWordWrap(true);
    labelTitle->setMargin(6);
    labelTitle->setStyleSheet("background-color: rgba(0, 0, 0, 100); color: white;");
    labelTitle->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    layout->addWidget(labelTitle);
    layout->setMargin(0);
    layout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    widgetTittle->setLayout(layout);
    widgetTittle->raise();

    setScene(scene);
    // enableFreeDraw();
    disableDraw();
}

void MyGraphicsView::setImgByPath(QString path)
{
    //原图
    QPixmap pixmap = QPixmap(path);
    imgOriginSize = pixmap.size();

    //缩放图
    // pixmap = pixmap.scaled(QSize(imgShowSize, imgShowSize), Qt::KeepAspectRatio, Qt::FastTransformation);
    imgScaleSize = pixmap.size();
    imgScaleRect.setRect(0, 0, imgScaleSize.width(), imgScaleSize.height());
    historyCenter.setX(imgScaleSize.width()/2);
    historyCenter.setY(imgScaleSize.height()/2);


    // 路径读不出来图片，显示一张小白图。路径有图片则显示图片并设置 图片信息label
    if(pixmap.isNull())
    {
        setDisabled(true);
        labelTitle->setText("");
        widgetTittle->setVisible(false);
        imgScaleItem->setPixmap(*defaultImg);
        imgScaleRect.setRect(0, 0, defaultImg->width(), defaultImg->height());
    }
    else
    {
        setDisabled(false);
        imgScaleItem->setPixmap(pixmap);

        // 设置图片信息 label
        widgetTittle->setVisible(true);
        fileInfo.setFile(path);
        labelTitle->setText(convertImgTitle(fileInfo.fileName()));
        labelTitle->setFixedSize(labelTitle->sizeHint());
        widgetTittle->setFixedSize(widgetTittle->sizeHint());
    }

    qDebug() << "Set image path: " << path << imgScaleRect;

    // 图片居中，主要是居中坐标更新了，后续拖动图片不会跳动
    scene->setSceneRect(imgScaleRect);
    fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}


void MyGraphicsView::enableRectDraw()
{
    rectDrawEnable = true;
    freeDrawEnable = false;
}

void MyGraphicsView::enableFreeDraw()
{
    freeDrawEnable = true;
    rectDrawEnable = false;
}

void MyGraphicsView::disableDraw()
{
    rectDrawEnable = false;
    freeDrawEnable = false;
}

bool MyGraphicsView::isRectDrawEnable()
{
    return rectDrawEnable;
}

bool MyGraphicsView::isFreeDrawEnable()
{
    return freeDrawEnable;
}


void MyGraphicsView::wheelEvent(QWheelEvent *event)
{
    qreal ratio = transform().m11();  //当前缩放比例

    zoomOutEnable = ratio > minScaleRatio ? true : false; //放大缩小限制
    zoomInEnable = ratio < maxScaleRatio ? true : false;


    if(event->angleDelta().y() < 0 && zoomOutEnable)
        scale(1-wheelScaleDelta, 1-wheelScaleDelta);

    if(event->angleDelta().y() > 0 && zoomInEnable)
        scale(1+wheelScaleDelta, 1+wheelScaleDelta);
}


void MyGraphicsView::mousePressEvent(QMouseEvent *event)
{
    isMousePressed = true;

    if(event->button() == Qt::LeftButton)  //assistItem初始化
    {
        startPoint = endPoint = (event->pos());
        assistItem->setPos(mapToParent(startPoint.toPoint())); //依据鼠标点击的控件位置设置元素的位置
        assistItem->setItemSize(0,0);
        assistItem->update();
        qDebug() << "Press(View): " << (startPoint.toPoint());
    }
    else if(event->button() == Qt::RightButton)  //恢复原样 删除标注框
    {
        qreal ratio = transform().m11();  //当前缩放比例
        scale(1/ratio, 1/ratio);

        QList<QGraphicsItem *> itemList = scene->items(); //新加的item在栈顶，是数组第一个元素
        itemList.removeLast();   //缩放背景图，还有实时显示的assistItem留下来不删除
        itemList.removeFirst();  //由于assistItem setZValue导致其在栈的位置发生改变
        foreach(auto item, itemList)
        {
            scene->removeItem(item);
            delete item;
        }
        assistItem->resetItem();  //防止还原后显示上次矩形框
        assistItem->update();
    }

    if(isRectDrawEnable())
    {
        assistItem->setRectDrawEnable();
        qDebug() << "\nsetRectDrawEnable";
    }
    else if(isFreeDrawEnable())
    {
        assistItem->setFreeDrawEnable();
        qDebug() << "\nsetFreeDrawEnable";
    }

    this->viewport()->update();
}


void MyGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    if(isMousePressed == false)
        return;

    scene->setSceneRect(imgScaleRect); //防止因为选框导致画面中心偏移
    endPoint = (event->pos());
    delta = endPoint - startPoint;

    //去除滚轮缩放影响导致鼠标不跟手
    delta.rx() = delta.x() / transform().m11();
    delta.ry() = delta.y() / transform().m22();

    // qDebug() << "Start:" << startPoint << "   End:" << endPoint  << "  delta:" << delta;

    assistItem->setItemSize(delta.x(), delta.y());

    if(isRectDrawEnable() && event->modifiers() == Qt::CTRL)//-----------------------------------计算矩形选框的宽高并更新显示
    {
        assistItem->update();
    }
    else if(isFreeDrawEnable() && event->modifiers() == Qt::SHIFT)//------------------------------自由画图框选更新显示
    {
        pointItemVector.append(assistItem->mapFromScene(mapToScene(endPoint.toPoint())));
        pointSceneVector.append(mapToScene(endPoint.toPoint()));
        assistItem->setFreeDrawPoints(&pointItemVector);
        assistItem->update();
    }
    else//----------------------------------------------------------左键 图片平移
    {
        centerOn(((QPoint(
            historyCenter.x() - delta.x(),
            historyCenter.y() - delta.y()
            ))));
    }

    this->viewport()->update();
}


void MyGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    isMousePressed = false;
    this->viewport()->update();

    historyCenter.setX(historyCenter.x() - delta.x());
    historyCenter.setY(historyCenter.y() - delta.y());

    // 移动超出边界，消除多余移动值

    // 左上角
    historyCenter.rx() = historyCenter.x() < width()/2/maxScaleRatio ?
                             width()/2/maxScaleRatio : historyCenter.x();

    historyCenter.ry() = historyCenter.y() < height()/2/maxScaleRatio ?
                             height()/2/maxScaleRatio : historyCenter.y();

    // 右下角
    historyCenter.rx() = historyCenter.x() > imgScaleRect.width()-width()/2/maxScaleRatio ?
                             imgScaleRect.width()-width()/2/maxScaleRatio : historyCenter.x();

    historyCenter.ry() = historyCenter.y() > imgScaleRect.height()-height()/2/maxScaleRatio ?
                             imgScaleRect.height()-height()/2/maxScaleRatio : historyCenter.y();


    qDebug() << "historyCenter" << historyCenter;

    qreal width, height;
    assistItem->getItemSize(width, height);
    if(width == 0 && height == 0)
        return;

    qreal scaleFactor = qMax(imgOriginSize.width(), imgOriginSize.height())/imgShowSize;  //缩放比例

    if(event->button() == Qt::LeftButton && isRectDrawEnable())  //画矩形框
    {
        //绘画缩放图上矩形框
        QPointF p = mapToScene(startPoint.toPoint());
        boxScaleRect = QRectF(p.x(), p.y(), delta.x(), delta.y());
        boxScaleRect = boxScaleRect.intersected(imgScaleRect);
        scene->addRect(boxScaleRect, pen);
        qDebug() << "Scale Rect" << boxScaleRect;

        //绘画原图上矩形框
        qreal x = boxScaleRect.x() * scaleFactor;
        qreal y = boxScaleRect.y() * scaleFactor;
        qreal w = boxScaleRect.width() * scaleFactor;
        qreal h = boxScaleRect.height() * scaleFactor;
        boxOriginRect.setRect(x, y, w, h);
        boxOriginRect = boxOriginRect.intersected(imgOriginRect);
        /*********************************************************************************************************************************/
        /*********************************************************************************************************************************/
        qDebug() << "Origin Rect" << boxOriginRect << "\n";
    }

    if(event->button() == Qt::LeftButton && isFreeDrawEnable())  //自由绘图框
    {
        //绘画缩放图上自由绘图框
        for(int i = 0; i < pointSceneVector.length(); i++)
        {
            if(!imgScaleRect.contains(pointSceneVector[i])) //自由绘图点不在图内，设为边框点
            {
                pointSceneVector[i].setX(  pointSceneVector[i].x() < 0 ? 0 : pointSceneVector[i].x()  );
                pointSceneVector[i].setY(  pointSceneVector[i].y() < 0 ? 0 : pointSceneVector[i].y()  );
                pointSceneVector[i].setX(  pointSceneVector[i].x() > imgScaleSize.width() ? imgScaleSize.width() : pointSceneVector[i].x()  );
                pointSceneVector[i].setY(  pointSceneVector[i].y() > imgScaleSize.height() ? imgScaleSize.height() : pointSceneVector[i].y()  );
                pointItemVector[i] = assistItem->mapFromScene(pointSceneVector[i]);
            }
        }
        scene->addPolygon(QPolygonF(pointSceneVector), pen);

        //绘画原图上自由绘图框
        for(int i = 0; i < pointSceneVector.length(); i++)  //缩放路径上的点数组
        {
            pointSceneVector[i] = (pointSceneVector[i])*scaleFactor;
            pointItemVector[i] = assistItem->mapFromScene(pointSceneVector[i]);
        }
        /*********************************************************************************************************************************/
        /*********************************************************************************************************************************/
        qDebug() << "Origin polygon" << "\n";
    }

    assistItem->resetItem();
    pointSceneVector.clear();
    pointItemVector.clear();
}


void MyGraphicsView::resizeEvent(QResizeEvent *event)
{
    scale(1/transform().m11(), 1/transform().m11());  // 恢复复上次缩放

    // 图片大于显示框，缩放适应显示
    qreal ratio = 1;
    qreal ratioWidth = 0;
    qreal ratioHeight = 0;
    ratioWidth = width() / imgScaleRect.width();
    ratioHeight = height() / imgScaleRect.height();
    ratio = qMin(ratioWidth, ratioHeight);
    scale(ratio, ratio);
    // qDebug() << ratioWidth << ratioHeight <<  ratio  << this->size().width() << this->size().height();
}


void MyGraphicsView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(isDialogOpen)
        return;

    isDialogOpen = true;

    qDebug() << "Open dialog...";

    // 创建一个模态对话框
    QDialog dialog;
    dialog.setWindowTitle(fileInfo.fileName());

    // 设置消息框的最小大小
    dialog.setMinimumSize(300, 300);

    // 获取屏幕尺寸
    QScreen *screen = QApplication::primaryScreen();
    QSize screenSize = screen->availableSize();

    // 设置消息框的最大尺寸为屏幕尺寸
    dialog.setMaximumSize(screenSize);
    dialog.resize(screenSize*0.8);

    // 创建一个垂直布局
    QVBoxLayout *layoutForWidget = new QVBoxLayout(&dialog);
    layoutForWidget->setContentsMargins(0, 0, 0, 0);

    // 创建一个widget用于存放MyGraphicsView
    QWidget *widget = new QWidget();
    widget->setContentsMargins(QMargins(0, 0, 0, 0));

    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); // 设置QWidget的大小策略为扩展，使其填满布局
    layoutForWidget->addWidget(widget);

    QVBoxLayout *layoutForImgView = new QVBoxLayout(widget);
    layoutForImgView->setContentsMargins(0, 0, 0, 0);

    MyGraphicsView *imgView = new MyGraphicsView(widget);

    imgView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);  // 设置MyGraphicsView的大小策略为Expanding，使其填满布局
    layoutForImgView->addWidget(imgView);

    imgView->isDialogOpen = true;
    imgView->setImgByPath(fileInfo.absoluteFilePath());


    // 显示消息框
    dialog.exec();

    // 在对话框关闭后删除imgView对象
    delete imgView;

    isDialogOpen = false;
}

















