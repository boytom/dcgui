#include "xitemdialog.h"
#include <QDebug>
#include <QRegion>
#include <QApplication>
#include <QDesktopWidget>
#include <QLabel>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QSpacerItem>
#include <QStandardItem>
#include <QMessageBox>
#include <QCloseEvent>
#include <QEvent>
#include <QTimer>
#include <QTextBrowser>
#include <QTreeView>
#include <QLocale>
#include "import.h"

#include "xaspectratiopixmaplabel.h"
#include "itemhelper.h"

#include <stdio.h>
#include <wchar.h>
#include <tchar.h>
#include <uchar.h>
#include <sys/types.h>
#include <sys/stat.h>

#if _WIN32
#include <windows.h>
#include <Shellapi.h>
#endif

#define UTF16LE_BOM     0xFEFF      // UTF16 Little Endian Byte Order Mark
#define UTF16BE_BOM     0xFFFE      // UTF16 Big Endian Byte Order Mark
#define BOM_MASK        0xFFFF      // Mask for testing Byte Order Mark
#define UTF8_BOM        0xBFBBEF    // UTF8 Byte Order Mark
#define UTF16_BOMLEN    2           // No of Bytes in a UTF16 BOM
#define UTF8_BOMLEN     3           // No of Bytes in a UTF8 BOM

XItemDialog::XItemDialog(QTreeView *tree_view,
                         QStandardItem *standard_item1,
                         QStandardItem *standard_item2,
                         QStandardItem *similarity_item,
                         bool is_image_item,
                         QWidget *parent) :
  is_image_item_{is_image_item}, tree_view_{tree_view},
  standard_item1_{standard_item1}, standard_item2_{standard_item2},
  similarity_item_{similarity_item}, QDialog(parent)
{
  setObjectName(QStringLiteral("XItemDialog"));

  setWindowFlags(Qt::Window);
  resize(380, 400);
  if (is_image_item_) {
    labelItem1_ = new XAspectRatioPixmapLabel(this);
    labelItem1_->setObjectName(QStringLiteral(u"labelItem1"));
    labelItem2_ = new XAspectRatioPixmapLabel(this);
    labelItem2_->setObjectName(QStringLiteral(u"labelItem2"));
  }
  else {
    textBrowser1_ = new QTextBrowser(this);
    textBrowser1_->setObjectName(QStringLiteral(u"textBrowser1"));
    textBrowser2_ = new QTextBrowser(this);
    textBrowser2_->setObjectName(QStringLiteral(u"textBrowser2"));
  }

  QString del_file = is_image_item_ ? QStringLiteral(u"删除此图片")
      : QStringLiteral( u"删除此文档");
  checkBoxItem1_ = new QCheckBox(this);
  checkBoxItem1_->setObjectName(QStringLiteral(u"checkBoxItem1"));
  checkBoxItem1_->setText(del_file);
  checkBoxItem2_ = new QCheckBox(this);
  checkBoxItem2_->setObjectName(QStringLiteral(u"checkBoxItem2"));
  checkBoxItem2_->setText(del_file);
  labelSimilarity_ = new QLabel(this);
  labelSimilarity_->setObjectName(QStringLiteral(u"labelSmilarity"));
  labelFileNameSimilarity_ = new QLabel(this);
  labelFileNameSimilarity_->setObjectName(QStringLiteral(u"labelFileNameSimilarity"));
  pushButtonClose_ = new QPushButton(QStringLiteral(u"关闭"));
  pushButtonClose_->setObjectName(QStringLiteral(u"pushButtonClose"));

  QFrame *horizontalLine = new QFrame(this);
  horizontalLine->setObjectName(QStringLiteral(u"horizontalLine"));
  horizontalLine->setFrameShape(QFrame::HLine);
  horizontalLine->setFrameShadow(QFrame::Sunken);

  QSpacerItem *horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

  QVBoxLayout *vbox1 = new QVBoxLayout;
  if (is_image_item_)
    vbox1->addWidget(labelItem1_);
  else
    vbox1->addWidget(textBrowser1_);
  vbox1->addWidget(checkBoxItem1_);
  QVBoxLayout *vbox2 = new QVBoxLayout;
  if (is_image_item_)
    vbox2->addWidget(labelItem2_);
  else
    vbox2->addWidget(textBrowser2_);
  vbox2->addWidget(checkBoxItem2_);

  QHBoxLayout *hbox1 = new QHBoxLayout;
  hbox1->addLayout(vbox1);
  hbox1->addLayout(vbox2);

  QHBoxLayout *hbox2 = new QHBoxLayout;
  hbox2->addWidget(labelSimilarity_);
  hbox2->addWidget(labelFileNameSimilarity_);
  hbox2->addItem(horizontalSpacer);
  hbox2->addWidget(pushButtonClose_);

  QVBoxLayout *vbox3 = new QVBoxLayout(this);
  vbox3->addLayout(hbox1);
  vbox3->addWidget(horizontalLine);
  vbox3->addLayout(hbox2);

#if 0
  setAttribute(Qt::WA_DeleteOnClose);
#endif

  QMetaObject::connectSlotsByName(this);

  QMetaObject::invokeMethod(this, "show_item", Qt::QueuedConnection,
                            Q_ARG(int, 0));
  //init_image_item(0);
#if 0
  // 下面这两个函数不能换顺序
  setMouseTransparentAndTopmost();
  setWindowMaxmize();
#endif
}

