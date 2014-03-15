#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTimer>
#include <QtSql>
#include <QStandardItemModel>
#include <QTableView>

QT_BEGIN_NAMESPACE
class QNetworkSession;
QT_END_NAMESPACE

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_btnConnect_clicked();
    void readyRead();
    void displayError(QAbstractSocket::SocketError socketError);
    void sessionOpened();
    void sendRequest();

    void on_fillTableButton_clicked();

private:
    Ui::MainWindow *ui;
    quint16 blockSize;
    QTcpSocket *tcpSocket;
    QNetworkSession *networkSession;
    QTimer *timer;
    quint16 m_nextBlockSize;
    QSqlDatabase database;
    QStandardItemModel *model;
    QTableView *table;
};

#endif // MAINWINDOW_H
