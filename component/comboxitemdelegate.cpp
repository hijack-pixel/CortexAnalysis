#include "comboxitemdelegate.h"
#include "qpainter.h"
#include <QDebug>

#pragma execution_character_set("utf-8")

void ComboxItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem myOption = option;
    initStyleOption(&myOption, index);

    if (index.data(Qt::UserRole).toBool() == true) {
        myOption.font.setBold(true);
        myOption.palette.setColor(QPalette::Text, Qt::darkGreen);
        // qDebug() << "ComBox set item style:" << index.data().toString();
    }

    // 绘制原始项
    QStyledItemDelegate::paint(painter, myOption, index);

    QVariant userRoleValue = index.data(Qt::UserRole);
    if (userRoleValue.isValid() && userRoleValue.toBool()) {

        // 准备绘制“已选择”文本
        QString selectedText = tr("√");
        QRect textRect = option.rect;
        QFontMetrics fm = painter->fontMetrics();
        int textWidth = fm.horizontalAdvance(selectedText);

        // 设置“已选择”文本的位置，位于项的右侧
        int x = textRect.right() - textWidth - 5; // 5为右边距
        int y = textRect.center().y() + (fm.ascent() / 2);

        // 设置文本样式
        painter->setPen(QColor("darkGreen"));
        QFont font = painter->font();
        font.setBold(true);
        painter->setFont(font);

        // 绘制“已选择”文本
        painter->drawText(QPointF(x, y), selectedText);
    }

    QStyledItemDelegate::paint(painter, myOption, index);
}
