#ifndef XASPECTRATIOPIXMAPLABEL_H
#define XASPECTRATIOPIXMAPLABEL_H

#include <QLabel>
#include <QPixmap>
#include <QResizeEvent>

class XAspectRatioPixmapLabel : public QLabel
{
    Q_OBJECT
public:
    explicit XAspectRatioPixmapLabel(QWidget *parent = 0);
    virtual int heightForWidth(int width) const;
    virtual QSize sizeHint() const;
    QPixmap scaledPixmap() const;
public slots:
    void setPixmap(const QPixmap &p);
    void resizeEvent(QResizeEvent *);
private:
    QPixmap pix_;
};

#endif // XASPECTRATIOPIXMAPLABEL_H
