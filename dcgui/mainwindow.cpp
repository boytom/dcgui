#include "mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QSlider>
#include <QSpinBox>
#include <QList>
#include <QDebug>
#include <QCloseEvent>
#include <QStringList>
#include <QDateTime>
#include <QDir>
#include <QLocale>
#include <QFont>
#include <QBrush>
#include <QVariant>
#include <QPixmap>
#include <QRect>
#include <QStatusBar>
#include <QSvgWidget>

#include <climits>
#include <cstdlib>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timeb.h>

#include "xitemdialog.h"
#include "xaboutdialog.h"
#include "xpaydialog.h"
#include "xaspectratiopixmaplabel.h"
#include "itemhelper.h"
#include "charicon.h"

#if _WIN32
#include <windows.h>
#include <Shellapi.h>
#endif

/*
 * https://msdn.microsoft.com/en-us/library/mt708823.aspx
 * This compiler directive is obsolete in Visual Studio 2015 Update 2. We
 * recommend that you use the /execution-charset:utf-8 or /utf-8 compiler options
 * together with using the u8 prefix on narrow character and string literals that
 * contain extended characters.
 */
  /*L"第３／８步，设置是否比较文件名及相关参数",*/
const wchar_t *MainWindow::step_tip_[] =
{
  L"第１／４步，选择比较方法，设置相关参数",
  L"第２／４步，设置参与比较文件的文件应该满足的条件",
  L"第３／４步，开始比较",
  L"第４／４步，比较结果",
  nullptr
};

MainWindow::MainWindow(QWidget *parent) : XWindowDragger(parent)
{
  setupUi(this);

  initSystemIcon();
  setup_ui_for_boot();

#if defined(__linux__)
  setAppStyleSheet(QString::fromStdWString(L":/css/dcguilinux.css"));
#elif defined(_MSC_VER)
  setAppStyleSheet(QString::fromStdWString(L":/css/dcguiwindows.css"));
#endif

  init_dupclean_function_table();
  dupclean_.init_compare();
}

MainWindow::~MainWindow()
{
  if (tree_model_result_)
    delete tree_model_result_;
  if (tree_model_file_attribute_)
    delete tree_model_file_attribute_;

  dupclean_.destroy_compare();
  destroy_dupclean_function_table();
}

void MainWindow::setAppStyleSheet(const QString &styleSheetResource)
{
  QFile file(styleSheetResource);
  file.open(QFile::ReadOnly);
  qApp->setStyleSheet(file.readAll());
  setStyleSheet(file.readAll());
  qApp->setPalette(QPalette(QColor("#F0F0F0")));
}

void MainWindow::initSystemIcon()
{
  QCoreApplication::setApplicationName(QStringLiteral(u"XCleaner"));

  //设置窗体标题栏隐藏
  setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);
  //setWindowFlags(Qt::CustomizeWindowHint);

  // 新建状态栏
  QStatusBar *statusBar = new QStatusBar(this);
  statusBar->setObjectName(QStringLiteral(u"statusBar"));
  verticalLayout_3->addWidget(statusBar);

  labelWindowText->setText(QString::fromWCharArray(L"XCleaner-重复文件查找和清理工具"));

  // 加载svg icon
  QSvgWidget *x_icon = new QSvgWidget(this);
  x_icon->load(QStringLiteral(u":/icon/icon-dcgui.svg"));
  x_icon->setMaximumHeight(32);
  x_icon->setMaximumWidth(32);
  x_icon->setMinimumHeight(32);
  x_icon->setMinimumWidth(32);
  horizontalLayout_windowTitle->insertWidget(0, x_icon);

  //安装事件监听器,让标题栏识别鼠标双击
  widgetWindowTitle->installEventFilter(this);
  CharIcon::Instance()->setClose(pushButtonClose);
  CharIcon::Instance()->setMaximum(pushButtonMax);
  CharIcon::Instance()->setMinimum(pushButtonMin);
}

void MainWindow::on_pushButtonClose_clicked()
{
  close();
}

void MainWindow::on_pushButtonMax_clicked()
{
  if (isMaximized()) {
    showNormal();
    CharIcon::Instance()->setMaximum(pushButtonMax);
  }
  else {
    showMaximized();
    CharIcon::Instance()->setRestore(pushButtonMax);
  }
}

void MainWindow::on_pushButtonMin_clicked()
{
  showMinimized();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
  if (event->type() == QEvent::MouseButtonDblClick) {
      on_pushButtonMax_clicked();
      return true;
  }
  return QObject::eventFilter(watched, event);
}

void MainWindow::on_addButton_clicked()
{
  QString dir{QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this, QStringLiteral(u"请选择要扫描的目录")))};
  if(dir.isEmpty())
    return;
  QList<QListWidgetItem *> item_list = listWidget->findItems(dir, Qt::MatchExactly);

  if (!item_list.empty()) {
    QMessageBox mess(this);
    mess.setWindowTitle(QString::fromWCharArray(L"提示"));
    mess.setIcon(QMessageBox::Information);
    mess.setText(QString::fromWCharArray(L"\"") + dir + QString::fromWCharArray(L"\" 已放入列表。"));
    mess.addButton(QString::fromWCharArray(L"确定"), QMessageBox::AcceptRole);
    mess.exec();
    return;
  }

  QStringList path_list;
  for (int i = 0; i < listWidget->count(); ++i)
    path_list.push_back(listWidget->item(i)->text());

  path_list.push_back(dir);

  QStringList::iterator iti, itj;
  for (iti = path_list.begin(); iti != path_list.end(); ++iti)
    for (itj = iti, ++itj; itj != path_list.end(); ++itj) {
      if (iti->contains(*itj) || itj->contains(*iti)) {
        QMessageBox mess(this);
        mess.setWindowTitle(QString::fromWCharArray(L"错误"));
        mess.setIcon(QMessageBox::Critical);
        if (iti->contains(*itj))
          mess.setText(QString::fromWCharArray(L"\"") + *iti + QString::fromWCharArray(L"\" 是 \"") + *itj + QString::fromWCharArray(L"\" 的子目录，这是不允许的，添加失败！"));
        else if (itj->contains(*iti))
          mess.setText(QString::fromWCharArray(L"\"") + *itj + QString::fromWCharArray(L"\" 是 \"") + *iti + QString::fromWCharArray(L"\" 的子目录，这是不允许的，添加失败！"));
        mess.addButton(QString::fromWCharArray(L"确定"), QMessageBox::AcceptRole);
        mess.exec();
        return;
      }
    }

  listWidget->addItem(dir);
  listWidget->sortItems();
}

void MainWindow::on_delButton_clicked()
{
  int row = listWidget->currentRow();

  if(row == -1) {
    QMessageBox msg(this);
    msg.setWindowTitle(QString::fromWCharArray(L"警告"));
    msg.setIcon(QMessageBox::Warning);
    msg.setText(QStringLiteral(u"请首先选择要删除的目录，再点击删除按钮（只从列表中删除，不会删除磁盘上的目录）。"));
    msg.addButton(QString::fromStdU16String(u"确定"), QMessageBox::AcceptRole);
    msg.exec();
  }
  QListWidgetItem *item = listWidget->takeItem(listWidget->currentRow());
  delete item;
#if 0
  if(listWidget->count() == 0)
    setup_ui_for_boot();
#endif
}

void MainWindow::on_startButton_clicked()
{

  if(listWidget->count() == 0) {
    QMessageBox message_box(this);
    message_box.setWindowTitle(QString::fromWCharArray(L"提示"));
    message_box.setIcon(QMessageBox::Information);
    message_box.setText(QStringLiteral(u"请先选择要对比的目录。"));
    message_box.addButton(QString::fromWCharArray(L"确定"), QMessageBox::AcceptRole);
    message_box.exec();
  }
  else if (!worker_thread_) {
    init_thread_parameter(thread_parameter_);

    if(thread_parameter_.dir_list->isEmpty())
      return;

#if defined(__FREE_VERSION__)
    {
      QMessageBox message_box(this);
      message_box.setWindowTitle(QString::fromWCharArray(L"友情提示"));
      message_box.setIcon(QMessageBox::Information);
      message_box.setText(QStringLiteral(u"免费版本没有对比文件哈希值的功能。"));
      message_box.addButton(QString::fromWCharArray(L"确定"), QMessageBox::AcceptRole);
      message_box.exec();
    }
#endif

    init_treeViewResult();

    startWorkInAThread(thread_parameter_);
  }
  else {
    QMessageBox message_box(this);
    message_box.setWindowTitle(QString::fromWCharArray(L"提示"));
    message_box.setIcon(QMessageBox::Information);
    message_box.setText(QStringLiteral(u"正在比较文件，请稍候……"));
    message_box.addButton(QString::fromWCharArray(L"确定"), QMessageBox::AcceptRole);
    message_box.exec();
  }
}

void MainWindow::on_stopButton_clicked()
{
  if(worker_thread_) {
    QMessageBox message_box(this);
    message_box.setIcon(QMessageBox::Information);
    message_box.addButton(QString::fromWCharArray(L"确定"), QMessageBox::AcceptRole);
    if (worker_thread_->get_stop()) {
      message_box.setWindowTitle(QString::fromWCharArray(L"提示"));
      message_box.setText(QStringLiteral(u"正在停止，请稍候……"));
      message_box.exec();
      return;
    }
    else {
      message_box.setWindowTitle(QString::fromWCharArray(L"询问"));
      message_box.setText(QStringLiteral(u"确定要结束比较吗？"));
      QPushButton *cancel_button = message_box.addButton(QString::fromWCharArray(L"取消"), QMessageBox::RejectRole);
      message_box.setDefaultButton(cancel_button);
      message_box.exec();
      if (message_box.clickedButton() == cancel_button)
        return;
    }

    if(worker_thread_->get_pause()) {
      emit resume_worker();
      pauseButton->setText(QString::fromWCharArray(L"暂停"));
    }
    stopButton->setEnabled(false);
    emit stop_worker();
#if 0
    worker_thread_->wait();
#endif
    stopButton->setEnabled(true);
  }
  else {
    QMessageBox message_box(this);
    message_box.setWindowTitle(QString::fromWCharArray(L"提示"));
    message_box.setIcon(QMessageBox::Information);
    message_box.setText(QStringLiteral(u"还没有开始比较！"));
    message_box.addButton(QString::fromWCharArray(L"确定"), QMessageBox::AcceptRole);
    message_box.exec();
  }
}

