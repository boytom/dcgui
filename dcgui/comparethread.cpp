#include "comparethread.h"
#include <QString>
#include <QProgressBar>
#include <QDebug>
#include <QLabel>
#include <QDirIterator>
#include <sys/types.h>
#include <sys/stat.h>

CompareThread::CompareThread(QObject *parent) :  QThread(parent)
{

}

CompareThread::~CompareThread()
{
 qDebug() << "be deleted.";
}

#if 0
void CompareThread::run()
{
  FILE *fp;
#if defined(linux)
  fp = fopen("/home/whg/abc.txt", "a");
#else
  fp = fopen("D:\\abc.txt", "at");
#endif

  QStringListIterator iter(dir_list_);
  QCryptographicHash hash(algorithm_);
  QFile file;

  while(iter.hasNext()) {
    QDirIterator dir_iter(iter.next(), QDirIterator::Subdirectories);
    while(dir_iter.hasNext()) {
      QString file_path = dir_iter.next();
      if(dir_iter.fileInfo().isDir())
        continue;
      file.setFileName(file_path);
      file.open(QIODevice::ReadOnly);

      if(hash.addData(&file))
        fprintf(fp, "%s %s\n", QString(hash.result().toHex()).toStdString()
                .c_str(), file_path.toStdString().c_str());
      else
        fprintf(fp, "error %s\n", file_path.toStdString().c_str());
      file.close();
      hash.reset();
    }
  }
  fclose(fp);
}
#endif

void CompareThread::run()
{
  file_total();

  if(file_total_ == 0.0) {
    emit update_text_msg(QStringLiteral(u"比较完成。"));
    emit crypt_hash_result_ready(nullptr);
    return;
  }

  crypt_hash_result_ = new QMap<QByteArray, QString>;

  if (crypt_hash_result_ == nullptr) {
    emit update_text_msg(QStringLiteral(u"比较完成。"));
    emit crypt_hash_result_ready(nullptr);
    return;
  }

  hash_ = new QCryptographicHash(parameter_.hash_algorithm);

  if (hash_ == nullptr) {
    delete crypt_hash_result_;
    emit update_text_msg(QStringLiteral(u"比较完成。"));
    emit crypt_hash_result_ready(nullptr);
    return;
  }

  QStringListIterator iter(*parameter_.dir_list);

  double file_count = 0.0;
  wchar_t msg[BUFSIZ];

  QDirIterator::IteratorFlags dir_iterator_flags{QDirIterator::NoIteratorFlags};

  emit update_progressbar(file_count * 100.0 / file_total_);

  if (parameter_.recursion)
    dir_iterator_flags |= QDirIterator::Subdirectories;
  if (parameter_.follow_symlinks)
    dir_iterator_flags |= QDirIterator::FollowSymlinks;

  emit update_text_msg(QString::fromWCharArray(L"开始计算……"));
  while(iter.hasNext() && !(volatile bool)stop_) {
    QDirIterator dir_iter(iter.next(), dir_iterator_flags);
    while(dir_iter.hasNext() && !(volatile bool)stop_) {
      mutex_.lock();
      while((volatile bool)pause_) {
        emit update_text_msg(QStringLiteral(u"计算暂停……"));
        condition_.wait(&mutex_);
      }
      dir_iter.next();
      if(dir_iter.fileInfo().isDir()) {
        mutex_.unlock();
        continue;
      }

      emit update_current_file_path(dir_iter.filePath());

#if defined(_MSC_VER)
      _snwprintf_s(msg, sizeof(msg) / sizeof(msg[0]),
                    sizeof(msg) / sizeof(msg[0]) - 1, L"%.0lf/%.0lf %ls", file_count,
                file_total_, dir_iter.fileName().toStdWString().c_str());
#else
      snprintf(msg, sizeof(msg), "%.0lf/%.0lf %s", file_count, file_total_,
               dir_iter.fileName().toStdString().c_str());
#endif
      emit update_text_msg(QString::fromWCharArray(msg));

      do_compare(dir_iter);

#if defined(_MSC_VER)
      _snwprintf_s(msg, sizeof(msg) / sizeof(msg[0]),
                    sizeof(msg) / sizeof(msg[0]) - 1, L"%.0lf/%.0lf %ls", ++file_count,
                file_total_, dir_iter.fileName().toStdWString().c_str());
#else
      snprintf(msg, sizeof(msg), "%.0lf/%.0lf %s", ++file_count, file_total_,
               dir_iter.fileName().toStdString().c_str());
#endif
      emit update_text_msg(QString::fromWCharArray(msg));
      emit update_progressbar(file_count * 100.0 / file_total_);
      mutex_.unlock();
    }
  }
  delete hash_;

  emit update_text_msg(QStringLiteral(u"正在比较……"));
  if ((parameter_.valid_flag & VIMAGE) == VIMAGE) {
    emit update_text_msg(QStringLiteral(u"正在比较图片……"));
    dupclean_.compare_all_image(parameter_.compare_handle, CompareResultCallback, reinterpret_cast<void *>(this));
  }

  if ((parameter_.valid_flag & VTEXT) == VTEXT) {
    emit update_text_msg(QStringLiteral(u"正在比较文档……"));
    dupclean_.compare_all_text(parameter_.compare_handle, CompareResultCallback, reinterpret_cast<void *>(this));
  }

  if ((parameter_.valid_flag & VFILE_NAME) == VFILE_NAME) {
    emit update_text_msg(QStringLiteral(u"正在比较文件名……"));
    dupclean_.compare_all_file_name(parameter_.compare_handle, CompareResultCallback, reinterpret_cast<void *>(this));
  }

  emit update_text_msg(QStringLiteral(u"比较完成。"));
  emit crypt_hash_result_ready(crypt_hash_result_);
}

