#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlTableModel>

#include <memory>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    std::shared_ptr<QSqlDatabase> db = nullptr;

private slots:
    void on_connect_clicked();

private:
    Ui::MainWindow *ui;

    QString getLogin();
    QString getPassword();
    void openWindow();
};

#endif // MAINWINDOW_H
