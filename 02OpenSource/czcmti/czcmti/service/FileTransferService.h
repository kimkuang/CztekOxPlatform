#ifndef FILETRANSFERSERVICE_H
#define FILETRANSFERSERVICE_H
#include <QString>
#include <QFile>
#include <QCryptographicHash>
#include "czcmtidefs.h"

class FileTransferService
{
public:
    static FileTransferService &GetInstance();

    void SetFilePathName(const QString &filePathName);
    QString GetFilePathName() const;
    int SetFileURL(const QString &url);
    int WriteFileContent(uint offset, const uchar *buffer, int len);
    int ExecuteFileCommand(E_FileType fileType, const char *buffer, int len);
    int GetFileURL(QString &url, int encrypt, const char *cipherText = "");
    int ReadFileContent(uint &offset, uchar *content, uint &contentLen);

private:
    static FileTransferService m_fileTransferService;
    FileTransferService();
    FileTransferService(const FileTransferService &) = delete;
    FileTransferService &operator=(const FileTransferService &) = delete;

private:
    enum {
        ENCRYPTION_TYPE_NONE = 0,
        ENCRYPTION_TYPE_DES3 = 1
    };
    uint m_encryptionType;
    uint m_transferFileLength;
    QString m_inputMd5Digest;
    QString m_password;
    QString m_filePathName;
    QString m_tempFilePathName; // temp file
    QFile m_file;
    QList<uint> m_receivedOffsetList;
    QCryptographicHash m_md5Algo;

    void reset();
    int writeDataSafely(QFile &file, const char *buffer, int len);
    int readDataSafely(QFile &file, char *buffer, int len);
};

#endif // FILETRANSFERSERVICE_H
