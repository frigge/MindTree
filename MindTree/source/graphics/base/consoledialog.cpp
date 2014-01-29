#include "consoledialog.h"
#include "QTextEdit"
#include "QVBoxLayout"

ConsoleDialog::ConsoleDialog(QWidget *parent)
    : QDialog(parent)
{
    QVBoxLayout *lay = new QVBoxLayout();
    lay->addWidget(new QTextEdit(this));
    setLayout(lay);
}

ConsoleDialog::~ConsoleDialog()
{
    
}
