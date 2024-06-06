#include "network.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>

#include "data.h"
#include "commands.h"
#include "mainwindow.h"

///////////////////////////////////////////////////////////////////////
/// SERVER                                                          ///
///////////////////////////////////////////////////////////////////////

NetworkServer::NetworkServer(Data * data, quint16 port, QObject *parent) :
    QObject(parent),
    mData(data),
    mWebSocketServer(new QWebSocketServer(QStringLiteral("Wargame Server"),
                                          QWebSocketServer::NonSecureMode, this)),
    mMessageIndex(0)
{
    mInfo = tr("Network server created.");
    emit infoUpdated(mInfo);
    qDebug() << mInfo;

    if (mWebSocketServer->listen(QHostAddress::Any, port)) {
        mInfo = QString("Network listening on port ") + QString::number(port);
        qDebug() << mInfo;
        emit infoUpdated(mInfo);
        connect(mWebSocketServer, &QWebSocketServer::newConnection,
                this, &NetworkServer::onNewConnection);
    }

    mId = QStringLiteral("Admin");
}


NetworkServer::~NetworkServer()
{
    mWebSocketServer->close();
    qDeleteAll(mClients.begin(), mClients.end());
}

//! [onNewConnection]
void NetworkServer::onNewConnection()
{
    QWebSocket *pSocket = mWebSocketServer->nextPendingConnection();

    connect(pSocket, &QWebSocket::textMessageReceived, this, &NetworkServer::processTextMessage);
    //    connect(pSocket, &QWebSocket::binaryMessageReceived, this, &NetworkServer::processBinaryMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &NetworkServer::socketDisconnected);

    mClients << pSocket;
}
//! [onNewConnection]

//! [processTextMessage]
void NetworkServer::processTextMessage(QString message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());

    mInfo = QString("Server received a message : ") + message;
    emit infoUpdated(mInfo);

    if (pClient) {
        QStringList sl = message.split(QChar(';'));

        QString playerId = sl[0].mid(7);
        int messageIndex = sl[1].mid(8).toInt();
        if(sl[3] == QString("RequestCurrentState")){
            qDebug() << "RequestCurrentState";
            pClient->sendTextMessage(wrapReply(QString("ShareCurrentState;") + mData->serializeData(false), messageIndex));
        }
        if(sl[3] == QString("NotifyOrders")){
            qDebug() << "NotifyOrders";
            qDebug().noquote() << sl[5];


            QJsonDocument doc = QJsonDocument::fromJson(sl[5].toUtf8());

            // check validity of the document
            if(!doc.isNull())
            {
                if(doc.isObject())
                {

                    QJsonObject obj = doc.object();
                    qDebug() << "Received orders " << obj;
                    mData->loadAdditionalOrders(obj);
                    //                    QJsonArray intentionArray = obj["intentions"].toArray();
                    //                    for(QJsonValue intentionValue : intentionArray){
                    //                        QJsonObject intention = intentionValue.toObject();
                    //                        Drawing drawing;
                    //                        drawing.read(intention);
                    //                        mData->createDrawing(drawing);
                    //                    }

                    //                    QJsonObject obj = doc.object();
                    //                    QJsonArray movementArray = obj["movement"].toArray();

                    //                    for(QJsonValue value : movementArray){
                    //                        QJsonObject lineAndId = value.toObject();
                    //                        qDebug().noquote() << lineAndId;
                    //                        QJsonArray line = lineAndId["line"].toArray();
                    //                        QPointF end;
                    //                        for(QJsonValue point : line){
                    //                            QPointF current = QPointF(point.toArray()[0].toDouble(), point.toArray()[1].toDouble());
                    //                            end = current;
                    //                        }


                    //                        int uniqueId = lineAndId["uniqueId"].toDouble();

                    //                        qDebug() << "Moving " << uniqueId << " to " << end;

                    //                        Unit * unit = mData->units()[uniqueId];
                    //                        Unit copy = Unit(*unit);
                    //                        copy.setPos(end);
                    //                        UpdateUnitCommand * command = new UpdateUnitCommand(mData, copy);
                    //                        dynamic_cast<MainWindow*>(parent())->undoStack()->push(command);

                    //                    }
                }
                else
                {
                    qDebug() << "Document is not an object" << endl;
                }
            }
            else
            {
                qDebug() << "Invalid JSON..." << endl;
            }
        }

    }
}
//! [processTextMessage]

