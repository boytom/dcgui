#ifndef XPayDialog_H
#define XPayDialog_H

#include <QDialog>

class QSvgWidget;

class XPayDialog : public QDialog
{
  Q_OBJECT
public:
  XPayDialog(QWidget *parent = nullptr);

private slots:
  void on_pushButtonClose_clicked();

private:
  QSvgWidget *angang_svg_{nullptr};
};

#endif // XPayDialog_H
