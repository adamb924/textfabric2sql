#ifndef READER
#define READER

#include <QtSql>
#include <QHash>
#include <QSet>
#include <QPair>

#include "tffile.h"

class DatabaseAdapter;

class Reader{
public:
    Reader(const QString & folderPath, AbstractDatabaseAdapter * db);
    ~Reader();

    void loadData();

private:
    void processOtypeFile();
    void createTables();

    QStringList mFilesToSkip;
    QHash<QString,QPair<unsigned int,unsigned int>> mOTypeRanges;

    QList<TFFile> mFiles;

    AbstractDatabaseAdapter * mDb;
    QDir mFolder;
};


#endif // READER

