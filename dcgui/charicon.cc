#include "charicon.h"

#include <QFont>
#include <QFontDatabase>
#include <QPushButton>


const QChar CharIcon::CLOSE_CHAR{0xf00d};
const QChar CharIcon::MAXIMUM_CHAR{0xf096};
const QChar CharIcon::RESTORE_CHAR{0xf079};
const QChar CharIcon::MINIMUM_CHAR{0xf068};
const QChar CharIcon::MENU_CHAR{0xf0c9};
const QChar CharIcon::MAINICON_CHAR{0xf015};

const QChar CharIcon::SETTING_GEAR_CHAR{0xf013};
const QChar CharIcon::PLUS_CHAR{0xf067};

const QChar CharIcon::DASHBOARD_CHAR{0xf0e4};
const QChar CharIcon::PIECHART_CHAR{0xf200};
const QChar CharIcon::ENVELOPE_CHAR{0xf003};
const QChar CharIcon::COMMENT_CHAR{0xf075};
const QChar CharIcon::LIST_CHAR{0xf03a};
const QChar CharIcon::CALENDAR_CHAR{0xf073};

const QChar CharIcon::CLOCK_CHAR{0xf017};
const QChar CharIcon::BELL_CHAR{0xf0f3};

const QString CharIcon::CLOSE_TIP{QString::fromStdWString(L"关闭")};
const QString CharIcon::MAXIMUM_TIP{QString::fromStdWString(L"最大化")};
const QString CharIcon::RESTORE_TIP{QString::fromStdWString(L"还原")};
const QString CharIcon::MINIMUM_TIP{QString::fromStdWString(L"最小化")};
const QString CharIcon::MENU_TIP{QString::fromStdWString(L"菜单")};

QMutex CharIcon::_mutex;
QFont CharIcon::_iconFont;
int CharIcon::init_{};

CharIcon::CharIcon(QObject *parent) : QObject(parent)
{
  QMutexLocker locker(&_mutex);

  if (init_)
    return;

  int fontId = QFontDatabase::addApplicationFont(QString::fromStdWString(L":/font/fontawesome-webfont.ttf"));
  QString fontName = QFontDatabase::applicationFontFamilies(fontId).at(0);
  _iconFont = QFont(fontName);
  init_ = 1;
}

CharIcon::~CharIcon()
{

}

void CharIcon::setClose(QAbstractButton *ab)
{
   setAbstractButtonIcon(ab, CLOSE_CHAR, &CLOSE_TIP);
}

void CharIcon::setMaximum(QAbstractButton *ab)
{
  setAbstractButtonIcon(ab, MAXIMUM_CHAR, &MAXIMUM_TIP);
}

void CharIcon::setMinimum(QAbstractButton *ab)
{
  setAbstractButtonIcon(ab, MINIMUM_CHAR, &MINIMUM_TIP);
}

void CharIcon::setRestore(QAbstractButton *ab)
{
  setAbstractButtonIcon(ab, RESTORE_CHAR, &RESTORE_TIP);
}

void CharIcon::setMenu(QAbstractButton *ab)
{
  setAbstractButtonIcon(ab, MENU_CHAR, &MENU_TIP);
}

void CharIcon::setSetting(QAbstractButton *ab)
{
  setAbstractButtonIcon(ab, SETTING_GEAR_CHAR);
}

void CharIcon::setAdd(QAbstractButton *ab)
{
  setAbstractButtonIcon(ab, PLUS_CHAR);
}

void CharIcon::setMainIcon(QLabel *lb)
{
  setLabelIcon(lb, MAINICON_CHAR);
}
