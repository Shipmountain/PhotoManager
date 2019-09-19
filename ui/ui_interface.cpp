#include "ui_interface.h"
#include "../stdafx.h"
#include "main_frame.h"
#include "msg_def.h"
#include "pic_view.h"

static MainFrame* k_main_win_ptr = NULL;
static CPicView* k_pic_view_win_ptr = NULL;

namespace ui
{

    bool init(void* hInstance)
    {
        CPaintManagerUI::SetInstance((HINSTANCE)hInstance);
        CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath());


#if defined(WIN32) && !defined(UNDER_CE)
        HRESULT Hr = ::CoInitialize(NULL);
#else
        HRESULT Hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
#endif
        if (FAILED(Hr))
        {
            return false;
        }

        ::LoadIcon((HINSTANCE)hInstance, L"IDI_ICON_PROGRAM32");

        k_pic_view_win_ptr = new CPicView();
        if (NULL == k_pic_view_win_ptr)
        {
            return false;
        }

        k_pic_view_win_ptr->Create(NULL, PROGRAM_MAIN_WIN_NAMEW, UI_WNDSTYLE_FRAME, WS_EX_STATICEDGE | WS_EX_APPWINDOW, 0, 0, 900, 700);
        k_pic_view_win_ptr->CenterWindow();
        ::ShowWindow(k_pic_view_win_ptr->GetHWND(), SW_SHOWMAXIMIZED);
        
        k_main_win_ptr = new MainFrame();
        if (NULL == k_main_win_ptr)
        {
            return false;
        }

        k_main_win_ptr->Create(NULL, PROGRAM_MAIN_WIN_NAMEW, UI_WNDSTYLE_FRAME, WS_EX_STATICEDGE | WS_EX_APPWINDOW, 0, 0, 900, 700);
        k_main_win_ptr->CenterWindow();
        ::ShowWindow(k_main_win_ptr->GetHWND(), SW_SHOWNORMAL);

        CPaintManagerUI::MessageLoop();
        delete k_pic_view_win_ptr;
        k_pic_view_win_ptr = NULL;
        delete k_main_win_ptr;
        k_main_win_ptr = NULL;

        CPaintManagerUI::Term();

        return true;
    }

    void* get_main_hwnd()
    {
        if (NULL == k_main_win_ptr)
        {
            return NULL;
        }
        return (void*)k_main_win_ptr->GetHWND();
    }

    void refresh_phone_device()
    {
        if (NULL == k_main_win_ptr)
        {
            return;
        }

        ::PostMessage(k_main_win_ptr->GetHWND(), EMsgRefreshPhoneDevice, 0, 0);
    }

    void refresh_result(int total, int count, bool is_success, bool is_local, int curr_page, const std::wstring& dir)
    {
        if (NULL == k_main_win_ptr)
        {
            return;
        }

        Result* result = new Result(total, count, is_success, is_local, curr_page, true);
        result->dir_ = dir;
        ::PostMessage(k_main_win_ptr->GetHWND(), EMsgRefreshResult, (WPARAM)result, 0);
    }

    void refresh_result(int total, int count, bool is_success, bool is_local, int curr_page, int file_type)
    {
        if (NULL == k_main_win_ptr)
        {
            return;
        }

        Result* result = new Result(total, count, is_success, is_local, curr_page, false);
        result->file_type_ = file_type;
        ::PostMessage(k_main_win_ptr->GetHWND(), EMsgRefreshResult, (WPARAM)result, 0);
    }

    std::wstring get_curr_path()
    {
        std::wstring curr_path;
        if (NULL != k_main_win_ptr)
        {
            curr_path = k_main_win_ptr->get_curr_path();
        }

        return std::move(curr_path);
    }

    void reload_data()
    {
        if (NULL == k_main_win_ptr)
        {
            return;
        }

        ::PostMessage(k_main_win_ptr->GetHWND(), EMsgReloadData, 0, 0);
    }

    void refresh_thumbnail_image(void* file_info_ptr, void* image_ptr)
    {
        if (NULL == k_main_win_ptr)
        {
            return;
        }

        ::PostMessage(k_main_win_ptr->GetHWND(), EMsgRefreshThumbnailImage, (WPARAM)file_info_ptr, (LPARAM)image_ptr);
    }

    void show_picture(bool is_phone_pic, const std::wstring& pic_name, const std::wstring& pic_path, const std::wstring& phone_sn)
    {
        if (NULL == k_pic_view_win_ptr)
        {
            return;
        }

        ShowPicParam* params = new ShowPicParam(is_phone_pic, pic_name, pic_path, phone_sn);
        ::PostMessage(k_pic_view_win_ptr->GetHWND(), EMsgShowPic, (WPARAM)params, 0);
    }

    void add_phone_sync_file_result(int index, int result)
    {
        if (NULL == k_main_win_ptr)
        {
            return;
        }

        ::PostMessage(k_main_win_ptr->GetHWND(), EMsgAddPhoneSyncFileResult, (WPARAM)result, (LPARAM)index);
    }

    void del_phone_sync_file_result(int index, bool result)
    {
        if (NULL == k_main_win_ptr)
        {
            return;
        }
        ::PostMessage(k_main_win_ptr->GetHWND(), EMsgDelPhoneSyncFileResult, (WPARAM)result, (LPARAM)index);
    }

    void location_file(int index)
    {
        if (NULL == k_main_win_ptr)
        {
            return;
        }
        ::PostMessage(k_main_win_ptr->GetHWND(), EMsgLocationFile, (WPARAM)index, 0);
    }

    void refresh_view_big_image(const std::string& pic_name, void* image_data)
    {
        if (NULL != k_pic_view_win_ptr)
        {
            std::string* name = new std::string(pic_name);
            ::PostMessage(k_pic_view_win_ptr->GetHWND(), EMsgRefreshViewBigImage, (WPARAM)name, (LPARAM)image_data);
        }
    }

    void refresh_view_image_list(void* image_lst)
    {
        if (NULL != k_pic_view_win_ptr)
        {
            ::PostMessage(k_pic_view_win_ptr->GetHWND(), EMsgRefreshViewImageList, (WPARAM)image_lst, 0);
        }
    }

    void refresh_view_thumbnail(int index, void* image, void* dark_image)
    {
        if (NULL != k_pic_view_win_ptr)
        {
            ViewThumbnailParam* params = new ViewThumbnailParam(index, image, dark_image);
            ::PostMessage(k_pic_view_win_ptr->GetHWND(), EMsgRefreshViewThumbnail, (WPARAM)params, 0);
        }
    }
}



