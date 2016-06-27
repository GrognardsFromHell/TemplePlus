
#pragma once

#include <QtQml/QQmlNetworkAccessManagerFactory>

class CustomNAMFactory : public QQmlNetworkAccessManagerFactory {
public:
	QNetworkAccessManager* create(QObject *parent) override;
};

