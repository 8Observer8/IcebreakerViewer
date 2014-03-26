#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtWidgets>
#include <QtNetwork>
#include <QFileInfo>
#include <SettingDialog.h>
#include <QtSql>
#include <QtNetwork>
#include <memory>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_nextBlockSize(0),
    mHostName(""),
    mPortNumber(0)
{
    ui->setupUi(this);

    mRowsForCurrent = 0;
    mColumnsForCurrent = 2;
    mCurrentDataModel = new QStandardItemModel(mRowsForCurrent, mColumnsForCurrent, this);
    setHeaderData(mCurrentDataModel);
    ui->currentTable->setModel(mCurrentDataModel);

    mRowsForRange = 0;
    mColumnsForRange = 2;
    mRangeDataModel = new QStandardItemModel(mRowsForRange, mColumnsForRange, this);
    setHeaderData(mRangeDataModel);
    ui->rangeTable->setModel(mRangeDataModel);

    QNetworkConfigurationManager manager;
    if (manager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired) {
        // Get saved network configuration
        QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
        settings.beginGroup(QLatin1String("QtNetwork"));
        const QString id = settings.value(QLatin1String("DefaultNetworkConfiguration")).toString();
        settings.endGroup();

        // If the saved network configuration is not currently discovered use the system default
        QNetworkConfiguration config = manager.configurationFromIdentifier(id);
        if ((config.state() & QNetworkConfiguration::Discovered) !=
                QNetworkConfiguration::Discovered) {
            config = manager.defaultConfiguration();
        }

        networkSession = new QNetworkSession(config, this);
        connect(networkSession, SIGNAL(opened()), this, SLOT(sessionOpened()));

        networkSession->open();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readyRead()
{
    QTcpSocket* clientSocket = (QTcpSocket*)sender();
    QDataStream in(clientSocket);
    in.setVersion(QDataStream::Qt_4_7);
    for (;;) {
        if (!m_nextBlockSize) {
            if (clientSocket->bytesAvailable() < (int)sizeof(quint16)) {
                break;
            }
            in >> m_nextBlockSize;
        }

        if (clientSocket->bytesAvailable() < m_nextBlockSize) {
            break;
        }

        QString inputData;
        in >> inputData;

        QStringList sensorsList = inputData.split(';');
        QStringList command = sensorsList[0].split(',');

        if (command[0] == QString("alailableRange")) {
            if (sensorsList.size() == 2) {
                QStringList command = sensorsList[1].split(',');
                if (command.size() == 2) {
                    // Begin of Range
                    ui->beginPossibleLineEdit->setText(command[0]);
                    // End of Range
                    ui->endPossibleLineEdit->setText(command[1]);

                    ui->getAvailableRangeButton->setEnabled(true);
                } else {
                    QMessageBox::information(this, QString("Error"), QString("command.size() != 2"));
                }
            } else {
                QMessageBox::information(this, QString("Error"), QString("sensorsList.size() != 2"));
            }
        } else if(command[0] == QString("getRequiredRange")) {
            updataTable(mRangeDataModel, ui->rangeTable, mRowsForRange, mColumnsForRange, sensorsList);
            ui->fillTableButton->setEnabled(true);
        } else if (command[0] == QString("currentValues")) {
            if (sensorsList.size() > 1) {
                updataTable(mCurrentDataModel, ui->currentTable, mRowsForCurrent, mColumnsForCurrent, sensorsList);
            }
        } else {
            QMessageBox::information(this, QString("Error"), QString("Unknown command: %1").arg(command[0]));
        }

        //        QStringList stringList = str.split(',');
        //        if (stringList.size() == 5) {
        //            int value01 = stringList[0].toInt();
        //            int value02 = stringList[1].toInt();
        //            int value03 = stringList[2].toInt();
        //            int value04 = stringList[3].toInt();
        //            int value05 = stringList[4].toInt();

        //            ui->sensor01LineEdit->setText(stringList[0]);
        //            ui->sensor02LineEdit->setText(stringList[1]);
        //            ui->sensor03LineEdit->setText(stringList[2]);
        //            ui->sensor04LineEdit->setText(stringList[3]);
        //            ui->sensor05LineEdit->setText(stringList[4]);

        //            // Save to database
        //            if (database.isOpen()) {
        //                QSqlQuery query;
        //                QString strQuery(QString("INSERT INTO valuesOfSensors(value, data_time, sensor_name) VALUES ('%1','%2', 'value01')").arg(value01).arg(QDateTime::currentDateTime().toMSecsSinceEpoch()));
        //                if (!query.exec(strQuery)) {
        //                    QMessageBox::information(this, tr("Client"),
        //                                             tr("Wrong query."));
        //                    return;
        //                }

        //                strQuery = QString("INSERT INTO valuesOfSensors(value, data_time, sensor_name) VALUES ('%1','%2', 'value02')").arg(value02).arg(QDateTime::currentDateTime().toMSecsSinceEpoch());
        //                if (!query.exec(strQuery)) {
        //                    QMessageBox::information(this, tr("Client"),
        //                                             tr("Wrong query."));
        //                    return;
        //                }

        //                strQuery = QString("INSERT INTO valuesOfSensors(value, data_time, sensor_name) VALUES ('%1','%2', 'value03')").arg(value03).arg(QDateTime::currentDateTime().toMSecsSinceEpoch());
        //                if (!query.exec(strQuery)) {
        //                    QMessageBox::information(this, tr("Client"),
        //                                             tr("Wrong query."));
        //                    return;
        //                }

        //                strQuery = QString("INSERT INTO valuesOfSensors(value, data_time, sensor_name) VALUES ('%1','%2', 'value04')").arg(value04).arg(QDateTime::currentDateTime().toMSecsSinceEpoch());
        //                if (!query.exec(strQuery)) {
        //                    QMessageBox::information(this, tr("Client"),
        //                                             tr("Wrong query."));
        //                    return;
        //                }

        //                strQuery = QString("INSERT INTO valuesOfSensors(value, data_time, sensor_name) VALUES ('%1','%2', 'value05')").arg(value05).arg(QDateTime::currentDateTime().toMSecsSinceEpoch());
        //                if (!query.exec(strQuery)) {
        //                    QMessageBox::information(this, tr("Client"),
        //                                             tr("Wrong query."));
        //                    return;
        //                }

        //                // Time of first data
        //                strQuery = QString("SELECT data_time FROM valuesOfSensors LIMIT 1");
        //                if (!query.exec(strQuery)) {
        //                    QMessageBox::information(this, tr("Client"),
        //                                             tr("Wrong query."));
        //                    return;
        //                } else {
        //                    if (query.next()) {
        //                        QDateTime time;
        //                        time.setMSecsSinceEpoch(query.value(0).toString().toULongLong());
        //                        ui->beginPossibleLineEdit->setText(time.toString());
        //                    }
        //                }

        //                // Time of last data
        //                strQuery = QString("SELECT data_time FROM valuesOfSensors WHERE ID = (SELECT MAX(ID) FROM valuesOfSensors);");
        //                if (!query.exec(strQuery)) {
        //                    QMessageBox::information(this, tr("Client"),
        //                                             tr("Wrong query."));
        //                    return;
        //                } else {
        //                    if (query.next()) {
        //                        QDateTime time;
        //                        time.setMSecsSinceEpoch(query.value(0).toULongLong());
        //                        ui->endPossibleLineEdit->setText(time.toString());
        //                    }
        //                }
        //            } else {
        //                QMessageBox::information(this, tr("Client"),
        //                                         tr("No connection to Database."));
        //                return;
        //            }
        //        } else {
        //            QMessageBox::information(this, tr("Client"),
        //                                     tr("stringList.size() != 5"));
        //            return;
        //        }

        m_nextBlockSize = 0;
    }
}

void MainWindow::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, tr("Client"),
                                 tr("The host was not found. Please check the "
                                    "host name and port settings."));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, tr("Client"),
                                 tr("The connection was refused by the peer. "
                                    "Make sure the server is running, "
                                    "and check that the host name and port "
                                    "settings are correct."));
        break;
    default:
        QMessageBox::information(this, tr("Client"),
                                 tr("The following error occurred: %1.")
                                 .arg(mTcpSocket->errorString()));
    }
}