void MainWindow::on_pauseButton_clicked()
{
  if(!worker_thread_) {
    QMessageBox message_box(this);
    message_box.setWindowTitle(QString::fromWCharArray(L"提示"));
    message_box.setIcon(QMessageBox::Information);
    message_box.setText(QStringLiteral(u"还没有开始比较！"));
    message_box.addButton(QString::fromWCharArray(L"确定"), QMessageBox::AcceptRole);
    message_box.exec();
  }
  else if(!worker_thread_->get_pause()) {
    pauseButton->setEnabled(false);
    emit pause_worker();
    pauseButton->setText(QString::fromWCharArray(L"继续"));
    pauseButton->setEnabled(true);
  }
  else {
    pauseButton->setEnabled(false);
    emit resume_worker();
    pauseButton->setText(QString::fromWCharArray(L"暂停"));
    pauseButton->setEnabled(true);
  }
}

void MainWindow::on_loadButton_clicked()
{
  // disable any buttons

  loadButton->setEnabled(false);

  QMap<QByteArray, QString> map;
  load_map(&map, get_realpath(lineEditFilePath->text()));
  show_crypt_hash_result_to_tree(&map);

  loadButton->setEnabled(true);
}

void MainWindow::on_pushButtonPrev_clicked()
{
  if (tabWidget->currentIndex() > 0)
    tabWidget->setCurrentIndex(tabWidget->currentIndex() - 1);
  labelStep->setText(QString::fromStdWString(step_tip_[tabWidget->currentIndex()]));
}

void MainWindow::on_pushButtonNext_clicked()
{
  tabWidget->setCurrentIndex(tabWidget->currentIndex() + 1);
  labelStep->setText(QString::fromStdWString(step_tip_[tabWidget->currentIndex()]));
}

void MainWindow::on_pushButtonJump_clicked()
{
  tabWidget->setCurrentIndex(2);
  labelStep->setText(QString::fromStdWString(step_tip_[tabWidget->currentIndex()]));
}

void MainWindow::on_pushButtonDelete_clicked()
{
  int i{0};
  std::wstring file_paths;
  std::list<QStandardItem *> item_list;
  QStandardItem *child_item1, *child_item2;

  while ((child_item1 = tree_text_item_->child(i, 0)) != nullptr
         && (child_item2 = tree_text_item_->child(i, 1)) != nullptr) {
    RecycleTreeViewItem(child_item1, &file_paths, &item_list);
    RecycleTreeViewItem(child_item2, &file_paths, &item_list);
    ++i;
  }

  i = 0;
  while ((child_item1 = tree_image_item_->child(i, 0)) != nullptr
         && (child_item2 = tree_image_item_->child(i, 1)) != nullptr) {
    RecycleTreeViewItem(child_item1, &file_paths, &item_list);
    RecycleTreeViewItem(child_item2, &file_paths, &item_list);
    ++i;
  }

  i = 0;
  while ((child_item1 = tree_crypt_hash_item_->child(i, 0)) != nullptr) {
    int j{0};
    while ((child_item2 = child_item1->child(j++, 0)) != nullptr)
      RecycleTreeViewItem(child_item2, &file_paths, &item_list);
    ++i;
  }

  if (file_paths.empty())
    return;

  file_paths.append(1llu, L'\0');
  RecycleFileOnWindows(file_paths);


  std::list<QStandardItem *>::iterator itl;
  for (itl = item_list.begin(); itl != item_list.end(); ++itl)
    if (_waccess_s((*itl)->text().toStdWString().c_str(), 06) != 0) { // 文件被删掉
      set_item_already_deleted(*itl, true);
    }

  std::map<std::wstring, unsigned long long int>::iterator itm;

  /* 上面是从treeView获取已标记删除的文件 */
  /* file_to_be_deleted_map_里也是已标记删除的文件，但是没有了对应的StandardItem */
  /* 将来可能会想出一个更好的方法统一上面两种操作 */
  itm = file_to_be_deleted_map_.begin();
  while (itm != file_to_be_deleted_map_.end()) {
    if (_waccess_s(itm->first.c_str(), 00) != 0) {
      file_to_be_deleted_size_ -= itm->second;
      itm = file_to_be_deleted_map_.erase(itm);
    }
    else
      ++itm;
  }

  show_file_to_be_deleted_size(file_to_be_deleted_map_.size(), file_to_be_deleted_size_);
}

void MainWindow::on_pushButtonAbout_clicked()
{
  XAboutDialog *about_dialog = new XAboutDialog(this);
  about_dialog->exec();
  delete about_dialog;
}

void MainWindow::on_pushButtonDonate_clicked()
{
  XPayDialog *pay_dialog = new XPayDialog(this);
  pay_dialog->exec();
  delete pay_dialog;
}

void MainWindow::on_checkBoxDocx_stateChanged(int state)
{
  on_ext_name_text_stateChanged(state);
}

void MainWindow::on_checkBoxTxt_stateChanged(int state)
{
  on_ext_name_text_stateChanged(state);
}

void MainWindow::on_checkBoxPng_stateChanged(int state)
{
  on_ext_name_image_stateChanged(state);
}

void MainWindow::on_checkBoxJpeg_stateChanged(int state)
{
  on_ext_name_image_stateChanged(state);
}

void MainWindow::on_checkBoxJpg_stateChanged(int state)
{
  on_ext_name_image_stateChanged(state);
}

void MainWindow::on_checkBoxBmp_stateChanged(int state)
{
  on_ext_name_image_stateChanged(state);
}


void MainWindow::on_checkBoxSys_stateChanged(int state)
{
  on_file_attribute_toggled(state);
}

void MainWindow::on_checkBoxHide_stateChanged(int state)
{
  on_file_attribute_toggled(state);
}

void MainWindow::on_checkBoxReadOnly_stateChanged(int state)
{
  on_file_attribute_toggled(state);
}
void MainWindow::on_checkBoxArchive_stateChanged(int state)
{
  on_file_attribute_toggled(state);
}

void MainWindow::on_groupBoxFileAttribute_toggled(bool on)
{
  on_file_attribute_toggled(on);
}

void MainWindow::on_groupBoxSize_toggled(bool on)
{
  on_file_attribute_toggled(on);
}

void MainWindow::on_groupBoxCreateTime_toggled(bool on)
{
  on_file_attribute_toggled(on);
}

void MainWindow::on_groupBoxModifyTime_toggled(bool on)
{
  on_file_attribute_toggled(on);
}

void MainWindow::on_groupBoxAccessTime_toggled(bool on)
{
  on_file_attribute_toggled(on);
}

void MainWindow::on_radioButtonLongestFilePath_toggled(bool checked)
{
  select_max_min_items(&items_sort_by_file_path_, true, checked);
}

void MainWindow::on_radioButtonShortestFilePath_toggled(bool checked)
{
  select_max_min_items(&items_sort_by_file_path_, false, checked);
}

void MainWindow::on_radioButtonLongestFileName_toggled(bool checked)
{
  select_max_min_items(&items_sort_by_file_name_, true, checked);
}

void MainWindow::on_radioButtonShortestFileName_toggled(bool checked)
{
  select_max_min_items(&items_sort_by_file_name_, false, checked);
}

void MainWindow::on_radioButtonLongestDirName_toggled(bool checked)
{
  select_max_min_items(&items_sort_by_dir_name_, true, checked);
}

void MainWindow::on_radioButtonShortestDirName_toggled(bool checked)
{
  select_max_min_items(&items_sort_by_dir_name_, false, checked);
}

void MainWindow::on_radioButtonNewestModifyTime_toggled(bool checked)
{
  select_max_min_items(&items_sort_by_modify_time_, true, checked);
}

void MainWindow::on_radioButtonOldestModifyTime_toggled(bool checked)
{
  select_max_min_items(&items_sort_by_modify_time_, false, checked);
}

void MainWindow::on_radioButtonNewestCreateTime_toggled(bool checked)
{
  select_max_min_items(&items_sort_by_create_time_, true, checked);
}

void MainWindow::on_radioButtonOldestCreateTime_toggled(bool checked)
{
  select_max_min_items(&items_sort_by_create_time_, false, checked);
}

void MainWindow::on_radioButtonLeftOnlyOne_toggled(bool checked)
{
  if (items_only_left_one_.empty())
    return;

  std::list<QStandardItem *>::iterator it;
  it = items_only_left_one_.begin();
#if 0
  if (checked) {
    QFont font_strike_out;
    QBrush brush_red;
    font_strike_out = (*it)->parent()->font();
    font_strike_out.setStrikeOut(true);
    brush_red = (*it)->parent()->foreground();
    brush_red.setColor(Qt::red);

    while (it != items_only_left_one_.end()) {
      QStandardItem *first_child = (*it)->parent()->child(0);
      if (first_child->checkState() == Qt::Checked) {
        first_child->setFont((*it)->parent()->font());
        first_child->setForeground((*it)->parent()->foreground());
        first_child->setCheckState(Qt::Unchecked);
      }
      (*it)->setFont(font_strike_out);
      (*it)->setForeground(brush_red);
      (*it)->setCheckState(Qt::Checked);
      ++it;
    }
  }
  else {
    QFont font_normal;
    QBrush brush_normal;
    font_normal = (*it)->parent()->font();
    brush_normal = (*it)->parent()->foreground();

    while (it != items_only_left_one_.end()) {
      (*it)->setFont(font_normal);
      (*it)->setForeground(brush_normal);
      (*it)->setCheckState(Qt::Unchecked);
      ++it;
    }
  }
#endif
  while (it != items_only_left_one_.end()) {
    QStandardItem *first_child = (*it)->parent()->child(0);
    if (is_item_will_be_deleted(first_child)) {
      set_item_will_be_deleted(first_child, false);
      count_file_to_be_deleted(first_child, false);
    }
    if (!is_item_already_deleted(*it)) {
      set_item_will_be_deleted(*it, checked);
      count_file_to_be_deleted(*it, checked);
    }
    ++it;
  }
}

