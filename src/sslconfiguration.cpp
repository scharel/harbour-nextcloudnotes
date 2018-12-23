#include "sslconfiguration.h"

#include <QDebug>

SslConfiguration::SslConfiguration(QObject *parent) : QObject(parent), _checkCert(true) {
    checkCertConfig = noCheckConfig = QSslConfiguration::defaultConfiguration();
    noCheckConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    QSslConfiguration::setDefaultConfiguration(checkCertConfig);
}

bool SslConfiguration::checkCert() {
    return _checkCert;
}

void SslConfiguration::setCheckCert(bool check) {
    if (_checkCert != check) {
        qDebug() << "Changing SSL Cert check to" << check;
        _checkCert = check;
        emit checkCertChanged(_checkCert);
        QSslConfiguration::setDefaultConfiguration(_checkCert ? checkCertConfig : noCheckConfig);
    }
}
