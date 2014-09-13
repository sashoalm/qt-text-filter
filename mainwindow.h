#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_lineEdit_editingFinished();

    void on_checkBoxWordWrap_toggled(bool checked);

private:
    bool eventFilter(QObject *obj, QEvent *event);

    Ui::MainWindow *ui;
    QString unfilteredText;
    QString lastQuery;
    QPalette normalPalette;
    QPalette readOnlyPalette;
    QTimer *timer;
};

#endif // MAINWINDOW_H