void MainWindow::on_radioButtonCancelAll_toggled(bool checked)
{
  if (!checked || items_only_left_one_.empty())
    return;

  std::list<QStandardItem *>::iterator it;
  it = items_only_left_one_.begin();
  while (it != items_only_left_one_.end()) {
    QStandardItem *first_child = (*it)->parent()->child(0);
    if (is_item_will_be_deleted(first_child)) {
      set_item_will_be_deleted(first_child, false);
      count_file_to_be_deleted(first_child, false);
    }
    if (is_item_will_be_deleted(*it)) {
      set_item_will_be_deleted(*it, false);
      count_file_to_be_deleted(*it, false);
    }
    ++it;
  }
}

void MainWindow::on_treeViewResult_clicked(const QModelIndex &index)
{
  on_treeview_item_clicked_show_file_attribute(index);
  on_treeview_crypt_hash_item_clicked_check_or_uncheck(index);
  on_treeview_image_item_clicked_show_image(index);
  on_treeview_text_item_clicked_show_text(index);
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
  labelStep->setText(QString::fromWCharArray(step_tip_[index]));
}

void MainWindow::handle_crypt_hash_result(QMap<QByteArray, QString> *crypt_hash_result)
{
#if defined(__linux__)
  delete worker_thread_;
#elif defined(_MSC_VER)

  /*
   *
  delete worker_thread_; //QThread: Destroyed while thread is still running 2016-07-20 06:45 am on windows
  */
#endif

  worker_thread_ = nullptr;
  delete thread_parameter_.dir_list;
  thread_parameter_.dir_list = nullptr;
  dupclean_.delete_compare(compare_handle_);
  compare_handle_ = nullptr;

  if (crypt_hash_result != nullptr) {
    show_crypt_hash_result_to_tree(crypt_hash_result);
    delete crypt_hash_result;
  }

  process_zero_count_row_tree(thread_parameter_);

#if 0
  show_map_to_tree(crypt_hash_result);
  dump_map(crypt_hash_result, get_realpath(lineEditFilePath->text().trimmed()));
  write_script(crypt_hash_result, get_realpath(lineEditFilePath->text().trimmed()));
#endif
}

void MainWindow::on_treeViewResult_collapsed(const QModelIndex &index)
{
  Q_UNUSED(index);
  treeViewResult->header()->resizeSections(/*QHeaderView::Interactive*/QHeaderView::ResizeToContents);
}

void MainWindow::on_treeViewResult_expanded(const QModelIndex &index)
{
  Q_UNUSED(index);
  treeViewResult->header()->resizeSections(/*QHeaderView::Interactive*/QHeaderView::ResizeToContents);
}

void MainWindow::on_treeViewSelectFileAttribute_collapsed(const QModelIndex &index)
{
  Q_UNUSED(index);
  treeViewSelectFileAttribute->header()->resizeSections(/*QHeaderView::Interactive*/QHeaderView::ResizeToContents);
}

void MainWindow::on_treeViewSelectFileAttribute_expanded(const QModelIndex &index)
{
  Q_UNUSED(index);
  treeViewSelectFileAttribute->header()->resizeSections(/*QHeaderView::Interactive*/QHeaderView::ResizeToContents);
}

void MainWindow::showFilePath()
{
  QString rp = get_realpath(lineEditFilePath->text().trimmed());
  labelFilePathTip->setText(QStringLiteral(u"二进制绝对文件路径：") + rp + QStringLiteral(u".bin\n")
                            + QStringLiteral(u"重复文件列表绝对路径：") + rp + QStringLiteral(u".txt\n")
                            + QStringLiteral(u"zsh 脚本绝对路径：") + rp + QStringLiteral(u".sh\n")
                            + QStringLiteral(u"bat 脚本绝对路径：") + rp + QString::fromWCharArray(L".bat"));
}

void MainWindow::delete_item_dialog(void *v_item, void *v_dialog)
{
  std::map<QStandardItem *, XItemDialog *>::iterator it;

  QStandardItem *item = reinterpret_cast<QStandardItem *>(v_item);
  XItemDialog *dialog = reinterpret_cast<XItemDialog *>(v_dialog);

  if ((it = item_dialog_map_.find(item)) != item_dialog_map_.end()
      && it->second == dialog)
    item_dialog_map_.erase(it);

  delete dialog;
}

void MainWindow::count_file_to_be_deleted(void *v_item, bool to_be_deleted)
{
  QStandardItem *item = reinterpret_cast<QStandardItem *>(v_item);

  if (to_be_deleted) {
    QVariant variant{item->data()};

    if (!variant.isValid() || variant.isNull())
      return;

    XFileAttribute file_attribute{variant.value<XFileAttribute>()};

    std::pair<std::map<std::wstring, long long unsigned int>::iterator, bool> res;

    res = file_to_be_deleted_map_.insert(std::make_pair(item->text().toStdWString().c_str(), file_attribute.size));
    if (res.second)
      file_to_be_deleted_size_ += file_attribute.size;
  }
  else {
    std::map<std::wstring, unsigned long long int>::iterator it;
    if ((it = file_to_be_deleted_map_.find(item->text().toStdWString())) != file_to_be_deleted_map_.end()) {
      file_to_be_deleted_size_ -= it->second;
      file_to_be_deleted_map_.erase(it);
    }
  }
  show_file_to_be_deleted_size(file_to_be_deleted_map_.size(), file_to_be_deleted_size_);
}

void MainWindow::show_file_to_be_deleted_size(unsigned long long count,
                                              unsigned long long int size)
{
  QString size_text{QString::fromWCharArray(L"预计删除 %0 个文件，回收空间 %1 %2")};
  double gib {1024.0 * 1024.0 * 1024.0},
      mib {1024.0 * 1024.0}, kib {1024.0};
  double dsize {(double)size};

  size_text = size_text.arg(count);
  if (dsize / gib >= 1.0)
    labelSizeTip->setText(size_text.arg(dsize / gib).arg(QStringLiteral(u"GiB")));
  else if (dsize / mib >= 1.0)
    labelSizeTip->setText(size_text.arg(dsize / mib).arg(QStringLiteral(u"MiB")));
  else if (dsize / kib >= 1.0)
    labelSizeTip->setText(size_text.arg(dsize / kib).arg(QStringLiteral(u"KiB")));
  else
    labelSizeTip->setText(size_text.arg(dsize).arg(QStringLiteral(u"字节")));
}

void MainWindow::dump_map(QMap<QByteArray, QString> *result, const QString &file_path)
{
  FILE *fp, *fbin;
#if defined(linux)
  fp = fopen((file_path + QString::fromWCharArray(L".txt")).toStdString().c_str(), "w");
  fbin = fopen((file_path + QString::fromWCharArray(L".bin")).toStdString().c_str(), "w");
#else
  fp = _wfopen((file_path + QString::fromWCharArray(L".txt")).toStdWString().c_str(), L"wt");
  fbin = _wfopen((file_path + QString::fromWCharArray(L".bin")).toStdWString().c_str(), L"wb");
#endif

  QList<QByteArray> keys;
  QList<QString> values;
  keys = result->uniqueKeys();
  QList<QByteArray>::iterator key_iter;

  for(key_iter = keys.begin(); key_iter != keys.end(); key_iter++) {
    values = result->values(*key_iter);
    MapFile map_file;
    memset(&map_file, 0, sizeof(MapFile));
    memcpy(map_file.key_data, key_iter->data(), map_file.key_length = key_iter->size());
    map_file.file_number = values.size();
    fwrite(&map_file, sizeof(MapFile), 1U, fbin);

    if(values.size() > 1)
      fwprintf(fp, L"%ls\nfile numbers = %d\n", QString((*key_iter).toHex()).toStdWString().c_str(), values.size());
    QList<QString>::iterator string_iter;
    for(string_iter = values.begin(); string_iter != values.end(); ++string_iter) {
      if(values.size() > 1)
        fwprintf(fp, L"\t%ls\n", string_iter->toStdWString().c_str());

      long long unsigned int length = string_iter->toStdString().length();
      fwrite(&length, sizeof(length), 1U, fbin);
      fwrite(string_iter->toStdString().c_str(), 1U, length, fbin);
    }
    if(values.size() > 1)
      fputwc(L'\n', fp);
  }
  fclose(fp);
  fclose(fbin);
}

void MainWindow::load_map(QMap<QByteArray, QString> *map, const QString &file_path)
{
  if(map == NULL)
    return;
  MapFile map_file;
  FILE *fbin;
#if defined(linux)
  fbin = fopen((file_path + QString::fromWCharArray(L".bin")).toStdString().c_str(), "r");
#else
  _wfopen_s(&fbin, (file_path + QString::fromWCharArray(L".bin")).toStdWString().c_str(), L"rb");
#endif

  if (fbin == nullptr)
    return;

  QByteArray key;

  while(!feof(fbin)
        && !ferror(fbin)
        && fread(&map_file, sizeof(MapFile), 1U, fbin) == 1) {
    key.clear();
    key.append(map_file.key_data, map_file.key_length);
    for(unsigned int i = 0U; i < map_file.file_number; ++i) {
      long long unsigned int length;
      char *buf;
      fread(&length, sizeof(length), 1U, fbin);
      if((buf = (char *)malloc(length + 1)) == NULL)
        return;
      fread(buf, 1U, length, fbin);
      buf[length] = '\0';
      map->insertMulti(key, QString(buf));
      free(buf);
    }
  }
  fclose(fbin);
}

