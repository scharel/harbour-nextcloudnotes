#ifndef SSLCONFIGURATION_H
#define SSLCONFIGURATION_H

#include <QObject>
#include <QSslConfiguration>
#include <QSslSocket>

class SslConfiguration : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool checkCert READ checkCert WRITE setCheckCert NOTIFY checkCertChanged)
public:
    explicit SslConfiguration(QObject *parent = nullptr);

public slots:
    bool checkCert();
    void setCheckCert(bool check);

signals:
    void checkCertChanged(bool check);

private:
    bool _checkCert;
    QSslConfiguration checkCertConfig;
    QSslConfiguration noCheckConfig;
};

#endif // SSLCONFIGURATION_H
