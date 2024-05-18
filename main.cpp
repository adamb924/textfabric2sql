#include <QCoreApplication>
#include <QtDebug>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QCommandLineParser>

#include "reader.h"
#include "mysqldatabaseadapter.h"
#include "sqlitedatabaseadapter.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("textfabric2sql");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Read TextFabric data into SQL databases.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("path-to-data", QCoreApplication::translate("main", "Path to folder containing .tf files (e.g., otype.tf)."));
    parser.addPositionalArgument("which-sql", QCoreApplication::translate("main", "sqlite | mysql"));
    parser.addPositionalArgument("connection-string", QCoreApplication::translate("main", "For sqlite, a filename, for MySQL, a string like this: hostname=myhost;databasename=mydatabase;username=myuser;password=mypassword"));

    parser.process(a);
    const QStringList args = parser.positionalArguments();
    if( args.count() < 3 )
    {
        parser.showHelp();
    }
    const QString dataPath = args.at(0);
    const QString whichSql = args.at(1);
    const QString connectionString = args.at(2);

    AbstractDatabaseAdapter * db;

    if( whichSql == "sqlite" )
    {
        db = new SqliteDatabaseAdapter(connectionString);
    }
    else if ( whichSql == "mysql" )
    {
        const QMap<QString, QString> params = MySqlDatabaseAdapter::parseConnectionString(connectionString);

        if( params.value("hostname").isEmpty() || params.value("databasename").isEmpty() || params.value("username").isEmpty() || params.value("password").isEmpty() )
        {
            parser.showHelp();
        }

        const QString hostname = params.value("hostname");
        const QString databasename = params.value("databasename");
        const QString username = params.value("username");
        const QString password = params.value("password");
        db = new MySqlDatabaseAdapter(hostname, databasename, username, password);
    }
    else
    {
        parser.showHelp();
    }

    if( !db->isOpen() )
    {
        qCritical() << "The database did not open. Check your parameters and try again.";
        return -1;
    }
    else
    {
        qInfo() << "Database opened.";
    }

    Reader r(dataPath, db);
    r.loadData();

    delete db;

    return 0;
}
