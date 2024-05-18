#ifndef DATABASEADAPTER_H
#define DATABASEADAPTER_H

#include "abstractdatabaseadapter.h"

class SqliteDatabaseAdapter : public AbstractDatabaseAdapter {
public:
    explicit SqliteDatabaseAdapter(const QString & filename);
    ~SqliteDatabaseAdapter() override;

    QString insertNodeDataQueryString(const QString &table, const QString &column) const override;
    QString insertEdgeDataQueryString(const QString &table) const override;
    QString createTableQueryString(const QString &table, const QSet<QString> &columns, const QHash<QString, QString> &columnTypes) const override;
    QString dropTableQueryString(const QString &table) const override;
    QString addTableColumnQueryString(const QString &table, const QString &column, const QString &columnType) const override;
    QString createOTypeTableQueryString() const override;

    QString integerType() const override;
    QString stringType() const override;
};

#endif // DATABASEADAPTER_H
