#include "reader.h"
#include "abstractdatabaseadapter.h"

#include <QString>
#include <QSetIterator>
#include <QTimer>

Reader::Reader(const QString &folderPath, AbstractDatabaseAdapter *db) : mDb(db), mFolder(folderPath)
{
    mFilesToSkip << "otype.tf" << "otext.tf" << "omap@2017-2021.tf" << "omap@c-2021.tf";
}

Reader::~Reader()
{
}

void Reader::loadData()
{
    mDb->beginTransaction();

    /// get the otypes
    processOtypeFile();

    mDb->setOtypeRanges( mOTypeRanges );

    /// load the files and read the relevant header-type information
    QFileInfoList fileList = mFolder.entryInfoList(QStringList("*.tf"),QDir::Files);
    foreach(QFileInfo info, fileList) {
        if( ! mFilesToSkip.contains( info.fileName() ) ) {
            mFiles << TFFile(info);
        }
    }

    /// create tables based on what was collected in the first pass
    createTables();


    for(int i=0; i<mFiles.count(); i++) {
        QElapsedTimer timer;
        timer.start();
        qInfo().noquote() << "Reading:" << mFiles.at(i).label();
        mFiles[i].addDataToDatabase(mDb);
        qDebug() << "Completed in" << timer.elapsed() << "milliseconds";
    }

    mDb->commitTransaction();
}

void Reader::processOtypeFile()
{
    QString path = mFolder.absoluteFilePath("otype.tf");
    QFile file( path );
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "File could not be opened: " << path;
    }

    QTextStream in(&file);

    // skip past header
    QString ln;
    do {
        ln = in.readLine();
    } while( ln.length() > 0 && ln.at(0) == "@" );

    QRegExp rx("^(.*)-(.*)\t(.*)$");
    do {
        ln = in.readLine();
        int pos = rx.indexIn(ln);
        if (pos > -1) {
            mOTypeRanges[ rx.cap(3) ] = QPair<unsigned int,unsigned int>( rx.cap(1).toUInt(), rx.cap(2).toUInt() );
        }
    } while( !in.atEnd() );
}

void Reader::createTables()
{
    /// NB: this able works differently from the others
    mDb->createOTypeTable();

    /// first the node tables
    foreach( QString otype, mOTypeRanges.keys() ) { /// word, book, chapter, clause... etc. Each will be a different table.
        QSet<QString> node_columns;
        QHash<QString, QString> node_columnTypes;
        QString dataType;

        node_columns << "_id";
        node_columnTypes["_id"] = "int primary key"; /// this works for both SQLite and MySQL
        mDb->createTable( otype, node_columns, node_columnTypes );
    }

    /// then the edge tables
    for(int i=0; i<mFiles.count(); i++) {
        if( mFiles.at(i).fileType() == TFFile::FileTypeEdge ) {
            QSet<QString> edge_columns;
            QHash<QString, QString> edge_columnTypes;

            edge_columns << "value";
            if( mFiles.at(i).valueType() == TFFile::ValueTypeInteger ) {
                edge_columnTypes["value"] = mDb->integerType();
            } else if( mFiles.at(i).valueType() == TFFile::ValueTypeString ) {
                edge_columnTypes["value"] = mDb->stringType();
            }

            edge_columns << "from_node" << "to_node";
            edge_columnTypes["from_node"] =  mDb->integerType();
            edge_columnTypes["to_node"] =  mDb->integerType();

            /// edge tables should be labled with the filename
            mDb->createTable( mFiles.at(i).label(), edge_columns, edge_columnTypes );
        }
    }

    /// TODO @config table?
}
