#ifndef SERVER_H
#define SERVER_H
#include <QMainWindow>
#include <QtCore>
#include <QtNetwork>
#include <QString>
#include <vector>

using namespace std;

namespace Ui{
    class Server;
}
class Server : public QMainWindow{
    Q_OBJECT
public:
    explicit Server(QWidget *parent = nullptr);
    ~Server();
signals:
    void dataReceived(QByteArray);
private slots:
    void newConnection();
    void disconnected();
    void readyRead();
private:
    Ui::Server *ui;
    QTcpServer *server;
    QHash<QTcpSocket *, QByteArray *> buffers;
    bool send_flag = false;
    uint32_t cnt = 0;
    vector<QTcpSocket *> clients;
};

#endif 