#include "FileTransferService.h"
#include <QDebug>
#include <QFileInfo>
#include <QProcess>
#include <fstream>
#include "czcmtidefs.h"
#include "common/Global.h"
#include "utils/utils.h"

FileTransferService FileTransferService::m_fileTransferService;
FileTransferService &FileTransferService::GetInstance()
{
    return m_fileTransferService;
}

FileTransferService::FileTransferService() :
    m_md5Algo(QCryptographicHash::Md5)
{
}

void FileTransferService::SetFilePathName(const QString &filePathName)
{
    m_filePathName = filePathName;
    reset();
}

QString FileTransferService::GetFilePathName() const
{
    return m_filePathName;
}

int FileTransferService::SetFileURL(const QString &url)
{
    // Local file format: local:<filename>.<extension>;<encryption>;<length in hex>;<md5sum>;<cipher text>
    // Cipher text is empty if the file has not encrypted.
    QStringList strList = url.split(";", QString::KeepEmptyParts);
    if (strList.count() < 5) {
        qCritical("the ';' number %d is too small", strList.count());
        return ERR_InvalidParameter;
    }
    int idx = strList[0].indexOf(":");
    if (idx < 0) {
        qCritical("can't find ':'");
        return ERR_InvalidParameter;
    }
//    QString fileFullName = strList[0].mid(idx + 1);

    bool ok = false;
    m_encryptionType = strList[1].toUInt(&ok);
    if (!ok) {
        qCritical("get encryption type failed");
        return ERR_InvalidParameter;
    }
    if (m_encryptionType != ENCRYPTION_TYPE_NONE && m_encryptionType != ENCRYPTION_TYPE_DES3) {
        qCritical("not support encryption type [%d]", m_encryptionType);
        return ERR_InvalidParameter;
    }

    m_transferFileLength = strList[2].toUInt(&ok, 16);
    if (!ok) {
        qCritical("get file length failed");
        return ERR_InvalidParameter;
    }

    m_inputMd5Digest = strList[3];
    m_password = strList[4];
    if (m_encryptionType != ENCRYPTION_TYPE_NONE) {
        if (m_password.isEmpty()) {
            qCritical("intput password is empty");
            return ERR_InvalidParameter;
        }
    }

    m_tempFilePathName = QFileInfo(m_filePathName).path() + "/mytemp";
    m_file.setFileName(m_tempFilePathName);
    if (!m_file.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        qCritical() << QString("Open %1 failed.").arg(m_tempFilePathName);
        return ERR_Failed;
    }
    qDebug() << m_tempFilePathName << m_encryptionType << m_transferFileLength << m_inputMd5Digest << m_password;

    return ERR_NoError;
}

int FileTransferService::WriteFileContent(uint offset, const uchar *buffer, int len)
{
    if (!m_file.isOpen()) {
        qCritical("The file[%s] is not opened.", m_filePathName.toLatin1().data());
        return ERR_Failed;
    }
    if (m_file.pos() < offset) { // empty hole, failed
        qCritical("Input offset[%d] greater than file pos[%d], FAILED.", offset, (int)m_file.pos());
        return ERR_InvalidParameter;
    }
    else if (m_file.pos() > offset) { // rewrite
        qDebug("Input offset[%d] smaller than file pos[%d], rewrite...", offset, (int)m_file.pos());
        m_file.seek(offset);
    }

    // Protocol is 4 byte aligned, the last packet may have dirty data.
    if (offset + len > m_transferFileLength)
        len = m_transferFileLength - offset;

    if (writeDataSafely(m_file, (const char*)buffer, len) != len) {
        qCritical("Write file failed.");
        return ERR_Failed;
    }
    m_file.flush();
    if (!m_receivedOffsetList.contains(offset)) { // avoid recalculating md5
        m_md5Algo.addData((const char*)buffer, len);
        m_receivedOffsetList.push_back(offset);
    }
    if (offset + len == m_transferFileLength) { // finished
        QString md5Sum = QString(m_md5Algo.result().toHex());
        if (QString::compare(m_inputMd5Digest, md5Sum, Qt::CaseInsensitive) != 0) {
            qCritical("md5sum check failed.");
            m_file.close();
            return ERR_Failed;
        }        
        if (QFile::exists(m_filePathName) && !QFile::remove(m_filePathName)) { // delete the existed file
            qCritical("Removing file failed.");
            return ERR_Failed;
        }
        if (!m_file.rename(m_filePathName)) {
            qCritical("Rename operation file failed.");
            return ERR_Failed;
        }
        qInfo("Recieve file has completed.");
        m_file.close();
    }
    return ERR_NoError;
}

