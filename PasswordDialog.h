#ifndef PASSWORDDIALOG_H
#define PASSWORDDIALOG_H

#include "ui_PasswordDialog.h"

// PasswordDialog
class PasswordDialog : public QDialog, private Ui::PasswordDialog
{
    Q_OBJECT

public:
    explicit PasswordDialog(QWidget *parent = Q_NULLPTR);

private: // GUI
    Q_SLOT void on_buttonGeneratePassword_clicked();
    Q_SLOT void on_buttonOk_clicked();
};

#endif // PASSWORDDIALOG_H
