#include "mysqldatabaseadapter.h"

#include <QtSql>

MySqlDatabaseAdapter::MySqlDatabaseAdapter(const QString &hostname, const QString &databasename, const QString &username, const QString &password) : AbstractDatabaseAdapter(hostname+databasename)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", mConnectionName);
    db.setHostName(hostname);
    db.setDatabaseName(databasename);
    db.setUserName(username);
    db.setPassword(password);
    if(!db.open())
    {
        qCritical() << "There was a problem in opening the database. The program said: " + db.lastError().databaseText();
        return;
    }

    QSqlQuery q(QSqlDatabase::database(mConnectionName));
    if( ! q.exec("SET AUTOCOMMIT=0;")  ) {
        qWarning() << "MySqlDatabaseAdapter::MySqlDatabaseAdapter" << q.lastError().text() << q.lastQuery();
    }
}

MySqlDatabaseAdapter::~MySqlDatabaseAdapter()
{

}

QString MySqlDatabaseAdapter::insertNodeDataQueryString(const QString &table, const QString &column) const
{
    return "INSERT INTO `"+table+"` (`"+column+"`,`_id`) VALUES (:value,:id) ON DUPLICATE KEY UPDATE `"+column+"`=VALUES(`"+column+"`);";
}

QString MySqlDatabaseAdapter::insertEdgeDataQueryString(const QString &table) const
{
    return "INSERT INTO `"+table+"` (`from_node`,`to_node`,`value`) VALUES (:from,:to,:value);";
}

QString MySqlDatabaseAdapter::createTableQueryString(const QString &table, const QSet<QString> &columns, const QHash<QString, QString> &columnTypes) const
{
    QString query = "CREATE TABLE `" + table + "` ( ";
    QSetIterator<QString> i(columns);
    while(i.hasNext()) {
        QString c = i.next();
        query += " `" + c + "` " + columnTypes.value(c, stringType() );
        if( i.hasNext() ) {
            query += ", ";
        }
    }
    query += ");";
    return query;
}

QString MySqlDatabaseAdapter::dropTableQueryString(const QString &table) const
{
    return "DROP TABLE IF EXISTS `" + table + "`;";
}

QString MySqlDatabaseAdapter::addTableColumnQueryString(const QString &table, const QString &column, const QString &columnType) const
{
    return "ALTER TABLE `" + table + "` ADD `" + column + "` "+columnType+";";
}

QString MySqlDatabaseAdapter::createOTypeTableQueryString() const
{
    return "INSERT INTO otype (`startNode`, `endNode`, `typeLabel`) VALUES (:startNode,:endNode,:typeLabel);";
}

QString MySqlDatabaseAdapter::integerType() const
{
    return "INT";
}

QString MySqlDatabaseAdapter::stringType() const
{
    return "VARCHAR(255)";
}

QMap<QString, QString> MySqlDatabaseAdapter::parseConnectionString(const QString &connectionString)
{
    QMap<QString, QString> params;

    const QStringList pairs = connectionString.split(';');
    for (const QString& pair : pairs) {
        const QStringList keyValue = pair.split('=');
        if (keyValue.size() == 2) {
            params[keyValue[0]] = keyValue[1];
        }
    }

    return params;
}