void XItemDialog::setMouseTransparentAndTopmost()
{
  Qt::WindowFlags flags = windowFlags();
  setWindowFlags( Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::ToolTip); // 使用 Qt::TooTip后，后面要调用show()函数，才能使showMaximized()生效)
  setAttribute(Qt::WA_TranslucentBackground); // 设置背景透明，允许鼠标穿透，完全透明的地方才穿透；
  setWindowOpacity(0.3); // 窗口整体透明度，０－１从全透明到不透明

#if defined(_WIN32) && defined(Q_OS_WIN)
  HWND hwnd = (HWND) winId();
  LONG styles = GetWindowLong(hwnd, GWL_EXSTYLE);
  SetWindowLong(hwnd, GWL_EXSTYLE, styles
                | WS_EX_TRANSPARENT  // 鼠标穿透，稍微透明一点，就穿透
                /*| WS_EX_COMPOSITED   // 双缓冲*/
                | WS_EX_TOPMOST      // 顶层
                | WS_EX_LAYERED      // WS_EX_LAYERED windows and all windows
                // when DWM is enabled are rendered into an offscreen buffer
                // and that buffer is then rendered to the screen.  WM_PAINT
                // messages can be skipped to update the onscreen buffer so
                // long as the offscreen buffer is valid.
                );

  SetLayeredWindowAttributes(hwnd, COLORREF(RGB(0, 0, 0)), 71, LWA_ALPHA);
  // From MSDN:
  // URL: https://msdn.microsoft.com/en-us/library/windows/desktop/ms632599%28v=vs.85%29.aspx#layered
  // if the layered window has the WS_EX_TRANSPARENT extended window style, the
  // shape of the layered window will be ignored and the mouse events will
  // be passed to other windows underneath the layered window.
#endif
#ifdef Q_OS_LINUX
  XShapeCombineRectangles(QX11Info::display(), winId(), ShapeInput, 0,
                          0, NULL, 0, ShapeSet, YXBanded);
#endif
}

void XItemDialog::setWindowMaxmize()
{
  show();  // 加上这个调用，是因为在setWindowFlag中，使用了Qt::ToolTip,  Qt::ToolTip会让窗口位于顶层，Qt::ToolTip会隐藏窗口的任务条按钮
  showMaximized();
  showFullScreen();
}

void XItemDialog::mySetMask(const QRect &newGlobalFrameGeometry)
{
  QRect myGloableRect = frameGeometry();
  myGloableRect.moveTopLeft(QApplication::desktop()->mapToGlobal(myGloableRect.topLeft()));

  QRegion myGloableRegion(myGloableRect);
  QRegion newGloableRegion(newGlobalFrameGeometry);
  myGloableRegion -= newGloableRegion;
  setMask(myGloableRegion);
}

void XItemDialog::set_text_browser_text(QTextBrowser *text_browser, const QString &file_path)
{
  if (_waccess_s(file_path.toStdWString().c_str(), 04) != 0)
    return;

  if (file_path.endsWith(QStringLiteral(u".docx"))
      || file_path.endsWith(QStringLiteral(u".doc"))) {
    XDocText doc_text;
    memset(&doc_text, 0, sizeof(doc_text));
    if (dupclean_.doc_to_text(file_path.toStdWString().c_str(), &doc_text) == 0) {
      text_browser->setText(QString::fromWCharArray(doc_text.text, doc_text.length));
      dupclean_.destroy_doctext(&doc_text);
    }
  }
  else if (file_path.endsWith(QStringLiteral(u".txt"))) {
    unsigned long long int nwchars;
    wchar_t *wbuf;
    if ((wbuf = read_file(file_path.toStdWString().c_str(), &nwchars)) != nullptr) {
      text_browser->setText(QString::fromWCharArray(wbuf, nwchars));
      free(wbuf);
    }
  }
}

