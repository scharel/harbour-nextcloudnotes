#ifndef ACCOUNTHASH_H
#define ACCOUNTHASH_H
#include <QObject>
#include <QCryptographicHash>

class AccountHash : public QObject {
    Q_OBJECT
public:
    Q_INVOKABLE QByteArray hash(const QString username, const QString url) {
        return QCryptographicHash::hash(QString("%1@%2").arg(username).arg(url).toUtf8(), QCryptographicHash::Sha256);
    }
};

#endif // ACCOUNTHASH_H
