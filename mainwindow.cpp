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

// This will work for both Qt4 and Qt5.
static QString escapeHtml(QString s)
{
#if QT_VERSION >= 0x050000
    return s.toHtmlEscaped();
#else
    return Qt::escape(s);
#endif
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

        bool useRegex = ui->checkBoxRegex->isChecked();
        QRegExp regex;
        if (useRegex) {
            regex.setPattern(query);
            regex.setCaseSensitivity(ui->checkBoxIgnoreCase->isChecked()
                                     ? Qt::CaseInsensitive : Qt::CaseSensitive);
        }

        while (!stream.atEnd()) {
            const QString &line = stream.readLine();

            int pos = 0;
            bool hasMatch = false;

            // We will highlight all the matches using a yellow background color.
            // Note that even if the search pattern appears more than once on the same line,
            // every occurence will be highlighted in yellow, not just the first one.
            for(;;) {
                int nextPos = useRegex ? regex.indexIn(line, pos) : line.indexOf(query, pos);
                if (-1 == nextPos) {
                    break;
                }

                if (!hasMatch) {
                    hasMatch = true;
                }

                // Add anything to the left of the matched string.
                int len = useRegex ? regex.matchedLength() : query.size();
                filteredText.append(escapeHtml(line.mid(pos, nextPos - pos)));

                // Add the matched string itself (in yellow background).
                filteredText.append("<span style=\"background:#FFFF00\">");
                filteredText.append(escapeHtml(line.mid(nextPos, len)));
                filteredText.append("</span>");

                pos = nextPos + len;
            }

            if (hasMatch) {
                filteredText.append(escapeHtml(line.mid(pos)));
                filteredText.append("<br>");
            }
        }

        ui->plainTextEdit->setPlainText("");
        ui->plainTextEdit->appendHtml(filteredText);
    }

    lastQuery = query;
}

void MainWindow::on_checkBoxWordWrap_toggled(bool checked)
{
    ui->plainTextEdit->setLineWrapMode(checked ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
}
