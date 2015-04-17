#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <QPainter>
#include <QMessageBox>
#include <QBitArray>
#include <bitset>
#include <chrono>

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
  ui->radioButton->setChecked(true);
  ui->horizontalSlider_2->setSingleStep(1);
  ui->horizontalSlider_2->setEnabled(false);
  ui->spinBox->setEnabled(false);

  QObject::connect(ui->pushButton, &QPushButton::clicked, [=](){
      ui->lineEdit->setText(QFileDialog::getOpenFileUrl(NULL, tr("Открыть файл"),
                                                        QUrl(QCoreApplication::applicationDirPath()),
                                                        "Bitmap (*.bmp)").toString(QUrl::PreferLocalFile));
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

      ui->horizontalSlider->setRange(0, divs->size() - 1);
      ui->horizontalSlider->setEnabled(true);
      ui->horizontalSlider->setValue(1);
      ui->horizontalSlider->setValue(0);

      replDivs = new QVector<quint32>;
      dataString = new QString("");
      for(auto i = 0; i < im2->byteCount(); i++)
        *dataString += QString::fromStdString(std::bitset<8>(*(im2->bits() + i)).to_string());
      for(auto i: dividers(dataString->length())){
          if(i <= 10)
            replDivs->append(i);
          else
            continue;
        }
      ui->horizontalSlider_2->setRange(0, replDivs->size() - 1);
      ui->horizontalSlider_2->setEnabled(true);
      ui->horizontalSlider_2->setValue(1);
      ui->horizontalSlider_2->setValue(0);

      ui->spinBox->setMinimum(2);
      ui->spinBox->setEnabled(true);
    });

  QObject::connect(ui->horizontalSlider, &QSlider::valueChanged,
                   [=](){ui->label_4->setText(QString::number(divs->at(ui->horizontalSlider->value())));});
  QObject::connect(ui->horizontalSlider_2, &QSlider::valueChanged,[=]() {
      ui->label_5->setText(QString::number(replDivs->at(ui->horizontalSlider_2->value())));
      ui->spinBox->setMaximum((dataString->length()) / (ui->label_5->text().toUInt()) );//- 1);
  });

  ui->pushButton_2->setText("=>");
  ui->pushButton_2->setMinimumSize(50,50);
  ui->pushButton_3->setText("=>");
  ui->pushButton_3->setMinimumSize(50,50);
  ui->label_3->setText("");
  ui->label_4->setText("");
  ui->label_5->setText("");

  QObject::connect(ui->pushButton_2, &QPushButton::clicked, [=](){
      if(ui->lineEdit->text().isEmpty())
        QMessageBox::warning(this, tr("Ошибка!"),
                             tr("Не выбран файл."), QMessageBox::Ok, QMessageBox::NoButton);
      else {
          QImage res(im1->size(), QImage::Format_RGB32);
          res = *im1;
          QString poliS = sel_poli((quint32)ui->label_4->text().toUInt());
          if(poliS == "!")
              QMessageBox::warning(this, tr("Ошибка!"),
                                   tr("Нет подходящего полинома под данную длину ключа."),
                                   QMessageBox::Ok, QMessageBox::NoButton);
          else{
              QString str = poliS;
              str.replace("x", "x<span style=\" vertical-align:super;\">");
              str.replace("+", "</span>+");
              str = "<html><head/><body><p><span style=\" font-size:12pt;\">Полином: " + str + "</span</p></body></html>";
              ui->label_3->setText(str);
              quint32 poli = 0;
              poliS.replace("+1", "+0");
              poliS.remove(QChar('x'), Qt::CaseInsensitive);
              poliS.replace("++", "+1+");
              for(int b = 0; b <= poliS.count("+"); b++)
                  poli |= 1 << (quint32)poliS.section('+', b, b).toUInt();

              QList<quint32> randlist = randSeq((quint32)ui->label_4->text().toUInt(),
                                                poli, (quint32)poliS.section('+', 0, 0).toUInt());
              QList<QRgb> pixelList;

               for(quint32 h = 0; h < (quint32)im1->height(); h++)
                 for(quint32 w = 0; w < (quint32)im1->width(); w++)
                     pixelList << im1->pixel(w, h);

               for(quint32 block = 0; block < (quint32)pixelList.length();
                   block += (quint32)randlist.length()){
                   for(quint32 i = 0; i < (quint32)randlist.length(); i++){
                       pixelList.swap(i + block, randlist[i] - 1 + block);   //вроде работает, но хз
                     }
                 }

               quint32 imwidth = im1->width();

               for(quint32 h = 0; h < (quint32)im1->height(); h++)
                 for(quint32 w = 0; w < imwidth; w++){
                   res.setPixel(w, h, pixelList[h * imwidth + w]);
                 }

               ui->modlabel->setPixmap(QPixmap::fromImage(res));
          }
        }

    });

  QObject::connect(ui->pushButton_3, &QPushButton::clicked, [=](){
      if(ui->lineEdit->text().isEmpty())
        QMessageBox::warning(this, tr("Ошибка!"),
                             tr("Не выбран файл."), QMessageBox::Ok, QMessageBox::NoButton);
      else {
          QImage res(im2->size(), QImage::Format_RGB32);

          quint32 len_block = (quint32)ui->label_5->text().toUInt();

          if(ui->radioButton->isChecked()){
              //QHash<QString, QString> *repltable = new QHash<QString, QString>;
              QHash<QString, QString> *repltable = new QHash<QString, QString>;
              QString *ldataString = new QString(*dataString); //копируем dataString в ldataString, чтобы dataString не изменилась

              QList<quint32> *psp = new QList<quint32>(randSeq(1 << len_block, 0b10000000011011, 13));

              for(quint32 i = 0; i < 1 << len_block; i++ ) { //заполнение таблицы
                  auto tmp = QByteArray::number(i, 2).rightJustified(len_block, '0');
                  repltable->insert(tmp, QByteArray::number(psp->at(i)-1, 2).rightJustified(len_block, '0'));
              }

              //quint32 step = ui->label_5->text().toUInt();    // step == len_block
              for(quint32 i = 0; i < ldataString->length(); i += len_block)
                  ldataString->replace(i, len_block, repltable->value(ldataString->mid(i, len_block)));
              for(quint32 i = 0; i < ldataString->length(); i += 8)
                  *(res.bits() + i/8) = (uchar)ldataString->mid(i, 8).toUInt(0, 2);
              ui->modlabel_2->setPixmap(QPixmap::fromImage(res));

              delete repltable;
              delete ldataString;
              delete psp;

            }
          else if(ui->radioButton_2->isChecked()){  //работает, но пролема c srand(time(0)) поэтому столбцы одинаковы
              QString *ldataString = new QString(*dataString); //копируем dataString в ldataString, чтобы dataString не изменилась
              quint32 kolvo_tab = ui->spinBox->text().toUInt(); //для 2 режима
              //quint32 kolvo_tab = (dataString->length()) / len_block; //для 3 режима

              for(quint32 j = 0; j < kolvo_tab; j++) {
                  QHash<QString, QString> repltable;
                  QList<quint32> psp = randSeq(1 << len_block, 0b10000000011011, 13);

                  for(quint32 i = 0; i < 1 << len_block; i++ ) { //заполнение таблицы
                      repltable[QByteArray::number(i, 2).rightJustified(len_block, '0')] =
                              QByteArray::number(psp[i]-1, 2).rightJustified(len_block, '0');
                  }

                  for(quint32 k = j * len_block; k < ldataString->length(); k += len_block * kolvo_tab)
                      ldataString->replace(k, len_block, repltable.value(ldataString->mid(k, len_block)));
              }

              for(quint32 i = 0; i < ldataString->length(); i += 8)
                  *(res.bits() + i/8) = (uchar)ldataString->mid(i, 8).toUInt(0, 2);
              ui->modlabel_2->setPixmap(QPixmap::fromImage(res));
              delete ldataString;
            }
          else if(ui->radioButton_3->isChecked()){  //работает, но пролема c srand(time(0)) поэтому столбцы одинаковы
              QString *ldataString = new QString(*dataString); //копируем dataString в ldataString, чтобы dataString не изменилась
              //quint32 kolvo_tab = ui->spinBox->text().toUInt(); //для 2 режима
              quint32 kolvo_tab = (dataString->length()) / len_block; //для 3 режима

              for(quint32 j = 0; j < kolvo_tab; j++) {
                  QHash<QString, QString> repltable;
                  QList<quint32> psp = randSeq(1 << len_block, 0b10000000011011, 13);

                  for(quint32 i = 0; i < 1 << len_block; i++ ) { //заполнение таблицы
                      repltable[QByteArray::number(i, 2).rightJustified(len_block, '0')] =
                              QByteArray::number(psp[i]-1, 2).rightJustified(len_block, '0');
                  }

                  for(quint32 k = j * len_block; k < ldataString->length(); k += len_block * kolvo_tab)
                      ldataString->replace(k, len_block, repltable.value(ldataString->mid(k, len_block)));
              }

              for(quint32 i = 0; i < ldataString->length(); i += 8)
                  *(res.bits() + i/8) = (uchar)ldataString->mid(i, 8).toUInt(0, 2);
              ui->modlabel_2->setPixmap(QPixmap::fromImage(res));
              delete ldataString;
            }
        }
    });

  QObject::connect(ui->pushButton_5, &QPushButton::clicked, [=](){
      switch (ui->tabWidget->tabBar()->currentIndex()){
      case 0:{
          QString path = QFileDialog::getSaveFileName(this, tr("Сохранить"), "./", tr("(*.bmp)"));
          QFile file(path);
          if(!file.open(QIODevice::WriteOnly)){
              QMessageBox::critical(this, tr("Ошибка"), tr("Ошибка сохранения"), QMessageBox::Ok, QMessageBox::NoButton);
              return -1;
          }
          ui->modlabel->pixmap()->save(&file, "BMP");
          file.close();
          break;
      }

      case 1:{
          QString path = QFileDialog::getSaveFileName(this, tr("Сохранить"), "./", tr("(*.bmp)"));
          QFile file(path);
          if(!file.open(QIODevice::WriteOnly)){
              QMessageBox::critical(this, tr("Ошибка"), tr("Ошибка сохранения"), QMessageBox::Ok, QMessageBox::NoButton);
              return -1;
          }

          ui->modlabel_2->pixmap()->save(&file, "BMP");
          file.close();
          break;
      }
      }
  });
}

