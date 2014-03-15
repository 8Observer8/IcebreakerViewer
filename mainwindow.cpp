#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtWidgets>
#include <QtNetwork>
#include <QFileInfo>
#include <memory>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_nextBlockSize(0)
{
    ui->setupUi(this);

    // Database for values of sensors
    database = QSqlDatabase::addDatabase("QSQLITE");
    QString path = QDir::currentPath()+QString("/database.sqlite");
    database.setDatabaseName(path);

    QFileInfo file(path);

    if (file.isFile()) {
        if (!database.open()) {
            QMessageBox::information(this, tr("Client"),
                                     tr("Database File was not opened."));
        }
    } else {
        QMessageBox::information(this, tr("Client"),
                                 tr("Database File does not exist"));
    }

    // find out name of this machine
    QString name = QHostInfo::localHostName();
    if (!name.isEmpty()) {
        ui->hostCombo->addItem(name);
        QString domain = QHostInfo::localDomainName();
        if (!domain.isEmpty())
            ui->hostCombo->addItem(name + QChar('.') + domain);
    }
    if (name != QString("localhost"))
        ui->hostCombo->addItem(QString("localhost"));
    // find out IP addresses of this machine
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // add non-localhost addresses
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (!ipAddressesList.at(i).isLoopback())
            ui->hostCombo->addItem(ipAddressesList.at(i).toString());
    }
    // add localhost addresses
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i).isLoopback())
            ui->hostCombo->addItem(ipAddressesList.at(i).toString());
    }

    ui->portLineEdit->setFocus();

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

        ui->btnConnect->setEnabled(false);
        networkSession->open();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnConnect_clicked()
{
    ui->btnConnect->setEnabled(false);
    blockSize = 0;
    //tcpSocket->abort();

    tcpSocket = new QTcpSocket(this);

    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));

    tcpSocket->connectToHost(ui->hostCombo->currentText(),
                             ui->portLineEdit->text().toInt());

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(sendRequest()));
    timer->start(1000);
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

        QString str;
        in >> str;

        QStringList stringList = str.split(',');
        if (stringList.size() == 5) {
            int value01 = stringList[0].toInt();
            int value02 = stringList[1].toInt();
            int value03 = stringList[2].toInt();
            int value04 = stringList[3].toInt();
            int value05 = stringList[4].toInt();

            ui->sensor01LineEdit->setText(stringList[0]);
            ui->sensor02LineEdit->setText(stringList[1]);
            ui->sensor03LineEdit->setText(stringList[2]);
            ui->sensor04LineEdit->setText(stringList[3]);
            ui->sensor05LineEdit->setText(stringList[4]);

            // Save to database
            if (database.isOpen()) {
                QSqlQuery query;
                QString strQuery(QString("INSERT INTO valuesOfSensors(value, data_time, sensor_name) VALUES ('%1','%2', 'value01')").arg(value01).arg(QDateTime::currentDateTime().toMSecsSinceEpoch()));
                if (!query.exec(strQuery)) {
                    QMessageBox::information(this, tr("Client"),
                                             tr("Wrong query."));
                    return;
                }

                strQuery = QString("INSERT INTO valuesOfSensors(value, data_time, sensor_name) VALUES ('%1','%2', 'value02')").arg(value02).arg(QDateTime::currentDateTime().toMSecsSinceEpoch());
                if (!query.exec(strQuery)) {
                    QMessageBox::information(this, tr("Client"),
                                             tr("Wrong query."));
                    return;
                }

                strQuery = QString("INSERT INTO valuesOfSensors(value, data_time, sensor_name) VALUES ('%1','%2', 'value03')").arg(value03).arg(QDateTime::currentDateTime().toMSecsSinceEpoch());
                if (!query.exec(strQuery)) {
                    QMessageBox::information(this, tr("Client"),
                                             tr("Wrong query."));
                    return;
                }

                strQuery = QString("INSERT INTO valuesOfSensors(value, data_time, sensor_name) VALUES ('%1','%2', 'value04')").arg(value04).arg(QDateTime::currentDateTime().toMSecsSinceEpoch());
                if (!query.exec(strQuery)) {
                    QMessageBox::information(this, tr("Client"),
                                             tr("Wrong query."));
                    return;
                }

                strQuery = QString("INSERT INTO valuesOfSensors(value, data_time, sensor_name) VALUES ('%1','%2', 'value05')").arg(value05).arg(QDateTime::currentDateTime().toMSecsSinceEpoch());
                if (!query.exec(strQuery)) {
                    QMessageBox::information(this, tr("Client"),
                                             tr("Wrong query."));
                    return;
                }

                // Time of first data
                strQuery = QString("SELECT data_time FROM valuesOfSensors LIMIT 1");
                if (!query.exec(strQuery)) {
                    QMessageBox::information(this, tr("Client"),
                                             tr("Wrong query."));
                    return;
                } else {
                    if (query.next()) {
                        QDateTime time;
                        time.setMSecsSinceEpoch(query.value(0).toString().toULongLong());
                        ui->beginPossibleLineEdit->setText(time.toString());
                    }
                }

                // Time of last data
                strQuery = QString("SELECT data_time FROM valuesOfSensors WHERE ID = (SELECT MAX(ID) FROM valuesOfSensors);");
                if (!query.exec(strQuery)) {
                    QMessageBox::information(this, tr("Client"),
                                             tr("Wrong query."));
                    return;
                } else {
                    if (query.next()) {
                        QDateTime time;
                        time.setMSecsSinceEpoch(query.value(0).toULongLong());
                        ui->endPossibleLineEdit->setText(time.toString());
                    }
                }
            } else {
                QMessageBox::information(this, tr("Client"),
                                         tr("No connection to Database."));
                return;
            }
        } else {
            QMessageBox::information(this, tr("Client"),
                                     tr("stringList.size() != 5"));
            return;
        }

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
                                 .arg(tcpSocket->errorString()));
    }

    ui->btnConnect->setEnabled(true);
}

