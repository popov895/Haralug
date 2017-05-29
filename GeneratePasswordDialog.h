#ifndef GENERATEPASSWORDDIALOG_H
#define GENERATEPASSWORDDIALOG_H

#include "ui_GeneratePasswordDialog.h"

// GeneratePasswordDialog
class GeneratePasswordDialog : public QDialog, private Ui::GeneratePasswordDialog
{
    Q_OBJECT

public:
    explicit GeneratePasswordDialog(QWidget *parent = Q_NULLPTR);

    QString password() const { return lineEditPassword->text(); }

private: // GUI
    Q_SLOT void on_lineEditPassword_textChanged(const QString &text);
    Q_SLOT void on_buttonGenerate_clicked();
};

#endif // GENERATEPASSWORDDIALOG_H
