#ifndef CHARICON_H
#define CHARICON_H

#include <QObject>
#include <QMutex>
#include <QLabel>
#include <QAbstractButton>
#include <QStringBuilder>

class QPushButton;

class CharIcon : public QObject
{
  Q_OBJECT

signals:

public slots:

public:

  virtual ~CharIcon();

  static const QChar CLOSE_CHAR;
  static const QChar MAXIMUM_CHAR;
  static const QChar RESTORE_CHAR;
  static const QChar MINIMUM_CHAR;
  static const QChar MENU_CHAR;
  static const QChar MAINICON_CHAR;

  static const QChar SETTING_GEAR_CHAR;
  static const QChar PLUS_CHAR;

  static const QChar DASHBOARD_CHAR;
  static const QChar PIECHART_CHAR;
  static const QChar ENVELOPE_CHAR;
  static const QChar COMMENT_CHAR;
  static const QChar LIST_CHAR;
  static const QChar CALENDAR_CHAR;

  static const QChar CLOCK_CHAR;
  static const QChar BELL_CHAR;

  static const QString CLOSE_TIP;
  static const QString MAXIMUM_TIP;
  static const QString RESTORE_TIP;
  static const QString MINIMUM_TIP;
  static const QString MENU_TIP;

  static CharIcon *Instance()
  {
    static CharIcon _instance;
    return &_instance;
  }

  inline void setAbstractButtonIcon(QAbstractButton *ab, const QChar &icon, const QString *tip = nullptr, int size = 10)
  {
    _iconFont.setPointSize(size);
    ab->setFont(_iconFont);
    ab->setText(icon);
    if (tip)
      ab->setToolTip(*tip);
  }

  inline void setAbstractButtonIcon(QAbstractButton *ab, const QChar &icon, const QString &text, const QString *tip = nullptr, int size = 10)
  {
    _iconFont.setPointSize(size);
    ab->setFont(_iconFont);
    ab->setText(QString(icon) % text);
    if (tip)
      ab->setToolTip(*tip);
  }

  inline void setLabelIcon(QLabel *lb, const QChar &icon, const QString *tip = nullptr, int size = 10)
  {
    _iconFont.setPointSize(size);
    lb->setFont(_iconFont);
    lb->setText(icon);
    if (tip)
      lb->setToolTip(*tip);
  }

  void setClose(QAbstractButton *ab);
  void setMaximum(QAbstractButton *ab);
  void setMinimum(QAbstractButton *ab);
  void setRestore(QAbstractButton *ab);
  void setMenu(QAbstractButton *ab);
  void setSetting(QAbstractButton *ab);
  void setAdd(QAbstractButton *ab);

  void setMainIcon(QLabel *lb);

private:
  explicit CharIcon(QObject *parent = 0);
  static QFont _iconFont;
  static QMutex _mutex;
  static int init_;
};

#endif // CHARICON_H
