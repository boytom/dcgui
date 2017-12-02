#include "XPayDialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSvgWidget>
#include <QLabel>
#include <QTimer>
#include <QDebug>

XPayDialog::XPayDialog(QWidget *parent)
  :QDialog(parent)
{
  setObjectName(QStringLiteral("XPayDialog"));

  setWindowFlags(Qt::Window);

  QLabel *labelInfo = new QLabel(this);
  labelInfo->setObjectName(QStringLiteral(u"labelInfo"));
  labelInfo->setText(QString::fromWCharArray(L"在目前，你可以无限制地传播和使用 XCleaner 1.02，"
                                             L"<br />激励我们把更酷炫的技术实用化，做出更好的软件。"
                                             L"<p>有任何疑问，欢迎垂询"
                                             L"<b><font color=#FB792C>昂昂解决方案</b>。</p>"
                                             L"QQ：3060871692。"));
  labelInfo->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

  QPushButton *pushButtonClose = new QPushButton(this);
  pushButtonClose->setObjectName(QStringLiteral(u"pushButtonClose"));
  pushButtonClose->setText(QStringLiteral(u"关闭"));

  QFrame *horizontalLine = new QFrame(this);
  horizontalLine->setObjectName(QStringLiteral(u"horizontalLine"));
  horizontalLine->setFrameShape(QFrame::HLine);
  horizontalLine->setFrameShadow(QFrame::Sunken);

  QVBoxLayout *vbox = new QVBoxLayout;
  vbox->addWidget(labelInfo);
  vbox->addWidget(horizontalLine);
  vbox->addWidget(pushButtonClose);

  QSvgWidget *angang_svg_ = new QSvgWidget(this);
  angang_svg_->setObjectName(QStringLiteral(u"angang_svg"));
  angang_svg_->load(QString::fromWCharArray(L":/logo/logo-dcgui-zh_CN.svg"));
  angang_svg_->setMaximumHeight(181);
  angang_svg_->setMaximumWidth(202);
  angang_svg_->setMinimumHeight(181);
  angang_svg_->setMinimumWidth(202);

  QHBoxLayout *hbox = new QHBoxLayout(this);
  hbox->addWidget(angang_svg_);
  hbox->addLayout(vbox);

  QMetaObject::connectSlotsByName(this);
}

void XPayDialog::on_pushButtonClose_clicked()
{
  close();
}