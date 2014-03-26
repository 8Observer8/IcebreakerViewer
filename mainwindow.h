#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTimer>
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
    void readyRead();
    void displayError(QAbstractSocket::SocketError socketError);
    void sessionOpened();
    void sendTimeoutRequest();
    void sendRequest(const QString& request);
    void on_fillTableButton_clicked();
    void on_actionSettings_triggered();
    void on_actionViewSettings_triggered();
    void setSettinsOfConnection(QString hostName, int portNumber);
    void setHeaderData(QStandardItemModel *model);
    void updataTable(QStandardItemModel *model, QTableView *table, int rows, int columns, const QStringList &sensorsList);

    void on_getAvailableRangeButton_clicked();

private:
    Ui::MainWindow *ui;
    quint16 blockSize;
    QTcpSocket *mTcpSocket;
    QNetworkSession *networkSession;
    QTimer *mTimer;
    quint16 m_nextBlockSize;
    QStandardItemModel *mCurrentDataModel;
    QStandardItemModel *mRangeDataModel;
    QString mHostName;
    int mPortNumber;
    int mRowsForCurrent;
    int mColumnsForCurrent;
    int mRowsForRange;
    int mColumnsForRange;
};

#endif // MAINWINDOW_H
