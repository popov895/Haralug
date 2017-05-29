#include "PasswordDialog.h"

#include "GeneratePasswordDialog.h"
#include "Settings.h"
#include "Utils.h"

// PasswordDialog

PasswordDialog::PasswordDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    auto updateControls = [this] () {
        const QString password = lineEditPassword->text();
        buttonOk->setEnabled(!password.isEmpty() && (lineEditConfirmPassword->text() == password));
    };

    connect(lineEditPassword, &QLineEdit::textChanged, [this, updateControls] (const QString &text) {
        updateControls();
        progressBar->setValue(Utils::passwordStrength(text));
    });
    connect(lineEditConfirmPassword, &QLineEdit::textChanged, updateControls);
}

void PasswordDialog::on_buttonGeneratePassword_clicked()
{
    GeneratePasswordDialog dialog(this);
    if (dialog.exec()) {
        const QString password = dialog.password();
        lineEditPassword->setText(password);
        lineEditConfirmPassword->setText(password);
        buttonOk->setFocus();
    }
}

void PasswordDialog::on_buttonOk_clicked()
{
    Settings::instance().setPassword(lineEditPassword->text());
    accept();
}