void MainWindow::sessionOpened()
{
    // Save the used configuration
    QNetworkConfiguration config = networkSession->configuration();
    QString id;
    if (config.type() == QNetworkConfiguration::UserChoice)
        id = networkSession->sessionProperty(QLatin1String("UserChoiceConfiguration")).toString();
    else
        id = config.identifier();

    QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
    settings.beginGroup(QLatin1String("QtNetwork"));
    settings.setValue(QLatin1String("DefaultNetworkConfiguration"), id);
    settings.endGroup();

}

void MainWindow::sendRequest()
{
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);
    out << quint16(0) << QString("currentValues");

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    tcpSocket->write(arrBlock);
}

void MainWindow::on_fillTableButton_clicked()
{
//    QString beginRequired = ui->beginRequiredLineEdit->text();
//    QString endRequired = ui->endRequiredLineEdit->text();

    QString beginRequired("Fri Mar 14 12:40:29 2014");
    QString endRequired("Fri Mar 14 12:45:35 2014");

    if (beginRequired.isEmpty() || endRequired.isEmpty()) {
        QMessageBox::information(this, tr("Client"),
                                 tr("You must fill test field begin and end of the range"));
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

//    qint64 begin = beginDateTime.toMSecsSinceEpoch();
//    qint64 end = endDateTime.toMSecsSinceEpoch();

    qint64 begin = 1394790029894;
    qint64 end = 1394790030697;

    // Check begin and end of the range
//    if (begin <= end) {
//        QMessageBox::information(this, tr("Client"),
//                                 tr("begin of the range must be less than end of the range"));
//        return;
//    }

    int ncols = 3;
    int nrows = 0;
    model = new QStandardItemModel(nrows, ncols, this);

    QSqlQuery query;
    QString strQuery = QString("SELECT * FROM valuesOfSensors WHERE data_time>='%1' AND data_time<='%2';").arg(begin).arg(end);
    if (!query.exec(strQuery)) {
        QMessageBox::information(this, tr("Client"),
                                 tr("Wrong query."));
        return;
    } else {
        QMessageBox::information(this, tr("Client"),
                                 tr("Good!"));
        //int fieldNo = query.record().indexOf("sensor_name");
        //QList<std::shared_ptr<QStandardItem> > items;
        QList<QStandardItem*> items;
        int j = 0;
        while (query.next()) {
            items.clear();
            for (int i = 1; i < 4; i++) {
                //std::shared_ptr<QStandardItem> ptrItem(new QStandardItem(QString(query.value(i).toString())));
                QStandardItem *item = new QStandardItem(QString(query.value(i).toString()));
                items.push_back(item);
                //QStandardItem *item = new QStandardItem(QString("Hello"));
                //model->setItem(j, i, item);
                //qDebug() << query.value(i).toString();
            }
            j++;
            qDebug() << items.size();
            model->appendRow(items);
        }

//        fieldNo = query.record().indexOf("value");
//        while (query.next()) {
//            qDebug() << query.value(fieldNo).toString();
//        }
//        qDebug() << "";

//        fieldNo = query.record().indexOf("data_time");
//        while (query.next()) {
//            qDebug() << query.value(fieldNo).toString();
//        }
//        qDebug() << "";

        //QList<QStandardItem *> items;
        //while (query.next()) {
//            qDebug() << query.value(fieldNo).toString();
//            QStandardItem *item = new QStandardItem(QString("hello"));
//            for (int i = 0; i < 3; ++i) {

//            }
//            if (query.size() == -1) {
//                QMessageBox::information(this, tr("Client"),
//                                         tr("Not found!"));
//                return;
//            }
//            qDebug() << query.size();
            //qDebug() << query.value(0).toString();
//            nrows++;
//        }
    }

    table = new QTableView;
//    qDebug() << model->rowCount();
//    qDebug() << model->columnCount();
    ui->table->setModel(model);

//    int nrows = 0;
//    ncols = 4;
//    model = new QStandardItemModel(nrows, ncols, this);

}
