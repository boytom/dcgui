#include "itemhelper.h"
#include <QStandardItem>

void set_item_will_be_deleted(QStandardItem *item, bool to_be_deleted)
{
  if (item == nullptr)
    return;

  if (to_be_deleted) {
    QFont ft = item->font();
    ft.setStrikeOut(true);
    item->setFont(ft);

    QBrush b = item->foreground();
    b.setColor(QColor(0x36, 0xD5, 0x81));
    item->setForeground(b);
    if (item->isCheckable())
      item->setCheckState(Qt::Checked);
  }
  else {
    QStandardItem *parent_item = item->parent();
    if (parent_item != nullptr) {
      item->setFont(parent_item->font());
      //item->setForeground(parent_item->foreground());
      item->setForeground(QBrush(QColor(0xE9, 0xE9, 0xE9)));
      if (item->isCheckable())
        item->setCheckState(Qt::Unchecked);
    }
  }
}

bool is_item_will_be_deleted(QStandardItem *item)
{
  return item != nullptr && item->foreground().color() == QColor(0x36, 0xD5, 0x81);
}

void set_item_already_deleted(QStandardItem *item, bool delete_success)
{
  if (!delete_success)
    return;

  QFont ft = item->font();
  ft.setStrikeOut(true);
  item->setFont(ft);
  QBrush b = item->foreground();
  b.setColor(Qt::red);
  item->setForeground(b);
  //if (item->isCheckable() && item->checkState() == Qt::Checked)
  item->setEnabled(false);
}

bool is_item_already_deleted(QStandardItem *item)
{
  return item != nullptr && item->foreground().color() == Qt::red;
}
