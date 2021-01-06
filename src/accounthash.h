#ifndef ACCOUNTHASH_H
#define ACCOUNTHASH_H
#include <QDebug>
#include <QObject>
#include <QCryptographicHash>

class AccountHash : public QObject {
    Q_OBJECT
public:
    Q_INVOKABLE QByteArray hash(const QString username, const QString url) {
        QByteArray data = QString("%1@%2").arg(username).arg(url).toUtf8();
        QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Md5);
        return hash.toHex();
    }
};

#endif // ACCOUNTHASH_H
