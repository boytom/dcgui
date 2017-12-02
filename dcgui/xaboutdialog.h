#ifndef XABOUTDIALOG_H
#define XABOUTDIALOG_H

#include <QDialog>

class QSvgWidget;

class XAboutDialog : public QDialog
{
  Q_OBJECT
public:
  XAboutDialog(QWidget *parent = nullptr);

private slots:
  void load_angang_svg();
  void on_pushButtonClose_clicked();

private:
  QSvgWidget *angang_svg_{nullptr};
};

#endif // XABOUTDIALOG_H