void MainWindow::sessionOpened()
{
    // Save the used configuration
    QNetworkConfiguration config = networkSession->configuration();
    QString id;
    if (config.type() == QNetworkConfiguration::UserChoice) {
        id = networkSession->sessionProperty(QLatin1String("UserChoiceConfiguration")).toString();
    } else {
        id = config.identifier();
    }

    QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
    settings.beginGroup(QLatin1String("QtNetwork"));
    settings.setValue(QLatin1String("DefaultNetworkConfiguration"), id);
    settings.endGroup();
}

void MainWindow::on_fillTableButton_clicked()
{
    QString beginRequired = ui->beginRequiredLineEdit->text();
    QString endRequired = ui->endRequiredLineEdit->text();

    if (beginRequired.isEmpty() || endRequired.isEmpty()) {
        QMessageBox::information(this, tr("Client"),
                                 tr("You must fill text-field begin and end of the range"));
        return;
    }

    QDateTime beginDateTime;
    beginDateTime = QDateTime::fromString(beginRequired);

    QDateTime endDateTime;
    endDateTime = QDateTime::fromString(endRequired);

    // Check dateTime
    if (!beginDateTime.isValid()) {
        QMessageBox::information(this, tr("Client"),
                                 tr("Begin of the range is not valid."));
        return;
    }

    if (!endDateTime.isValid()) {
        QMessageBox::information(this, tr("Client"),
                                 tr("End of the range is not valid."));
        return;
    }

    QString request = QString("getRequiredRange,0;%1,%2").arg(beginRequired).arg(endRequired);
    if (!mHostName.isEmpty()) {
        sendRequest(request);
    } else {
        QMessageBox::information(this, tr("Error"),
                                 tr("You need to connect with server."));
        return;
    }

    ui->fillTableButton->setEnabled(false);

    //    qint64 begin = beginDateTime.toMSecsSinceEpoch();
    //    qint64 end = endDateTime.toMSecsSinceEpoch();

    //    int ncols = 3;
    //    int nrows = 0;
    //    model = new QStandardItemModel(nrows, ncols, this);

    //    QSqlQuery query;
    //    QString strQuery = QString("SELECT * FROM valuesOfSensors WHERE data_time>='%1' AND data_time<='%2';").arg(begin).arg(end);
    //    if (!query.exec(strQuery)) {
    //        QMessageBox::information(this, tr("Client"),
    //                                 tr("Wrong query."));
    //        return;
    //    } else {
    //        QList<QStandardItem*> items;
    //        int j = 0;
    //        while (query.next()) {
    //            items.clear();
    //            for (int i = 1; i < 4; i++) {
    //                QStandardItem *item = new QStandardItem(QString(query.value(i).toString()));
    //                items.push_back(item);
    //            }
    //            j++;
    //            model->appendRow(items);
    //        }
    //    }

    //    table = new QTableView;
    //    ui->table->setModel(model);
}

