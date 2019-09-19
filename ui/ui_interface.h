#ifndef UI_INTERFACE_H
#define UI_INTERFACE_H
#include <string>

namespace ui
{
    //初始化
    bool init(void* hInstance);

    //获取主窗口句柄
    void* get_main_hwnd();

    //刷新手机列表
    void refresh_phone_device();

    //刷新结果
    void refresh_result(int total, int count, bool is_success, bool is_local, int curr_page, const std::wstring& dir);
    void refresh_result(int total, int count, bool is_success, bool is_local, int curr_page, int file_type);

    //获取当前路径
    std::wstring get_curr_path();

    //重新加载数据
    void reload_data();

    //刷新刷新缩略图图片
    void refresh_thumbnail_image(void* file_info_ptr, void* image_ptr);

    //显示图片
    void show_picture(bool is_phone_pic, const std::wstring& pic_name, const std::wstring& pic_path, const std::wstring& phone_sn);

    //添加手机同步文件结果
    void add_phone_sync_file_result(int index, int result);

    //删除手机同步文件结果
    void del_phone_sync_file_result(int index, bool result);

    //定位文件
    void location_file(int index);

    //刷新查看大图
    void refresh_view_big_image(const std::string& pic_name, void* image_data);

    //显示查看图片列表
    void refresh_view_image_list(void* image_lst);

    //刷新查看缩略图
    void refresh_view_thumbnail(int index, void* image, void* dark_image);
}

#endif