MainWindow::~MainWindow()
{
  delete ui;
}

QList<quint32> MainWindow::randSeq(quint32 k, quint32 poli, quint32 st) //st -- степень многочлена
{
  QList<quint32> res;
  quint32 n=1, randd;

  n <<= st;
  poli = poli & (((n << 1) - 1) >> 1); //убирается первая единица из полинома
  //должно быть k < n !!!

  qsrand(std::chrono::duration_cast<std::chrono::nanoseconds> //Инициализация генератора
        (std::chrono::system_clock::now().time_since_epoch()).count()); //с помощью <chrono>

  randd=qrand() % (n - 1) + 1; //иницилизирующее число (0 < randd < n)

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

  return divs;
}

QString MainWindow::sel_poli(quint32 len_key) //выбор полинома
{
    QList<QString> poli_list;
    poli_list << "x5+x2+1" << "x6+x1+1" << "x7+x3+1" << "x9+x4+1" << "x10+x3+1"
              << "x11+x2+1" << "x12+x6+x4+x1+1" << "x13+x4+x3+x1+1" << "x14+x10+x6+x1+1" << "x15+x1+1"
              << "x16+x12+x3+x1+1" << "x17+x3+1" << "x18+x7+1" << "x19+x5+x2+x1+1" << "x20+x3+1"
              << "x21+x2+1" << "x22+x1+1" << "x23+x5+1" << "x25+x3+1" << "x28+x3+1"
              << "x29+x2+1" << "x31+x3+1";// << "x33+x13+1" << "x35+x2+1" << "x36+x11+1"
//              << "x39+x4+1" << "x41+x20+1" << "x47+x20+1" << "x49+x22+1";

    quint32 bit_key = 0; //количество бит в длине ключа

    for(quint32 tmp = len_key; tmp != 0; bit_key++){
        tmp >>= 1;
    }

    QString poli;
    for(int i = 0; i < poli_list.size(); i++){
        poli = poli_list.at(i);
        if(poli.section('+', 0, 0).remove(QChar('x')).toUInt() > bit_key)
            return poli;
    }
    return "!";
}