void CompareThread::set_thread_parameter(const XThreadParameter &parameter)
{
  parameter_ = parameter;

  XCompareParameter compare_parameter;

  compare_parameter.compare_option = parameter_.compare_option;
  compare_parameter.text_threshold = parameter_.text_threshold;
  compare_parameter.image_threshold = parameter_.image_threshold;
  compare_parameter.file_name_threshold = parameter_.file_name_threshold;

  dupclean_.set_compare_parameter(parameter_.compare_handle, &compare_parameter);
}

void CompareThread::stop()
{
  stop_ = true;
  dupclean_.stop_compare_all_image(parameter_.compare_handle);
  dupclean_.stop_compare_all_text(parameter_.compare_handle);
  dupclean_.stop_compare_all_file_name(parameter_.compare_handle);
}

bool CompareThread::get_stop()
{
  return stop_;
}

void CompareThread::pause()
{
  pause_ = true;
  mutex_.lock();
}

bool CompareThread::get_pause()
{
  return pause_;
}

void CompareThread::resume()
{
  pause_ = false;
  condition_.wakeOne();
  mutex_.unlock();
}

void CompareThread::file_total()
{
  wchar_t buf[BUFSIZ];

  QStringListIterator iter(*parameter_.dir_list);

  file_total_ = 0.0;

  QDirIterator::IteratorFlags dir_iterator_flags{QDirIterator::NoIteratorFlags};

  if (parameter_.recursion)
    dir_iterator_flags |= QDirIterator::Subdirectories;
  if (parameter_.follow_symlinks)
    dir_iterator_flags |= QDirIterator::FollowSymlinks;

  emit update_text_msg(QStringLiteral(u"计算文件总数……"));
  while(iter.hasNext() && !(volatile bool)stop_) {
    QDirIterator dir_iter(iter.next(), dir_iterator_flags);
    while(dir_iter.hasNext() && !(volatile bool)stop_) {
      mutex_.lock();
      while((volatile bool)pause_)
        condition_.wait(&mutex_);
      QString file_path = dir_iter.next();
      if(dir_iter.fileInfo().isDir()) {
        mutex_.unlock();
        continue;
      }
      ++file_total_;
#if defined(_MSC_VER)
      _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]),
                   sizeof(buf) / sizeof(buf[0]) - 1, L"计算文件总数…… %.0lf %ls", file_total_,
                dir_iter.fileName().toStdWString().c_str());
#else
      snprintf(buf, sizeof(buf), "计算文件总数…… %.0lf %s", file_total_,
               dir_iter.fileName().toStdString().c_str());
#endif
      emit update_text_msg(QString::fromWCharArray(buf));
      mutex_.unlock();
    }
  }
#if defined(_MSC_VER)
  _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]),
      sizeof(buf) / sizeof(buf[0]) - 1, L"计算文件总数完毕。 %.0lf", file_total_);
#else
  snprintf(buf, sizeof(buf), "计算文件总数完毕。 %.0lf", file_total_);
#endif

  emit update_text_msg(QString::fromWCharArray(buf));
}

