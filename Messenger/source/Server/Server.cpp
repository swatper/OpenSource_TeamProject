#include "Server.h"
#include "ui_Server.h"

Server::Server(QWidget *parent): QMainWindow(parent),ui(new Ui::Server){
    ui->setupUi(this);
    server = new QTcpServer(this);
    connect(server, SIGNAL(newConnection()), this,
            SLOT(newConnection()));
    QString socket_data;
    socket_data.sprintf("Listening: %s\n", server->listen(QHostAddress::Any, 8888) ? "true" : "false");
    ui->textEdit->insertPlainText(socket_data);
}
Server::~Server(){
    delete ui;
}
void Server::newConnection(){
    while (server->hasPendingConnections()){
        QTcpSocket *socket = server->nextPendingConnection();
        clients.push_back(socket);
        socket->setObjectName(QString::number(cnt));
        ui->textEdit->insertPlainText("Connected from " + QString::number(cnt++) + "\r\n");
        connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
        connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
        QByteArray *buffer = new QByteArray();
        buffers.insert(socket, buffer);
    }
}
void Server::disconnected(){
    QTcpSocket *socket = static_cast<QTcpSocket *>(sender());
    QByteArray *buffer = buffers.value(socket);
    // 클라이언트 삭제
    ui->textEdit->insertPlainText("Client " + socket->objectName() + " disconnected.\n");
    // 목록에서 클라이언트 제거
    clients.erase(remove(clients.begin(), clients.end(), socket), clients.end());
    buffers.remove(socket);
    delete buffer;             // 버퍼 메모리 해제
    socket->deleteLater();     // 소켓 메모리 해제
}

void Server::readyRead() {
    QTcpSocket *socket = static_cast<QTcpSocket *>(sender());
    QByteArray *buffer = buffers.value(socket);

    while (socket->bytesAvailable() > 0) {
        buffer->append(socket->readAll());

        if (buffer->startsWith("FILE:")) { // 파일 데이터 처리
            int headerEnd = buffer->indexOf("\n\n");
            if (headerEnd != -1) {
                QByteArray header = buffer->mid(0, headerEnd);
                QList<QByteArray> headerParts = header.split(':');
                if (headerParts.size() < 3) {
                    buffer->clear();
                    return;
                }

                QString fileName = QString(headerParts[1]); // 파일명
                int fileSize = headerParts[2].toInt();      // 파일 크기

                QByteArray fileData = buffer->mid(headerEnd + 2);
                if (fileData.size() >= fileSize) {
                    QByteArray broadcastData = "FILE:Client " + socket->objectName().toUtf8() +
                                               ":" + fileName.toUtf8() +
                                               ":" + QByteArray::number(fileSize) + "\n\n";
                    broadcastData.append(fileData.left(fileSize));

                    for (QTcpSocket *client : clients) {
                        if (client != socket) { // 송신자는 제외
                            client->write(broadcastData);
                        }
                    }

                    buffer->remove(0, headerEnd + 2 + fileSize);
                    ui->textEdit->insertPlainText("File broadcasted from Client " +
                                                  socket->objectName() + ": " + fileName + "\n");
                } else {
                    return; // 파일 데이터가 아직 부족하면 대기
                }
            }
        } else if (buffer->startsWith("MESSAGE:")) { // 메시지 데이터 처리
            int messageEnd = buffer->indexOf("\n");
            if (messageEnd != -1) {
                QByteArray messageData = buffer->mid(8, messageEnd - 8); // "MESSAGE:" 이후 메시지 내용
                QByteArray broadcastData = "MESSAGE:Client " + socket->objectName().toUtf8() +
                                           ": " + messageData + "\n";

                for (QTcpSocket *client : clients) {
                    if (client != socket) { // 송신자는 제외
                        client->write(broadcastData);
                    }
                }

                buffer->remove(0, messageEnd + 1);
                ui->textEdit->insertPlainText("Message broadcasted from Client " +
                                              socket->objectName() + ": " + QString(messageData) + "\n");
            } else {
                return; // 메시지가 아직 완전하지 않음
            }
        } else {
            buffer->clear(); // 알 수 없는 데이터 형식은 무시
        }
    }
}

int main(int argc, char **argv){
    QApplication app(argc, argv);
    Server *window = new Server();
    window->show();
    return app.exec();
}