void MainWindow::show_map_to_tree(QMap<QByteArray, QString> *crypt_hash_result)
{
  QList<QByteArray> keys;
  QList<QString> values;
  keys = crypt_hash_result->uniqueKeys();

  QList<QByteArray>::iterator key_iter;

  tree_model_result_->clear();
  tree_model_result_->setHorizontalHeaderLabels(QStringList() << QString::fromStdWString(L"Hash value/File Name")
                                         << QString::fromStdWString(L"File Count/File Size(bytes)")
                                         << QString::fromStdWString(L"File Size(bytes)"));

  QStandardItem *parentItem = tree_model_result_->invisibleRootItem();

  unsigned long long int total_bytes{0llu}, total_bytes_exclude{0llu};
  for(key_iter = keys.begin(); key_iter != keys.end(); key_iter++) {
    unsigned long long int bytes;

    values = crypt_hash_result->values(*key_iter);

    if(values.size() == 1)
      continue;

    QStandardItem *key_item = new QStandardItem(QString((*key_iter).toHex()));

    parentItem->appendRow(key_item);

    key_item->setEditable(false);

    //key_item->appendColumn(QList<QStandardItem *>() << new QStandardItem(QString("%0").arg(values.size())));
    tree_model_result_->setItem(tree_model_result_->indexFromItem(key_item).row(), 1, new QStandardItem(QString("%0").arg(values.size())));

    QList<QString>::iterator string_iter = values.begin();
    tree_model_result_->setItem(key_item->index().row(), 2, new QStandardItem(QString("%0").arg(QFile(*string_iter).size())));

    total_bytes_exclude -= QFile(*string_iter).size();

    for(string_iter = values.begin(); string_iter != values.end(); ++string_iter) {
      QStandardItem *value_item;
      total_bytes += bytes = QFile(*string_iter).size();
      total_bytes_exclude += bytes;

      value_item = new QStandardItem(*string_iter);

      key_item->appendRow(value_item);

      //value_item->appendColumn(QList<QStandardItem *>() << new QStandardItem(QString("%0").arg(values.size())));
      //tree_model_result_->setItem(value_item->index().row(), 1, new QStandardItem(QString("%0").arg(values.size())));
      //key_item->appendColumn(QList<QStandardItem *>() << new QStandardItem(QString("%0").arg(values.size())));

      key_item->setChild(value_item->index().row(), 1, new QStandardItem(QString("%0").arg(bytes)));
      key_item->setChild(value_item->index().row(), 2, new QStandardItem(QString("%0").arg(bytes)));
      value_item->setEditable(false);
    }
  }
  QStandardItem *total_item = new QStandardItem(QStringLiteral(u"总计"));
  total_item->setEditable(false);
  parentItem->appendRow(total_item);

  QStandardItem *item = new QStandardItem(QString::fromWCharArray(L"%0（所有）").arg(total_bytes));
  item->setEditable(false);
  tree_model_result_->setItem(total_item->index().row(), 1, item);

  item = new QStandardItem(QString::fromWCharArray(L"%0（去一个重复）").arg(total_bytes_exclude));
  item->setEditable(false);
  tree_model_result_->setItem(total_item->index().row(), 2, item);

  QStandardItem *size_item = new QStandardItem(QStringLiteral(u"总大小 KiB"));
  total_item->appendRow(size_item);
  item = new QStandardItem(QString::fromWCharArray(L"%0（所有）").arg((double)total_bytes / 1024.0));
  total_item->setChild(size_item->index().row(), 1, item);
  item = new QStandardItem(QString::fromWCharArray(L"%0（去一个重复）").arg((double)total_bytes_exclude / 1024.0));
  total_item->setChild(size_item->index().row(), 2, item);

  size_item = new QStandardItem(QStringLiteral(u"总大小 MiB"));
  total_item->appendRow(size_item);
  item = new QStandardItem(QString::fromWCharArray(L"%0（所有）").arg((double)total_bytes / (1024.0 * 1024.0)));
  total_item->setChild(size_item->index().row(), 1, item);
  item = new QStandardItem(QString::fromWCharArray(L"%0（去一个重复）").arg((double)total_bytes_exclude / (1024.0 * 1024.0)));
  total_item->setChild(size_item->index().row(), 2, item);

  size_item = new QStandardItem(QStringLiteral(u"总大小 GiB"));
  total_item->appendRow(size_item);
  item = new QStandardItem(QString::fromWCharArray(L"%0（所有）").arg((double)total_bytes / (1024.0 * 1024.0 * 1024.0)));
  total_item->setChild(size_item->index().row(), 1, item);
  item = new QStandardItem(QString::fromWCharArray(L"%0（去一个重复）").arg((double)total_bytes_exclude / (1024.0 * 1024.0 * 1024.0)));
  total_item->setChild(size_item->index().row(), 2, item);

  treeViewResult->header()->resizeSections(/*QHeaderView::Interactive*/QHeaderView::ResizeToContents);
  treeViewResult->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::show_crypt_hash_result_to_tree(QMap<QByteArray, QString> *crypt_hash_result)
{
  QList<QByteArray> keys;
  QList<QString> values;

  keys = crypt_hash_result->uniqueKeys();
  QList<QByteArray>::iterator key_iter;

  for(key_iter = keys.begin(); key_iter != keys.end(); key_iter++) {

    values = crypt_hash_result->values(*key_iter);
    if(values.size() == 1)
      continue;

    QStandardItem *key_item = new QStandardItem(QString((*key_iter).toHex()));
    tree_crypt_hash_item_->appendRow(key_item);
    tree_crypt_hash_item_->setChild(key_item->row(), 1, new QStandardItem(QString("%0").arg(values.size())));
    key_item->setEditable(false);

    XMaxMinItem file_path_item, file_name_item, dir_name_item, modify_time_item, create_time_item;
    file_path_item.parent = key_item;
    file_name_item.parent = key_item;
    dir_name_item.parent = key_item;
    modify_time_item.parent = key_item;
    create_time_item.parent = key_item;

    int max_file_path_len{0}, min_file_path_len{INT_MAX},
    max_file_name_len{0}, min_file_name_len{INT_MAX},
    max_dir_name_len{0}, min_dir_name_len{INT_MAX};
    long long int newest_modify_time{0ll}, oldest_modify_time{LLONG_MAX},
    newest_create_time{0ll}, oldest_create_time{LLONG_MAX};

    QList<QString>::iterator string_iter;

    for(string_iter = values.begin(); string_iter != values.end(); ++string_iter) {

      QStandardItem *value_item;
      value_item = new QStandardItem(*string_iter);
      if (value_item == nullptr)
        continue;
      key_item->appendRow(value_item);
      value_item->setEditable(false);
      value_item->setCheckable(true);
      value_item->setCheckState(Qt::Unchecked);

      QVariant file_variant(QVariant::UserType);
      XFileAttribute file_attribute;

      get_file_attribute(*string_iter, &file_attribute);
      file_variant.setValue<XFileAttribute>(file_attribute);
      value_item->setData(file_variant);

      /* 最大路径长度 */
      if (max_file_path_len < file_attribute.path_length) {
        max_file_path_len = file_attribute.path_length;
        file_path_item.max = value_item;
      }

      /* 最小路径长度 */
      if (min_file_path_len > file_attribute.path_length) {
        min_file_path_len = file_attribute.path_length;
        file_path_item.min = value_item;
      }

      /* 最大文件名长度 */
      if (max_file_name_len < file_attribute.name_length) {
        max_file_name_len = file_attribute.name_length;
        file_name_item.max = value_item;
      }

      /* 最小文件名长度 */
      if (min_file_name_len > file_attribute.name_length) {
        min_file_name_len = file_attribute.name_length;
        file_name_item.min = value_item;
      }

      /* 最大目录长度 */
      if (max_dir_name_len < file_attribute.dir_length) {
        max_dir_name_len = file_attribute.dir_length;
        dir_name_item.max = value_item;
      }

      /* 最小目录长度 */
      if (min_dir_name_len > file_attribute.dir_length) {
        min_dir_name_len = file_attribute.dir_length;
        dir_name_item.min = value_item;
      }

      /* 最新创建 */
      if (newest_create_time < file_attribute.create_time) {
        newest_create_time = file_attribute.create_time;
        create_time_item.max = value_item;
      }

      /* 最旧创建 */
      if (oldest_create_time > file_attribute.create_time) {
        oldest_create_time = file_attribute.create_time;
        create_time_item.min = value_item;
      }

      /* 最新修改 */
      if (newest_modify_time < file_attribute.modify_time) {
        newest_modify_time = file_attribute.modify_time;
        modify_time_item.max = value_item;
      }

      /* 最旧修改 */
      if (oldest_modify_time > file_attribute.modify_time) {
        oldest_modify_time = file_attribute.modify_time;
        modify_time_item.min = value_item;
      }

      if (string_iter != values.begin())
        items_only_left_one_.push_back(value_item);
    }
    treeViewResult->expand(key_item->index());
    items_sort_by_file_path_.push_back(file_path_item);
    items_sort_by_file_name_.push_back(file_name_item);
    items_sort_by_dir_name_.push_back(dir_name_item);
    items_sort_by_create_time_.push_back(create_time_item);
    items_sort_by_modify_time_.push_back(modify_time_item);
  }

  treeViewResult->expand(tree_crypt_hash_item_->index());

  if (radioButtonLongestFilePath->isChecked())
    on_radioButtonLongestFilePath_toggled(true);

  if (radioButtonShortestFilePath->isChecked())
    on_radioButtonShortestFilePath_toggled(true);

  if (radioButtonLongestFileName->isChecked())
    on_radioButtonLongestFileName_toggled(true);

  if (radioButtonShortestFileName->isChecked())
    on_radioButtonShortestFileName_toggled(true);

  if (radioButtonLongestDirName->isChecked())
    on_radioButtonLongestDirName_toggled(true);

  if (radioButtonShortestDirName->isChecked())
    on_radioButtonShortestDirName_toggled(true);

  if (radioButtonNewestCreateTime->isChecked())
    on_radioButtonNewestCreateTime_toggled(true);

  if (radioButtonOldestCreateTime->isChecked())
    on_radioButtonOldestCreateTime_toggled(true);

  if (radioButtonNewestModifyTime->isChecked())
    on_radioButtonNewestModifyTime_toggled(true);

  if (radioButtonOldestModifyTime->isChecked())
    on_radioButtonOldestModifyTime_toggled(true);

  if (radioButtonLeftOnlyOne->isChecked())
    on_radioButtonLeftOnlyOne_toggled(true);
}

void MainWindow::show_compare_result_to_tree(const XCompareResult *compare_result)
{
  QLocale zh_locale;

  QStandardItem *file_path_item1, *file_path_item2, *similarity_item;
  QString file_path1(QString::fromWCharArray(compare_result->file_path1));
  QString file_path2(QString::fromWCharArray(compare_result->file_path2));

  file_path_item1 = new QStandardItem(file_path1);

  if (file_path_item1 == nullptr) {
    delete const_cast<XCompareResult *>(compare_result);
    return;
  }

  file_path_item2 = new QStandardItem(file_path2);

  if (file_path_item2 == nullptr) {
    delete file_path_item1;
    delete const_cast<XCompareResult *>(compare_result);
    return;
  }

  QVariant file_variant(QVariant::UserType);
  XFileAttribute file_attribute;

  get_file_attribute(file_path1, &file_attribute);
  file_variant.setValue<XFileAttribute>(file_attribute);
  file_path_item1->setData(file_variant);
  file_path_item1->setEditable(false);

  get_file_attribute(file_path2, &file_attribute);
  file_variant.setValue<XFileAttribute>(file_attribute);
  file_path_item2->setData(file_variant);
  file_path_item2->setEditable(false);

  switch (compare_result->flag) {
    case FIMAGE: {
        double max_similarity = __max(compare_result->image.dct_similarity, compare_result->image.mh_similarity);
        max_similarity = __max(max_similarity, compare_result->image.radial_similarity);
        similarity_item = new QStandardItem(zh_locale.toString(max_similarity));
        tree_image_item_->appendRow(file_path_item1);
        tree_image_item_->setChild(file_path_item1->index().row(), 1, file_path_item2);
        tree_image_item_->setChild(file_path_item1->index().row(), 2, similarity_item);
        similarity_item->setEditable(false);
        treeViewResult->expand(tree_image_item_->index());
        break;
      }
    case FTEXT:
      similarity_item = new QStandardItem(zh_locale.toString(compare_result->text.similarity));
      tree_text_item_->appendRow(file_path_item1);
      tree_text_item_->setChild(file_path_item1->index().row(), 1, file_path_item2);
      tree_text_item_->setChild(file_path_item1->index().row(), 2, similarity_item);
      similarity_item->setEditable(false);
      treeViewResult->expand(tree_text_item_->index());
      break;
    case FFILE_NAME:
      similarity_item = new QStandardItem(zh_locale.toString(compare_result->file_name.similarity));
      tree_file_path_item_->appendRow(file_path_item1);
      tree_file_path_item_->setChild(file_path_item1->index().row(), 1, file_path_item2);
      tree_file_path_item_->setChild(file_path_item1->index().row(), 2, similarity_item);
      similarity_item->setEditable(false);
      {
        QString tip_string(QStringLiteral(u"不建议使用文件名是否相似决定文件是否删除。"));
        file_path_item1->setToolTip(tip_string);
        file_path_item2->setToolTip(tip_string);
      }
      treeViewResult->expand(tree_file_path_item_->index());
      break;
    default:
      delete file_path_item1;
      delete file_path_item2;
      file_path_item1 = nullptr;
      file_path_item2 = nullptr;
      break;
  }
  delete const_cast<XCompareResult *>(compare_result);
}

void MainWindow::process_zero_count_row_tree(XThreadParameter &thread_parameter)
{
  QString common_msg(QString::fromWCharArray(L"没有启用此比较方法"));
  QStandardItem *zero_item{nullptr};

  if (tree_text_item_ != nullptr && tree_text_item_->rowCount() == 0) {
    if ((thread_parameter.valid_flag & VTEXT) == VTEXT)
      zero_item = new QStandardItem(QStringLiteral(u"无相似文档"));
    else
      zero_item = new QStandardItem(common_msg);
    zero_item->setEditable(false);
    tree_text_item_->appendRow(zero_item);
    treeViewResult->expand(tree_text_item_->index());
  }

  if (tree_image_item_ != nullptr && tree_image_item_->rowCount() == 0) {
    if ((thread_parameter.valid_flag & VIMAGE) == VIMAGE)
      zero_item = new QStandardItem(QStringLiteral(u"无相似图片"));
    else
      zero_item = new QStandardItem(common_msg);
    zero_item->setEditable(false);
    tree_image_item_->appendRow(zero_item);
    treeViewResult->expand(tree_image_item_->index());
  }

  if (tree_crypt_hash_item_ != nullptr && tree_crypt_hash_item_->rowCount() == 0) {
    if ((thread_parameter.valid_flag & VCRYPT_HASH) == VCRYPT_HASH)
      zero_item = new QStandardItem(QStringLiteral(u"无精确相似文件"));
    else
      zero_item = new QStandardItem(common_msg);
    zero_item->setEditable(false);
    tree_crypt_hash_item_->appendRow(zero_item);
    treeViewResult->expand(tree_crypt_hash_item_->index());
  }

  if (tree_file_path_item_ != nullptr && tree_file_path_item_->rowCount() == 0) {
    if ((thread_parameter.valid_flag & VFILE_NAME) == VFILE_NAME)
      zero_item = new QStandardItem(QStringLiteral(u"无相似文件名"));
    else
      zero_item = new QStandardItem(common_msg);
    zero_item->setEditable(false);
    tree_file_path_item_->appendRow(zero_item);
    treeViewResult->expand(tree_file_path_item_->index());
  }
}

void MainWindow::write_script(QMap<QByteArray, QString> *result, const QString &file_path)
{
  FILE *script;
#if defined(linux)
  if((script = fopen((file_path + QString::fromWCharArray(L".sh")).toStdString().c_str(), "w")) == NULL)
    return;
  fputws(L"#!/bin/zsh\n", script);

  const wchar_t *comment = L"# ";
  const wchar_t *command = L"rm -f ";
#elif defined(_MSC_VER)
  if((script = _wfopen((file_path + QString::fromWCharArray(L".bat")).toStdWString().c_str(), L"wt")) == NULL)
    return;
  fputws(L"@echo off\n", script);
  fputws(L"setlocal enabledelayedexpansion ENABLEEXTENSIONS\n", script);

  const wchar_t *comment = L"rem ";
  const wchar_t *command = L"del ";
#endif

  QList<QByteArray> keys;
  QList<QString> values;
  keys = result->uniqueKeys();

  QList<QByteArray>::iterator key_iter;

  for(key_iter = keys.begin(); key_iter != keys.end(); key_iter++) {
    values = result->values(*key_iter);

    if(values.size() == 1)
      continue;

    QList<QString>::iterator string_iter = values.begin();

    fwprintf(script, L"%ls%ls\n%lsfile numbers = %d\n",
             comment,
             QString((*key_iter).toHex()).toStdWString().c_str(),
             comment,
             values.size());

    fwprintf(script, L"%ls\"%ls\"\n",
             comment,
             (*string_iter).toStdWString().c_str());

    while(++string_iter != values.end())
      fwprintf(script, L"%ls\"%ls\"\n", command, QDir::toNativeSeparators(*string_iter).toStdWString().c_str());
  }
#if defined(linux)
  fputws(L"exit 0\n", script);
#elif defined(_MSC_VER)
  fputws(L"@echo on\n", script);
#endif
  fclose(script);
}

QString MainWindow::get_realpath(const QString &relative_path)
{
  QString real_path;
#if defined(__linux__)
  char resolved[PATH_MAX]{0};
  realpath(relative_path.toStdString().c_str(), resolved);
  real_path = resolved;
#elif defined(_MSC_VER)
  wchar_t resolved[_MAX_PATH]{0};
  _wfullpath(resolved, relative_path.toStdWString().c_str(), sizeof(resolved) / sizeof(resolved[0]));
  real_path = QString::fromWCharArray(resolved);
#endif
  return real_path;
}

int MainWindow::get_file_attribute(const QString &file_path,
                                   XFileAttribute *file_attribute)
{
  if (file_attribute == nullptr)
    return -1;

  file_attribute->path_length = file_path.size();
  QDir dir = QDir(file_path);
  /* 当用一个文件路径构造一个 QDir 实例时，dirName 就是文件名 */
  file_attribute->name_length = dir.dirName().size();
  file_attribute->dir_length = file_attribute->path_length - file_attribute->name_length;

  struct __stat64 st;
  if (_wstat64(file_path.toStdWString().c_str(), &st) != 0)
    return -1;

  file_attribute->create_time = st.st_ctime;
  file_attribute->modify_time = st.st_mtime;
  file_attribute->size = st.st_size;

  return 0;
}

void MainWindow::setup_ui_for_boot()
{
  // 文档
  groupBoxText->setCheckable(true);
  groupBoxText->setChecked(true);
  checkBoxDocx->setChecked(true);
  checkBoxTxt->setChecked(true);
  horizontalSliderTextThreshold->setValue(70);
  spinBoxTextThreshold->setValue(70);
  connect(horizontalSliderTextThreshold, &QSlider::valueChanged,
          spinBoxTextThreshold, &QSpinBox::setValue);
  connect(spinBoxTextThreshold, SIGNAL(valueChanged(int)),
          horizontalSliderTextThreshold, SLOT(setValue(int)));

  // 图片
  groupBoxImage->setCheckable(true);
  groupBoxImage->setChecked(true);
  checkBoxPng->setChecked(true);
  checkBoxJpeg->setChecked(true);
  checkBoxJpg->setChecked(true);
  checkBoxBmp->setChecked(true);
  horizontalSliderImageThreshold->setValue(65);
  spinBoxImageThreshold->setValue(65);
  connect(horizontalSliderImageThreshold, &QSlider::valueChanged,
          spinBoxImageThreshold, &QSpinBox::setValue);
  connect(spinBoxImageThreshold, SIGNAL(valueChanged(int)),
          horizontalSliderImageThreshold, SLOT(setValue(int)));

  // 文件名
  groupBoxFileName->setCheckable(false);
  groupBoxFileName->setChecked(true);
  groupBoxFileName->setEnabled(false);
  groupBoxFileName->setVisible(false);
  checkBoxOnlyForSimilarTextOrImage->setChecked(true);
  horizontalSliderFileNameThreshold->setValue(70);
  spinBoxFileNameThreshold->setValue(70);
  connect(horizontalSliderFileNameThreshold, &QSlider::valueChanged,
          spinBoxFileNameThreshold, &QSpinBox::setValue);
  connect(spinBoxFileNameThreshold, SIGNAL(valueChanged(int)),
          horizontalSliderImageThreshold, SLOT(setValue(int)));

  // 文件属性
#if 0
  groupBoxFileAttribute->setCheckable(true);
  groupBoxFileAttribute->setChecked(true);
#endif

  groupBoxSize->setChecked(false);
  lineEditSize1->setInputMask("9999999999");
  lineEditSize1->setText("512");
  radioButtonSize1Byte->setChecked(true);

  lineEditSize2->setInputMask(lineEditSize1->inputMask());
  //lineEditGreateEqualSize->setText("1073741824");
  lineEditSize2->setText("1");
  radioButtonSize2GiB->setChecked(true);
  radioButtonSizeInclude->setChecked(true);

  groupBoxCreateTime->setChecked(false);
  radioButtonCreateTimeInclude->setChecked(true);
  dateTimeEditCreateTime1->setDateTime(QDateTime::currentDateTime());
  dateTimeEditCreateTime2->setDateTime(QDateTime::currentDateTime());

  groupBoxModifyTime->setChecked(false);
  radioButtonModifyTimeInclude->setChecked(true);
  dateTimeEditModifyTime1->setDateTime(QDateTime::currentDateTime());
  dateTimeEditModifyTime2->setDateTime(QDateTime::currentDateTime());

  groupBoxAccessTime->setChecked(false);
  groupBoxAccessTime->setVisible(false);
  radioButtonAccessTimeInclude->setChecked(true);
  dateTimeEditAccessTime1->setDateTime(QDateTime::currentDateTime());
  dateTimeEditAccessTime2->setDateTime(QDateTime::currentDateTime());

  checkBoxSys->setChecked(false);
  checkBoxHide->setChecked(true);
  checkBoxReadOnly->setChecked(true);
  checkBoxArchive->setChecked(true);
  checkBoxArchive->setToolTip(QString::fromWCharArray(L"必须要选中归档属性。"));
  checkBoxArchive->setEnabled(false);

  // 哈希算法
  hash_select->setChecked(true);
  checkBoxCryptHashOnlyAsFallback->setChecked(false);
  Md5->setChecked(true);

  // 选择目录
  checkBoxRecursion->setChecked(true);
  checkBoxFollowSymlinks->setChecked(true);
  checkBoxIgnoreHardlinks->setChecked(true);
  checkBoxIgnoreJunction->setChecked(true);

  checkBoxIgnoreHardlinks->setVisible(false);
  checkBoxIgnoreJunction->setVisible(false);

  // 比较结果
  tree_model_result_ = new QStandardItemModel(this);
  treeViewResult->setModel(tree_model_result_);
  treeViewResult->setEditTriggers(QAbstractItemView::NoEditTriggers);
  lineEditFilePath->setText(QStringLiteral(u"abc"));
  labelFilePathTip->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  connect(lineEditFilePath, &QLineEdit::editingFinished, this, &MainWindow::showFilePath);
  showFilePath();
  labelFilePath->setVisible(false);
  lineEditFilePath->setVisible(false);
  labelFilePathTip->setVisible(false);
  tree_model_file_attribute_ = new QStandardItemModel(this);
  treeViewSelectFileAttribute->setModel(tree_model_file_attribute_);
  treeViewSelectFileAttribute->setEditTriggers(QAbstractItemView::NoEditTriggers);
  connect(this, &MainWindow::treeViewSelectFileAttribute_expand_all,
         treeViewSelectFileAttribute, &QTreeView::expandAll, Qt::QueuedConnection);

  init_treeViewResult();

  //Ui_MainWindow::statusBar->setVisible(false);
  loadButton->setVisible(false);
  tabWidget->setCurrentIndex(0);
  labelStep->setText(QString::fromStdWString(step_tip_[tabWidget->currentIndex()]));

#if !defined(__FREE_VERSION__)
  pushButtonDonate->setVisible(false);
#endif
}

void MainWindow::enable_something_disabled_when_boot()
{
  delButton->setEnabled(true);
  startButton->setEnabled(true);
}

void MainWindow::on_ext_name_text_stateChanged(int state)
{
  Q_UNUSED(state);

  std::vector<QCheckBox *> check_box;

  check_box.push_back(checkBoxDocx);
  check_box.push_back(checkBoxTxt);

  check_if_all_checkbox_are_selected(check_box, groupBoxText);
}

void MainWindow::on_ext_name_image_stateChanged(int state)
{
  Q_UNUSED(state);

  std::vector<QCheckBox *> check_box;

  check_box.push_back(checkBoxPng);
  check_box.push_back(checkBoxJpeg);
  check_box.push_back(checkBoxJpg);
  check_box.push_back(checkBoxBmp);

  check_if_all_checkbox_are_selected(check_box, groupBoxImage);
}

void MainWindow::check_if_all_checkbox_are_selected(
    const std::vector<QCheckBox *> &checkBoxExtName, QGroupBox *parent)
{
  bool checked{false};
  for (const auto &it : checkBoxExtName)
    checked = (*it).isChecked() || checked;
  parent->setChecked(checked);
}

void MainWindow::on_treeview_crypt_hash_item_clicked_check_or_uncheck(const QModelIndex &index)
{
  QStandardItem *current_item = (qobject_cast<const QStandardItemModel *>(index.model()))->itemFromIndex(index);

  if (!current_item->isCheckable() || is_item_already_deleted(current_item))
    return;

  set_item_will_be_deleted(current_item, current_item->checkState() == Qt::Checked);
  count_file_to_be_deleted(current_item, current_item->checkState() == Qt::Checked);

  QStandardItem *parent = (qobject_cast<const QStandardItemModel *>(index.model()))->itemFromIndex(index.parent());

  if (parent == nullptr)
    return;

  int i = 0;
  bool all_checked{true};
  QStandardItem *child_item;

  while ((child_item = parent->child(i++)) != nullptr)
    if (child_item->isCheckable() && child_item->checkState() != Qt::Checked) {
      all_checked = false;
      break;
    }

  if (all_checked) {
    QFont mono_font(QString::fromWCharArray(L"Consolas"));
    QMessageBox message_box(this);
    message_box.setFont(mono_font);
    message_box.setIcon(QMessageBox::Question);
    message_box.setWindowTitle(QString::fromWCharArray(L"询问"));
    wchar_t wbuf[128];
    int nwchars;
    memset(wbuf, 0, sizeof(wbuf));
#if defined(_MSC_VER)
      nwchars = _snwprintf_s(wbuf, sizeof(wbuf) / sizeof(wbuf[0]),
                    sizeof(wbuf) / sizeof(wbuf[0]) - 1, L"全部选中会<b><FONT COLOR='#ff0000'>删除所有的</FONT></b>哈希值是"
                                                        L"<p>%ls</p>"
                                                        L"的文件。确定要全部删除吗？",
          parent->text().toStdWString().c_str());
#else
      nwchars = _snwprintf_s(wbuf, sizeof(wbuf) / sizeof(wbuf[0]),
                  sizeof(wbuf) / sizeof(wbuf[0]) - 1, L"全部选中会删除哈希值是\n"
                                                      L"%ls\n"
                                                      L"的<b><FONT COLOR='#ff0000'>所有的</FONT></b>文件。确定要全部选中吗？",
        parent->text().toStdWString().c_str());
#endif
    //message_box.setText(QString::fromWCharArray(L"全部选中会删除哈希值是") + parent->text() + QString::fromWCharArray(L"的<b><FONT COLOR='#ff0000'>所有的</FONT></b>文件。确定要全部选中吗？"));
    message_box.setText(QString::fromWCharArray(wbuf, nwchars));
    message_box.addButton(QString::fromWCharArray(L"是"), QMessageBox::YesRole);
    QPushButton *no_button = message_box.addButton(QString::fromWCharArray(L"否"), QMessageBox::NoRole);
    message_box.setDefaultButton(no_button);
    message_box.exec();
    if (message_box.clickedButton() == no_button) {
      set_item_will_be_deleted(current_item, false);
      count_file_to_be_deleted(current_item, false);
    }
  }
}

void MainWindow::on_treeview_item_clicked_show_file_attribute(const QModelIndex &index)
{
  QStandardItem *current_item = (qobject_cast<const QStandardItemModel *>(index.model()))->itemFromIndex(index);

  QStandardItem *parent_item = current_item->parent();

  if (parent_item == nullptr)
    return;

  QStandardItem *child = parent_item->child(current_item->index().row(), 0);
  QVariant variant{child->data()};
  if (!variant.isValid() || variant.isNull())
    return;

  XFileAttribute file_attribute{variant.value<XFileAttribute>()};

  tree_model_file_attribute_->clear();
  tree_model_file_attribute_->setHorizontalHeaderLabels(QStringList()
                                                       << QStringLiteral(u"文件路径/属性名称")
                                         << QStringLiteral(u"属性值"));

  do_show_file_attribute(file_attribute, child->text());

  if ((child = parent_item->child(current_item->index().row(), 1)) != nullptr) {
    variant = child->data();
    if (variant.isValid() && !variant.isNull())
      file_attribute = variant.value<XFileAttribute>();
    do_show_file_attribute(file_attribute, child->text());
  }
  /* 不管用什么方法，都会出现下面的这个错误。
  //HEAP[dcgui.exe]: HEAP: Free Heap block 000000CD939C51D0 modified at 000000CD939C5244 after it was freed
  //QMetaObject::invokeMethod(treeViewSelectFileAttribute, "expandAll");
  //treeViewSelectFileAttribute->expandAll();
  //emit treeViewSelectFileAttribute_expand_all();
  */
}

void MainWindow::do_show_file_attribute(const XFileAttribute &file_attribute,
                                        const QString &file_path)
{
  QStandardItem *parent_item = tree_model_file_attribute_->invisibleRootItem();

  QStandardItem *file_path_item = new QStandardItem(file_path);
  parent_item->appendRow(file_path_item);
  file_path_item->setEditable(false);

  QStandardItem *key_item = new QStandardItem(QStringLiteral(u"文件路径长度"));
  QStandardItem *value_item
      = new QStandardItem(QString::fromWCharArray(L"%0 字符")
                          .arg(file_attribute.path_length));
  key_item->setEditable(false);
  value_item->setEditable(false);
  file_path_item->appendRow(key_item);
  file_path_item->setChild(key_item->index().row(), 1, value_item);

  key_item = new QStandardItem(QStringLiteral(u"文件名长度"));
  value_item
      = new QStandardItem(QString::fromWCharArray(L"%0 字符")
                          .arg(file_attribute.name_length));
  key_item->setEditable(false);
  value_item->setEditable(false);
  file_path_item->appendRow(key_item);
  file_path_item->setChild(key_item->index().row(), 1, value_item);

  key_item = new QStandardItem(QStringLiteral(u"目录长度"));
  value_item
      = new QStandardItem(QString::fromWCharArray(L"%0 字符")
                          .arg(file_attribute.name_length));
  key_item->setEditable(false);
  value_item->setEditable(false);
  file_path_item->appendRow(key_item);
  file_path_item->setChild(key_item->index().row(), 1, value_item);

  key_item = new QStandardItem(QStringLiteral(u"文件大小"));
  value_item
      = new QStandardItem(QString::fromWCharArray(L"%0 字节")
                          .arg(file_attribute.size));
  key_item->setEditable(false);
  value_item->setEditable(false);
  file_path_item->appendRow(key_item);
  file_path_item->setChild(key_item->index().row(), 1, value_item);

  struct tm local_tm;
  wchar_t wbuf[64];

  _localtime64_s(&local_tm, &file_attribute.create_time);
  memset(wbuf, 0, sizeof(wbuf));
  wcsftime(wbuf, sizeof(wbuf) / sizeof(wbuf[0]), L"%c %A", &local_tm);
  key_item = new QStandardItem(QStringLiteral(u"创建时间"));
  value_item = new QStandardItem(QString::fromWCharArray(wbuf));
  key_item->setEditable(false);
  value_item->setEditable(false);
  file_path_item->appendRow(key_item);
  file_path_item->setChild(key_item->index().row(), 1, value_item);

  _localtime64_s(&local_tm, &file_attribute.modify_time);
  memset(wbuf, 0, sizeof(wbuf));
  wcsftime(wbuf, sizeof(wbuf) / sizeof(wbuf[0]), L"%c %A", &local_tm);
  key_item = new QStandardItem(QStringLiteral(u"修改时间"));
  value_item = new QStandardItem(QString::fromWCharArray(wbuf));
  key_item->setEditable(false);
  value_item->setEditable(false);
  file_path_item->appendRow(key_item);
  file_path_item->setChild(key_item->index().row(), 1, value_item);

  treeViewSelectFileAttribute->expand(file_path_item->index());
}

void MainWindow::on_treeview_image_item_clicked_show_image(const QModelIndex &index)
{
  QStandardItem *current_item = (qobject_cast<const QStandardItemModel *>(index.model()))->itemFromIndex(index);

  if (current_item->parent() != tree_image_item_)
    return;

  QStandardItem *image_item2{tree_image_item_->child(current_item->index().row(), 1)};

  if (image_item2 == nullptr)
    return;

  QStandardItem *image_item1{tree_image_item_->child(current_item->index().row(), 0)};

  XItemDialog *image_dialog;
  std::map<QStandardItem *, XItemDialog *>::iterator it;

  if ((it = item_dialog_map_.find(image_item1)) != item_dialog_map_.end())
    image_dialog = it->second;
  else
  {
    QStandardItem *similarity_item{tree_image_item_->child(current_item->index().row(), 2)};

    if (similarity_item == nullptr)
      return;

    image_dialog
        = new XItemDialog(treeViewResult, image_item1, image_item2, similarity_item, true, this);

    if (image_dialog == nullptr)
      return;

    item_dialog_map_.insert(std::make_pair(image_item1, image_dialog));
  }
  image_dialog->show();
  image_dialog->raise();
  image_dialog->activateWindow();
}

void MainWindow::on_treeview_text_item_clicked_show_text(const QModelIndex &index)
{
  QStandardItem *current_item = (qobject_cast<const QStandardItemModel *>(index.model()))->itemFromIndex(index);

  if (current_item->parent() != tree_text_item_)
    return;

  QStandardItem *text_item2{tree_text_item_->child(current_item->index().row(), 1)};

  if (text_item2 == nullptr)
    return;

  QStandardItem *text_item1{tree_text_item_->child(current_item->index().row(), 0)};

  XItemDialog *text_dialog;
  std::map<QStandardItem *, XItemDialog *>::iterator it;

  if ((it = item_dialog_map_.find(text_item1)) != item_dialog_map_.end())
    text_dialog = it->second;
  else
  {
    QStandardItem *similarity_item{tree_text_item_->child(current_item->index().row(), 2)};

    if (similarity_item == nullptr)
      return;

    text_dialog
        = new XItemDialog(treeViewResult, text_item1, text_item2, similarity_item, false, this);

    if (text_dialog == nullptr)
      return;

    item_dialog_map_.insert(std::make_pair(text_item1, text_dialog));
  }
  text_dialog->show();
  text_dialog->raise();
  text_dialog->activateWindow();
}

void MainWindow::on_file_attribute_toggled(bool on)
{
  bool all_checked{on};
  all_checked |= groupBoxSize->isChecked();
  all_checked |= groupBoxCreateTime->isChecked();
  all_checked |= groupBoxModifyTime->isChecked();
  all_checked |= groupBoxAccessTime->isChecked();
  all_checked |= checkBoxSys->isChecked();
  all_checked |= checkBoxHide->isChecked();
  all_checked |= checkBoxReadOnly->isChecked();
  all_checked |= checkBoxArchive->isChecked();
  groupBoxFileAttribute->setChecked(all_checked);
}

void MainWindow::init_thread_parameter(XThreadParameter &thread_parameter)
{
  memset(&thread_parameter, 0, sizeof(XThreadParameter));

  if (groupBoxText->isChecked()) {
    thread_parameter.valid_flag |= VTEXT;

    if (checkBoxDocx->isChecked())
      thread_parameter.compare_option |= ODOCX;

    if (checkBoxTxt->isChecked())
      thread_parameter.compare_option |= OTXT;

    thread_parameter.text_threshold =
        static_cast<double>(spinBoxTextThreshold->value()) / 100.0;
  }

  if (groupBoxImage->isChecked()) {
    thread_parameter.valid_flag |= VIMAGE;

    if (checkBoxPng->isChecked())
      thread_parameter.compare_option |= OPNG;

    if (checkBoxJpeg->isChecked())
      thread_parameter.compare_option |= OJPEG;

    if (checkBoxJpg->isChecked())
      thread_parameter.compare_option |= OJPG;

    if (checkBoxBmp->isChecked())
      thread_parameter.compare_option |= OBMP;

    thread_parameter.image_threshold
        = static_cast<double>(spinBoxImageThreshold->value()) / 100.0;
  }

  if (groupBoxFileName->isChecked()) {
    thread_parameter.valid_flag |= VFILE_NAME;

    if (!checkBoxOnlyForSimilarTextOrImage->isChecked())
      thread_parameter.compare_option |= OFILE_NAME;

    thread_parameter.file_name_threshold
        = static_cast<double>(spinBoxFileNameThreshold->value()) / 100.0;
  }

#if 0
  if (groupBoxFileAttribute->isChecked())
#endif
  {
    thread_parameter.valid_flag |= VFILE_ATTRIBUTE;

    if (groupBoxSize->isChecked()) {
      thread_parameter.valid_flag |= VFILE_SIZE;

      thread_parameter.size_include = true;

      if (radioButtonSizeExlude->isChecked())
        thread_parameter.size_include = false;

      thread_parameter.larger_size = lineEditSize1->text().toInt();
      if (radioButtonSize1Byte->isChecked())
        ; // do nothing
      else if (radioButtonSize1Kib->isChecked())
        thread_parameter.larger_size *= 1024LL;
      else if (radioButtonSize1MiB->isChecked())
        thread_parameter.larger_size *= 1024LL * 1024LL;
      else if (radioButtonSize1GiB->isChecked())
        thread_parameter.larger_size *= 1024LL * 1024LL * 1024LL;

      thread_parameter.smaller_size = lineEditSize2->text().toInt();
      if (radioButtonSize2Byte->isChecked())
        ; // do nothing
      else if (radioButtonSize2Kib->isChecked())
        thread_parameter.smaller_size *= 1024LL;
      else if (radioButtonSize2MiB->isChecked())
        thread_parameter.smaller_size *= 1024LL * 1024LL;
      else if (radioButtonSize2GiB->isChecked())
        thread_parameter.smaller_size *= 1024LL * 1024LL * 1024LL;

      if (thread_parameter.larger_size < thread_parameter.smaller_size) {
        long long int tmp{thread_parameter.larger_size};
        thread_parameter.larger_size = thread_parameter.smaller_size;
        thread_parameter.smaller_size = tmp;
      }
    }

    if (groupBoxCreateTime->isChecked()) {
      thread_parameter.valid_flag |= VFILE_CREATE_TIME;

      thread_parameter.create_time_include = true;

      if (radioButtonCreateTimeExclude->isChecked())
        thread_parameter.create_time_include = false;

      thread_parameter.later_create_time
          = dateTimeEditCreateTime1->dateTime().toLocalTime().toTime_t();

      thread_parameter.early_create_time
          = dateTimeEditCreateTime2->dateTime().toLocalTime().toTime_t();

      if (thread_parameter.later_create_time < thread_parameter.early_create_time) {
        long long int tmp{thread_parameter.later_create_time};
        thread_parameter.later_create_time = thread_parameter.early_create_time;
        thread_parameter.early_create_time = tmp;
      }
    }

    if (groupBoxModifyTime->isChecked()) {
      thread_parameter.valid_flag |= VFILE_MODIFY_TIME;

      thread_parameter.modify_time_include = true;

      if (radioButtonModifyTimeExclude->isChecked())
        thread_parameter.modify_time_include = false;

      thread_parameter.later_modify_time
          = dateTimeEditModifyTime1->dateTime().toLocalTime().toTime_t();

      thread_parameter.early_modify_time
          = dateTimeEditModifyTime2->dateTime().toLocalTime().toTime_t();

      if (thread_parameter.later_modify_time < thread_parameter.early_modify_time) {
        long long int tmp{thread_parameter.later_modify_time};
        thread_parameter.later_modify_time = thread_parameter.early_modify_time;
        thread_parameter.early_modify_time = tmp;
      }
    }

    if (groupBoxAccessTime->isChecked()) {
      thread_parameter.valid_flag |= VFILE_ACCESS_TIME;

      thread_parameter.access_time_include = true;

      if (radioButtonAccessTimeExclude->isChecked())
        thread_parameter.access_time_include = false;

      thread_parameter.later_access_time
          = dateTimeEditAccessTime1->dateTime().toLocalTime().toTime_t();

      thread_parameter.early_access_time
          = dateTimeEditAccessTime2->dateTime().toLocalTime().toTime_t();

      if (thread_parameter.later_access_time < thread_parameter.early_access_time) {
        long long int tmp{thread_parameter.later_access_time};
        thread_parameter.later_access_time = thread_parameter.early_access_time;
        thread_parameter.early_access_time = tmp;
      }
    }

    if (checkBoxSys->isChecked())
      thread_parameter.file_attribute |= FILE_ATTRIBUTE_SYSTEM;

    if (checkBoxHide->isChecked())
      thread_parameter.file_attribute |= FILE_ATTRIBUTE_HIDDEN;

    if (checkBoxReadOnly->isChecked())
      thread_parameter.file_attribute |= FILE_ATTRIBUTE_READONLY;

    if (checkBoxArchive->isChecked())
      thread_parameter.file_attribute |= FILE_ATTRIBUTE_ARCHIVE;

    thread_parameter.file_attribute |= FILE_ATTRIBUTE_NORMAL;
  }

  if (hash_select->isChecked()) {
    thread_parameter.valid_flag |= VCRYPT_HASH;

    thread_parameter.crypt_hash_only_as_fallback
        = checkBoxCryptHashOnlyAsFallback->isChecked();

    thread_parameter.hash_algorithm = QCryptographicHash::Md5;

    if (Md4->isChecked())
      thread_parameter.hash_algorithm = QCryptographicHash::Md4;
    else if (Md5->isChecked())
      thread_parameter.hash_algorithm = QCryptographicHash::Md5;
    else if (Sha1->isChecked())
      thread_parameter.hash_algorithm = QCryptographicHash::Sha1;
    else if (Sha224->isChecked())
      thread_parameter.hash_algorithm = QCryptographicHash::Sha224;
    else if (Sha256->isChecked())
      thread_parameter.hash_algorithm = QCryptographicHash::Sha256;
    else if (Sha384->isChecked())
      thread_parameter.hash_algorithm = QCryptographicHash::Sha384;
    else if (Sha512->isChecked())
      thread_parameter.hash_algorithm = QCryptographicHash::Sha512;
    else if (Sha3_224->isChecked())
      thread_parameter.hash_algorithm = QCryptographicHash::Sha3_224;
    else if (Sha3_256->isChecked())
      thread_parameter.hash_algorithm = QCryptographicHash::Sha3_256;
    else if (Sha3_384->isChecked())
      thread_parameter.hash_algorithm = QCryptographicHash::Sha3_384;
    else if (Sha3_512->isChecked())
      thread_parameter.hash_algorithm = QCryptographicHash::Sha3_512;
  }

  int i = 0;
  QListWidgetItem *item;

  thread_parameter.dir_list = new QStringList;
  while((item = listWidget->item(i++)) != NULL)
    thread_parameter.dir_list->push_back(item->text());

  thread_parameter.recursion = checkBoxRecursion->isChecked();
  thread_parameter.follow_symlinks = checkBoxFollowSymlinks->isChecked();
  thread_parameter.ignore_hardlinks = checkBoxIgnoreHardlinks->isChecked();
  thread_parameter.ignore_junction = checkBoxIgnoreJunction->isChecked();
  thread_parameter.compare_handle = compare_handle_ = dupclean_.new_compare();
}

void MainWindow::init_treeViewResult()
{
  tree_model_result_->clear();

  tree_model_result_->setHorizontalHeaderLabels(QStringList() << QString::fromStdWString(L"比较方法/文件路径/哈希值")
                                         << QString::fromStdWString(L"文件路径/相同文件的个数")
                                         << QString::fromStdWString(L"相似度"));

  tree_parent_item_ = tree_model_result_->invisibleRootItem();
  tree_parent_item_->setEditable(false);

  tree_text_item_ = new QStandardItem(QStringLiteral(u"文档"));
  tree_image_item_ = new QStandardItem(QString::fromWCharArray(L"图片"));
  tree_crypt_hash_item_ = new QStandardItem(QString::fromWCharArray(L"精确比较（文件完全相同）"));
#if 0
  tree_file_path_item_ = new QStandardItem(QStringLiteral(u"文件名"));
#endif
  treeViewResult->expandAll();

  tree_text_item_->setEditable(false);
  tree_image_item_->setEditable(false);
  tree_crypt_hash_item_->setEditable(false);
#if 0
  tree_file_path_item_->setEditable(false);
#endif

  tree_parent_item_->appendRow(tree_text_item_);
  tree_parent_item_->appendRow(tree_image_item_);
  tree_parent_item_->appendRow(tree_crypt_hash_item_);
#if 0
  tree_parent_item_->appendRow(tree_file_path_item_);
#endif

  items_sort_by_file_path_.clear();
  items_sort_by_file_name_.clear();
  items_sort_by_dir_name_.clear();
  items_sort_by_create_time_.clear();
  items_sort_by_modify_time_.clear();
  items_only_left_one_.clear();

  tree_model_file_attribute_->clear();

  for (auto &it : item_dialog_map_)
     it.second->setUnuseable();
  item_dialog_map_.clear();

  file_to_be_deleted_map_.clear();
  file_to_be_deleted_size_ = 0llu;
  show_file_to_be_deleted_size(file_to_be_deleted_map_.size(), file_to_be_deleted_size_);
}

void MainWindow::select_max_min_items(
    std::list<XMaxMinItem> *current_max_min_item,
    bool maxmin, bool checked)
{
  if (current_max_min_item->empty())
    return;

  std::list<XMaxMinItem>::iterator it;

  for (it = current_max_min_item->begin();
       it != current_max_min_item->end(); ++it) {
    // true max, false min
    if (maxmin && !is_item_already_deleted(it->max)) {
      set_item_will_be_deleted(it->max, checked);
      count_file_to_be_deleted(it->max, checked);
    }
    else if (!maxmin && !is_item_already_deleted(it->min)) {
      set_item_will_be_deleted(it->min, checked);
      count_file_to_be_deleted(it->min, checked);
    }
  }
}

void MainWindow::RecycleTreeViewItem(QStandardItem *item,
                                     std::wstring *file_paths,
                                     std::list<QStandardItem *> *item_list)
{
  // 已被删除，                          被标记删除
  if (is_item_already_deleted(item) || !is_item_will_be_deleted(item))
    return;

  file_paths->append(item->text().toStdWString());
  file_paths->append(1llu, L'\0');
  item_list->push_back(item);
  //set_item_already_deleted(item, RecycleFileOnWindows(file_path) == 0);
}

int MainWindow::RecycleFileOnWindows(const std::wstring &file_paths)
{
  SHFILEOPSTRUCT shfos{};
  shfos.hwnd   = (HWND)winId();       // handle to window that will own generated windows, if applicable
  shfos.wFunc  = FO_DELETE;
  shfos.pFrom  = file_paths.c_str();
  shfos.pTo    = nullptr;       // not used for deletion operations
  shfos.fFlags = FOF_ALLOWUNDO | FOF_SIMPLEPROGRESS; // use the recycle bin
  shfos.lpszProgressTitle = L"重复文件清理工具";

  return SHFileOperation(&shfos) == 0 ? 0 : -1;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
  if(worker_thread_ == NULL)
    return;
  bool itsme = false;
  QMessageBox msg(this);
  msg.setWindowTitle(QString::fromWCharArray(L"问题"));
  msg.setIcon(QMessageBox::Question);
  msg.setText(QStringLiteral(u"正在比较文件，真的要退出吗？"));
  QAbstractButton *sureButton = msg.addButton(QString::fromWCharArray(L"确定"),
                                              QMessageBox::AcceptRole);
  msg.addButton(QString::fromWCharArray(L"取消"), QMessageBox::RejectRole);
  if(!worker_thread_->get_pause()) {
    on_pauseButton_clicked();
    itsme = true;
  }
  msg.exec();
  if(msg.clickedButton() == sureButton) {
    // 这个块里是 on_stopButton_clicked()的停止方法。因为on_stopButton_clicked
    // 添加了问询提示，所以就复制到这里了。
    if(worker_thread_->get_pause()) {
      emit resume_worker();
      pauseButton->setText(QString::fromWCharArray(L"暂停"));
    }
    emit stop_worker();
    //worker_thread_->terminate();
    worker_thread_->wait();
  }
  else {
    if(itsme)
      on_pauseButton_clicked();
    event->ignore();
  }
}

void MainWindow::startWorkInAThread(XThreadParameter &thread_parameter)
{
  worker_thread_ = new CompareThread(this);
  worker_thread_->set_thread_parameter(thread_parameter);
  connect(this, &MainWindow::stop_worker, worker_thread_, &CompareThread::stop);
  connect(this, &MainWindow::pause_worker, worker_thread_, &CompareThread::pause);
  connect(this, &MainWindow::resume_worker, worker_thread_, &CompareThread::resume);
  connect(worker_thread_, &CompareThread::crypt_hash_result_ready, this, &MainWindow::handle_crypt_hash_result);
  connect(worker_thread_, &CompareThread::update_compare_result, this, &MainWindow::show_compare_result_to_tree);
  connect(worker_thread_, &CompareThread::finished, worker_thread_, &QObject::deleteLater);
  connect(worker_thread_, &CompareThread::update_text_msg, labelPercent, &QLabel::setText, Qt::QueuedConnection);
  //connect(worker_thread_, &CompareThread::update_current_file_path, labelCurrentPath, &QLabel::setText, Qt::QueuedConnection);
  connect(worker_thread_, &CompareThread::update_progressbar, progressBar, &QProgressBar::setValue, Qt::QueuedConnection);
  //QObject::connect(*(_DWORD *)(v1 + 220), "2actionEffectsOptions()", v1, "1actionEffectsOptions()", 0);
  worker_thread_->start();
}

//http://bastian.rieck.ru/blog/posts/2014/selections_qt_osg/
