#ifndef CLIENT_H
#define CLIENT_H
#include <QMainWindow>
#include <QtCore>
#include <QtNetwork>
#include <QtWidgets>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>

struct FileData{
    QString fileName;
    qint64 fileSize;
    QByteArray data;
};

namespace Ui{
    class Client;
}
class Client : public QMainWindow{
    Q_OBJECT
public:
    explicit Client(QWidget *parent = nullptr);
    ~Client();
public slots:
    bool connectToHost(QString host);
    bool writeData(QByteArray data);
private slots:
    void on_pushButton_clicked();
    void on_pushButton2_clicked();
    void on_pushButton3_clicked();
private slots:
    void readyRead();
private:
    Ui::Client *ui;
    QTcpSocket *socket;
    bool fd_flag = false;
    bool send_flag = false;
};
#endif