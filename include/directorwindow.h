#ifndef DIRECTORWINDOW_H
#define DIRECTORWINDOW_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlTableModel>

#include <memory>

namespace Ui {
class DirectorWindow;
}

class DirectorWindow : public QWidget
{
    Q_OBJECT

public:
    explicit DirectorWindow(std::shared_ptr<QSqlDatabase> dataBase, QWidget *parent = 0);
    ~DirectorWindow();

private slots:
    void on_hireButton_clicked();

    void on_fireButton_clicked();

    void on_saveButton_clicked();

    void on_salaryButton_clicked();

    void on_sumbitButton_clicked();

    void on_buyButton_clicked();

    void on_staffTable_doubleClicked(const QModelIndex &index);

private:
    Ui::DirectorWindow *ui;

    QSqlTableModel* model1;
    QSqlTableModel* model2;
    QSqlTableModel* model3;
    std::shared_ptr<QSqlDatabase> db = nullptr;

    void updateTables();
};

#endif // DIRECTORWINDOW_H
