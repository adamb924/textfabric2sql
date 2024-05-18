#include "sqlitedatabaseadapter.h"

#include <QtSql>

SqliteDatabaseAdapter::SqliteDatabaseAdapter(const QString & filename) : AbstractDatabaseAdapter(filename)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", mConnectionName);
    db.setDatabaseName(mConnectionName);
    db.setHostName("hostname");
    if(!db.open())
    {
        qCritical() << "There was a problem in opening the database. The program said: " + db.lastError().databaseText();
        return;
    }
    db.exec("PRAGMA TEMP_STORE = MEMORY;");
    db.exec("PRAGMA JOURNAL_MODE = OFF;");
    db.exec("PRAGMA SYNCHRONOUS = OFF;");
    db.exec("PRAGMA LOCKING_MODE = EXCLUSIVE;");
    db.exec("PRAGMA encoding=\"UTF-8\";");
}

SqliteDatabaseAdapter::~SqliteDatabaseAdapter()
{
}

QString SqliteDatabaseAdapter::insertNodeDataQueryString(const QString &table, const QString &column) const
{
    return "INSERT INTO `"+table+"` ('"+column+"','_id') VALUES (:value,:id) ON CONFLICT (_id) DO UPDATE SET '"+column+"'=excluded.'"+column+"';";
}

QString SqliteDatabaseAdapter::insertEdgeDataQueryString(const QString &table) const
{
    return "INSERT INTO `"+table+"` ('from_node','to_node','value') VALUES (:from,:to,:value);";
}

QString SqliteDatabaseAdapter::createTableQueryString(const QString &table, const QSet<QString> &columns, const QHash<QString, QString> &columnTypes) const
{
    QString query = "CREATE TABLE `" + table + "` ( ";
    QSetIterator<QString> i(columns);
    while(i.hasNext()) {
        QString c = i.next();
        query += " \"" + c + "\" " + columnTypes.value(c,"TEXT");
        if( i.hasNext() ) {
            query += ", ";
        }
    }
    query += ");";
    return query;
}

QString SqliteDatabaseAdapter::dropTableQueryString(const QString &table) const
{
    return "DROP TABLE IF EXISTS `" + table + "`;";
}

QString SqliteDatabaseAdapter::addTableColumnQueryString(const QString &table, const QString &column, const QString &columnType) const
{
    return "ALTER TABLE `" + table + "` ADD \"" + column + "\" "+columnType+";";
}

QString SqliteDatabaseAdapter::createOTypeTableQueryString() const
{
    return "INSERT INTO otype (startNode, endNode, typeLabel) VALUES (:startNode,:endNode,:typeLabel);";
}

QString SqliteDatabaseAdapter::integerType() const
{
    return "int";
}

QString SqliteDatabaseAdapter::stringType() const
{
    return "text";
}
