#include "GeneratePasswordDialog.h"

#include "PasswordGenerator.h"
#include "Utils.h"

// GeneratePasswordDialog

GeneratePasswordDialog::GeneratePasswordDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
}

void GeneratePasswordDialog::on_lineEditPassword_textChanged(const QString &text)
{
    buttonOk->setEnabled(!text.isEmpty());
    progressBar->setValue(Utils::passwordStrength(text));
}

void GeneratePasswordDialog::on_buttonGenerate_clicked()
{
    PasswordGenerator::Options options;

    if (checkBoxDigits->isChecked())
        options |= PasswordGenerator::DigitsOption;

    if (checkBoxMinus->isChecked())
        options |= PasswordGenerator::MinusOption;

    if (checkBoxUnderline->isChecked())
        options |= PasswordGenerator::UnderlineOption;

    if (checkBoxSpace->isChecked())
        options |= PasswordGenerator::SpaceOption;

    if (checkBoxSpecialSymbols->isChecked())
        options |= PasswordGenerator::SpecialOption;

    if (checkBoxBrackets->isChecked())
        options |= PasswordGenerator::BracketsOption;

    lineEditPassword->setText(PasswordGenerator::generate(options, spinBoxPasswordLength->value()));
    lineEditPassword->setFocus();
    lineEditPassword->selectAll();
}
