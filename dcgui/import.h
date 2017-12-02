#ifndef IMPORT_H
#define IMPORT_H

#include <windows.h>
#include "vc_c_api.h"

struct DupCleanFunctionTable
{
  HMODULE hmodule;

  void (*init_compare)(void);
  void (*destroy_compare)(void);
  XCompareHandle (*new_compare)(void);
  void (*delete_compare)(XCompareHandle handle);
  XCompareParameter (*get_compare_parameter)(XCompareHandle handle);
  void (*set_compare_parameter)(XCompareHandle handle,
      const XCompareParameter *parameter);
  /*  �ɹ�����0��ʧ�ܷ���-1�����Ƚ�ͼ����ı�����0 */
  int (*hash)(XCompareHandle handle, const wchar_t *file_path);
  /* ����1��true����������0��false�� */
  int (*compare_two)(XCompareHandle handle, XCompareResult *compare_result,
      const wchar_t * __restrict file_path1,
      const wchar_t *__restrict file_path2);
  /* ����1��true����������0��false�� */
  int (*compare_two_file_name)(XCompareHandle handle,
      XCompareResult *compare_result,
      const wchar_t *__restrict file_path1,
      const wchar_t *__restrict file_path2);
  /* �ɹ���������ͼƬ�Ķ������п�����0����ʧ�ܷ���-1 */
  int (*compare_all_image)(XCompareHandle handle,
      CompareResultCallback result_callback, void *arg);
  /* �ɹ����������ı��ļ��Ķ������п�����0����ʧ�ܷ���-1 */
  int (*compare_all_text)(XCompareHandle handle,
      CompareResultCallback result_callback, void *arg);
  /* �ɹ����������ļ����Ķ������п�����0����ʧ�ܷ���-1 */
  int (*compare_all_file_name)(XCompareHandle handle,
      CompareResultCallback result_callback, void *arg);
  void (*stop_compare_all_image)(XCompareHandle handle);
  void (*stop_compare_all_text)(XCompareHandle handle);
  void (*stop_compare_all_file_name)(XCompareHandle handle);
  int (*doc_to_text)(const wchar_t *__restrict doc_path, XDocText *__restrict doc_text);
  void (*destroy_doctext)(XDocText *doc_text);
};

extern DupCleanFunctionTable dupclean_;

void init_dupclean_function_table();
void destroy_dupclean_function_table();

#endif // IMPORT_H
