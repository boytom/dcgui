#include "xaspectratiopixmaplabel.h"
//#include <QDebug>

XAspectRatioPixmapLabel::XAspectRatioPixmapLabel(QWidget *parent) :
    QLabel(parent)
{
    this->setMinimumSize(1, 1);
    setScaledContents(false);
}

void XAspectRatioPixmapLabel::setPixmap(const QPixmap &p)
{
    pix_ = p;
    QLabel::setPixmap(scaledPixmap());
}

int XAspectRatioPixmapLabel::heightForWidth(int width) const
{
    return pix_.isNull() ? this->height() : ((qreal)pix_.height() * width) / pix_.width();
}

QSize XAspectRatioPixmapLabel::sizeHint() const
{
    int w = this->width();
    return QSize(w, heightForWidth(w));
}

QPixmap XAspectRatioPixmapLabel::scaledPixmap() const
{
    return pix_.scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void XAspectRatioPixmapLabel::resizeEvent(QResizeEvent *e)
{
    if(!pix_.isNull())
        QLabel::setPixmap(scaledPixmap());
}
