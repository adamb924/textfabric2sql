#ifndef ABSTRACTDATABASEADAPTER_H
#define ABSTRACTDATABASEADAPTER_H

#include <QVariant>
#include <QHash>
#include <QSet>
#include <QSqlQuery>

#include "tffile.h"

typedef QPair<unsigned int, QVariant> NodeValue;
typedef QPair<unsigned int, unsigned int> Edge;
typedef QPair<Edge, QVariant> EdgeValue;

class AbstractDatabaseAdapter
{
public:
    explicit AbstractDatabaseAdapter(const QString &connectionName);
    virtual ~AbstractDatabaseAdapter();

    bool isOpen() const;

    void createTable(const QString & tableName, const QSet<QString> &columns , const QHash<QString, QString> &columnTypes = QHash<QString,QString>()) const;
    void maybeAddTableColumn(const QString & table, const QString & column, const QString &columnType);
    void addTableColumn(const QString & table, const QString & column, const QString &columnType);

    void insertNodeData(const QString &column, const QString &columnType, const QVariantList &ids, const QVariantList &values);
    void insertEdgeData(const QString & table, const QVariantList &froms, const QVariantList &tos, const QVariantList &values) const;

    void setOtypeRanges(QHash<QString, QPair<unsigned int, unsigned int> > oTypeRanges);
    QString getOTypeFromNode(unsigned int node) const;

    void createOTypeTable() const;

    void beginTransaction() const;
    void commitTransaction() const;

    QString sqlDataType( TFFile::ValueType t ) const;

    virtual QString integerType() const = 0;
    virtual QString stringType() const = 0;

protected:
    void performInsertNodeData(const QString &table, const QString &column, const QString &columnType, const QVariantList &ids, const QVariantList &values);


    /// virtual void functions that provide the query strings
    virtual QString insertNodeDataQueryString(const QString &table, const QString &column) const = 0;
    virtual QString insertEdgeDataQueryString(const QString &table) const = 0;
    virtual QString createTableQueryString(const QString &table, const QSet<QString> &columns, const QHash<QString, QString> & columnTypes) const = 0;
    virtual QString dropTableQueryString(const QString &table) const = 0;
    virtual QString addTableColumnQueryString(const QString &table, const QString &column, const QString &columnType) const = 0;
    virtual QString createOTypeTableQueryString() const = 0;

    QString mConnectionName;
    QHash<QString,QSet<QString>> mTableColumns;
    QHash<QString,QPair<unsigned int,unsigned int>> mOTypeRanges;
};

#endif // ABSTRACTDATABASEADAPTER_H
