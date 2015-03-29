#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <QPainter>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  ui->lineEdit->setReadOnly(true);
  ui->tabWidget->tabBar()->setTabText(0, tr("Шифр перестановки"));
  ui->tabWidget->tabBar()->setTabText(1, tr("Шифр замены"));
//  ui->orlabel->setMaximumSize(ui->frame->size());
  ui->orlabel->setScaledContents(true);
  ui->modlabel->setScaledContents(true);
  ui->orlabel_2->setScaledContents(true);
  ui->modlabel_2->setScaledContents(true);

  QObject::connect(ui->pushButton, &QPushButton::clicked, [=](){
      ui->lineEdit->setText(QFileDialog::getOpenFileUrl(NULL, tr("Открыть файл"),
                                                        QUrl(QCoreApplication::applicationDirPath()), "Bitmap (*.bmp)").toString(QUrl::PreferLocalFile));
      QFile *file = new QFile(ui->lineEdit->text());
      file->open(QIODevice::ReadOnly);
      bytepic = new QByteArray(file->readAll());

      im1 = new QImage();
      im1->loadFromData(*bytepic);
      im1->convertToFormat(QImage::Format_RGB32);

      im2 = new QImage();
      im2->loadFromData(*bytepic);
      im2->convertToFormat(QImage::Format_RGB32);


      QPixmap *pm = new QPixmap(ui->lineEdit->text());
      ui->orlabel->setPixmap(*pm);
      ui->orlabel_2->setPixmap(*pm);
    });

  ui->pushButton_2->setText("=>");
  ui->pushButton_2->setMinimumSize(50,50);
  ui->pushButton_3->setText("=>");
  ui->pushButton_3->setMinimumSize(50,50);

  QObject::connect(ui->pushButton_2, &QPushButton::clicked, [=](){
      if(ui->lineEdit->text().isEmpty())
        QMessageBox::warning(this, tr("Ошибка!"),
                             tr("Не выбран файл."), QMessageBox::Ok, QMessageBox::NoButton);
      else {
          QImage res(im1->size(), QImage::Format_RGB32);
          res = *im1;
          QList<unsigned int> randlist = randSeq(100, 0b1000011);
          //qDebug() << randlist << randlist.length();

          QList<QImage> tmp;

          quint16 h = im1->height();
          for(auto i = 0; i < im1->width(); i++)
            tmp << im1->copy(i,0, 1, h);

          QPainter pntr(&res);

          for(int i = 0; i < im1->width(); i++){
              pntr.drawImage(i, 0, tmp[randlist[i]-1]);
              ui->modlabel->setPixmap(QPixmap::fromImage(res));
            }

        }

    });

}

MainWindow::~MainWindow()
{
  delete ui;
}

QList<unsigned int> MainWindow::randSeq(unsigned int k, unsigned poli)
{
  QList<unsigned int> res;
  unsigned int n=1, len=0, randd;

  for(unsigned int tmp = poli; tmp != 0; len++){
      tmp >>= 1;
      n <<= 1;
    }
  srand(time(0));
  randd=random()%n; //иницилизирующее число
  poli=poli & ((n-1)>>1);

  for(unsigned int i = 0; i<n-1; i++){
      unsigned int tmp = poli & randd;
      unsigned int sum = 0;
      while(tmp != 0){
          sum ^= (tmp & 1);
          tmp >>= 1;
        }
      randd = (randd >> 1) | (sum << (len - 1));
      if(randd <= k)
        res << randd;
    }
  return res;
}
