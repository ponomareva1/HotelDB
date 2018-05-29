#include "housemaidwindow.h"
#include "ui_housemaidwindow.h"

#include <QMessageBox>
#include <QSqlQuery>
#include <QString>

HousemaidWindow::HousemaidWindow(std::shared_ptr<QSqlDatabase> dataBase, QWidget *parent) :
    db(dataBase), QWidget(parent),
    ui(new Ui::HousemaidWindow)
{
    ui->setupUi(this);

    if (!(db.get()->isOpen())){
        QMessageBox::critical(this, "Ошибка", "Невозможно подключиться к базе данных");
    }
    ui->tabWidget->setCurrentIndex(0);

    updateTables();
}

HousemaidWindow::~HousemaidWindow()
{
    delete ui;
}

void HousemaidWindow::updateTables()
{
    model1 = new QSqlTableModel(this, *db);
    model1->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model1->setTable("Requests");
    model1->select();

    ui->requestsView->setModel(model1);
    ui->requestsView->show();
    ui->requestsView->resizeColumnsToContents();
    ui->requestsView->resizeRowsToContents();

    model2 = new QSqlTableModel(this, *db);
    model2->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model2->setTable("Menu");
    model2->select();

    ui->menuView->setModel(model2);
    ui->menuView->hideColumn(0);
    ui->menuView->hideColumn(2);
    ui->menuView->show();
    ui->menuView->resizeColumnsToContents();
    ui->menuView->resizeRowsToContents();

    ui->notCleanRoomsWidget->setRowCount(0);
    QSqlQuery queryR("SELECT IdRoom,Free FROM Rooms Where Clean=0");
    queryR.setForwardOnly(true);
    ui->notCleanRoomsWidget->setColumnCount(2);
    while (queryR.next()) {
        QString idRoom = queryR.value(0).toString();
        QString free = queryR.value(1).toString();
        int row = ui->notCleanRoomsWidget->rowCount();
        ui->notCleanRoomsWidget->insertRow(row);
        ui->notCleanRoomsWidget->setItem(row, 0, new QTableWidgetItem(idRoom));
        ui->notCleanRoomsWidget->setItem(row, 1, new QTableWidgetItem(free));
    }
    ui->notCleanRoomsWidget->setHorizontalHeaderLabels(QString("IdRoom;Free").split(";"));
    ui->notCleanRoomsWidget->resizeColumnsToContents();
    ui->notCleanRoomsWidget->resizeRowsToContents();
}

void HousemaidWindow::on_requestButton_clicked()
{
    QModelIndex requestsRow = ui->requestsView->selectionModel()->currentIndex();
    QString requestsId = requestsRow.data().toString();
    bool check = true;
    requestsId.toInt(&check, 10);
    if (!check){
        QMessageBox::critical(this, "Ошибка", "Выберете индекс запроса");
        return;
    }

    if (QMessageBox::question(this, "Подтверждение", "Выполнить запрос "+requestsId+"?",
                              QMessageBox::Yes, QMessageBox::No) == QMessageBox::No){
        return;
    }

    QSqlQuery querySelect;
    querySelect.prepare("SELECT Dish FROM Requests WHERE IdRequest=?;");
    querySelect.bindValue(0, requestsId);
    if (!querySelect.exec()) {
        QMessageBox::critical(this, "Ошибка", "Невозможно выполнить запрос");
        return;
    }
    querySelect.next();
    QString dishId = querySelect.value(0).toString();

    QSqlQuery queryUpdate;
    queryUpdate.prepare("UPDATE Menu SET Quantity=Quantity-1 WHERE IdDish=?;");
    queryUpdate.bindValue(0, dishId);
    if (!queryUpdate.exec()) {
        QMessageBox::critical(this, "Ошибка", "Невозможно выполнить запрос");
        return;
    }

    QSqlQuery queryDelete;
    queryDelete.prepare("DELETE FROM Requests WHERE IdRequest=?;");
    queryDelete.bindValue(0, requestsId);
    if (!queryDelete.exec()) {
        QMessageBox::critical(this, "Ошибка", "Невозможно выполнить запрос");
        return;
    }

    updateTables();
}

void HousemaidWindow::on_notCleanRoomsWidget_doubleClicked(const QModelIndex &index)
{
    QString roomId = index.data().toString();
    bool check = true;
    roomId.toInt(&check, 10);
    if (!check){
        QMessageBox::critical(this, "Ошибка", "Выберете индекс номера");
        return;
    }

    if (QMessageBox::question(this, "Подтверждение", "Убраться в номере "+roomId+"?",
                              QMessageBox::Yes, QMessageBox::No) == QMessageBox::No){
        return;
    }

    QSqlQuery queryUpdate;
    queryUpdate.prepare("UPDATE Rooms SET Clean=1 WHERE IdRoom=?;");
    queryUpdate.bindValue(0, roomId);
    if (!queryUpdate.exec()) {
        QMessageBox::critical(this, "Ошибка", "Невозможно выполнить запрос");
        return;
    }

    updateTables();
}
