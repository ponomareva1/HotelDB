#include "include/directorwindow.h"
#include "ui_directorwindow.h"

#include <QMessageBox>
#include <QSqlQuery>
#include <QString>
#include <QDateTime>

DirectorWindow::DirectorWindow(std::shared_ptr<QSqlDatabase> dataBase, QWidget *parent) :
    db(dataBase), QWidget(parent),
    ui(new Ui::DirectorWindow)
{
    ui->setupUi(this);

    if (!(db.get()->isOpen())){
        QMessageBox::critical(this, "Ошибка", "Невозможно подключиться к базе данных");
    }

    ui->tabWidget->setCurrentIndex(0);

    ui->positionBox->addItem("Администратор");
    ui->positionBox->addItem("Горничная");

    ui->dateEdit->setDateTime(QDateTime::currentDateTime());

    updateTables();
}

DirectorWindow::~DirectorWindow()
{
    delete ui;
}

void DirectorWindow::updateTables()
{
    model1 = new QSqlTableModel(this, *db);
    model1->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model1->setTable("Staff");
    model1->select();

    ui->staffTable->setModel(model1);
    ui->staffTable->show();
    ui->staffTable->resizeColumnsToContents();
    ui->staffTable->resizeRowsToContents();

    model2 = new QSqlTableModel(this, *db);
    model2->setTable("Bookkeeping");
    model2->select();

    ui->bkTable->setModel(model2);
    ui->bkTable->show();
    ui->bkTable->resizeColumnsToContents();
    ui->bkTable->resizeRowsToContents();

    model3 = new QSqlTableModel(this, *db);
    model3->setTable("Menu");
    model3->select();

    ui->menuView->setModel(model3);
    ui->menuView->show();
    ui->menuView->resizeColumnsToContents();
    ui->menuView->resizeRowsToContents();
}

void DirectorWindow::on_hireButton_clicked()
{
    QString position;
    if (ui->positionBox->currentIndex() == 0){
        position = "Администратор";
    } else {
        position = "Горничная";
    }

    QString name = ui->nameEdit->toPlainText();
    QString passport = ui->passportEdit->toPlainText();
    if (passport.size() != 10){
        QMessageBox::critical(this, "Ошибка", "Номер паспорта должен состоять из 10 цифр");
        return;
    }
    QString salary = QString::number(ui->salaryBox->value());

    if (name.isEmpty() || passport.isEmpty() || salary == "0.00"){
        QMessageBox::critical(this, "Ошибка", "Проверьте поля для ввода");
        return;
    }

    if (QMessageBox::question(this, "Подтверждение", "Нанять нового сотрудника?",
                              QMessageBox::Yes, QMessageBox::No) == QMessageBox::No){
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT Staff(Position,Name,Passport,Salary) "
                      "VALUES (?, ?, ?, ?);");
    query.bindValue(0, position);
    query.bindValue(1, name);
    query.bindValue(2, passport);
    query.bindValue(3, salary);
    if (!query.exec()) {
        QMessageBox::critical(this, "Ошибка", "Невозможно выполнить запрос");
        return;
    }

    updateTables();

    ui->positionBox->setCurrentIndex(0);
    ui->nameEdit->setText("");
    ui->passportEdit->setText("");
    ui->salaryBox->setValue(0);
}

void DirectorWindow::on_fireButton_clicked()
{
    QModelIndex rowidx = ui->staffTable->selectionModel()->currentIndex();
    QString index = rowidx.data().toString();
    bool check = true;
    index.toInt(&check, 10);
    if (!check){
        QMessageBox::critical(this, "Ошибка", "Выберете индекс сотрудника");
        return;
    }

    if (QMessageBox::question(this, "Подтверждение", "Уволить сотрудника с id "+index+"?",
                              QMessageBox::Yes, QMessageBox::No) == QMessageBox::No){
        return;
    }

    QString salary, sumOfOp;

    QSqlQuery querySelect;
    querySelect.prepare("SELECT Salary FROM Staff WHERE IdEmployee=?");
    querySelect.bindValue(0, index);
    querySelect.exec();
    querySelect.next();
    salary = querySelect.value(0).toString();
    sumOfOp = QString::number(-salary.toDouble());

    QString dateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");

    QSqlQuery queryInsert;
    queryInsert.prepare("INSERT Bookkeeping(DateOfOp,Executor,Operation,SumOfOp) "
                   "VALUES (?, 1, 4, ?);");
    queryInsert.bindValue(0, dateTime);
    queryInsert.bindValue(1, sumOfOp);
    if (!queryInsert.exec()) {
        QMessageBox::critical(this, "Ошибка", "Невозможно выполнить запрос");
        return;
    }

    QSqlQuery queryDelete;
    queryDelete.prepare("DELETE FROM Staff WHERE IdEmployee=?;");
    queryDelete.bindValue(0, index);
    if (!queryDelete.exec()) {
        QMessageBox::critical(this, "Ошибка", "Невозможно выполнить запрос");
        return;
    }

    updateTables();
}

