#include "clickablewidget.h"
#include <QDebug>



ClickableWidget::ClickableWidget(QWidget *parent) : QWidget(parent)
{
    setAttribute (Qt::WA_StyledBackground);  //Indicates the widget should be drawn using a styled background.

    m_animation = new StyleSheetAnimation(this, "", this);
    m_animation->setDuration(400);
    m_animation->setEasingCurve(QEasingCurve::InCubic);

    m_animationChecked = new StyleSheetAnimation(this, "", this);
    m_animationChecked->setDuration(3000);
    m_animationChecked->setLoopCount(-1);
    m_animationChecked->setEasingCurve(QEasingCurve::InCubic);

    // connect(m_animationChecked, &QPropertyAnimation::finished, this, &ClickableWidget::updateAnimationState);
}


void ClickableWidget::setChecked(bool state)
{
    m_checked = state;
    if(m_checked)
    {
        m_animationChecked->setStartValue(m_color1);
        m_animationChecked->setKeyValueAt(0.5, m_color2);
        m_animationChecked->setEndValue(m_color1);
        m_animationChecked->start();
    }
    else
    {
        m_animationChecked->stop();
        setStyleSheet("background-color: transparent;");
    }
}


void ClickableWidget::updateAnimationState()
{
    if(m_checked)
    {
        m_animationChecked->start();
    }
    else
    {
        m_animationChecked->stop();
    }
}


// 鼠标进入的效果
void ClickableWidget::enterEvent(QEvent *event)
{
    if(!m_checked)
    {
        m_animation->setStartValue(m_color1);
        m_animation->setEndValue(m_color2);
        m_animation->start();
    }
    QWidget::enterEvent(event);
}


// 鼠标离开的效果
void ClickableWidget::leaveEvent(QEvent *event)
{
    if(!m_checked)
    {
        m_animation->setStartValue(m_color2);
        m_animation->setEndValue(m_colorTransparent);
        m_animation->start();
    }
    QWidget::leaveEvent(event);
}


// 鼠标按下时的效果
void ClickableWidget::mousePressEvent(QMouseEvent *event)
{
    if (!m_checked && event->button() == Qt::LeftButton) {
        m_animation->setStartValue(m_color1);
        m_animation->setEndValue(m_color2);
        m_animation->start();
    }
    QWidget::mousePressEvent(event);
}


// 鼠标释放时的效果
void ClickableWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(!m_checked)
    {
        m_animation->setStartValue(m_color2);
        m_animation->setEndValue(m_color1);
        m_animation->start();
    }

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
