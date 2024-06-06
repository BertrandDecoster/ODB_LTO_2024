#ifndef NETWORK_H
#define NETWORK_H

#include <QObject>

#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>

class Data;

///////////////////////////////////////////////////////////////////////
/// SERVER                                                          ///
///////////////////////////////////////////////////////////////////////



class NetworkServer : public QObject
{
    Q_OBJECT
public:
    explicit NetworkServer(Data *data, quint16 port, QObject *parent = nullptr);
    virtual ~NetworkServer();

public slots:
    void onNewConnection();
    void processTextMessage(QString message);
    //    void processBinaryMessage(QByteArray message);
    void socketDisconnected();

    void processRequestCurrentState(const QString & message);
    void processFullState(const QString & message);
    void pushCurrentState();
    void pushUnitUpdate();
    void pushTimeCountdown(int seconds);

private:
    QString wrapReply(const QString & coreMessage, int requestIndex);
    QString wrapMessage(const QString & coreMessage);

signals:
    void infoUpdated(const QString & message);

private:
    Data * mData;

    QWebSocketServer *mWebSocketServer;
    QList<QWebSocket *> mClients;
    QString mInfo;

    QString mId;
    quint64 mMessageIndex;
};


///////////////////////////////////////////////////////////////////////
/// CLIENT                                                          ///
///////////////////////////////////////////////////////////////////////

class NetworkClient : public QObject
{
    Q_OBJECT

public:
    explicit NetworkClient(Data * data, const QUrl &url, QObject *parent = Q_NULLPTR);
    virtual ~NetworkClient();


    QString id() const;
    void setId(const QString &id);

public slots:
    void connectToServer(bool b = true);
    void disconnectFromServer(bool b = false);
    void onConnected();
    void onTextMessageReceived(QString message);
    //    void onBinaryMessageReceived(const QByteArray & message);

    void requestCurrentState();
    void requestFullState();
    void requestCustomMessage(const QString & message);
    void pushOrders();

private:
    QString wrapMessage(const QString & coreMessage);

signals:
    void infoUpdated(const QString & message);


private:
    Data * mData;


    QWebSocket m_webSocket;
    QUrl m_url;
    QString mInfo;

    QString mId;
    quint64 mMessageIndex;


};

#endif // NETWORK_H