void CompareThread::do_compare(const QDirIterator &dir_iter)
{
  if (parameter_.valid_flag == VNONE)
    return;

  QString file_path = QDir::toNativeSeparators(dir_iter.filePath());

#define FLAG_VALID(flag) ((parameter_.valid_flag & flag) == flag)

  if (FLAG_VALID(VFILE_ATTRIBUTE)) {
    struct __stat64 st;

    if (_wstat64(file_path.toStdWString().c_str(), &st) == -1)
      return;

    if (FLAG_VALID(VFILE_SIZE)) {
      if (parameter_.size_include
          && (st.st_size < parameter_.smaller_size || st.st_size > parameter_.larger_size))
        return;
      else if (!parameter_.size_include
               && (st.st_size > parameter_.smaller_size && st.st_size < parameter_.larger_size))
        return;
    }

    if (FLAG_VALID(VFILE_CREATE_TIME)) {
      if (parameter_.create_time_include
          && (st.st_ctime < parameter_.early_create_time || st.st_ctime > parameter_.later_create_time))
          return;
      else if(!parameter_.create_time_include
              && (st.st_ctime > parameter_.early_create_time && st.st_ctime < parameter_.later_create_time))
        return;
    }

    if (FLAG_VALID(VFILE_MODIFY_TIME)) {
      if (parameter_.modify_time_include
          && (st.st_mtime < parameter_.early_modify_time || st.st_mtime > parameter_.later_modify_time))
          return;
      else if(!parameter_.modify_time_include
              && (st.st_mtime > parameter_.early_modify_time && st.st_mtime < parameter_.later_modify_time))
        return;
    }

    if (FLAG_VALID(VFILE_ACCESS_TIME)) {
      if (parameter_.access_time_include
          && (st.st_atime < parameter_.early_access_time || st.st_ctime > parameter_.later_access_time))
          return;
      else if(!parameter_.access_time_include
              && (st.st_atime > parameter_.early_access_time && st.st_ctime < parameter_.later_access_time))
        return;
    }

    /* 这些都是 windows 中的文件属性 */
    /* GetFileAttributesExW 也可以取得三个时间和文件大小，不过我更习惯用 _wstat64 获取 */
    /* 而且 dir_iter中的fileInfo也不能提供需要的信息 */
    WIN32_FILE_ATTRIBUTE_DATA file_attribute_data;

    if (!GetFileAttributesExW(file_path.toStdWString().c_str(), GetFileExInfoStandard,
                              &file_attribute_data))
      return;

#define NOT_NEED_ATTRIBUTE(attr) ((file_attribute_data.dwFileAttributes & attr) == attr \
  && (parameter_.file_attribute & attr) != attr)

    if (NOT_NEED_ATTRIBUTE(FILE_ATTRIBUTE_SYSTEM))
      return;
    if (NOT_NEED_ATTRIBUTE(FILE_ATTRIBUTE_HIDDEN))
      return;
    if (NOT_NEED_ATTRIBUTE(FILE_ATTRIBUTE_READONLY))
      return;
    if (NOT_NEED_ATTRIBUTE(FILE_ATTRIBUTE_ARCHIVE))
      return;
    if (NOT_NEED_ATTRIBUTE(FILE_ATTRIBUTE_NORMAL))
      return;
  }

  int dupclean_hash_result{-1};

  if (FLAG_VALID(VTEXT) || FLAG_VALID(VIMAGE) || FLAG_VALID(VFILE_NAME))
    dupclean_hash_result
        = dupclean_.hash(parameter_.compare_handle, file_path.toStdWString().c_str());

#if !defined(__FREE_VERSION__)
  if (FLAG_VALID(VCRYPT_HASH)) {
    if (!parameter_.crypt_hash_only_as_fallback || dupclean_hash_result == -1)
      do_crypt_hash(file_path);
  }
#endif
}

void CompareThread::do_crypt_hash(const QString &file_path)
{
  file_.setFileName(file_path);
  file_.open(QIODevice::ReadOnly);
  if(hash_->addData(&file_))
    crypt_hash_result_->insertMulti(hash_->result(), file_path);
  file_.close();
  hash_->reset();
}

void CompareThread::emit_update_compare_result(const XCompareResult *result)
{
  emit update_compare_result(result);
}

void CompareThread::emit_update_progressbar(const XCompareResult *result)
{
  int value = (int)(result->progress.count * 100.0 / result->progress.total);
  emit update_progressbar(value);
}

int CompareThread::CompareResultCallback(XCompareHandle handle,
                                         const XCompareResult *result, void *arg)
{
  Q_UNUSED(handle);
  CompareThread *me = reinterpret_cast<CompareThread *>(arg);

  me->emit_update_progressbar(result);

  if (result->similar) {
    XCompareResult *result_copy = new XCompareResult;
    if (result_copy == nullptr)
      return 0;
#if 0
    {
      FILE *fp;
      _wfopen_s(&fp, L"D:\\temp\\image.txt", L"atS,ccs=UTF-16LE");
      fputws(L"=========================================================\n", fp);
      fwprintf_s(fp, L"file_path1 = %ls\n"
                 L"file_path2 = %ls\n"
                 L"radial = %d\n"
                 L"dct = %d\n"
                 L"mh = %d\n"
                 L"radial_similarity = %lf\n"
                 L"dct_hamming_distance = %d\n"
                 L"mh_hamming_distance = %lf\n"
                 L"dct_similarity = %lf\n"
                 L"mh_similarity = %lf\n",
                 result->file_path1, result->file_path2,
                 result->image.radial, result->image.dct, result->image.mh,
                 result->image.radial_similarity, result->image.dct_hamming_distance,
                 result->image.mh_hamming_distance, result->image.dct_similarity,
                 result->image.mh_similarity);
      fclose(fp);
    }
#endif
    memset(result_copy, 0, sizeof(XCompareResult));
    *result_copy = *result;
    me->emit_update_compare_result(result_copy);
  }

  return 0;
}
