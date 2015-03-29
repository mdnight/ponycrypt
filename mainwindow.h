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
  QList<quint16> randSeq(quint16 k, qint16 poli);
  QImage *im1, *im2;

private:
  Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
