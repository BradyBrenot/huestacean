#pragma once
#include <QAbstractListModel>
#include <QVector>

class ObjectModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit ObjectModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex&) const override;
    QVariant data(const QModelIndex& index, int role) const override;

public slots:
    void insert(QObject* item);
    void insert(const QVector<QObject*>& insertItems);
    void remove(QObject* item);
    void reset();

signals:
    void modelChanged();

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    QVector<QObject*> items;
};