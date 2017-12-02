#ifndef ITEMHELPER_H
#define ITEMHELPER_H

class QStandardItem;

void set_item_will_be_deleted(QStandardItem *item, bool to_be_deleted);
bool is_item_will_be_deleted(QStandardItem *item);
void set_item_already_deleted(QStandardItem *item, bool delete_success);
bool is_item_already_deleted(QStandardItem *item);

#endif // ITEMHELPER_H
