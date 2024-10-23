#include "clickablewidget.h"
#include <QDebug>



ClickableWidget::ClickableWidget(QWidget *parent) : QWidget(parent)
{
    setAttribute (Qt::WA_StyledBackground);  //Indicates the widget should be drawn using a styled background.

    m_animation = new StyleSheetAnimation(this, "", this);
    m_animation->setDuration(400);
    m_animation->setEasingCurve(QEasingCurve::InCubic);

}


void ClickableWidget::enterEvent(QEvent *event)
{
    m_animation->setStartValue(m_color1);
    m_animation->setEndValue(m_color2);
    m_animation->start();
    QWidget::enterEvent(event);
}

void ClickableWidget::leaveEvent(QEvent *event)
{
    m_animation->setStartValue(m_color2);
    m_animation->setEndValue(m_colorTransparent);
    m_animation->start();
    QWidget::leaveEvent(event);
}

void ClickableWidget::mousePressEvent(QMouseEvent *event)
{
    // 鼠标按下时的效果
    if (event->button() == Qt::LeftButton) {
        m_animation->setStartValue(m_color1);
        m_animation->setEndValue(m_color2);
        m_animation->start();
    }
    QWidget::mousePressEvent(event);
}

void ClickableWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_animation->setStartValue(m_color2);
    m_animation->setEndValue(m_color1);
    m_animation->start();

    emit clicked();

    QWidget::mouseReleaseEvent(event);
}

// void ClickableWidget::paintEvent(QPaintEvent *event)
// {
//     QPainter painter(this);
//     // 绘制背景
//     painter.fillRect(rect(), Qt::gray);
//     // 绘制文本或其他内容
//     // painter.drawText(rect(), Qt::AlignBottom, tr("成像预览"));
// }
