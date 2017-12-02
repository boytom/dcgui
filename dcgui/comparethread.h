#ifndef COMPARETHREAD_H
#define COMPARETHREAD_H

#include <QThread>
#include <QStringList>
#include <QByteArray>
#include <QMutex>
#include <QWaitCondition>
#include <QFile>
#include <QDir>
#include <QMap>
#include <QCryptographicHash>

#include "import.h"

class QProgressBar;
class QLabel;
class QCryptographicHash;

enum XValidFlag : unsigned int
{
  VNONE              = 0x0u,
  VTEXT              = 0x1u,
  VIMAGE             = 0x2u,
  VFILE_NAME         = 0x4u,
  VFILE_ATTRIBUTE    = 0x8u,
  VFILE_SIZE         = 0x10u,
  VFILE_CREATE_TIME  = 0x20u,
  VFILE_MODIFY_TIME  = 0x40u,
  VFILE_ACCESS_TIME  = 0x80u,
  VCRYPT_HASH        = 0x100u
};

struct XThreadParameter
{
  unsigned int valid_flag;

  bool size_include, create_time_include, modify_time_include,
      access_time_include, crypt_hash_only_as_fallback,
      recursion, follow_symlinks, ignore_hardlinks, ignore_junction;

  unsigned int compare_option;
  double text_threshold, image_threshold, file_name_threshold;

  long long int larger_size, smaller_size, later_create_time, early_create_time,
  later_modify_time, early_modify_time, later_access_time, early_access_time;

  unsigned long int file_attribute;

  QCryptographicHash::Algorithm hash_algorithm;

  XCompareHandle compare_handle;
  QStringList *dir_list;
};

class CompareThread : public QThread
{
  Q_OBJECT
public:
  explicit CompareThread(QObject *parent = 0);
  virtual ~CompareThread();

  void run() Q_DECL_OVERRIDE;

  void set_thread_parameter(const XThreadParameter &parameter);

  bool get_stop();
  bool get_pause();

signals:
  void crypt_hash_result_ready(QMap<QByteArray, QString> *crypt_hash_result);
  void update_compare_result(const XCompareResult *compare_result);
  void update_text_msg(const QString &msg);
  void update_current_file_path(const QString &msg);
  void update_progressbar(int value);

public slots:
  void stop();
  void pause();
  void resume();

private:
  QMap<QByteArray, QString> *crypt_hash_result_{nullptr};

  QCryptographicHash *hash_{nullptr}; //< used for crypt hash
  QFile file_{nullptr};               //< used for crypt hash

  bool stop_{false}, pause_{false};
  double file_total_{0.0};

  QMutex mutex_;
  QWaitCondition condition_;

  XThreadParameter parameter_;

  void file_total();

  void do_compare(const QDirIterator &dir_iter);

  void do_crypt_hash(const QString &file_path);

  void emit_update_compare_result(const XCompareResult *result);
  void emit_update_progressbar(const XCompareResult *result);
  static int CompareResultCallback(XCompareHandle handle,
                            const XCompareResult *result, void *arg);
};

#endif // COMPARETHREAD_H
