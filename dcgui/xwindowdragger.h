#ifndef XWINDOWDRAGGER_H
#define XWINDOWDRAGGER_H

#include <QWidget>

class XWindowDragger : public QWidget
{
  Q_OBJECT
public:
  explicit XWindowDragger(QWidget *parent = 0);

protected:
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void paintEvent(QPaintEvent *event) override;

private:
  bool _mousePressed{};
  QPoint _deltaOffset;

signals:

public slots:
};

#endif // XWINDOWDRAGGER_H
