#ifndef DUALSLIDER_H
#define DUALSLIDER_H

#include <QWidget>
#include <QColor>

class DualSlider : public QWidget {
    Q_OBJECT

public:
    explicit DualSlider(QWidget *parent = nullptr);
    void setRange(int min, int max);
    void setLeftValue(int value);
    void setRightValue(int value);
    void setHandleSize(int size);
    void setHandleColor(const QColor &color);

signals:
    void valuesChanged(int left, int right);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    int m_leftValue, m_rightValue;
    int m_minValue, m_maxValue;
    bool m_draggingLeft;
    int m_handleSize;
    QColor m_handleColor;

    int valueToPosition(int value) const;
    int positionToValue(int pos) const;
    void drawLeftHandle(QPainter *painter, int x);
    void drawRightHandle(QPainter *painter, int x);
};

#endif // DUALSLIDER_H
