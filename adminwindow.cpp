#include "adminwindow.h"
#include "ui_adminwindow.h"

#include <QMessageBox>
#include <QSqlQuery>
#include <QString>
#include <QDateTime>

AdminWindow::AdminWindow(std::shared_ptr<QSqlDatabase> dataBase, QWidget *parent) :
    db(dataBase), QWidget(parent),
    ui(new Ui::AdminWindow)
{
    ui->setupUi(this);
    if (!(db.get()->isOpen())){
        QMessageBox::critical(this, "Ошибка", "Невозможно подключиться к базе данных");
    }

    ui->tabWidget->setCurrentIndex(0);
    ui->dateEdit->setDateTime(QDateTime::currentDateTime());

    updateTables();
}

AdminWindow::~AdminWindow()
{
    delete ui;
}

void AdminWindow::updateTables()
{
    model1 = new QSqlTableModel(this, *db);
    model1->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model1->setTable("Guests");
    model1->select();

    ui->guestsView->setModel(model1);
    ui->guestsView->show();
    ui->guestsView->resizeColumnsToContents();
    ui->guestsView->resizeRowsToContents();

    model2 = new QSqlTableModel(this, *db);
    model2->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model2->setTable("Menu");
    model2->select();

    ui->menuView->setModel(model2);
    ui->menuView->show();
    ui->menuView->resizeColumnsToContents();
    ui->menuView->resizeRowsToContents();

    ui->freeRoomsWidget->setRowCount(0);
    QSqlQuery queryR("SELECT IdRoom,RoomType,NumOfBeds,PriceForNight FROM Rooms Where Free=1,Clean=1");
    queryR.setForwardOnly(true);
    ui->freeRoomsWidget->setColumnCount(4);
    while (queryR.next()) {
        QString idRoom = queryR.value(0).toString();
        QString roomType = queryR.value(1).toString();
        QString numOfBeds = queryR.value(2).toString();
        QString priceForNight = queryR.value(3).toString();
        int row = ui->freeRoomsWidget->rowCount();
        ui->freeRoomsWidget->insertRow(row);
        ui->freeRoomsWidget->setItem(row, 0, new QTableWidgetItem(idRoom));
        ui->freeRoomsWidget->setItem(row, 1, new QTableWidgetItem(roomType));
        ui->freeRoomsWidget->setItem(row, 2, new QTableWidgetItem(numOfBeds));
        ui->freeRoomsWidget->setItem(row, 3, new QTableWidgetItem(priceForNight));
    }
    ui->freeRoomsWidget->setHorizontalHeaderLabels(QString("IdRoom;RoomType;NumOfBeds;PriceForNight").split(";"));
    ui->freeRoomsWidget->resizeColumnsToContents();
    ui->freeRoomsWidget->resizeRowsToContents();

    ui->guestsWidget->setRowCount(0);
    QSqlQuery queryG("SELECT IdGuest,Name,Room,Surcharge FROM Guests WHERE Room!=''");
    queryG.setForwardOnly(true);
    ui->guestsWidget->setColumnCount(4);
    while (queryG.next()) {
        QString idGuest = queryG.value(0).toString();
        QString name = queryG.value(1).toString();
        QString room = queryG.value(2).toString();
        QString surcharge = queryG.value(3).toString();
        int row = ui->guestsWidget->rowCount();
        ui->guestsWidget->insertRow(row);
        ui->guestsWidget->setItem(row, 0, new QTableWidgetItem(idGuest));
        ui->guestsWidget->setItem(row, 1, new QTableWidgetItem(name));
        ui->guestsWidget->setItem(row, 2, new QTableWidgetItem(room));
        ui->guestsWidget->setItem(row, 3, new QTableWidgetItem(surcharge));
    }
    ui->guestsWidget->setHorizontalHeaderLabels(QString("IdGuest;Name;Room;Surcharge").split(";"));
    ui->guestsWidget->resizeColumnsToContents();
    ui->guestsWidget->resizeRowsToContents();
}

