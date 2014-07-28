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

private:
    Ui::MainWindow *ui;
    QString unfilteredText;
    QString lastQuery;
    QPalette normalPalette;
    QPalette readOnlyPalette;
};

#endif // MAINWINDOW_H