//! [processBinaryMessage]
//void NetworkServer::processBinaryMessage(QByteArray message)
//{
//    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
//    if (mDebug)
//        qDebug() << "Binary Message received:" << message;
//    if (pClient) {
//        pClient->sendBinaryMessage(message);
//    }
//}
//! [processBinaryMessage]

//! [socketDisconnected]
void NetworkServer::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    mInfo = QString("Socket Disconnected from server.");
    emit infoUpdated(mInfo);
    if (pClient) {
        mClients.removeAll(pClient);
        pClient->deleteLater();
    }
}

void NetworkServer::processRequestCurrentState(const QString &message)
{

}

void NetworkServer::processFullState(const QString &message)
{

}

void NetworkServer::pushCurrentState()
{
    QString currentState = mData->serializeData();
    for(QWebSocket * pClient : mClients){
        pClient->sendTextMessage(wrapMessage(QString("ShareCurrentState;") + currentState));
    }
}

void NetworkServer::pushUnitUpdate()
{
    QString currentUnitsState = mData->serializeUnits();
    for(QWebSocket * pClient : mClients){
        pClient->sendTextMessage(wrapMessage(QString("UpdateUnits;") + currentUnitsState));
    }
    QString time = mData->getCurrentTime().toString("hh:mm:ss");
    for(QWebSocket * pClient : mClients){
        pClient->sendTextMessage(wrapMessage(QString("UpdateTime;") + time));
    }
}

void NetworkServer::pushTimeCountdown(int seconds)
{
    for(QWebSocket * pClient : mClients){
        pClient->sendTextMessage(wrapMessage(QString("StartCountdown;") + QString::number(seconds)));
    }
}

QString NetworkServer::wrapReply(const QString &coreMessage, int requestIndex)
{
    QString message = mId + ';';
    message += "Message_" + QString::number(mMessageIndex) + ';';
    message += QStringLiteral("Reply_") + QString::number(requestIndex) + ';';
    message += coreMessage;
    message += ';';
    mMessageIndex++;
    return message;
}

QString NetworkServer::wrapMessage(const QString &coreMessage)
{
    QString message = QString("Admin;");
    message += "Message_" + QString::number(mMessageIndex) + ';';
    message += QStringLiteral("Spontaneous;");
    message += coreMessage;
    message += ';';
    mMessageIndex++;
    return message;
}
//! [socketDisconnected]






///////////////////////////////////////////////////////////////////////
/// CLIENT                                                          ///
///////////////////////////////////////////////////////////////////////

NetworkClient::NetworkClient(Data * data, const QUrl &url, QObject *parent) :
    QObject(parent),
    mData(data),
    m_url(url),
    mMessageIndex(0)
{
    qDebug() << "WebSocket client:" << url;
    connect(&m_webSocket, &QWebSocket::connected, this, &NetworkClient::onConnected);

    mId = dynamic_cast<MainWindow*>(parent)->sessionName();
}

NetworkClient::~NetworkClient()
{

}

void NetworkClient::connectToServer(bool b)
{
    if(b){
        mInfo = QString("Connecting...");
        emit infoUpdated(mInfo);
        m_webSocket.open(m_url);
    }
    else{
        mInfo = QString("Disconnected.");
        emit infoUpdated(mInfo);
        m_webSocket.close();
    }
}

void NetworkClient::disconnectFromServer(bool b)
{

}



//! [onConnected]
void NetworkClient::onConnected()
{
    qDebug() << "WebSocket connected";
    connect(&m_webSocket, &QWebSocket::textMessageReceived,
            this, &NetworkClient::onTextMessageReceived);
    //    connect(&m_webSocket, &QWebSocket::binaryMessageReceived,
    //            this, &NetworkClient::onBinaryMessageReceived);

    requestCurrentState();
}
//! [onConnected]

