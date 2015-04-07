#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QByteArray>
#include <cstdlib>
#include <QBitmap>

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();
  QByteArray *bytepic;
  QList<unsigned int> randSeq(unsigned int k, unsigned int poli, unsigned int st);
  QImage *im1, *im2;
  QVector<quint32> dividers(quint32 n);
  QString sel_poli(quint32 len_key);

signals:
  void onPixmapHasChanged();

private:
  Ui::MainWindow *ui;
  QVector<quint32> *divs;
  QVector<quint32> *replDivs;
  QString *dataString;
};

#endif // MAINWINDOW_H
