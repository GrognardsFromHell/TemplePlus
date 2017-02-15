
#pragma once

#include <QtQml/QQmlNetworkAccessManagerFactory>

class TPNetworkAccessManagerFactory : public QQmlNetworkAccessManagerFactory {
public:
	QNetworkAccessManager* create(QObject *parent) override;
};

