#include "objectmodel.h"

ObjectModel::ObjectModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

int ObjectModel::rowCount(const QModelIndex&) const {
    return items.size();
}

QVariant ObjectModel::data(const QModelIndex& index, int /*role*/) const {
    QObject* item = items.at(index.row());
    return QVariant::fromValue(item);
}

void ObjectModel::insert(QObject* item) {
    beginInsertRows(QModelIndex(), 0, 0);
    items.push_front(item);
    endInsertRows();
}

void ObjectModel::insert(const QVector<QObject*>& insertItems) {
    beginInsertRows(QModelIndex(), 0, 0);
    items.append(insertItems);
    endInsertRows();
}

void ObjectModel::remove(QObject* item) {
    for (int i = 0; i < items.size(); ++i) {
        if (items.at(i) == item) {
            beginRemoveRows(QModelIndex(), i, i);
            items.remove(i);
            endRemoveRows();
            break;
        }
    }
}

void ObjectModel::reset() {
    beginResetModel();
    items.clear();
    endResetModel();
}

QHash<int, QByteArray> ObjectModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[Qt::UserRole + 1] = "item";
    return roles;
}