void AdminWindow::on_registerButton_clicked()
{
    QString name = ui->nameEdit->toPlainText();
    QString passport = ui->passportEdit->toPlainText();
    if (passport.size() != 10){
        QMessageBox::critical(this, "Ошибка", "Номер паспорта должен состоять из 10 цифр");
        return;
    }
    QString dateOfDeparture = ui->dateEdit->date().toString("yyyy-MM-dd");
    QString dateOfArrival = QDateTime::currentDateTime().toString("yyyy-MM-dd");

    if (name.isEmpty() || passport.isEmpty()){
        QMessageBox::critical(this, "Ошибка", "Проверьте поля для ввода");
        return;
    }
    if (QString::compare(dateOfArrival, dateOfDeparture)==0){
        QMessageBox::critical(this, "Ошибка", "Дата отъезда некорректна");
        return;
    }

    if (QMessageBox::question(this, "Подтверждение", "Оформить нового гостя?",
                              QMessageBox::Yes, QMessageBox::No) == QMessageBox::No){
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT Guests(Name,Passport,DateOfArrival,DateOfDeparture)"
                  " VALUES (?,?,?,?);");
    query.bindValue(0, name);
    query.bindValue(1, passport);
    query.bindValue(2, dateOfArrival);
    query.bindValue(3, dateOfDeparture);
    if (!query.exec()) {
        QMessageBox::critical(this, "Ошибка", "Невозможно выполнить запрос");
        return;
    }

    updateTables();

    ui->nameEdit->setText("");
    ui->passportEdit->setText("");
    ui->dateEdit->setDateTime(QDateTime::currentDateTime());
}

void AdminWindow::on_evictButton_clicked()
{
    QModelIndex rowidx = ui->guestsView->selectionModel()->currentIndex();
    QString index = rowidx.data().toString();
    bool check = true;
    index.toInt(&check, 10);
    if (!check){
        QMessageBox::critical(this, "Ошибка", "Выберете индекс гостя");
        return;
    }

    QSqlQuery querySelect;
    querySelect.prepare("SELECT DateOfDeparture FROM Guests WHERE IdGuest=?");
    querySelect.bindValue(0, index);
    querySelect.exec();
    querySelect.next();
    QString dateOfDeparture = querySelect.value(0).toString();
    QString currentDate = QDateTime::currentDateTime().toString("yyyy-MM-dd");

    if (QString::compare(currentDate, dateOfDeparture)!=0){
        QMessageBox::critical(this, "Ошибка", "Дата отъезда должна совпадать с датой сегодня!");
        return;
    }

    if (QMessageBox::question(this, "Подтверждение", "Выселить гостя с id "+index+"?",
                              QMessageBox::Yes, QMessageBox::No) == QMessageBox::No){
        return;
    }

    QSqlQuery queryRoom;
    queryRoom.prepare("SELECT Room FROM Guests WHERE IdGuest=?");
    queryRoom.bindValue(0, index);
    queryRoom.exec();
    queryRoom.next();
    QString room = queryRoom.value(0).toString();

    QSqlQuery queryUpdate;
    queryUpdate.prepare("UPDATE Rooms SET Clean=0,Free=1 WHERE IdRoom=?;");
    queryUpdate.bindValue(0, room);
    if (!queryUpdate.exec()) {
        QMessageBox::critical(this, "Ошибка", "Невозможно выполнить запрос");
        return;
    }

    QSqlQuery queryDelete;
    queryDelete.prepare("DELETE FROM Guests WHERE IdGuest=?;");
    queryDelete.bindValue(0, index);
    if (!queryDelete.exec()) {
        QMessageBox::critical(this, "Ошибка", "Невозможно выполнить запрос");
        return;
    }

    updateTables();
}

