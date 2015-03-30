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
  ui->horizontalSlider->setEnabled(false);
  ui->lineEdit_2->setEnabled(false);

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

      ui->horizontalSlider->setEnabled(true);
      ui->horizontalSlider->setRange(1, im1->width()*im1->height());
      ui->lineEdit_2->setEnabled(true);
    });

  QObject::connect(ui->horizontalSlider, &QSlider::valueChanged,
                   [=](){ui->lineEdit_2->setText(QString::number(ui->horizontalSlider->value()));});
  QObject::connect(ui->lineEdit_2, &QLineEdit::textEdited, [=](){
                   ui->horizontalSlider->setValue(ui->lineEdit_2->text().toInt());});

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
          QString poliS = "x10+x3+1", len_p;
          ui->lineEdit_3->setText(poliS);  //не проверяется на пустоту строки
          unsigned int poli = 0;
          poliS.remove(QChar(' '), Qt::CaseInsensitive);
          poliS.replace("+1", "+0");
          poliS.remove(QChar('x'), Qt::CaseInsensitive);
          poliS.replace("++", "+1+");
          for(unsigned int b = 0; b <= poliS.count("+"); b++)
              poli |= 1 << poliS.section('+', b, b).toUInt();
          //qDebug() << bin << poli;

          QList<unsigned int> randlist = randSeq(im1->width(), poli, poliS.section('+', 0, 0).toUInt());
          //QList<unsigned int> randlist = randSeq(im1->width(), 0b10000001001, 10);
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

QList<unsigned int> MainWindow::randSeq(unsigned int k, unsigned poli, unsigned st) //st -- степень многочлена
{
  QList<unsigned int> res;
  unsigned int n=1, randd;

  n <<= st;
  //qDebug() << len << n;
  poli = poli & (((n << 1) - 1) >> 1); //убирается первая единица из полинома
  //должно быть k < n !!!
  srand(time(0));
  randd=random() % (n - 1) + 1; //иницилизирующее число (0 < randd < n)

  for(unsigned int i = 0; i<n-1; i++){
      unsigned int tmp = poli & randd;
      unsigned int sum = 0;
      while(tmp != 0){
          sum ^= (tmp & 1);
          tmp >>= 1;
        }
      randd = (randd >> 1) | (sum << (st - 1));
      if(randd <= k)
        res << randd;
    }
  return res;
}