wchar_t *XItemDialog::read_file(const wchar_t *__restrict file_path,
                   unsigned long long *__restrict nwchars)
{
  FILE *fp;

  if (_wfopen_s(&fp, file_path, L"rbS") != 0)
    return nullptr;

  int fbom;
  unsigned long long int bytes_read;

  bytes_read = fread_s(&fbom, sizeof(fbom), 1llu, UTF8_BOMLEN, fp);

  fclose(fp);

  wchar_t *wbuf;

  int const count = (int)(bytes_read & 0xFFFFFFFFU);

  switch(count) {
    case -1:
      return nullptr;
      break;
    case UTF8_BOMLEN:
      if (fbom == UTF8_BOM) {
        wbuf = read_file_utf8(file_path, nwchars);
        break;
      }

    case UTF16_BOMLEN:
      if ((fbom & BOM_MASK) == UTF16BE_BOM) {
        wbuf = read_file_utf16(file_path, nwchars, UTF16BE_BOM);
        break;
      }

      if ((fbom & BOM_MASK) == UTF16LE_BOM) {
        wbuf = read_file_utf16(file_path, nwchars, UTF16LE_BOM);
        break;
      }

    default:
      /* ANSI */
      wbuf = read_file_ansi(file_path, nwchars);

      break;
  }

  return wbuf;
}

wchar_t *XItemDialog::read_file_utf8(const wchar_t *__restrict file_path,
                                     unsigned long long *__restrict nwchars)
{
  FILE *fp;

  if (_wfopen_s(&fp, file_path, L"rtS,ccs=utf-8") != 0)
    return nullptr;

  struct __stat64 st;

  if (_fstat64(_fileno(fp), &st) != 0) {
    fclose(fp);
    return nullptr;
  }

  if (st.st_size == 0llu)
    return nullptr;

  wchar_t *wbuf;

  if ((wbuf = (wchar_t *)malloc((st.st_size + 1) * sizeof(wchar_t))) == nullptr) {
    fclose(fp);
    return nullptr;
  }

  memset(wbuf, 0, (st.st_size + 1) * sizeof(wchar_t));

  size_t inter_nwchars = fread_s(wbuf,
      (st.st_size + 1) * sizeof(wchar_t),
      sizeof(wchar_t),
      st.st_size,
      fp);

  fclose(fp);

  if (inter_nwchars == 0llu) {
    free(wbuf);
    return nullptr;
  }

  if (nwchars != nullptr)
    *nwchars = inter_nwchars;

  return wbuf;
}

wchar_t *XItemDialog::read_file_utf16(const wchar_t *__restrict file_path,
                                      unsigned long long *__restrict nwchars,
                                      int fbom)
{
  FILE *fp;

  if (_wfopen_s(&fp, file_path, L"rbS,ccs=utf-16le") != 0)
    return nullptr;

  struct __stat64 st;

  if (_fstat64(_fileno(fp), &st) != 0) {
    fclose(fp);
    return nullptr;
  }

  if (st.st_size == 0llu)
    return nullptr;

  wchar_t *wbuf = (wchar_t *)malloc(st.st_size + sizeof(wchar_t));

  if (wbuf == nullptr) {
    fclose(fp);
    return nullptr;
  }

  memset(wbuf, 0, st.st_size + sizeof(wchar_t));

  size_t inter_nwchars = fread_s(wbuf,
      st.st_size + sizeof(wchar_t),
      sizeof(wchar_t),
      st.st_size / sizeof(wchar_t),
      fp);

  fclose(fp);

  if (inter_nwchars == 0llu) {
    free(wbuf);
    return nullptr;
  }

  if (fbom == UTF16BE_BOM) {
    wchar_t *bg, *ed;
    for (bg = wbuf, ed = wbuf + inter_nwchars; bg != ed; ++bg)
      *bg = ((*bg & 0xFFU) << 8) | ((*bg >> 8) & 0xFFU);
  }

  //token_list = split(gswh, &wbuf[1], nwchars - 1);

  if (nwchars != nullptr)
    *nwchars = inter_nwchars;

  return wbuf;
}

