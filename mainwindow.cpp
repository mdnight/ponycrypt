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
  ui->orlabel->setScaledContents(true);
  ui->modlabel->setScaledContents(true);
  ui->orlabel_2->setScaledContents(true);
  ui->modlabel_2->setScaledContents(true);
  ui->horizontalSlider->setSingleStep(1);
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

      divs = new QVector<quint32>(dividers(im1->width()*im1->height()));
      qDebug() << *divs;
      ui->horizontalSlider->setRange(0, divs->size() - 1);
      ui->horizontalSlider->setEnabled(true);
    });

  QObject::connect(ui->horizontalSlider, &QSlider::valueChanged,
                   [=](){ui->lineEdit_2->setText(QString::number(divs->at(ui->horizontalSlider->value())));});
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
          QString poliS = sel_poli((quint32)im1->width() * (quint32)im1->height()); //длину ключа должна передавать
          if(poliS == "!")
              QMessageBox::warning(this, tr("Ошибка!"),
                                   tr("Нет подходящего полинома под данную длину ключа."), QMessageBox::Ok, QMessageBox::NoButton);
          else{
              ui->lineEdit_3->setText(poliS);
              quint32 poli = 0;
              poliS.replace("+1", "+0");
              poliS.remove(QChar('x'), Qt::CaseInsensitive);
              poliS.replace("++", "+1+");
              for(int b = 0; b <= poliS.count("+"); b++)
                  poli |= 1 << poliS.section('+', b, b).toUInt();
              //qDebug() << bin << poli;

              QList<quint32> randlist = randSeq((quint32)im1->width(), poli, (quint32)poliS.section('+', 0, 0).toUInt());
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
        }

    });

}

MainWindow::~MainWindow()
{
  delete ui;
  delete im1;
  delete im2;
  delete divs;
}

QList<quint32> MainWindow::randSeq(quint32 k, quint32 poli, quint32 st) //st -- степень многочлена
{
  QList<quint32> res;
  quint32 n=1, randd;

  n <<= st;
  //qDebug() << len << n;
  poli = poli & (((n << 1) - 1) >> 1); //убирается первая единица из полинома
  //должно быть k < n !!!
  srand(time(0));
  randd=random() % (n - 1) + 1; //иницилизирующее число (0 < randd < n)

  for(quint32 i = 0; i<n-1; i++){
      quint32 tmp = poli & randd;
      quint32 sum = 0;
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

QVector<quint32> MainWindow::dividers(quint32 n)
{
  if (n == 0)
    return QVector<quint32>();

  // Вектор делителей (пока содержит только само число)
  QVector<quint32> divs(1, n);

  // Если проверяем 1 - выводим уже готовый вектор, содержащий только саму 1
  if (n == 1)
    return divs;

  // делителем n
  for (quint32 d = n / 2; d > 1; --d)
    // Проверка на делимость нацело
    if (n % d == 0)
      // Если очередное число является делителем, добавляем его в вектор
      divs.push_back(d);

  // Добавляем делитель любого из чисел - 1
  //divs.push_back(1);

  // Возвращаем найденные делители
  return divs;
}

QString MainWindow::sel_poli(quint32 len_key) //выбор полинома
{
    QList<QString> poli_list;
    poli_list << "x5+x2+1" << "x6+x1+1" << "x7+x3+1" << "x9+x4+1" << "x10+x3+1"
              << "x11+x2+1" << "x12+x1+1" << "x13+x1+1" << "x14+x1+1" << "x15+x4+1"
              << "x16+x7+1" << "x17+x3+1" << "x18+x7+1" << "x19+x7+1" << "x20+x3+1"
              << "x21+x2+1" << "x22+x1+1" << "x23+x5+1" << "x25+x3+1" << "x28+x9+1"
              << "x29+x2+1" << "x31+x3+1";// << "x33+x13+1" << "x35+x2+1" << "x36+x11+1"
//              << "x39+x4+1" << "x41+x20+1" << "x47+x20+1" << "x49+x22+1";

    quint32 bit_key = 0; //количество бит в длине ключа

    for(quint32 tmp = len_key; tmp != 0; bit_key++){
        tmp >>= 1;
    }
    //qDebug() << bit_key;
    QString poli;
    for(int i = 0; i < poli_list.size(); i++){
        poli = poli_list.at(i);
        if(poli.section('+', 0, 0).remove(QChar('x')).toUInt() > bit_key)
            return poli;
    }
    //qDebug() << "выбранный полином: " << poli;
    return "!";
}
