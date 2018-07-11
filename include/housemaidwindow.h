#ifndef HOUSEMAIDWINDOW_H
#define HOUSEMAIDWINDOW_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlTableModel>

#include <memory>

namespace Ui {
class HousemaidWindow;
}

class HousemaidWindow : public QWidget
{
    Q_OBJECT

public:
    explicit HousemaidWindow(std::shared_ptr<QSqlDatabase> dataBase, QWidget *parent = 0);
    ~HousemaidWindow();

private slots:
    void on_requestButton_clicked();

    void on_notCleanRoomsWidget_doubleClicked(const QModelIndex &index);

private:
    Ui::HousemaidWindow *ui;

    QSqlTableModel* model1;
    QSqlTableModel* model2;
    std::shared_ptr<QSqlDatabase> db = nullptr;

    void updateTables();
};

#endif // HOUSEMAIDWINDOW_H
