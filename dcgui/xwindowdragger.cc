#include "xwindowdragger.h"
#include <QMouseEvent>
#include <QStyleOption>
#include <QPainter>

XWindowDragger::XWindowDragger(QWidget *parent) : QWidget(parent)
{

}

void XWindowDragger::mousePressEvent(QMouseEvent *event)
{
  event->setAccepted(false); // 为什么这样也可以？处理了这样的事件，不应该设为true吗？

  QWidget *parent = parentWidget();

  QPoint nowpos;


  if (parent) {
    parent->setWindowOpacity(0.9);
    nowpos = parent->pos();
  }
  else
    nowpos = pos();

  if (event->button() == Qt::LeftButton) {
    _deltaOffset = event->globalPos() - nowpos;
    _mousePressed = true;
  }
}

void XWindowDragger::mouseMoveEvent(QMouseEvent *event)
{
  event->setAccepted(false); // 我需要后续的处理

  QWidget *parent = parentWidget();

  if (_mousePressed && (event->buttons() & Qt::LeftButton) && !isMaximized()) {
    if (parent)
      parent->move(event->globalPos() - _deltaOffset);
    else {
      setWindowOpacity(0.9);
      move(event->globalPos() - _deltaOffset);
    }
  }
}

void XWindowDragger::mouseReleaseEvent(QMouseEvent *event)
{
  QWidget *parent = parentWidget();

  if (parent)
    parent->setWindowOpacity(1.0);
  else
    setWindowOpacity(1.0);

  _mousePressed = false;
  event->setAccepted(false);
}

void XWindowDragger::paintEvent(QPaintEvent *event)
{
  Q_UNUSED(event);

  QStyleOption styleOption;
  styleOption.init(this);
  QPainter painter(this);
  style()->drawPrimitive(QStyle::PE_Widget, &styleOption, &painter, this);
}
