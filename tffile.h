#ifndef TFFILE_H
#define TFFILE_H

#include <QString>
#include <QFileInfo>
#include <QTextStream>

class Reader;
class AbstractDatabaseAdapter;

class TFFile
{
    friend Reader;
public:
    enum ValueType { ValueTypeString, ValueTypeInteger };
    enum FileType { FileTypeNode, FileTypeEdge, FileTypeConfig };

    explicit TFFile(const QFileInfo & info);
    ~TFFile();

    QFileInfo info() const;

    FileType fileType() const;
    ValueType valueType() const;

    /// filename minus the extension
    QString label() const;

    void addDataToDatabase( AbstractDatabaseAdapter * db );

    static unsigned int max(QSet<unsigned int> set);
    static QString unescape(QString string);
    static QSet<unsigned int> nodeRangeToSet(const QString & range);

    static FileType fileTypeFromString(const QString & str);
    static ValueType valueTypeFromString(const QString & str);

private:
    void addNodesToDatabase(AbstractDatabaseAdapter * db, QTextStream * stream );
    void addEdgesToDatabase(AbstractDatabaseAdapter *db, QTextStream * stream );
    void addConfigToDatabase(AbstractDatabaseAdapter * db, QTextStream * stream );

    /// read the first line of the file (@node, @edge) and return the string
    FileType readFileType(QTextStream *stream);

    /// read the values of @valueType from header
    ValueType readValueType(QTextStream * stream);

    void skipOverHeader(QTextStream * stream);

    bool getHasEdgeValues(QTextStream *stream);

private:
    QFileInfo mInfo;
    FileType mFileType;
    ValueType mValueType;
    bool mHasEdgeValues;
};

QDebug operator<<(QDebug dbg, const TFFile &key);

#endif // TFFILE_H
