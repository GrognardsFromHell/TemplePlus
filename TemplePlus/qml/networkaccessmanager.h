
#pragma once

#include <QQmlNetworkAccessManagerFactory>
#include <QtNetwork/QNetworkAccessManager>


class TPNetworkAccessManagerFactory : public QQmlNetworkAccessManagerFactory {
public:
	QNetworkAccessManager* create(QObject *parent) override;
};

/**
* This extends the network access manager to retrieve content from TIO.
*/
class TPNetworkAccessManager : public QNetworkAccessManager {
Q_OBJECT
public:
	TPNetworkAccessManager(QObject *parent);
protected:
	QNetworkReply *createRequest(Operation op, const QNetworkRequest &req, QIODevice *outgoingData) override;
};