void DirectorWindow::on_saveButton_clicked()
{
    if (QMessageBox::question(this, "Подтверждение", "Сохранить все внесенные изменения?",
                              QMessageBox::Yes, QMessageBox::No) == QMessageBox::No){
        return;
    }
    if (!model1->submitAll()) {
        QMessageBox::critical(this, "Ошибка", "Невозможно сохранить изменения");
    }
}

void DirectorWindow::on_salaryButton_clicked()
{
    QModelIndex rowidx = ui->staffTable->selectionModel()->currentIndex();
    QString index = rowidx.data().toString();
    bool check = true;
    index.toInt(&check, 10);
    if (!check){
        QMessageBox::critical(this, "Ошибка", "Выберете индекс сотрудника или строку");
        return;
    }

    if (QMessageBox::question(this, "Подтверждение", "Выплатить зарплату сотруднику с id "+index+"?",
                              QMessageBox::Yes, QMessageBox::No) == QMessageBox::No){
        return;
    }

    QString salary, sumOfOp;

    QSqlQuery querySelect;
    querySelect.prepare("SELECT Salary FROM Staff WHERE IdEmployee=?");
    querySelect.bindValue(0, index);
    querySelect.exec();
    querySelect.next();
    salary = querySelect.value(0).toString();
    sumOfOp = QString::number(-salary.toDouble());

    QString dateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");

    QSqlQuery queryInsert;
    queryInsert.prepare("INSERT Bookkeeping(DateOfOp,Executor,Operation,SumOfOp) "
                   "VALUES (?, 1, 3, ?);");
    queryInsert.bindValue(0, dateTime);
    queryInsert.bindValue(1, sumOfOp);
    if (!queryInsert.exec()) {
        QMessageBox::critical(this, "Ошибка", "Невозможно выполнить запрос");
        return;
    }

    updateTables();
}

void DirectorWindow::on_sumbitButton_clicked()
{
    QString selectedDate = ui->dateEdit->date().toString("yyyy-MM-dd");

    QSqlQuery querySelect;
    querySelect.prepare("SELECT SUM(SumOfOp) AS Ravenue FROM Bookkeeping WHERE CAST(DateOfOp AS date)=?");
    querySelect.bindValue(0, selectedDate);
    querySelect.exec();

    if (!querySelect.first()){
        ui->reviewEdit->setText("");
    } else {
        QString review = querySelect.value(0).toString();
        ui->reviewEdit->setText(review);
    }
}

void DirectorWindow::on_buyButton_clicked()
{
    QString quantity = QString::number(ui->quantityBox->value());

    if (quantity == "0"){
        QMessageBox::critical(this, "Ошибка", "Количество не может быть равно нулю");
        return;
    }

    QModelIndex rowidx = ui->menuView->selectionModel()->currentIndex();
    QString index = rowidx.data().toString();
    bool check = true;
    index.toInt(&check, 10);
    if (!check){
        QMessageBox::critical(this, "Ошибка", "Выберете индекс продукта");
        return;
    }

    if (QMessageBox::question(this, "Подтверждение", "Заказать блюдо "+index+"?",
                              QMessageBox::Yes, QMessageBox::No) == QMessageBox::No){
        return;
    }

    QSqlQuery querySelect;
    querySelect.prepare("SELECT Price FROM Menu WHERE IdDish=?");
    querySelect.bindValue(0, index);
    if (!querySelect.exec()) {
        QMessageBox::critical(this, "Ошибка", "Невозможно выполнить запрос");
        return;
    }
    querySelect.next();
    QString price = querySelect.value(0).toString();
    QString sumOfOp = QString::number((-price.toDouble())*quantity.toInt());

    QString dateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");

    QSqlQuery queryInsert;
    queryInsert.prepare("INSERT Bookkeeping(DateOfOp,Executor,Operation,SumOfOp) "
                   "VALUES (?, 1, 2, ?);");
    queryInsert.bindValue(0, dateTime);
    queryInsert.bindValue(1, sumOfOp);
    if (!queryInsert.exec()) {
        QMessageBox::critical(this, "Ошибка", "Невозможно выполнить запрос");
        return;
    }

    QSqlQuery queryUpdate;
    queryUpdate.prepare("UPDATE Menu SET Quantity=Quantity+? WHERE IdDish=?;");
    queryUpdate.bindValue(0, quantity);
    queryUpdate.bindValue(1, index);
    if (!queryUpdate.exec()) {
        QMessageBox::critical(this, "Ошибка", "Невозможно выполнить запрос");
        return;
    }

    updateTables();
}

void DirectorWindow::on_staffTable_doubleClicked(const QModelIndex &index)
{
    if (index.column() == 0 || index.column() == 1){
        ui->staffTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    } else {
        ui->staffTable->setEditTriggers(QAbstractItemView::DoubleClicked);
    }
}
