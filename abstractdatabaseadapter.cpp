#include "abstractdatabaseadapter.h"

#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QtDebug>

AbstractDatabaseAdapter::AbstractDatabaseAdapter(const QString & connectionName) : mConnectionName(connectionName)
{

}

AbstractDatabaseAdapter::~AbstractDatabaseAdapter()
{
    QSqlDatabase::removeDatabase(mConnectionName);
}

bool AbstractDatabaseAdapter::isOpen() const
{
    return QSqlDatabase::database(mConnectionName).isOpen();
}

void AbstractDatabaseAdapter::setOtypeRanges(QHash<QString, QPair<unsigned int, unsigned int> > oTypeRanges)
{
    mOTypeRanges = oTypeRanges;
}

void AbstractDatabaseAdapter::beginTransaction() const
{
    QSqlDatabase::database(mConnectionName).transaction();
}

void AbstractDatabaseAdapter::commitTransaction() const
{
    QSqlDatabase::database(mConnectionName).commit();
}

QString AbstractDatabaseAdapter::sqlDataType(TFFile::ValueType t) const
{
    switch(t)
    {
    case TFFile::ValueTypeString:
        return stringType();
    case TFFile::ValueTypeInteger:
        return integerType();
    }
}

QString AbstractDatabaseAdapter::getOTypeFromNode(unsigned int node) const
{
    QHashIterator<QString,QPair<unsigned int,unsigned int>> i( mOTypeRanges );
    while (i.hasNext()) {
        i.next();
        if( node >= i.value().first && node <= i.value().second ) {
            return i.key();
        }
    }
    qCritical() << "AbstractDatabaseAdapter::getOTypeFromNode: Value not found. Here is mOTypeRanges:";
    qCritical() << mOTypeRanges;
    QCoreApplication::exit(0);

    return "";
}



void AbstractDatabaseAdapter::insertNodeData(const QString &column, const QString &columnType, const QVariantList &ids, const QVariantList &values)
{
    /// the table will be the same for all nodes
    Q_ASSERT(!ids.isEmpty());

    /// Different node values need to be placed in different tables
    /// The strategy here is to collect all the nodes into lists that
    /// belong to the same table, and then send them off to performInsertNodeData.
    /// This doesn't assume that the nodes are in a particular order, but it
    /// works more efficiently if they are so ordered (as also happens to be
    /// true of the TextFabric file format, currently).

    /// note that the table name and the otype here are the same
    QString currentTable = getOTypeFromNode( ids.first().toUInt() );
    /// I don't think we're going outside of int range
    int lastInCurrentRange = static_cast<int>(mOTypeRanges.value(currentTable).second);

    QVariantList short_ids, short_values;
    short_ids << ids.first();
    short_values << values.first();

    for(int i=1; i< ids.length(); i++)
    {
        if( ids.at(i).toInt() <= lastInCurrentRange )
        {
            short_ids << ids.at(i);
            short_values << values.at(i);
        }
        else
        {
            /// first insert the current data
            performInsertNodeData( currentTable, column, columnType, short_ids, short_values );
            /// now reset the lists
            short_ids.clear();
            short_values.clear();
            /// add in the current item
            short_ids << ids.at(i);
            short_values << values.at(i);
            currentTable = getOTypeFromNode( ids.at(i).toUInt() );
            lastInCurrentRange = static_cast<int>(mOTypeRanges.value(currentTable).second);
        }
    }
    if( short_ids.length() > 0 )
    {
        performInsertNodeData( currentTable, column, columnType, short_ids, short_values );
    }
}

void AbstractDatabaseAdapter::performInsertNodeData(const QString &table, const QString &column, const QString &columnType, const QVariantList &ids, const QVariantList &values)
{
    maybeAddTableColumn(table,column,columnType);

    QSqlQuery q(QSqlDatabase::database(mConnectionName));
    QString queryString(insertNodeDataQueryString(table,column));
    if( !q.prepare(queryString) ) {
        qWarning() << "AbstractDatabaseAdapter::performInsertNodeData" << q.lastError().text() << queryString;
        return;
    }

    q.bindValue(":value",values);
    q.bindValue(":id",ids);
    if( !q.execBatch() )
        qWarning() << "AbstractDatabaseAdapter::performInsertNodeData" << q.lastError().text() << q.executedQuery();
}

void AbstractDatabaseAdapter::insertEdgeData(const QString &table, const QVariantList &froms, const QVariantList &tos, const QVariantList &values) const
{
    QSqlQuery q(QSqlDatabase::database(mConnectionName));
    QString queryString(insertEdgeDataQueryString(table));
    if( !q.prepare(queryString) ) {
        qWarning() << "AbstractDatabaseAdapter::insertEdgeData" << q.lastError().text() << queryString;
        return;
    }

    q.bindValue(":from", froms);
    q.bindValue(":to", tos);
    q.bindValue(":value", values);

    if( !q.execBatch() )
        qWarning() << "AbstractDatabaseAdapter::insertNodeData" << q.lastError().text() << q.executedQuery();
}

void AbstractDatabaseAdapter::createTable(const QString & tableName, const QSet<QString> &columns, const QHash<QString, QString> &columnTypes ) const
{
    QSqlQuery q(QSqlDatabase::database(mConnectionName));

    if( !q.exec(dropTableQueryString(tableName)) ) {
        qWarning() << "AbstractDatabaseAdapter::createTable" << q.lastError().text() << q.lastQuery();
    }

    QString query = createTableQueryString(tableName,columns,columnTypes);

    if( !q.exec(query) ) {
        qWarning() << "AbstractDatabaseAdapter::createTable" << q.lastError().text() << query;
    }
}

void AbstractDatabaseAdapter::maybeAddTableColumn(const QString &table, const QString &column, const QString &columnType)
{
    if( !( mTableColumns.value(table).contains(column) ) ) {
        addTableColumn(table,column,columnType);
    }
}

void AbstractDatabaseAdapter::addTableColumn(const QString &table, const QString &column, const QString &columnType)
{
    QSqlQuery q(QSqlDatabase::database(mConnectionName));
    if( !q.exec(addTableColumnQueryString(table,column,columnType)) ) {
        qWarning() << "AbstractDatabaseAdapter::addTableColumn" << q.lastError().text() << q.lastQuery();
        return;
    }
    mTableColumns[table] << column;
}

void AbstractDatabaseAdapter::createOTypeTable() const
{
    QSet<QString> columns;
    columns << "startNode" << "endNode" << "typeLabel";
    QHash<QString, QString> columnTypes;
    columnTypes["startNode"] = integerType();
    columnTypes["endNode"] = integerType();
    columnTypes["typeLabel"] = stringType();

    createTable( "otype", columns, columnTypes );

    QSqlQuery q(QSqlDatabase::database(mConnectionName));
    QString queryString(createOTypeTableQueryString());
    if( !q.prepare(queryString) ) {
        qWarning() << "AbstractDatabaseAdapter::createOTypeTable" << q.lastError().text() << queryString;
        return;
    }

    QHashIterator<QString,QPair<unsigned int,unsigned int>> i( mOTypeRanges );
    while (i.hasNext()) {
        i.next();
        q.bindValue(":startNode", i.value().first);
        q.bindValue(":endNode", i.value().second);
        q.bindValue(":typeLabel",i.key());
        if( !q.exec() )
            qWarning() << "AbstractDatabaseAdapter::createOTypeTable" << q.lastError().text() << q.executedQuery();
    }
}
