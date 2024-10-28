#ifndef COMBOXITEMDELEGATE_H
#define COMBOXITEMDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>


/**
 * @brief The ComboxItemDelegate class
 *
 * 格式化 ComBox 下拉框 item 样式
 */
class ComboxItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    ComboxItemDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {};

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

};

#endif // COMBOXITEMDELEGATE_H
