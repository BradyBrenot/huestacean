#pragma once
#include <QAbstractListModel>
#include <QVector>

class ObjectModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit ObjectModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex&) const override;
    QVariant data(const QModelIndex& index, int role) const override;

    //*WARNING* - ONLY use with QVectors of QObject subclasses
    template<class T>
    void insertArray(const QVector<T*>& insertItems)
    {
        const QVector<QObject*>& objectArray = *reinterpret_cast<const QVector<QObject*>*>(&insertItems);
        beginInsertRows(QModelIndex(), 0, 0);
        items.append(objectArray);
        endInsertRows();
    }

public slots:
    void insert(QObject* item);

    void remove(QObject* item);
    void reset();

signals:
    void modelChanged();

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    QVector<QObject*> items;
};