#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlTableModel>

#include <memory>

namespace Ui {
class AdminWindow;
}

class AdminWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AdminWindow(std::shared_ptr<QSqlDatabase> dataBase, QWidget *parent = 0);
    ~AdminWindow();

private slots:
    void on_registerButton_clicked();

    void on_evictButton_clicked();

    void on_checkInButton_clicked();

    void on_requestButton_clicked();

    void on_guestsView_doubleClicked(const QModelIndex &index);

private:
    Ui::AdminWindow *ui;

    QSqlTableModel* model1;
    QSqlTableModel* model2;
    std::shared_ptr<QSqlDatabase> db = nullptr;

    void updateTables();
};

#endif // ADMINWINDOW_H