//! [onTextMessageReceived]
void NetworkClient::onTextMessageReceived(QString message)
{
    //    mInfo = QString("Message received:") + message;
    //    emit infoUpdated(mInfo);


    QStringList sl = message.split(QChar(';'));

    QString admin = sl[0];                        // Admin
    int messageIndex = sl[1].mid(8).toInt();      // Message_
    if(sl[3] == QString("ShareCurrentState")){
        qDebug() << "ShareCurrentState";
        QJsonDocument doc = QJsonDocument::fromJson(sl[4].toUtf8());

        // check validity of the document
        if(!doc.isNull())
        {
            if(doc.isObject())
            {
                QJsonObject obj = doc.object();
                mData->loadJson(obj);
                MainWindow * mainWindow = dynamic_cast<MainWindow*>(parent());
                mainWindow->undoStack()->clear();
                if(!mainWindow->adminView()){
                    mainWindow->setPlayerView(mainWindow->playerView());
                }
                //                LoadCommand * loadCommand = new LoadCommand(mData, obj);
                //                dynamic_cast<MainWindow*>(parent())->undoStack()->push(loadCommand);
            }
            else
            {
                qDebug() << "Document is not an object" << endl;
            }
        }
        else
        {
            qDebug() << "Invalid JSON..." << endl;
        }

    }
    if(sl[3] == QString("UpdateUnits")){
        qDebug() << "Update units";
        QJsonDocument doc = QJsonDocument::fromJson(sl[4].toUtf8());

        // check validity of the document
        if(!doc.isNull())
        {
            if(doc.isObject())
            {
                QJsonObject obj = doc.object();
                mData->loadUpdateToUnits(obj);
                mData->loadOrders(QJsonObject());
                MainWindow * mainWindow = dynamic_cast<MainWindow*>(parent());
                mainWindow->undoStack()->clear();
                if(!mainWindow->adminView()){
                    mainWindow->setPlayerView(mainWindow->playerView());
                }
            }
            else
            {
                qDebug() << "Document is not an object" << endl;
            }
        }
        else
        {
            qDebug() << "Invalid JSON..." << endl;
        }
    }

    if(sl[3] == QString("UpdateTime")){
        qDebug() << "Update time : " << sl[4];
        QDateTime datetime = QDateTime::fromString(sl[4], "hh:mm:ss");
        qDebug() << "Time received " << datetime;
        mData->setCurrentTime(datetime);
        mData->emitCurrentTime();
    }

    if(sl[3] == QString("StartCountdown")){
        int nbSeconds = sl[4].toInt();
        MainWindow * mainWindow = dynamic_cast<MainWindow*>(parent());
        mainWindow->startCountdown(nbSeconds);
    }
}

void NetworkClient::requestCurrentState()
{
    m_webSocket.sendTextMessage(wrapMessage("RequestCurrentState"));
}

void NetworkClient::requestFullState()
{
    m_webSocket.sendTextMessage(wrapMessage("RequestFullState"));
}

void NetworkClient::requestCustomMessage(const QString &message)
{
    m_webSocket.sendTextMessage(wrapMessage(message));
}

void NetworkClient::pushOrders()
{

    QJsonObject orders;
    mData->writeOrders(orders);
    QJsonDocument ordersDoc = QJsonDocument(orders);

    qDebug() << "PLAYER SENDS " << ordersDoc.toJson(QJsonDocument::Compact);
    m_webSocket.sendTextMessage(wrapMessage(QString("NotifyOrders;")) + ordersDoc.toJson(QJsonDocument::Compact));
}

QString NetworkClient::wrapMessage(const QString &coreMessage)
{
    QString message = QStringLiteral("Player_") + mId + ';';
    message += "Message_" + QString::number(mMessageIndex) + ';';
    message += QStringLiteral("Spontaneous;");
    message += coreMessage;
    message += ';';
    mMessageIndex++;
    return message;
}

QString NetworkClient::id() const
{
    return mId;
}

void NetworkClient::setId(const QString &id)
{
    mId = id;
}

//void NetworkClient::onBinaryMessageReceived(const QByteArray &message)
//{
//    //    labelMain->setText(QString("Binary received"));
//    QPixmap pixmap;
//    pixmap.loadFromData(message, "JPG");
//    pixmap = pixmap.scaledToHeight(600);
//    labelMain->setPixmap(pixmap);
//    static int i=1;
//    m_webSocket.sendTextMessage(QStringLiteral("Gimme your screenshot! #") + QString::number(i));
//    i++;

//}
//! [onTextMessageReceived]