wchar_t *XItemDialog::read_file_ansi(const wchar_t *__restrict file_path,
                                     unsigned long long *__restrict nwchars)
{
  FILE *fp;

  if (_wfopen_s(&fp, file_path, L"rbS") != 0)
    return nullptr;

  struct __stat64 st;

  if (_fstat64(_fileno(fp), &st) != 0) {
    fclose(fp);
    return nullptr;
  }

  if (st.st_size == 0llu)
    return nullptr;

  char *buf;

  unsigned long long int buf_size = st.st_size + 1;

  buf = (char *)malloc(buf_size);
  if (buf == nullptr) {
    fclose(fp);
    return nullptr;
  }

  memset(buf, 0, buf_size);

  size_t nchars = fread_s(buf,
      buf_size,
      sizeof(char),
      buf_size,
      fp);

  fclose(fp);

  if (nchars == 0llu) {
    free(buf);
    return nullptr;
  }

  unsigned long long int inter_nwchars;
  wchar_t *wbuf = mbstr_to_wstr(buf, nchars, &inter_nwchars);

  free(buf);

  if (wbuf == nullptr)
    return nullptr;

  if (nwchars != nullptr)
    *nwchars = inter_nwchars;

  return wbuf;
}

wchar_t *XItemDialog::mbstr_to_wstr(const char *const __restrict src,
  unsigned long long int length_in_chars,
  unsigned long long int * __restrict length_in_wchars)
{
  if (src == NULL)
    return NULL;

  size_t wchars_required;

  if (mbstowcs_s(&wchars_required, NULL, 0u, src, 0u) != 0)
    return NULL;

  wchar_t *dest = (wchar_t *)malloc(wchars_required * sizeof(wchar_t));

  if (dest == NULL)
    return NULL;

  wmemset(dest, 0, wchars_required);

  size_t wchars_converted;
  mbstowcs_s(&wchars_converted, dest, wchars_required, src, wchars_required);
  if (length_in_wchars)
    *length_in_wchars = wchars_converted - 1;
  return dest;
}

void XItemDialog::onSizeChanged(const QRect *newGlobalFrameGeometry)
{
  mySetMask(*newGlobalFrameGeometry);
}

void XItemDialog::onPositionChanged(const QRect *newGlobalFrameGeometry)
{
  mySetMask(*newGlobalFrameGeometry);
}

void XItemDialog::setItem(QStandardItem *standard_item1,
                          QStandardItem *standard_item2,
                          QStandardItem *similarity_item)
{

  if (standard_item1 == nullptr || standard_item1 == nullptr
      || standard_item1 == nullptr)
    return;

  standard_item1_ = standard_item1;
  standard_item2_ = standard_item2;
  similarity_item_ = similarity_item;

  show_item(0);
}

void XItemDialog::setUnuseable()
{
  checkBoxItem1_->setDisabled(true);
  checkBoxItem1_->setEnabled(false);

  checkBoxItem2_->setDisabled(true);
  checkBoxItem2_->setEnabled(false);

  labelSimilarity_->setDisabled(true);
  labelSimilarity_->setEnabled(false);

  standard_item1_ = standard_item2_ = nullptr;
  tree_view_ = nullptr;
}

void XItemDialog::closeEvent(QCloseEvent *event)
{
  using PQStandardItem = QStandardItem *;
  using PXItemDialog = XItemDialog *;

  QMetaObject::invokeMethod(parent(), "delete_item_dialog",
                            Qt::QueuedConnection,
                            Q_ARG(void*, reinterpret_cast<void *>(standard_item1_)),
                            Q_ARG(void*, reinterpret_cast<void *>(this)));
  event->accept();
}

void XItemDialog::changeEvent(QEvent *event)
{
  if(event->type() != QEvent::ActivationChange || !isActiveWindow())
    return;

  // 现在，这个窗口是个被激活的窗口
  if (standard_item1_ == nullptr || standard_item2_ == nullptr
      || tree_view_ == nullptr) // 窗口已不可用
    return;

  tree_view_->setCurrentIndex(standard_item1_->index());
}

void XItemDialog::on_checkBoxItem1_stateChanged(int state)
{
  check_item_when_checkbox_stateChanged(state == Qt::Checked, standard_item1_);
  check_if_all_check_box_is_checked(standard_item1_);
}

