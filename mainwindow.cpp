#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTextStream>
#include <QTimer>
#include <QRegExp>
#include <QSettings>

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
    ui->plainTextEdit->installEventFilter(this);
    loadSettings();
}

MainWindow::~MainWindow()
{
    saveSettings();
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

// QPlainTextEdit::ZoomIn is not available in Qt4
// so we need to use a wrapper function.
static void zoomIn(QPlainTextEdit *w)
{
#if QT_VERSION >= 0x050000
    w->zoomIn();
#else
    QFont f = w->font();
    const int newSize = f.pointSize() + 1;
    if (newSize <= 0)
        return;
    f.setPointSize(newSize);
    w->setFont(f);
#endif
}

// QPlainTextEdit::ZoomOut is not available in Qt4
// so we need to use a wrapper function.
static void zoomOut(QPlainTextEdit *w)
{
#if QT_VERSION >= 0x050000
    w->zoomOut();
#else
    QFont f = w->font();
    const int newSize = f.pointSize() - 1;
    if (newSize <= 0)
        return;
    f.setPointSize(newSize);
    w->setFont(f);
#endif
}

void MainWindow::on_lineEdit_editingFinished()
{
    const QString &query = ui->lineEdit->text();

    // We need this for a corner case: if the user enters some text in the text
    // box below (but NOT in the query box), and then clicks on the Regex or
    // IngCase checkbox, the text will disappear. This here 'if' takes care of
    // that corner-case.
    if (query.isEmpty() && lastQuery.isEmpty()) {
        return;
    }

    // Don't repeat the search if query has not changed. Note that if the slot
    // was called because the "Ignore case" or "Regular expression" checkbox
    // was toggled, then we need to repeat the search EVEN IF the query is
    // unchanged, that is why we check that "sender() == timer" (this means
    // the slot was indeed called because the user has finished typing.
    if (query == lastQuery && !sender()->inherits("QCheckBox")) {
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

                // Some regular expressions can match even an empty string.
                // Examples: "|", "a|", etc. We do not want that, because it
                // will cause the program to endlessly insert <span></span>,
                // until its memory usage rises and eventually it crashes. So
                // if we ever match an empty pattern, we treat the regex as
                // invalid and abort.
                if (len <= 0) {
                    filteredText = "";
                    goto label_exit_double_loop;
                }

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

label_exit_double_loop:
        ui->plainTextEdit->setPlainText("");
        ui->plainTextEdit->appendHtml(filteredText);
    }

    lastQuery = query;
}

void MainWindow::on_checkBoxWordWrap_toggled(bool checked)
{
    ui->plainTextEdit->setLineWrapMode(checked ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    // Enable Zoom In/Zoom Out on Ctrl+Wheel.
    if(obj == ui->plainTextEdit && event->type() == QEvent::Wheel ) {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        if( wheelEvent->modifiers() == Qt::ControlModifier ){
            if (wheelEvent->delta() > 0) {
                zoomIn(ui->plainTextEdit);
            } else {
                zoomOut(ui->plainTextEdit);
            }
            // Since we handled this event, return true so it doesn't get sent
            // to the plain text edit.
            return true;
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::loadSettings()
{
    QSettings settings;
    ui->checkBoxIgnoreCase->setChecked(settings.value("IgnoreCase", true).toBool());
    ui->checkBoxRegex->setChecked(settings.value("Regex", true).toBool());
    ui->checkBoxWordWrap->setChecked(settings.value("WordWrap", false).toBool());
}

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.setValue("IgnoreCase", ui->checkBoxIgnoreCase->isChecked());
    settings.setValue("Regex", ui->checkBoxRegex->isChecked());
    settings.setValue("WordWrap", ui->checkBoxWordWrap->isChecked());
}
