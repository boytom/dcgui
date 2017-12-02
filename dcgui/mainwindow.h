#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCryptographicHash>
#include <QPainter>
#include <QStandardItem>
#include <QStandardItemModel>

#include <vector>
#include <list>
#include <map>
#include <string>
#include <algorithm>

#include "ui_mainwindow.h"
#include "comparethread.h"

class XItemDialog;

#ifdef linux
enum {MY_PATH_MAX = PATH_MAX};
#else
enum {MY_PATH_MAX = 4096};
#endif

#if defined(_MSC_VER)
#pragma warning(disable : 4200)
#endif

class QGroupBox;

struct FilePath {
  unsigned int length;
  char path[0];
};

struct MapFile {
  unsigned int key_length;
  unsigned int file_number;
  char key_data[64];
};

class MainWindow : public XWindowDragger, private Ui::MainWindow
{
  Q_OBJECT

public:
  /* 因为要用来实例化QVariant，所以才放到了 public 里 */
  struct XFileAttribute
  {
    int path_length, name_length, dir_length;
    long long int create_time, modify_time;
    unsigned long long int size;
  };

  explicit MainWindow(QWidget *parent = 0);
  virtual ~MainWindow();

  void startWorkInAThread(XThreadParameter &thread_parameter);
  void setAppStyleSheet(const QString &styleSheetResource);
  void initSystemIcon();

signals:
  void stop_worker();
  void pause_worker();
  void resume_worker();
  void treeViewSelectFileAttribute_expand_all();

private slots:
  void on_pushButtonClose_clicked();
  void on_pushButtonMax_clicked();
  void on_pushButtonMin_clicked();

  void on_addButton_clicked();
  void on_delButton_clicked();
  void on_startButton_clicked();
  void on_stopButton_clicked();
  void on_pauseButton_clicked();
  void on_loadButton_clicked();
  void on_pushButtonPrev_clicked();
  void on_pushButtonNext_clicked();
  void on_pushButtonJump_clicked();
  void on_pushButtonDelete_clicked();
  void on_pushButtonAbout_clicked();
  void on_pushButtonDonate_clicked();

  void on_checkBoxDocx_stateChanged(int state);
  void on_checkBoxTxt_stateChanged(int state);
  void on_checkBoxPng_stateChanged(int state);
  void on_checkBoxJpeg_stateChanged(int state);
  void on_checkBoxJpg_stateChanged(int state);
  void on_checkBoxBmp_stateChanged(int state);

  void on_checkBoxSys_stateChanged(int state);
  void on_checkBoxHide_stateChanged(int state);
  void on_checkBoxReadOnly_stateChanged(int state);
  void on_checkBoxArchive_stateChanged(int state);

  void on_groupBoxFileAttribute_toggled(bool on);
  void on_groupBoxSize_toggled(bool on);
  void on_groupBoxCreateTime_toggled(bool on);
  void on_groupBoxModifyTime_toggled(bool on);
  void on_groupBoxAccessTime_toggled(bool on);

  void on_radioButtonLongestFilePath_toggled(bool checked);
  void on_radioButtonShortestFilePath_toggled(bool checked);
  void on_radioButtonLongestFileName_toggled(bool checked);
  void on_radioButtonShortestFileName_toggled(bool checked);
  void on_radioButtonLongestDirName_toggled(bool checked);
  void on_radioButtonShortestDirName_toggled(bool checked);
  void on_radioButtonNewestCreateTime_toggled(bool checked);
  void on_radioButtonOldestCreateTime_toggled(bool checked);
  void on_radioButtonNewestModifyTime_toggled(bool checked);
  void on_radioButtonOldestModifyTime_toggled(bool checked);
  void on_radioButtonLeftOnlyOne_toggled(bool checked);
  void on_radioButtonCancelAll_toggled(bool checked);

  void on_treeViewResult_clicked(const QModelIndex &index);

  void on_tabWidget_currentChanged(int index);

  void handle_crypt_hash_result(QMap<QByteArray, QString> *crypt_hash_result);

