#ifndef CLICKABLEWIDGET_H
#define CLICKABLEWIDGET_H

#include <QObject>
#include <QWidget>
#include <QPropertyAnimation>
#include <QPainter>
#include <QEnterEvent>
#include <QPair>
#include <QColor>
#include <QDebug>

using  GradientColorPair = QPair<QColor, QColor>;


/**
 * @brief The StyleSheetAnimation class 动画效果，实现背景色的渐变
 */
class StyleSheetAnimation: public QPropertyAnimation{
public:
    using QPropertyAnimation::QPropertyAnimation;

    //动画播放时值会改变，每次都会更新其Value
    void updateCurrentValue(const QVariant &value) override
    {
        auto color = value.value<QColor>();
        // qDebug() << color;

        qobject_cast<QWidget*>(targetObject())->setStyleSheet(
            QString("ClickableWidget{padding:5px;border: 1px solid grey; border-radius: 6px;"
                    "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, "
                       "stop:0 rgba(%1,%2,%3,%4), "
                       "stop:1 rgba(%5,%6,%7,%8));}")
                        .arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha())
                        .arg(255-color.red()).arg(255-color.green()).arg(255-color.blue()).arg(color.alpha()));
    }
};



/**
 * @brief The ClickableWidget class
 * 继承 QWidget，实现可点击的 widget，添加动画效果。
 */
class ClickableWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ClickableWidget(QWidget *parent = nullptr);


signals:
    void clicked();

public slots:
    /**
     * @brief setChecked  设置widget是否选中,亦即是否点击了.
     * @param state       点击了设为true
     */
    void setChecked(bool state);

    /**
     * @brief updateAnimationState 根据选中状态决定是否循环动画
     */
    void updateAnimationState();


protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    // void paintEvent(QPaintEvent *event) override;

private:
    StyleSheetAnimation *m_animation;

    StyleSheetAnimation *m_animationChecked;

    bool m_checked = false;  //设置是否选中

    QColor m_color1 =  QColor(7,208,255, 200);
    QColor m_color2 =  QColor(231,80,229, 200);
    QColor m_colorTransparent =  QColor(240, 240, 240, 0);
};

#endif // CLICKABLEWIDGET_H
