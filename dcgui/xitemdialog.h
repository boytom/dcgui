#ifndef XItemDialog_H
#define XItemDialog_H

#include <QDialog>

class QCheckBox;
class XAspectRatioPixmapLabel;
class QLabel;
class QPushButton;
class QStandardItem;
class QString;
class QTextBrowser;
class QTreeView;

class XItemDialog : public QDialog
{
  Q_OBJECT
public:
  explicit XItemDialog(QTreeView *tree_view = nullptr,
                       QStandardItem *standard_item1 = nullptr,
                       QStandardItem *standard_item2 = nullptr,
                       QStandardItem *similarity_item = nullptr,
                       bool is_image_item = true,
                       QWidget *parent = nullptr);

  void setUnuseable();

private:
  bool is_image_item_{true};
  QTreeView *tree_view_{nullptr};
  QCheckBox *checkBoxItem1_{nullptr}, *checkBoxItem2_{nullptr};
  XAspectRatioPixmapLabel *labelItem1_{nullptr}, *labelItem2_{nullptr};
  QTextBrowser *textBrowser1_{nullptr}, *textBrowser2_{nullptr};
  QStandardItem *standard_item1_{nullptr}, *standard_item2_{nullptr}, *similarity_item_;
  QLabel *labelSimilarity_{nullptr}, *labelFileNameSimilarity_{nullptr};
  QPushButton *pushButtonClose_{nullptr};

  bool nead_to_set_back_{false};

  void setMouseTransparentAndTopmost();
  void setWindowMaxmize();
  void mySetMask(const QRect &newGlobalFrameGeometry);

  void set_text_browser_text(QTextBrowser *text_browser, const QString &file_path);
  wchar_t *read_file(const wchar_t *__restrict file_path,
                     unsigned long long *__restrict nwchars);
  wchar_t *read_file_utf8(const wchar_t *__restrict file_path,
                          unsigned long long *__restrict nwchars);
  wchar_t *read_file_utf16(const wchar_t *__restrict file_path,
                           unsigned long long *__restrict nwchars, int fbom);
  wchar_t *read_file_ansi(const wchar_t *__restrict file_path,
                          unsigned long long *__restrict nwchars);
  wchar_t *mbstr_to_wstr(const char *const __restrict src,
    unsigned long long int length_in_chars,
    unsigned long long int * __restrict length_in_wchars);

protected:
  virtual void closeEvent(QCloseEvent *event) override;
  virtual void changeEvent(QEvent *event) override;

private slots:
  void onSizeChanged(const QRect *newGlobalFrameGeometry);
  void onPositionChanged(const QRect *newGlobalFrameGeometry);

  void on_checkBoxItem1_stateChanged(int state);
  void on_checkBoxItem2_stateChanged(int state);

  void on_pushButtonClose_clicked();

  void check_item_when_checkbox_stateChanged(bool checked, QStandardItem *standard_item);
  void check_if_all_check_box_is_checked(QStandardItem *standard_item);
  void show_item(int unused);

signals:

public slots:
  void setItem(QStandardItem *standard_item1, QStandardItem *standard_item2,
               QStandardItem *similarity_item);
};

#endif // XItemDialog_H


/*
As I know this set the item as selected:
setCurrentIndex(indexAt(event->pos()));
or
treeView->setCurrentIndex(somemodelindex);

0
KA51O 5 years ago

Thanks broadpeak @setCurrentIndex(indexAt(event->pos()));@ worked.
I tried this

QPersistentModelIndex nextIndex = indexAt(QCursor::pos());
setCurrentIndex(nextIndex);

before but that didn't work. Any ideas why?
0
KA51O 5 years ago

@Vass
Your suggestion also worked when I used event->pos() instead of QCursor::pos().

Guess I forgot to map the Cursor position from global to widget coordinates. Stupid me.
0
broadpeak 5 years ago

    before but that didn¡¯t work. Any ideas why?

Because the index comes from the model and the mousePressEvent comes from the event object (through view).
The two is not the same.

*/