  void on_treeViewResult_collapsed(const QModelIndex &index);
  void on_treeViewResult_expanded(const QModelIndex &index);

  void on_treeViewSelectFileAttribute_collapsed(const QModelIndex &index);
  void on_treeViewSelectFileAttribute_expanded(const QModelIndex &index);

  void showFilePath(void);
  void delete_item_dialog(void *v_item, void *v_dialog);
  void count_file_to_be_deleted(void *v_item, bool to_be_deleted);
  void show_file_to_be_deleted_size(unsigned long long int count,
                                    unsigned long long int size);
private:
  struct XMaxMinItem
  {
    QStandardItem *parent, *max, *min;
  };

  unsigned long long int file_to_be_deleted_size_{0llu};

  CompareThread *worker_thread_{nullptr};
  QStandardItemModel *tree_model_result_{nullptr}, *tree_model_file_attribute_{nullptr};
  XThreadParameter thread_parameter_;
  XCompareHandle compare_handle_{nullptr};

  QStandardItem *tree_parent_item_{nullptr}, *tree_text_item_{nullptr},
  *tree_image_item_{nullptr}, *tree_crypt_hash_item_{nullptr},
  *tree_file_path_item_{nullptr};

  std::list<XMaxMinItem> items_sort_by_file_path_, items_sort_by_file_name_,
  items_sort_by_dir_name_, items_sort_by_create_time_, items_sort_by_modify_time_;

  std::list<QStandardItem *> items_only_left_one_;

  std::map<QStandardItem *, XItemDialog *> item_dialog_map_;

  std::map<std::wstring, unsigned long long int> file_to_be_deleted_map_;

  static const wchar_t *step_tip_[];

  bool eventFilter(QObject *watched, QEvent *event) override;

  void dump_map(QMap<QByteArray, QString> *result, const QString &file_path);
  void load_map(QMap<QByteArray, QString> *map, const QString &file_path);

  void show_map_to_tree(QMap<QByteArray, QString> *crypt_hash_result);
  void show_crypt_hash_result_to_tree(QMap<QByteArray, QString> *crypt_hash_result);
  void show_compare_result_to_tree(const XCompareResult *compare_result);
  void process_zero_count_row_tree(XThreadParameter &thread_parameter);

  // TODO: write_script 需要升级以支持 XCompareResult vector
  void write_script(QMap<QByteArray, QString> *crypt_hash_result, const QString &file_path);

  QString get_realpath(const QString &relative_path);
  int get_file_attribute(const QString &file_path, XFileAttribute *file_attribute);

  void setup_ui_for_boot();
  void enable_something_disabled_when_boot();

  void on_ext_name_text_stateChanged(int state);
  void on_ext_name_image_stateChanged(int state);
  void check_if_all_checkbox_are_selected(
      const std::vector<QCheckBox *> &checkBoxExtName, QGroupBox *parent);

  void on_treeview_crypt_hash_item_clicked_check_or_uncheck(const QModelIndex &index);
  void on_treeview_item_clicked_show_file_attribute(const QModelIndex &index);
  void do_show_file_attribute(const XFileAttribute &file_attribute, const QString &file_path);
  void on_treeview_image_item_clicked_show_image(const QModelIndex &index);
  void on_treeview_text_item_clicked_show_text(const QModelIndex &index);

  void on_file_attribute_toggled(bool on);

  void init_thread_parameter(XThreadParameter &thread_parameter);
  void init_treeViewResult();

  void select_max_min_items(std::list<XMaxMinItem> *current_max_min_item,
                            bool maxmin, bool checked);

  void RecycleTreeViewItem(QStandardItem *item, std::wstring *file_paths,
                           std::list<QStandardItem *> *item_list);
  int RecycleFileOnWindows(const std::wstring &file_paths);

  QRect makeRectangle( const QPoint& first, const QPoint& second );
  void closeEvent(QCloseEvent *event) final;
};

Q_DECLARE_METATYPE(MainWindow::XFileAttribute)
#endif // MAINWINDOW_H
