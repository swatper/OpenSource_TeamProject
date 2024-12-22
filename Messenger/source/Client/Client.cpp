#include "Client.h"
#include "ui_Client.h"

Client::Client(QWidget *parent) : QMainWindow(parent), ui(new Ui::Client){
    ui->setupUi(this);
    socket = new QTcpSocket(this);
    fd_flag = connectToHost("127.0.0.1"); // localhost
    if (!fd_flag){
        ui->textEdit->insertPlainText("Socket connect fail\n");
        ui->label2->setText("Disconnected");
        ui->pushButton3->setEnabled(true);
    }
    else{
        ui->label2->setText("Connected");
        ui->pushButton3->setEnabled(false);
    }
}
Client::~Client(){
    delete ui;
}
bool Client::connectToHost(QString host){
    socket->connectToHost(host, 8888); // ip, port
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    return socket->waitForConnected();
}
bool Client::writeData(QByteArray data){
    if (socket->state() == QAbstractSocket::ConnectedState){
        socket->write(data); // write the data itself
        return socket->waitForBytesWritten();
    }
    else{
        return false;
    }
}
void Client::on_pushButton_clicked(){
    if (fd_flag){
        QString sendData = ui->lineEdit->text();
        QString fileName = ui->label->text();
        if(fileName != "File is Empty"){
            QFile file(fileName);
            //파일 열기 
            if (file.open(QIODevice::ReadOnly)) {
                QByteArray fData = file.readAll();
                file.close();
                QString header = QString("FILE:%1:%2\n\n")
                      .arg(QFileInfo(fileName).fileName())
                      .arg(fData.size());
                QByteArray data = header.toUtf8() + fData;
                send_flag = writeData(data);
                if (send_flag) {
                    ui->textEdit->insertPlainText("You sent a file: " + fileName + "\n");
                } 
                else {
                    ui->textEdit->insertPlainText("File send fail\n");
                }
            } 
        }
        if(!sendData.isEmpty()){
            QString message = QString("MESSAGE:%1\n").arg(sendData);
            send_flag = writeData(message.toUtf8());
            if (send_flag) {
                // 자신의 화면에 직접 메시지 표시
                ui->textEdit->insertPlainText("You: " + sendData + "\n");
            } else {
                ui->textEdit->insertPlainText("Message send fail\n");
            }
            ui->lineEdit->setText("");
        }
        ui->lineEdit->setEnabled(true);
        ui->label->setText("File is Empty");
        if (sendData.isEmpty() && fileName == "File is Empty")
            ui->textEdit->insertPlainText("Socket send fail\n");
    }
}

void Client::on_pushButton2_clicked(){
    QString fileName = QFileDialog::getOpenFileName(this, "파일 선택", "", "모든 파일 (*);;텍스트 파일 (*.txt)");
    if (!fileName.isEmpty()) {
        ui->label->setText(fileName);
        ui->lineEdit->setText("");
        ui->lineEdit->setEnabled(false);
    }else{
        ui->label->setText("File is Empty");
        ui->lineEdit->setEnabled(true);
    }
}

void Client::on_pushButton3_clicked(){
    fd_flag = connectToHost("127.0.0.1"); // localhost
    if (!fd_flag){
        ui->textEdit->insertPlainText("Socket connect fail\n");
        ui->label2->setText("Disconnected");
        ui->pushButton3->setEnabled(true);
    }
    else{
        ui->label2->setText("Connected");
        ui->pushButton3->setEnabled(false);
    }
}

void Client::readyRead() {
    static QByteArray buffer;

    while (socket->bytesAvailable() > 0) {
        buffer.append(socket->readAll());

        // 파일 데이터 처리
        if (buffer.startsWith("FILE:")) {
            int headerEnd = buffer.indexOf("\n\n");
            if (headerEnd != -1) {
                QByteArray header = buffer.mid(0, headerEnd);
                QList<QByteArray> headerParts = header.split(':');
                if (headerParts.size() < 4) { // FILE:Client <ID>:<FileName>:<FileSize>
                    ui->textEdit->insertPlainText("Invalid file header format\n");
                    buffer.clear();
                    return;
                }

                QString clientId = headerParts[1].trimmed();  // "Client 1"에서 ID 추출
                QString fileName = QString(headerParts[2]);   // 받은 파일명
                int fileSize = headerParts[3].toInt();        // 파일 크기

                QByteArray fileData = buffer.mid(headerEnd + 2);
                if (fileData.size() >= fileSize) { // 충분한 데이터를 수신한 경우
                    // 화면에 출력
                    ui->textEdit->insertPlainText("File received: received_" + clientId + ": " + fileName + "\n");

                    // 파일을 로컬에 저장 (received_<FileName>)
                    QFile file("received_" + fileName);
                    if (file.open(QIODevice::WriteOnly)) {
                        file.write(fileData.left(fileSize)); // 파일 데이터를 저장
                        file.close();
                        ui->textEdit->insertPlainText("File saved as: received_" + fileName + "\n");
                    } else {
                        ui->textEdit->insertPlainText("Failed to save file: received_" + fileName + "\n");
                    }

                    buffer.remove(0, headerEnd + 2 + fileSize);
                } else {
                    // 파일 데이터가 아직 부족하면 대기
                    return;
                }
            }
        }
        // 메시지 데이터 처리
        else if (buffer.startsWith("MESSAGE:")) {
            int messageEnd = buffer.indexOf("\n");
            if (messageEnd != -1) {
                QString message = QString(buffer.mid(8, messageEnd - 8).trimmed());
                ui->textEdit->insertPlainText(message + "\n");

                buffer.remove(0, messageEnd + 1);
            }
        } else {
            ui->textEdit->insertPlainText("Unknown data format, clearing buffer\n");
            buffer.clear();
        }
    }
}

int main(int argc, char **argv){
    QApplication app(argc, argv);
    Client *window = new Client();
    window->show();
    return app.exec();
}