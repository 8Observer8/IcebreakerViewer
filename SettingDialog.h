#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <QDialog>

namespace Ui {
class SettingDialog;
}

class SettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingDialog(QWidget *parent = 0);
    ~SettingDialog();

signals:
    void signalHostNameAndPortNumber(QString hostName, int portNumber);

private slots:
    void on_connectButton_clicked();

    void on_cancelButton_clicked();

private:
    Ui::SettingDialog *ui;
};

#endif // SETTINGDIALOG_H
