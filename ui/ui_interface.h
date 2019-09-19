#ifndef UI_INTERFACE_H
#define UI_INTERFACE_H
#include <string>

namespace ui
{
    //��ʼ��
    bool init(void* hInstance);

    //��ȡ�����ھ��
    void* get_main_hwnd();

    //ˢ���ֻ��б�
    void refresh_phone_device();

    //ˢ�½��
    void refresh_result(int total, int count, bool is_success, bool is_local, int curr_page, const std::wstring& dir);
    void refresh_result(int total, int count, bool is_success, bool is_local, int curr_page, int file_type);

    //��ȡ��ǰ·��
    std::wstring get_curr_path();

    //���¼�������
    void reload_data();

    //ˢ��ˢ������ͼͼƬ
    void refresh_thumbnail_image(void* file_info_ptr, void* image_ptr);

    //��ʾͼƬ
    void show_picture(bool is_phone_pic, const std::wstring& pic_name, const std::wstring& pic_path, const std::wstring& phone_sn);

    //����ֻ�ͬ���ļ����
    void add_phone_sync_file_result(int index, int result);

    //ɾ���ֻ�ͬ���ļ����
    void del_phone_sync_file_result(int index, bool result);

    //��λ�ļ�
    void location_file(int index);

    //ˢ�²鿴��ͼ
    void refresh_view_big_image(const std::string& pic_name, void* image_data);

    //��ʾ�鿴ͼƬ�б�
    void refresh_view_image_list(void* image_lst);

    //ˢ�²鿴����ͼ
    void refresh_view_thumbnail(int index, void* image, void* dark_image);
}

#endif