#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTextStream>
#include <QTimer>
#include <QRegExp>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(1000);
    connect(ui->lineEdit, SIGNAL(textEdited(QString)), timer, SLOT(start()));
    connect(timer, SIGNAL(timeout()), this, SLOT(on_lineEdit_editingFinished()));
    connect(ui->checkBoxRegex, SIGNAL(clicked()), SLOT(on_lineEdit_editingFinished()));
    connect(ui->checkBoxIgnoreCase, SIGNAL(clicked()), SLOT(on_lineEdit_editingFinished()));
    normalPalette = ui->plainTextEdit->palette();
    readOnlyPalette = normalPalette;
    readOnlyPalette.setColor(QPalette::Base, palette().color(QPalette::Window));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_lineEdit_editingFinished()
{
    const QString &query = ui->lineEdit->text();

    // Don't repeat the search if query has not changed. Note that if the slot
    // was called because the "Ignore case" or "Regular expression" checkbox
    // was toggled, then we need to repeat the search EVEN IF the query is
    // unchanged, that is why we check that "sender() == timer" (this means
    // the slot was indeed called because the user has finished typing.
    if (query == lastQuery && sender() == timer) {
        return;
    }

    if (query.isEmpty()) {
        ui->plainTextEdit->setReadOnly(false);
        ui->plainTextEdit->setPalette(normalPalette);
        ui->plainTextEdit->setPlainText(unfilteredText);
    } else {
        if (lastQuery.isEmpty()) {
            ui->plainTextEdit->setReadOnly(true);
            ui->plainTextEdit->setPalette(readOnlyPalette);
            unfilteredText = ui->plainTextEdit->toPlainText();
        }

        QString filteredText;
        QTextStream stream(&unfilteredText);

        if (ui->checkBoxRegex->isChecked()) {
            QRegExp regex(query);
            regex.setCaseSensitivity(ui->checkBoxIgnoreCase->isChecked()
                                     ? Qt::CaseInsensitive : Qt::CaseSensitive);
            while (!stream.atEnd()) {
                const QString &line = stream.readLine();
                if (line.contains(regex)) {
                    filteredText.append(line + "\n");
                }
            }
        } else {
            while (!stream.atEnd()) {
                const QString &line = stream.readLine();
                if (line.contains(query)) {
                    filteredText.append(line + "\n");
                }
            }
        }

        ui->plainTextEdit->setPlainText(filteredText);
    }

    lastQuery = query;
}

void MainWindow::on_checkBoxWordWrap_toggled(bool checked)
{
    ui->plainTextEdit->setLineWrapMode(checked ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
}
