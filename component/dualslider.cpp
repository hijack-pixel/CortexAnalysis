#include "dualslider.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainterPath>
#include <QLinearGradient>
#include <QDebug>

DualSlider::DualSlider(QWidget *parent)
    : QWidget(parent), m_leftValue(0), m_rightValue(100), m_minValue(0), m_maxValue(100), m_draggingLeft(false), m_handleSize(20), m_handleColor(QColor("#dcdcdc"))
{
    setMinimumHeight(10);
    setMaximumHeight(12);
    setStyleSheet("QWidget { border: none; }");
}

void DualSlider::setRange(int min, int max) {
    if (min > max) {
        qWarning("Invalid range: min (%d) is greater than max (%d)", min, max);
        return;
    }
    m_minValue = min;
    m_maxValue = max;
    // setLeftValue(max * 0.25);
    // setRightValue(max * 0.75);
    update();
}

void DualSlider::setLeftValue(int value) {
    m_leftValue = qBound(m_minValue, value, m_rightValue - 1); // Ensure no overlap
    update();
    emit valuesChanged(m_leftValue, m_rightValue);
}

void DualSlider::setRightValue(int value) {
    m_rightValue = qBound(m_leftValue + 1, value, m_maxValue); // Ensure no overlap
    update();
    emit valuesChanged(m_leftValue, m_rightValue);
}

void DualSlider::setHandleSize(int size) {
    m_handleSize = size;
    update();
}

void DualSlider::setHandleColor(const QColor &color) {
    m_handleColor = color;
    update();
}

void DualSlider::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制渐变背景
    QLinearGradient gradient(rect().topLeft(), rect().topRight());
    gradient.setColorAt(0, QColor("#cccedb"));
    gradient.setColorAt(1, QColor("#cccedb"));
    QPainterPath path;
    painter.setPen(QPen(QColor("#838383")));
    path.addRoundedRect(rect(), rect().height() / 2, rect().height() / 2);
    painter.fillPath(path, gradient);
    painter.drawPath(path);

    // 圆角矩形遮罩
    painter.setClipPath(path);

    // 计算滑块位置
    int leftX = valueToPosition(m_leftValue);
    int rightX = valueToPosition(m_rightValue);
    int min = valueToPosition(m_minValue);
    int max = valueToPosition(m_maxValue);

    // 绘制选定范围以外加遮罩
    painter.save();
    painter.setBrush(Qt::white);
    painter.setOpacity(0.8);
    painter.setPen(Qt::NoPen);
    painter.drawRect(QRect(QPoint(min, 1), QPoint(leftX + 4, height() - 2)));    //  多的 4 为抵消圆角矩形的圆角
    painter.drawRect(QRect(QPoint(rightX - 4, 1), QPoint(max, height() - 2)));   //  多的 4 为抵消圆角矩形的圆角
    painter.restore();

    // 绘制滑块
    painter.setOpacity(1.0);
    drawLeftHandle(&painter, leftX);
    drawRightHandle(&painter, rightX);
}

void DualSlider::mousePressEvent(QMouseEvent *event) {
    int pos = event->pos().x();
    if (pos < 0 || pos > width()) {
        return; // Ensure position is within bounds
    }

    int leftX = valueToPosition(m_leftValue);
    int rightX = valueToPosition(m_rightValue);

    if (abs(pos - leftX) < abs(pos - rightX)) {
        m_draggingLeft = true;
        setLeftValue(positionToValue(pos));
    } else {
        m_draggingLeft = false;
        setRightValue(positionToValue(pos));
    }
}

void DualSlider::mouseMoveEvent(QMouseEvent *event) {
    int pos = event->pos().x();
    if (pos < 0 || pos > width()) {
        return; // Ensure position is within bounds
    }
    if (m_draggingLeft) {
        setLeftValue(positionToValue(pos));
    } else {
        setRightValue(positionToValue(pos));
    }
}

int DualSlider::valueToPosition(int value) const {
    if (m_maxValue == m_minValue) {
        return 0; // Avoid division by zero
    }
    return (value - m_minValue) * width() / (m_maxValue - m_minValue);
}

int DualSlider::positionToValue(int pos) const {
    return m_minValue + pos * (m_maxValue - m_minValue) / width();
}

void DualSlider::drawLeftHandle(QPainter *painter, int x) {
    QRect handleRect(x, 1, m_handleSize, height() - 2);
    QPainterPath path;
    path.addRoundedRect(handleRect, handleRect.height() / 2, handleRect.height() / 2);
    painter->fillPath(path, m_handleColor);
    painter->drawPath(path);
}

void DualSlider::drawRightHandle(QPainter *painter, int x) {
    QRect handleRect(x - m_handleSize, 1, m_handleSize, height() - 2);
    QPainterPath path;
    path.addRoundedRect(handleRect, handleRect.height() / 2, handleRect.height() / 2);
    painter->fillPath(path, m_handleColor);
    painter->drawPath(path);
}
