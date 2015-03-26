#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  ui->lineEdit->setReadOnly(true);
  ui->tabWidget->tabBar()->setTabText(0, tr("Шифр перестановки"));
  ui->tabWidget->tabBar()->setTabText(1, tr("Шифр замены"));
  ui->orlabel->setMaximumSize(ui->frame->size());

  QObject::connect(ui->pushButton, &QPushButton::clicked, [=](){
        ui->lineEdit->setText(QFileDialog::getOpenFileUrl(NULL, tr("Открыть файл"),
          QUrl(QCoreApplication::applicationDirPath()), "Bitmap (*.bmp)").toString(QUrl::PreferLocalFile));
        QFile *file = new QFile(ui->lineEdit->text());
        file->open(QIODevice::ReadOnly);
        bytepic = new QByteArray(file->readAll());
        QPixmap *pm = new QPixmap();
        pm->loadFromData(*bytepic);
        ui->orlabel->setPixmap(*pm);
    });

  ui->pushButton_2->setText("=>");
  ui->pushButton_2->setMinimumSize(50,50);
}

MainWindow::~MainWindow()
{
  delete ui;
}

