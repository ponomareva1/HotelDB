#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>

#include "directorwindow.h"
#include "adminwindow.h"
#include "housemaidwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->logins->addItem("Директор");
    ui->logins->addItem("Администратор");
    ui->logins->addItem("Горничная");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_connect_clicked()
{
    QString login = getLogin();
    QString password = getPassword();
    if ((login == "") || (password == "")) {
        return;
    }

    db = std::make_shared<QSqlDatabase>(QSqlDatabase::addDatabase("QODBC"));
    QString connectString = QString("Driver={SQL Server};");
    connectString.append("Server=localhost,1433;");
    connectString.append("Database=Hotel;");
    connectString.append("Trusted_Connection=yes;");
    db->setDatabaseName(connectString);

    if(!db->open()) {
        QMessageBox::critical(this, "Невозможно подключиться", "Проблема с подключением к базе данных");
        return;
    }

    ui->password->clear();

    openWindow();
}

QString MainWindow::getLogin()
{
    switch (ui->logins->currentIndex()){
    case 0:
        return QString("director");
        break;
    case 1:
        return QString("admin");
        break;
    case 2:
        return QString("housemaid");
        break;
    default:
        QMessageBox::critical(this, "Невозможно подключиться", "Выберете пользователя");
        return QString("");
    }
}

QString MainWindow::getPassword()
{
    QString psw = ui->password->toPlainText();
    switch (ui->logins->currentIndex()){
    case 0:
        if (psw == "qwerty"){
            return psw;
        }
        break;
    case 1:
        if (psw == "12345"){
           return psw;
        }
        break;
    case 2:
        if (psw == "67890") {
            return psw;
        }
        break;
    }
    QMessageBox::critical(this, "Невозможно подключиться", "Неверный пароль");
    return QString("");
}

void MainWindow::openWindow()
{
    switch (ui->logins->currentIndex()){
    case 0:
    {
        DirectorWindow* dw = new DirectorWindow(db);
        dw->show();
        break;
    }
    case 1:
    {
        AdminWindow* aw = new AdminWindow(db);
        aw->show();
        break;
    }
    case 2:
        HousemaidWindow* hw = new HousemaidWindow(db);
        hw->show();
        break;
    }
}