void XItemDialog::on_checkBoxItem2_stateChanged(int state)
{
  check_item_when_checkbox_stateChanged(state == Qt::Checked, standard_item2_);
  check_if_all_check_box_is_checked(standard_item2_);
}

void XItemDialog::on_pushButtonClose_clicked()
{
  close();
}

void XItemDialog::check_item_when_checkbox_stateChanged(bool checked,
                                                       QStandardItem *standard_item)
{
  if (!nead_to_set_back_) {
    nead_to_set_back_ = true;
    return;
  }

  QStandardItem *parent_item{standard_item->parent()};

  if (parent_item == nullptr)
    return;

  set_item_will_be_deleted(standard_item, checked);
  QMetaObject::invokeMethod(parent(), "count_file_to_be_deleted",
                            Qt::QueuedConnection,
                            Q_ARG(void*, reinterpret_cast<void *>(standard_item)),
                            Q_ARG(bool, checked));
}

void XItemDialog::check_if_all_check_box_is_checked(QStandardItem *standard_item)
{
  if (!checkBoxItem1_->isChecked() || !checkBoxItem2_->isChecked())
    return;

  QStandardItem *parent_item{standard_item->parent()};

  if (parent_item == nullptr)
    return;

  QFont mono_font(QString::fromWCharArray(L"Consolas"));
  QMessageBox message_box(this);
  message_box.setFont(mono_font);
  message_box.setIcon(QMessageBox::Question);
  message_box.setWindowTitle(QString::fromWCharArray(L"询问"));
  message_box.setText(is_image_item_ ? QString::fromWCharArray(L"确定这两个图片<b><FONT COLOR='#ff0000'>都要删除</FONT></b>吗？")
                                     : QString::fromWCharArray(L"确定这两个文档<b><FONT COLOR='#ff0000'>都要删除</FONT></b>吗？"));
  message_box.addButton(QString::fromWCharArray(L"是"), QMessageBox::YesRole);
  QPushButton *no_button = message_box.addButton(QString::fromWCharArray(L"否"), QMessageBox::NoRole);
  message_box.setDefaultButton(no_button);
  message_box.exec();
  if (message_box.clickedButton() == no_button) {
    (qobject_cast<QCheckBox *>(sender()))->setCheckState(Qt::Unchecked);
    set_item_will_be_deleted(standard_item, false);
  }
}

void XItemDialog::show_item(int unused)
{
  Q_UNUSED(unused);

  // 在 init_image_item 触发 stateChanged 之后，在 checkbox 的 stateChanged
  // handler 里，会根据 checkBox 的状态设置 standard_item 的状态。这个设置的方
  // 式在用户点击checkBox时是有用的。在这里是根据 standard_item 的状设置checkbox
  // 的状态，所以不需要再设真回 standard_item

  if (is_image_item_) {
    labelSimilarity_->setText(QStringLiteral(u"图片相似度：") + similarity_item_->text());

    QPixmap image1(standard_item1_->text());
    QPixmap image2(standard_item2_->text());
    labelItem1_->setPixmap(image1);
    labelItem2_->setPixmap(image2);
  }
  else {
    labelSimilarity_->setText(QStringLiteral(u"文档相似度：") + similarity_item_->text());
    set_text_browser_text(textBrowser1_, standard_item1_->text());
    set_text_browser_text(textBrowser2_, standard_item2_->text());
  }

  XCompareHandle compare_handle;
  if ((compare_handle = dupclean_.new_compare()) != nullptr) {
    XCompareResult compare_result;
    dupclean_.compare_two_file_name(compare_handle, &compare_result,
                                    standard_item1_->text().toStdWString().c_str(),
                                    standard_item2_->text().toStdWString().c_str());
    dupclean_.delete_compare(compare_handle);
    QLocale zh_locale;
    labelFileNameSimilarity_->setText(QStringLiteral(u"文件名相似度：")
                                      + zh_locale.toString(compare_result.file_name.similarity));
  }

  // 如果有任何一个项目是选中的，need_to_set_back 就是 false;
  nead_to_set_back_ = !is_item_will_be_deleted(standard_item1_);
  // 如果没有选中，不会触发 stateChange
  checkBoxItem1_->setChecked(is_item_will_be_deleted(standard_item1_));

  nead_to_set_back_ = !is_item_will_be_deleted(standard_item2_);
  checkBoxItem2_->setChecked(is_item_will_be_deleted(standard_item2_));
}