void AdminWindow::on_checkInButton_clicked()
{
    QModelIndex guestRow = ui->guestsView->selectionModel()->currentIndex();
    QString guestId = guestRow.data().toString();
    bool check = true;
    guestId.toInt(&check, 10);
    if (!check){
        QMessageBox::critical(this, "Ошибка", "Выберете индекс гостя");
        return;
    }

    QModelIndex roomRow = ui->freeRoomsWidget->selectionModel()->currentIndex();
    QString roomId = roomRow.data().toString();
    roomId.toInt(&check, 10);
    if (!check){
        QMessageBox::critical(this, "Ошибка", "Выберете индекс номера");
        return;
    }

    if (QMessageBox::question(this, "Подтверждение", "Заселить гостя "+guestId+" в номер "+roomId+"?",
                              QMessageBox::Yes, QMessageBox::No) == QMessageBox::No){
        return;
    }

    QSqlQuery queryUpdateRoom;
    queryUpdateRoom.prepare("UPDATE Rooms SET Free=0 WHERE IdRoom=?;");
    queryUpdateRoom.bindValue(0, roomId);
    if (!queryUpdateRoom.exec()) {
        QMessageBox::critical(this, "Ошибка", "Невозможно выполнить запрос");
        return;
    }

    QSqlQuery queryUpdateGuest;
    queryUpdateGuest.prepare("UPDATE Guests SET Room=? WHERE IdGuest=?;");
    queryUpdateGuest.bindValue(0, roomId);
    queryUpdateGuest.bindValue(1, guestId);
    if (!queryUpdateGuest.exec()) {
        QMessageBox::critical(this, "Ошибка", "Невозможно выполнить запрос");
        return;
    }

    QSqlQuery queryPrice;
    queryPrice.prepare("SELECT PriceForNight FROM Rooms WHERE IdRoom=?;");
    queryPrice.bindValue(0, roomId);
    if (!queryPrice.exec()) {
        QMessageBox::critical(this, "Ошибка", "Невозможно выполнить запрос");
        return;
    }
    queryPrice.next();
    QString price = queryPrice.value(0).toString();

    QSqlQuery queryDays;
    queryDays.prepare("SELECT DATEDIFF(day, DateOfArrival, DateOfDeparture) as Diff FROM Guests WHere IdGuest=?;");
    queryDays.bindValue(0, guestId);
    if (!queryDays.exec()) {
        QMessageBox::critical(this, "Ошибка", "Невозможно выполнить запрос");
        return;
    }
    queryDays.next();
    QString days = queryDays.value(0).toString();

    QString finalPrice = QString::number(price.toInt()*days.toInt());

    QString dateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");

    QSqlQuery queryInsert;
    queryInsert.prepare("INSERT Bookkeeping(DateOfOp,Executor,Operation,SumOfOp) "
                   "VALUES (?, 2, 1, ?);");
    queryInsert.bindValue(0, dateTime);
    queryInsert.bindValue(1, finalPrice);
    if (!queryInsert.exec()) {
        QMessageBox::critical(this, "Ошибка", "Невозможно выполнить запрос");
        return;
    }

    updateTables();
}

void AdminWindow::on_requestButton_clicked()
{
    QModelIndex guestRow = ui->guestsWidget->selectionModel()->currentIndex();
    QString guestId = guestRow.data().toString();
    bool check = true;
    guestId.toInt(&check, 10);
    if (!check){
        QMessageBox::critical(this, "Ошибка", "Выберете индекс гостя");
        return;
    }

    QModelIndex dishRow = ui->menuView->selectionModel()->currentIndex();
    QString dishId = dishRow.data().toString();
    dishId.toInt(&check, 10);
    if (!check){
        QMessageBox::critical(this, "Ошибка", "Выберете индекс блюда");
        return;
    }

    if (QMessageBox::question(this, "Подтверждение", "Создать запрос гостя "+guestId+" с блюдом "+dishId+"?",
                              QMessageBox::Yes, QMessageBox::No) == QMessageBox::No){
        return;
    }

    QSqlQuery queryPrice;
    queryPrice.prepare("SELECT Price FROM Menu WHERE IdDish=?;");
    queryPrice.bindValue(0, dishId);
    if (!queryPrice.exec()) {
        QMessageBox::critical(this, "Ошибка", "Невозможно выполнить запрос");
        return;
    }
    queryPrice.next();
    QString price = queryPrice.value(0).toString();

    QString dateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");

    QSqlQuery queryInsert;
    queryInsert.prepare("INSERT Bookkeeping(DateOfOp,Executor,Operation,SumOfOp) "
                   "VALUES (?, 2, 5, ?);");
    queryInsert.bindValue(0, dateTime);
    queryInsert.bindValue(1, price);
    if (!queryInsert.exec()) {
        QMessageBox::critical(this, "Ошибка", "Невозможно выполнить запрос");
        return;
    }

    QSqlQuery queryInsertReq;
    queryInsertReq.prepare("INSERT Requests(Guest,Dish) VALUES (?, ?);");
    queryInsertReq.bindValue(0, guestId);
    queryInsertReq.bindValue(1, dishId);
    if (!queryInsertReq.exec()) {
        QMessageBox::critical(this, "Ошибка", "Невозможно выполнить запрос");
        return;
    }

    QSqlQuery queryUpdateGuest;
    queryUpdateGuest.prepare("UPDATE Guests SET Surcharge=Surcharge+? WHERE IdGuest=?;");
    queryUpdateGuest.bindValue(0, price);
    queryUpdateGuest.bindValue(1, guestId);
    if (!queryUpdateGuest.exec()) {
        QMessageBox::critical(this, "Ошибка", "Невозможно выполнить запрос");
        return;
    }

    updateTables();
}

void AdminWindow::on_guestsView_doubleClicked(const QModelIndex &index)
{
    if (index.column() == 1 || index.column() == 2 || index.column() == 5){
        ui->guestsView->setEditTriggers(QAbstractItemView::DoubleClicked);
    } else {
        ui->guestsView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }
}