void MainWindow::on_actionSettings_triggered()
{
    SettingDialog *dialog = new SettingDialog(this);
    connect(dialog, SIGNAL(signalHostNameAndPortNumber(QString,int)), this, SLOT(setSettinsOfConnection(QString,int)));
    dialog->show();
}

void MainWindow::on_actionViewSettings_triggered()
{
    QMessageBox::information(this, QString("Information"), QString("Host Name: %1\nPort Number: %2").arg(mHostName).arg(mPortNumber));
}

void MainWindow::setSettinsOfConnection(QString hostName, int portNumber)
{
    mHostName = hostName;
    mPortNumber = portNumber;

    mTcpSocket = new QTcpSocket(this);

    connect(mTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));
    connect(mTcpSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));

    mTcpSocket->connectToHost(hostName, portNumber);

    mTimer = new QTimer(this);
    mTimer->start(1000);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(sendTimeoutRequest()));

    blockSize = 0;
}

void MainWindow::setHeaderData(QStandardItemModel *model)
{
    model->setHeaderData(0, Qt::Horizontal, QString("Sensor"));
    model->setHeaderData(1, Qt::Horizontal, QString("Value Of Sensor"));
    //mModel->setHeaderData(2, Qt::Horizontal, QString("Name of Port"));
}

void MainWindow::updataTable(QStandardItemModel *model, QTableView *table, int rows, int columns, const QStringList &sensorsList)
{
    if ((sensorsList.size()-1) != model->rowCount()) {
        delete model;
        rows = sensorsList.size()-1;
        model = new QStandardItemModel(rows, columns, this);
        setHeaderData(model);
        table->setModel(model);
    }

    for (int i = 0; i < sensorsList.size()-1; ++i) {
        QStringList list = sensorsList[i+1].split(',');
        if (list.size() == model->columnCount()) {
            QModelIndex index;
            // Set data to the table
            for (int j = 0; j < list.size(); ++j) {
                index = model->index(i, j);
                model->setData(index, list[j]);
            }
        } else {
            QMessageBox::information(this, QString("Error"), QString("list.size() != model->columnCount()"));
        }
    }
}

void MainWindow::sendTimeoutRequest()
{
    QString request = QString("currentValues,0");
    sendRequest(request);
}

void MainWindow::sendRequest(const QString &request)
{
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);

    out << quint16(0) << request;

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    mTcpSocket->write(arrBlock);
}

void MainWindow::on_getAvailableRangeButton_clicked()
{
    QString request = QString("getAvailableRange");
    if (!mHostName.isEmpty()) {
        sendRequest(request);
    } else {
        QMessageBox::information(this, tr("Error"),
                                 tr("You need to connect with server."));
        return;
    }

    ui->getAvailableRangeButton->setEnabled(false);
}
