#include "import.h"
#include <QDebug>
#include <QDir>
#include <cassert>

DupCleanFunctionTable dupclean_;

enum {MAX_MSG_SIZE = 128 << 2};

static const wchar_t *error_wcstr(int unsigned long errcode)
{
  static wchar_t wemsg[MAX_MSG_SIZE];

  int unsigned long wemsgwchars;

  wmemset(wemsg, L'\0',  MAX_MSG_SIZE / sizeof(wemsg[0]));

  wemsgwchars = 0LU;

  wemsgwchars += FormatMessage(
      FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
      errcode,  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      wemsg + wemsgwchars, MAX_MSG_SIZE - 1 - wemsgwchars, NULL);

  wemsgwchars -= 2;   /* 为了去掉结尾的\r\n，所以减 2 */
  wemsgwchars += FormatMessage(
      FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
      errcode,  MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
      wemsg + wemsgwchars, MAX_MSG_SIZE - 1 - wemsgwchars,
      NULL);

  wemsgwchars -= 2;   /* 为了去掉结尾的\r\n，所以减 2 */
  wemsgwchars += _snwprintf(wemsg + wemsgwchars,
      MAX_MSG_SIZE - 1 - wemsgwchars, L"(%lu)\n", errcode);
  return wemsg;
}

static const char *error_cstr(int unsigned long errcode)
{
  static char emsg[MAX_MSG_SIZE];

  const wchar_t *pwemsg = error_wcstr(errcode);

  unsigned long long int bytes_required;

  if ((bytes_required = wcstombs(NULL, pwemsg, 0u) + 1u) == 0u) {
    fwprintf(stderr, L"%ls", _wcserror(errno));
    return nullptr;
  }

  if (bytes_required > MAX_MSG_SIZE)
    bytes_required = MAX_MSG_SIZE;

  memset(emsg, 0, MAX_MSG_SIZE);
  wcstombs(emsg, pwemsg, bytes_required); // must reach last L'\0'
}

void init_dupclean_function_table()
{
  if ((dupclean_.hmodule = LoadLibraryW(L"libevqdmfbo.dll")) == nullptr) {
    qDebug() << QString::fromWCharArray(error_wcstr(GetLastError()));
    qDebug() << QDir::currentPath();
    assert(dupclean_.hmodule != nullptr);
    return;
  }

  dupclean_.init_compare = (void (*)(void))GetProcAddress(dupclean_.hmodule, MAKEINTRESOURCEA(1));
  assert(dupclean_.init_compare != nullptr);
  dupclean_.destroy_compare = (void (*)(void))GetProcAddress(dupclean_.hmodule, MAKEINTRESOURCEA(2));
  assert(dupclean_.destroy_compare != nullptr);
  dupclean_.new_compare = (XCompareHandle (*)(void))GetProcAddress(dupclean_.hmodule, MAKEINTRESOURCEA(3));
  assert(dupclean_.new_compare != nullptr);
  dupclean_.delete_compare = (void (*)(XCompareHandle handle))GetProcAddress(dupclean_.hmodule, MAKEINTRESOURCEA(4));
  assert(dupclean_.delete_compare != nullptr);
  dupclean_.get_compare_parameter = (XCompareParameter (*)(XCompareHandle handle))GetProcAddress(dupclean_.hmodule, MAKEINTRESOURCEA(5));
  assert(dupclean_.get_compare_parameter != nullptr);
  dupclean_.set_compare_parameter = (void (*)(XCompareHandle handle, const XCompareParameter *parameter))GetProcAddress(dupclean_.hmodule, MAKEINTRESOURCEA(6));
  assert(dupclean_.set_compare_parameter != nullptr);
  dupclean_.hash = (int (*)(XCompareHandle handle, const wchar_t *file_path))GetProcAddress(dupclean_.hmodule, MAKEINTRESOURCEA(7));
  assert(dupclean_.hash != nullptr);
  dupclean_.compare_two = (int (*)(XCompareHandle handle, XCompareResult *compare_result,
      const wchar_t * __restrict file_path1,
      const wchar_t *__restrict file_path2))GetProcAddress(dupclean_.hmodule, MAKEINTRESOURCEA(8));
  assert(dupclean_.compare_two != nullptr);
  dupclean_.compare_two_file_name = (int (*)(XCompareHandle handle,
      XCompareResult *compare_result, const wchar_t *__restrict file_path1,
      const wchar_t *__restrict file_path2))GetProcAddress(dupclean_.hmodule, MAKEINTRESOURCEA(9));
  assert(dupclean_.compare_two_file_name != nullptr);
  dupclean_.compare_all_image = (int (*)(XCompareHandle handle,
      CompareResultCallback result_callback, void *arg))GetProcAddress(dupclean_.hmodule, MAKEINTRESOURCEA(10));
  assert(dupclean_.compare_all_image != nullptr);
  dupclean_.compare_all_text = (int (*)(XCompareHandle handle,
      CompareResultCallback result_callback, void *arg))GetProcAddress(dupclean_.hmodule, MAKEINTRESOURCEA(11));
  assert(dupclean_.compare_all_text != nullptr);
  dupclean_.compare_all_file_name = (int (*)(XCompareHandle handle,
      CompareResultCallback result_callback, void *arg))GetProcAddress(dupclean_.hmodule, MAKEINTRESOURCEA(12));
  assert(dupclean_.compare_all_file_name != nullptr);
  dupclean_.stop_compare_all_image = (void (*)(XCompareHandle handle))
      GetProcAddress(dupclean_.hmodule, MAKEINTRESOURCEA(13));
  assert(dupclean_.stop_compare_all_image != nullptr);
  dupclean_.stop_compare_all_text = (void (*)(XCompareHandle handle))
      GetProcAddress(dupclean_.hmodule, MAKEINTRESOURCEA(14));
  assert(dupclean_.stop_compare_all_text != nullptr);
  dupclean_.stop_compare_all_file_name = (void (*)(XCompareHandle handle))
      GetProcAddress(dupclean_.hmodule, MAKEINTRESOURCEA(15));
  assert(dupclean_.stop_compare_all_file_name != nullptr);
  dupclean_.doc_to_text = (int (*)(const wchar_t *__restrict doc_path, XDocText *__restrict doc_text))
      GetProcAddress(dupclean_.hmodule, MAKEINTRESOURCEA(16));
  assert(dupclean_.doc_to_text != nullptr);
  dupclean_.destroy_doctext = (void (*)(XDocText *doc_text))
      GetProcAddress(dupclean_.hmodule, MAKEINTRESOURCEA(17));
}

void destroy_dupclean_function_table()
{
  FreeLibrary(dupclean_.hmodule);
  memset(&dupclean_, 0, sizeof(DupCleanFunctionTable));
}
