#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include "ui_AboutDialog.h"

// AboutDialog
class AboutDialog : public QDialog, private Ui::AboutDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = Q_NULLPTR);
};

#endif // ABOUTDIALOG_H
