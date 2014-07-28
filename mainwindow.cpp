#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTextStream>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(1000);
    connect(ui->lineEdit, SIGNAL(textEdited(QString)), timer, SLOT(start()));
    connect(timer, SIGNAL(timeout()), this, SLOT(on_lineEdit_editingFinished()));
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
    if (query == lastQuery) {
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
        while (!stream.atEnd()) {
            const QString &line = stream.readLine();
            if (line.contains(query)) {
                filteredText.append(line + "\n");
            }
        }

        ui->plainTextEdit->setPlainText(filteredText);
    }

    lastQuery = query;
}