int FileTransferService::ExecuteFileCommand(E_FileType fileType, const char *buffer, int len)
{
    Q_UNUSED(buffer);
    Q_UNUSED(len);
//    QString command(buffer);
//    if (!command.contains("file:")) {
//        qCritical() << QString("input command %1 error").arg(command);
//        return ERR_InvalidParameter;
//    }

    QFileInfo fileInfo(m_filePathName);
    if (fileType == FileType_Upgrade) {
        if (ENCRYPTION_TYPE_DES3 == m_encryptionType) {
            qInfo("Execute upgrade command");
            QString command = QString("upgrader fs %1 %2 %3 %4").arg(fileInfo.absolutePath()).
                    arg(fileInfo.fileName()).arg(m_password).arg("noui");
            if (!QProcess::startDetached(command)) {
                qCritical("Call startDetached failed. [%s]", command.toLatin1().data());
                return ERR_Failed;
            }
        }
    }
    else if (fileType == FileType_Otp) {
        if (m_filePathName.endsWith(".zip")) {
            QString command = QString("unzip -o %1 -d %2").arg(m_filePathName).arg(fileInfo.absolutePath());
            int ret = QProcess::execute(command);
            qDebug("unzip process ret = %d", ret);
            if (ret != 0) {
                qCritical("Call startDetached failed. [%s]", command.toLatin1().data());
                return ERR_Failed;
            }
        }
    }
    return ERR_NoError;
}

int FileTransferService::GetFileURL(QString &url, int encrypt, const char *cipherText)
{
    m_file.setFileName(m_filePathName);
    if (!m_file.isOpen() && !m_file.open(QIODevice::ReadOnly)) {
        qCritical("The file[%s] is not opened.", m_filePathName.toLatin1().data());
        return ERR_Failed;
    }
    QString fileFullName = QFileInfo(m_filePathName).fileName();
    QString md5Digest;
    Utils::Md5SumFile(md5Digest, m_filePathName);
    m_transferFileLength = (uint)m_file.size();
    char szUrl[128];
    sprintf(szUrl, "local:%s;%d;0x%x;%s;%s", fileFullName.toLatin1().data(), encrypt, m_transferFileLength, md5Digest.toLatin1().data(), cipherText);
    url = QString::fromLatin1(szUrl);
    return ERR_NoError;
}

int FileTransferService::ReadFileContent(uint &offset, uchar *content, uint &contentLen)
{
    if (!m_file.isOpen()) {
        qCritical("The file[%s] is not opened.", m_filePathName.toLatin1().data());
        return ERR_Failed;
    }
    offset = (uint)m_file.pos();
    int toRead = contentLen;
    int hasRead = 0;
    if ((hasRead = readDataSafely(m_file, (char *)content, toRead)) == -1) {
        qCritical("Read file failed.");
        m_file.close();
        return ERR_Failed;
    }
    contentLen = hasRead;
    if (m_file.atEnd())
        m_file.close();
    return ERR_NoError;
}

void FileTransferService::reset()
{
    if (m_file.isOpen()) { // close last file
        m_file.close();
    }
    m_encryptionType = ENCRYPTION_TYPE_NONE;
    m_transferFileLength = 0;
    m_inputMd5Digest.clear();
    m_password.clear();
    m_receivedOffsetList.clear();
    m_md5Algo.reset();
}

int FileTransferService::writeDataSafely(QFile &file, const char *buffer, int len)
{
    int toWrite = len;
    int totalWrittenBytes = 0;
    while (toWrite > 0) {
        int writtenBytes = file.write(buffer + totalWrittenBytes, toWrite);
        if (writtenBytes >= 0) {
            totalWrittenBytes += writtenBytes;
            toWrite -= writtenBytes;
        }
        else {
            qCritical()<<"Calling writeData() failed.";
            break;
        }
    }
    return totalWrittenBytes;
}

int FileTransferService::readDataSafely(QFile &file, char *buffer, int len)
{
    int toRead = len;
    int totalHasReadBytes = 0;
    while (!file.atEnd() && (toRead > 0)) {
        int hasReadBytes = file.read(buffer + totalHasReadBytes, toRead);
        if (hasReadBytes >= 0) {
            totalHasReadBytes += hasReadBytes;
            toRead -= hasReadBytes;
        }
        else {
            qCritical()<<"Calling readData() failed.";
            return -1;
        }
    }
    return totalHasReadBytes;
}
