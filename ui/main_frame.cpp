
#include "../stdafx.h"
#include <windows.h>
#if !defined(UNDER_CE)
#include <shellapi.h>
#endif
#include "../resource.h"

#include "main_frame.h"
#include "../business/BusinessInterface.h"
#include "msg_def.h"

#include "msg_box.h"
#include "ui_interface.h"
#include "setting.h"
#include <dbt.h>

#include <opencv/highgui.h>

#define THUMBNAIL_HEIGHT 122
#define THUMBNAIL_WIDTH 126

#define MENU_ITEM_DELETE        L"menu_delete"
#define MENU_ITEM_RENAME        L"menu_rename"
#define MENU_ITEM_SYNC          L"menu_sync"
#define MENU_ITEM_CANCEL_SYNC   L"menu_cancel_sync"

#define FILE_LIST_COM           L"lay_file_list_com"
#define LIST_COM_SELECT         L"chk_list_com_slelect"
#define LIST_COM_IMAGE          L"ctrl_list_com_image"
#define LIST_COM_CHANGE         L"edt_list_com_change"
#define LIST_COM_NAME           L"lbl_list_com_name"
#define LIST_COM_TIME           L"lbl_list_com_time"
#define LIST_COM_SIZE           L"lbl_list_com_size"
#define LIST_COM_PATH           L"lbl_list_com_path"

#define THUMBNAIL_COM           L"lay_thumbnail_com"
#define THUMBNAIL_COM_PIC       L"ctrl_file_pic"
#define THUMBNAIL_COM_NAME      L"edt_file_name"

MainFrame::MainFrame() :
ModuleLog("MainFrame")
, lay_title_ptr_(nullptr)
, opt_local_ptr_(nullptr)
, ctrl_local_ptr_(nullptr)
, opt_phone_ptr_(nullptr)
, ctrl_phone_ptr_(nullptr)
, btn_phone_name_ptr_(nullptr)
, btn_sync_ptr_(nullptr)
, btn_setting_ptr_(nullptr)
, minbtn_ptr_(nullptr)
, maxbtn_ptr_(nullptr)
, restorebtn_ptr_(nullptr)
, closebtn_ptr_(nullptr)
, lay_left_ptr_(nullptr)
, lay_left_file_list_ptr_(nullptr)
, ctrl_left_file_list_ptr_(nullptr)
, opt_left_file_list_ptr_(nullptr)
, lay_left_image_ptr_(nullptr)
, ctrl_left_image_ptr_(nullptr)
, opt_left_image_ptr_(nullptr)
, lay_left_vedio_ptr_(nullptr)
, ctrl_left_vedio_ptr_(nullptr)
, opt_left_vedio_ptr_(nullptr)
, lay_left_music_ptr_(nullptr)
, ctrl_left_music_ptr_(nullptr)
, opt_left_music_ptr_(nullptr)
, lay_left_doc_ptr_(nullptr)
, ctrl_left_doc_ptr_(nullptr)
, opt_left_doc_ptr_(nullptr)
, lay_left_other_ptr_(nullptr)
, ctrl_left_other_ptr_(nullptr)
, opt_left_other_ptr_(nullptr)
, lay_right_ptr_(nullptr)
, btn_right_back_ptr_(nullptr)
, btn_right_forward_ptr_(nullptr)
, btn_right_flush_ptr_(nullptr)
, lay_right_dir_path_ptr_(nullptr)
, opt_right_switch_list_ptr_(nullptr)
, opt_right_switch_thumbnail_ptr_(nullptr)
, opt_right_switch_date_ptr_(nullptr)
, lay_right_thumbnail_ptr_(nullptr)
, lay_right_thumbnail_content_ptr_(nullptr)
, lst_right_list_ptr_(nullptr)
, lay_right_date_ptr_(nullptr)
, lay_right_date_content_ptr_(nullptr)
, lbl_right_item_count_ptr_(nullptr)
, lay_right_page_ptr_(nullptr)
, btn_page_forward_ptr_(nullptr)
, opt_page_1_ptr_(nullptr)
, opt_page_2_ptr_(nullptr)
, opt_page_3_ptr_(nullptr)
, opt_page_4_ptr_(nullptr)
, opt_page_5_ptr_(nullptr)
, opt_page_6_ptr_(nullptr)
, btn_page_back_ptr_(nullptr)
, lbl_page_total_ptr_(nullptr)
, gif_loading_ptr_(nullptr)

, curr_pos_(0)
, curr_thumbnail_(nullptr)
, last_move_thumbnail_(nullptr)
, need_total_numbers_(true)
, match_key_len_(0)
{
    memset(file_match_key_, 0, 10);
    ::InitializeCriticalSection(&file_match_key_lock_);    
}

MainFrame::~MainFrame()
{
    DeleteCriticalSection(&file_match_key_lock_);
}

LPCTSTR MainFrame::GetWindowClassName() const
{
	return _T("MainFrame");
}

CDuiString MainFrame::GetSkinFile()
{
	return _T("main_frame.xml");
}

CDuiString MainFrame::GetSkinFolder()
{
	return  _T("skin\\PhotoManager");
}

LRESULT MainFrame::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    ShowWindow(false);

	PostQuitMessage(0);
	bHandled = TRUE;
	return 0;
}
void MainFrame::OnClick(TNotifyUI& msg)
{
    if (msg.pSender == btn_right_back_ptr_)
    {
        ::PostMessage(this->GetHWND(), EMsgPathBack, 0, 0);
    }
    else if (msg.pSender == btn_right_forward_ptr_)
    {
        ::PostMessage(this->GetHWND(), EMsgPathForward, 0, 0);
    }
    else if (msg.pSender == btn_right_flush_ptr_)
    {
        ::PostMessage(this->GetHWND(), EMsgRefresh, 0, 0);
    }
    else if (msg.pSender == btn_page_back_ptr_)
    {
        on_page_back();
    }
    else if (msg.pSender == btn_page_forward_ptr_)
    {
        on_page_forward();
    }
    else if (msg.pSender == btn_sync_ptr_)
    {
    }
    else if (msg.pSender == btn_setting_ptr_)
    {
        ::PostMessage(this->GetHWND(), EMsgShowSettingWin, 0, 0);
    }
    else if (msg.pSender == btn_phone_name_ptr_)
    {
        ::PostMessage(this->GetHWND(), EMsgShowPhoneListMenu, 0, 0);
    }
    else if (msg.pSender == lst_right_list_ptr_)
    {
        lst_right_list_ptr_->SelectItem(-1);
    }
	__super::OnClick(msg);
}

LRESULT MainFrame::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (EMsgReloadData == uMsg)
    {
        on_msg_reload_data();
    }
    else if (EMsgExchangeDataMode == uMsg)
    {
        on_msg_exchange_data_mode();
    }
    else if (EMsgRefreshPhoneDevice == uMsg)
    {
        on_msg_refresh_phone_device();
    }
    else if (EMsgRefreshResult == uMsg)
    {
        on_msg_refresh_result(wParam);
    }
    else if (EMsgRefreshList == uMsg)
    {
        on_msg_refresh_list();
    }
    else if (EMsgRefreshDateList == uMsg)
    {
        on_msg_refresh_date_list();
    }
    else if (EMsgRefreshThumbnail == uMsg)
    {
        on_msg_refresh_thumbnail();
    }
    else if (WM_LBUTTONDOWN == uMsg)
    {
        on_msg_mouse_lbuttondown(wParam, lParam);
    }
    else if (WM_MOUSEHOVER == uMsg)
    {
        on_msg_mouse_hover(wParam, lParam);
    }
    else if (WM_MOUSEMOVE == uMsg)
    {
        on_msg_mouse_move(wParam, lParam);
    }
    else if (WM_LBUTTONDBLCLK == uMsg)
    {
        on_msg_mouse_dbclick(wParam, lParam);
    }
    else if (EMsgOpenFile == uMsg)
    {
        on_msg_open_file(wParam);
    }
    else if (EMsgPathForward == uMsg)
    {
        on_msg_path_forward();
    }
    else if (EMsgPathBack == uMsg)
    {
        on_msg_path_back();
    }
    else if (EMsgRefresh == uMsg)
    {
        on_msg_refresh();
    }
    else if (EMsgGotoPath == uMsg)
    {
        on_msg_goto_path();
    }
    else if (EMsgShowDirFolderMenu == uMsg)
    {
        on_msg_show_dir_folder_menu(wParam, lParam);
    }
    else if (EMsgInitPageItem == uMsg)
    {
        on_msg_init_page_item(wParam, lParam);
    }
    else if (EMsgGotoPage == uMsg)
    {
        on_msg_goto_page(wParam, lParam);
    }
    else if (EMsgShowSettingWin == uMsg)
    {
        on_msg_show_setting_win();
    }    
    else if (WM_DEVICECHANGE == uMsg)
    {
        on_msg_device_change(wParam, lParam);
    }
    else if (EMsgModuleOptionChange == uMsg)
    {
        on_msg_module_option_change();
    }
    else if (EMsgRefreshThumbnailImage == uMsg)
    {
        on_msg_refresh_thumbnail_image(wParam, lParam);
    }
    else if (EMsgShowPhoneListMenu == uMsg)
    {
        on_msg_show_phone_list_menu();
    }
    else if (WM_RBUTTONDOWN == uMsg)
    {
        on_msg_mouse_rbuttondown(wParam, lParam);
    }
    else if (EMsgShowMainRightMenu == uMsg)
    {
        on_msg_show_main_right_menu(wParam, lParam);
    }
    else if (EMsgAddPhoneSyncFile == uMsg)
    {
        on_msg_add_phone_sync_file(wParam);
    }
    else if (EMsgDelPhoneSyncFile == uMsg)
    {
        on_msg_del_phone_sync_file(wParam);
    }
    else if (EMsgAddPhoneSyncFileResult == uMsg)
    {
        on_msg_add_phone_sync_file_result(wParam, lParam);
    }
    else if (EMsgDelPhoneSyncFileResult == uMsg)
    {
        on_msg_del_phone_sync_file_result(wParam, lParam);
    }
    else if (EMsgLocationFile == uMsg)
    {
        on_msg_location_file(wParam);
    }
    else if (EMsgDeleteFile == uMsg)
    {
        on_msg_delete_file(wParam);
    }
    else if (EMsgRenameFile == uMsg)
    {
        on_msg_rename_file(wParam);
    }

	return __super::HandleMessage(uMsg, wParam, lParam);
}

void MainFrame::OnItemSelect(TNotifyUI& msg)
{
    if (_tcsicmp(msg.pSender->GetName().GetData(), FILE_LIST_COM) == 0)
    {
        CCheckBoxUI* chk_ptr = static_cast<CCheckBoxUI*>(m_PaintManager.FindSubControlByName(msg.pSender, LIST_COM_SELECT));
        if (!chk_ptr->IsSelected() && msg.wParam)
        {
            chk_ptr->Selected(true);
        }
        else if (chk_ptr->IsSelected() && !msg.wParam)
        {
            chk_ptr->Selected(false);
        }
    }
//     if (combo_phone_name_ptr_ == msg.pSender)
//     {
//         CControlUI* ctrl_ptr = combo_phone_name_ptr_->GetItemAt(combo_phone_name_ptr_->GetCurSel());
//         combo_phone_name_ptr_->SetUserData(ctrl_ptr->GetUserData());
//     }
}

void MainFrame::InitWindow()
{
    lay_title_ptr_ = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("lay_title")));
    opt_local_ptr_ = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("opt_local")));
    ctrl_local_ptr_ = static_cast<CControlUI*>(m_PaintManager.FindControl(_T("ctrl_local")));
    opt_phone_ptr_ = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("opt_phone")));
    ctrl_phone_ptr_ = static_cast<CControlUI*>(m_PaintManager.FindControl(_T("ctrl_phone")));
    btn_phone_name_ptr_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_phone_name")));
    btn_sync_ptr_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_sync")));
    btn_setting_ptr_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_setting")));
    minbtn_ptr_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("minbtn")));
    maxbtn_ptr_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("maxbtn")));
    restorebtn_ptr_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("restorebtn")));
    closebtn_ptr_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("closebtn")));
    lay_left_ptr_ = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("lay_left")));
    lay_left_file_list_ptr_ = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("lay_left_file_list")));
    ctrl_left_file_list_ptr_ = static_cast<CControlUI*>(m_PaintManager.FindControl(_T("ctrl_left_file_list")));
    opt_left_file_list_ptr_ = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("opt_left_file_list")));
    lay_left_image_ptr_ = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("lay_left_image")));
    ctrl_left_image_ptr_ = static_cast<CControlUI*>(m_PaintManager.FindControl(_T("ctrl_left_image")));
    opt_left_image_ptr_ = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("opt_left_image")));
    lay_left_vedio_ptr_ = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("lay_left_vedio")));
    ctrl_left_vedio_ptr_ = static_cast<CControlUI*>(m_PaintManager.FindControl(_T("ctrl_left_vedio")));
    opt_left_vedio_ptr_ = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("opt_left_vedio")));
    lay_left_music_ptr_ = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("lay_left_music")));
    ctrl_left_music_ptr_ = static_cast<CControlUI*>(m_PaintManager.FindControl(_T("ctrl_left_music")));
    opt_left_music_ptr_ = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("opt_left_music")));
    lay_left_doc_ptr_ = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("lay_left_doc")));
    ctrl_left_doc_ptr_ = static_cast<CControlUI*>(m_PaintManager.FindControl(_T("ctrl_left_doc")));
    opt_left_doc_ptr_ = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("opt_left_doc")));
    lay_left_other_ptr_ = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("lay_left_other")));
    ctrl_left_other_ptr_ = static_cast<CControlUI*>(m_PaintManager.FindControl(_T("ctrl_left_other")));
    opt_left_other_ptr_ = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("opt_left_other")));
    lay_right_ptr_ = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("lay_right")));
    btn_right_back_ptr_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_right_back")));
    btn_right_forward_ptr_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_right_forward")));
    btn_right_flush_ptr_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_right_flush")));
    lay_right_dir_path_ptr_ = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("lay_right_dir_path")));
    opt_right_switch_list_ptr_ = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("opt_right_switch_list")));
    opt_right_switch_thumbnail_ptr_ = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("opt_right_switch_thumbnail")));
    opt_right_switch_date_ptr_ = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("opt_right_switch_date")));    
    lay_right_thumbnail_ptr_ = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("lay_right_thumbnail")));
    lay_right_thumbnail_content_ptr_ = static_cast<CTileLayoutUI*>(m_PaintManager.FindControl(_T("lay_right_thumbnail_content")));
    lst_right_list_ptr_ = static_cast<CListUI*>(m_PaintManager.FindControl(_T("lst_right_list")));
    lay_right_date_ptr_ = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("lay_right_date")));
    lay_right_date_content_ptr_ = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("lay_right_date_content")));
    lbl_right_item_count_ptr_ = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lbl_right_item_count")));
    lay_right_page_ptr_ = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("lay_right_page")));
    btn_page_forward_ptr_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_page_forward")));
    opt_page_1_ptr_ = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("opt_page_1")));
    opt_page_2_ptr_ = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("opt_page_2")));
    opt_page_3_ptr_ = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("opt_page_3")));
    opt_page_4_ptr_ = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("opt_page_4")));
    opt_page_5_ptr_ = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("opt_page_5")));
    opt_page_6_ptr_ = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("opt_page_6")));
    btn_page_back_ptr_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_page_back")));
    lbl_page_total_ptr_ = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lbl_page_total")));
    gif_loading_ptr_ = static_cast<CGifUI*>(m_PaintManager.FindControl(_T("gif_loading")));
}

void MainFrame::OnPrepare(TNotifyUI& msg)
{
    SetIcon(IDI_ICON_PROGRAM16);

    RECT rc = lay_right_thumbnail_content_ptr_->GetPos();
    int max_column_numbers = (rc.right - rc.left) / (THUMBNAIL_WIDTH);
    lay_right_thumbnail_content_ptr_->SetColumns(max_column_numbers);

    opt_phone_ptr_->SetEnabled(false);

    ctrl_local_ptr_->SetTag(ctrl_local_ptr_->GetBkColor());
    ctrl_phone_ptr_->SetTag(ctrl_phone_ptr_->GetBkColor());
    ctrl_left_file_list_ptr_->SetTag(ctrl_left_file_list_ptr_->GetBkColor());
    ctrl_left_image_ptr_->SetTag(ctrl_left_image_ptr_->GetBkColor());
    ctrl_left_vedio_ptr_->SetTag(ctrl_left_vedio_ptr_->GetBkColor());
    ctrl_left_music_ptr_->SetTag(ctrl_left_music_ptr_->GetBkColor());
    ctrl_left_doc_ptr_->SetTag(ctrl_left_doc_ptr_->GetBkColor());
    ctrl_left_other_ptr_->SetTag(ctrl_left_other_ptr_->GetBkColor());

    ctrl_local_ptr_->SetBkColor(lay_title_ptr_->GetBkColor());
    ctrl_phone_ptr_->SetBkColor(lay_title_ptr_->GetBkColor());
    ctrl_left_image_ptr_->SetBkColor(lay_left_ptr_->GetBkColor());
    ctrl_left_vedio_ptr_->SetBkColor(lay_left_ptr_->GetBkColor());
    ctrl_left_music_ptr_->SetBkColor(lay_left_ptr_->GetBkColor());
    ctrl_left_doc_ptr_->SetBkColor(lay_left_ptr_->GetBkColor());
    ctrl_left_other_ptr_->SetBkColor(lay_left_ptr_->GetBkColor());

    opt_page_1_ptr_->SetUserData(L"1");
    opt_page_2_ptr_->SetUserData(L"2");
    opt_page_3_ptr_->SetUserData(L"3");
    opt_page_4_ptr_->SetUserData(L"4");
    opt_page_5_ptr_->SetUserData(L"5");
    opt_page_6_ptr_->SetUserData(L"6");

    lbl_right_item_count_ptr_->SetText(_T(""));
    opt_local_ptr_->Selected(true);
    SetTimer(eTimerGetDevices, 2 * 1000);
}

void MainFrame::Notify(TNotifyUI& msg)
{
	if (_tcsicmp(msg.sType, _T("windowinit")) == 0)
	{
		OnPrepare(msg);
	}
	else if (_tcsicmp(msg.sType, DUI_MSGTYPE_ITEMSELECT) == 0)
	{
		OnItemSelect(msg);
	}
	else if (_tcsicmp(msg.sType, DUI_MSGTYPE_SELECTCHANGED) == 0)
	{
		OnSelectChange(msg);
	}
    else if (_tcsicmp(msg.sType, DUI_MSGTYPE_RBUTTONDOWN) == 0)
    {
        OnRButtonDown(msg);
    }
    else if (_tcsicmp(msg.sType, DUI_MSGTYPE_MENUITEMCLICK) == 0)
    {
        OnMenuItemClick(msg);
    }
    else if (_tcsicmp(msg.sType, DUI_MSGTYPE_ITEMDBCLICK) == 0)
    {
        OnItemDBClick(msg);
    }
//     else if (_tcsicmp(msg.sType, DUI_MSGTYPE_TEXTCHANGED) == 0)
//     {
//         OnTextChange(msg);
//     }

//  	wchar_t tmp[50] = { 0 };
//  	wsprintf(tmp, _T("\r\n***Notify msg:%s"), msg.sType);
//  	OutputDebugString(tmp);

	WindowImplBase::Notify(msg);
}

UILIB_RESOURCETYPE MainFrame::GetResourceType() const
{
#ifdef _DEBUG
	return  UILIB_FILE;
#else
	return UILIB_ZIPRESOURCE;
#endif
}

LPCTSTR MainFrame::GetResourceID() const
{
	return MAKEINTRESOURCE(IDR_ZIPRES);
}


void MainFrame::OnSelectChange(TNotifyUI& msg)
{
    if (opt_local_ptr_ == msg.pSender ||
        opt_phone_ptr_ == msg.pSender)
    {
        ctrl_local_ptr_->SetBkColor(lay_title_ptr_->GetBkColor());
        ctrl_phone_ptr_->SetBkColor(lay_title_ptr_->GetBkColor());
        std::wstring name = msg.pSender->GetName();
        name.replace(0, 3, L"ctrl");
        CControlUI* ctrl_ptr = m_PaintManager.FindSubControlByName(lay_title_ptr_, name.c_str());
        ctrl_ptr->SetBkColor(ctrl_ptr->GetTag());

        ::PostMessage(this->GetHWND(), EMsgExchangeDataMode, 0, 0);
    }
    else if (opt_right_switch_date_ptr_ == msg.pSender)
    {
        lst_right_list_ptr_->SetVisible(false);
        lay_right_thumbnail_ptr_->SetVisible(false);
        lay_right_date_ptr_->SetVisible(true);
        ::PostMessage(this->GetHWND(), EMsgRefreshDateList, 0, 0);
    }
    else if (opt_right_switch_list_ptr_ == msg.pSender)
    {
        lst_right_list_ptr_->SetVisible(true);
        lay_right_thumbnail_ptr_->SetVisible(false);
        lay_right_date_ptr_->SetVisible(false);
        ::PostMessage(this->GetHWND(), EMsgRefreshList, 0, 0);
    }
    else if (opt_right_switch_thumbnail_ptr_ == msg.pSender)
    {
        lst_right_list_ptr_->SetVisible(false);
        lay_right_thumbnail_ptr_->SetVisible(true);
        lay_right_date_ptr_->SetVisible(false);
        ::PostMessage(this->GetHWND(), EMsgRefreshThumbnail, 0, 0);
    }
    else if (opt_left_file_list_ptr_ == msg.pSender ||
        opt_left_image_ptr_ == msg.pSender ||
        opt_left_vedio_ptr_ == msg.pSender ||
        opt_left_music_ptr_ == msg.pSender ||
        opt_left_doc_ptr_ == msg.pSender ||
        opt_left_other_ptr_ == msg.pSender)
    {
        ctrl_left_file_list_ptr_->SetBkColor(lay_left_ptr_->GetBkColor());
        ctrl_left_image_ptr_->SetBkColor(lay_left_ptr_->GetBkColor());
        ctrl_left_vedio_ptr_->SetBkColor(lay_left_ptr_->GetBkColor());
        ctrl_left_music_ptr_->SetBkColor(lay_left_ptr_->GetBkColor());
        ctrl_left_doc_ptr_->SetBkColor(lay_left_ptr_->GetBkColor());
        ctrl_left_other_ptr_->SetBkColor(lay_left_ptr_->GetBkColor());

        std::wstring name = msg.pSender->GetName();
        name.replace(0, 3, L"lay");

        CControlUI* lay_ptr = m_PaintManager.FindSubControlByName(lay_left_ptr_, name.c_str());
        name.replace(0, 3, L"ctrl");
        CControlUI* ctrl_ptr = m_PaintManager.FindSubControlByName(lay_ptr, name.c_str());
        ctrl_ptr->SetBkColor(ctrl_ptr->GetTag());

        ::PostMessage(this->GetHWND(), EMsgModuleOptionChange, 0, 0);
        ::PostMessage(this->GetHWND(), EMsgReloadData, 0, 0);
    }
    else if (opt_page_1_ptr_ == msg.pSender ||
        opt_page_2_ptr_ == msg.pSender ||
        opt_page_3_ptr_ == msg.pSender ||
        opt_page_4_ptr_ == msg.pSender || 
        opt_page_5_ptr_ == msg.pSender || 
        opt_page_6_ptr_ == msg.pSender)
    {
        int num = _wtoi(msg.pSender->GetUserData().GetData());
        ::PostMessage(this->GetHWND(), EMsgGotoPage, msg.pSender->GetTag(), num);
    }
}

LRESULT MainFrame::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (eTimerGetDevices == wParam)
    {
        KillTimer(eTimerGetDevices);
        theBusiness::instance().do_get_device_lst();
    }
    else if (eTimerFileSearch == wParam)
    {
        EnterCriticalSection(&file_match_key_lock_);
        int index = 0;
        if (lay_right_thumbnail_ptr_->IsVisible())
        {
            if (NULL != curr_thumbnail_)
            {
                index = curr_thumbnail_->GetTag();
                ++index;
            }
        }
        else if (lst_right_list_ptr_->IsVisible())
        {
            int sel = lst_right_list_ptr_->GetCurSel();
            if (-1 != sel)
            {
                index = lst_right_list_ptr_->GetItemAt(sel)->GetTag();
                ++index;
            }
        }

        theBusiness::instance().do_match_file(index, file_match_key_);
        memset(file_match_key_, 0, 10);
        match_key_len_ = 0;
        KillTimer(eTimerFileSearch);
        LeaveCriticalSection(&file_match_key_lock_);
    }
    
    bHandled = TRUE;
	return 0;
}

LRESULT MainFrame::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LRESULT ret = __super::OnSize(uMsg, wParam, lParam, bHandled);
//     if (NULL != lay_function_ptr_ && lay_function_ptr_->IsVisible())
//     {
//         ::PostMessage(this->GetHWND(), EMsgResetFunctionMenuPos, 0, 0);
//     }
    return ret;
}

LRESULT MainFrame::ResponseDefaultKeyEvent(WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    if (VK_RETURN == wParam)
    {
    }
    return 0;
}


void MainFrame::OnRButtonDown(TNotifyUI& msg)
{
    if (msg.pSender == lst_right_list_ptr_)
    {
        if (lst_right_list_ptr_->GetCount() > 0)
        {
            int index = lst_right_list_ptr_->GetCurSel();
            if (-1 != index)
            {
                int tag = lst_right_list_ptr_->GetItemAt(index)->GetTag();
                POINT* pt_ptr = new POINT;
                *pt_ptr = msg.ptMouse;
                ::PostMessage(this->GetHWND(), EMsgShowMainRightMenu, tag, (LPARAM)pt_ptr);
            }
        }
    }
}

void MainFrame::add_path_menu_item(CMenuUI*  pMenu, void* params)
{
    std::vector<std::wstring> folder_lst;
    std::pair<MainFrame*, int>* info = (std::pair<MainFrame*, int>*)params;
    int index = info->second;
    split_wstring(info->first->path_folder_lst_[index], L'*', folder_lst);
    delete info;
    if (folder_lst.empty())
    {
        return;
    }

    std::wstring image_path = L"file='pic/FileType/Small/FolderType.png' dest='4,3,22,21'";
    CMenuElementUI* item_ptr = NULL;
    for (auto itor = folder_lst.begin(); itor != folder_lst.end(); ++itor)
    {
        item_ptr = new CMenuElementUI();
        pMenu->Add(item_ptr); 
        item_ptr->SetText(itor->c_str());
        item_ptr->SetTag(index);
        item_ptr->SetFixedHeight(24);
        item_ptr->SetFixedWidth(150);
        item_ptr->SetName(L"dir_folder_menu");
        item_ptr->SetToolTip(item_ptr->GetText().GetData());
        item_ptr->SetBkImage(image_path.c_str());
    }
    pMenu->Invalidate();
}

void MainFrame::path_menu_item_click(CControlUI* ctrl_ptr, void* params)
{
    MainFrame* frame = (MainFrame*)params;
    int pos = ctrl_ptr->GetTag();
    CButtonUI* btn_ptr = (CButtonUI*)frame->lay_right_dir_path_ptr_->GetItemAt(pos * 2 + 1);
    btn_ptr->SetUserData(L"0");
    frame->set_path_folder_id_image(btn_ptr, 0);
    
    btn_ptr = (CButtonUI*)frame->lay_right_dir_path_ptr_->GetItemAt(pos * 2 + 2);
    if (btn_ptr->GetText() == ctrl_ptr->GetText())
    {
        return;
    }
    
    frame->curr_pos_ = pos + 1;
    frame->history_dir_lst_[frame->curr_pos_].curr_page_ = 1;
    frame->open_dir(ctrl_ptr->GetText().GetData());
}

void MainFrame::OnMenuItemClick(TNotifyUI& msg)
{
}

void MainFrame::OnItemDBClick(TNotifyUI& msg)
{
    CControlUI* parent_ptr = msg.pSender->GetParent();
    if (NULL != parent_ptr)
    {
        parent_ptr = parent_ptr->GetParent();
        if (NULL != parent_ptr && parent_ptr == lst_right_list_ptr_)
        {
            CControlUI* ctrl = lst_right_list_ptr_->GetItemAt(lst_right_list_ptr_->GetCurSel());
            if (NULL != ctrl)
            {
                ::PostMessage(this->GetHWND(), EMsgOpenFile, ctrl->GetTag(), 0);
            }
        }
    }
}

void MainFrame::on_msg_reload_data()
{
    show_loading(true);
    bool is_local = opt_local_ptr_->IsSelected();
    bool is_dir = opt_left_file_list_ptr_->IsSelected();
    std::wstring phone_sn;
    if (!is_local)
    {
        phone_sn = btn_phone_name_ptr_->GetUserData().GetData();
    }

    int curr_page = history_dir_lst_[curr_pos_].curr_page_;

    if (is_dir)
    {
        theBusiness::instance().do_get_dir_file(curr_path_, curr_page, phone_sn, is_local, need_total_numbers_);
    }
    else
    {
        int file_type = 0;
        if (opt_left_image_ptr_->IsSelected())
        {
            file_type = eFilePic;
        }
        else if (opt_left_vedio_ptr_->IsSelected())
        {
            file_type = eFileVideo;
        }
        else if (opt_left_music_ptr_->IsSelected())
        {
            file_type = eFileMusic;
        }
        else if (opt_left_doc_ptr_->IsSelected())
        {
            file_type = eFileDoc;
        }
        else if (opt_left_other_ptr_->IsSelected())
        {
            file_type = eFileOther;
        }

        theBusiness::instance().do_get_all_file(file_type, curr_page, phone_sn, is_local, need_total_numbers_);
    }    
}

void MainFrame::on_msg_exchange_data_mode()
{
    if (opt_left_file_list_ptr_->IsSelected())
    {
        ::PostMessage(this->GetHWND(), EMsgModuleOptionChange, 0, 0);
        ::PostMessage(this->GetHWND(), EMsgReloadData, 0, 0);
    }
    else
    {
        opt_left_file_list_ptr_->Selected(true);
    }
}

void MainFrame::on_msg_refresh_phone_device()
{
    auto phone_lst = theDataFactory::instance().get_phone_list();
    if (phone_lst.empty())
    {
        opt_local_ptr_->Selected(true);
        btn_phone_name_ptr_->SetVisible(false);
        opt_phone_ptr_->SetEnabled(false);
    }
    else
    {
        btn_phone_name_ptr_->SetVisible(true);
        if (btn_phone_name_ptr_->GetText().IsEmpty())
        {
            auto itor = phone_lst.begin();
            btn_phone_name_ptr_->SetText(itor->product_.c_str());
            btn_phone_name_ptr_->SetUserData(itor->serial_no_.c_str());
            theBusiness::instance().set_curr_phone(itor->serial_no_);
        }
        else
        {
            std::wstring serial_no = btn_phone_name_ptr_->GetUserData().GetData();
            auto itor = phone_lst.begin();
            for (; itor != phone_lst.end(); ++itor)
            {
                if (serial_no == itor->serial_no_)
                {
                    break;
                }
            }

            if (itor == phone_lst.end())
            {
                itor = phone_lst.begin();
                btn_phone_name_ptr_->SetText(itor->product_.c_str());
                btn_phone_name_ptr_->SetUserData(itor->serial_no_.c_str());
                theBusiness::instance().set_curr_phone(itor->serial_no_);
            }
        }
        
        opt_phone_ptr_->SetEnabled(true);
    }
}

void MainFrame::on_msg_refresh_result(WPARAM wParam)
{
    show_loading(false);

    Result* result = (Result*)wParam;
    if (!result->is_success_)
    {
        remove_thumbnail_lst_image();
        thumbnail_map_.clear();
        curr_thumbnail_ = NULL;
        last_move_thumbnail_ = NULL;        
        lay_right_thumbnail_content_ptr_->RemoveAll();
        lst_right_list_ptr_->RemoveAll();
        lay_right_date_content_ptr_->RemoveAll();
        ::PostMessage(this->GetHWND(), EMsgInitPageItem, result->total_, 0);
        delete result;
        return;
    }

    //显示分页
    ::PostMessage(this->GetHWND(), EMsgInitPageItem, result->total_, result->count_);
    delete result;
    
    //添加路径上的文件夹
    add_path_folder();

    if (opt_right_switch_list_ptr_->IsSelected())
    {
        ::PostMessage(this->GetHWND(), EMsgRefreshList, 0, 0);
    }
    else if (opt_right_switch_thumbnail_ptr_->IsSelected())
    {
        ::PostMessage(this->GetHWND(), EMsgRefreshThumbnail, 0, 0);
    }
    else
    {
        ::PostMessage(this->GetHWND(), EMsgRefreshDateList, 0, 0);
    }
}

std::wstring MainFrame::get_curr_path()
{
    return curr_path_;
}

void MainFrame::on_msg_refresh_list()
{
    CDataFactory& factory = theDataFactory::instance();
    lock::Lock lock(&factory.file_lst_lock_);
    lst_right_list_ptr_->RemoveAll();
    if (NULL == factory.file_lst_)
    {
        return;
    }

    wchar_t tmp[20] = { 0 };
    double kb = 0.0;
    int i = 0;
    for (auto itor = factory.file_lst_->begin(); itor != factory.file_lst_->end(); ++itor, ++i)
    {
        if (!itor->is_del_)
        {
            CDialogBuilder dlg_build;
            CListContainerElementUI* lay_ptr = static_cast<CListContainerElementUI*>(dlg_build.Create(_T("list_com.xml"), (UINT)0, NULL, &m_PaintManager));
            lay_ptr->SetTag(i);
            if (itor->is_sync_)
            {
                lay_ptr->SetUserData(L"1");
            }
            lst_right_list_ptr_->Add(lay_ptr);

            CCheckBoxUI* chk_ptr = static_cast<CCheckBoxUI*>(m_PaintManager.FindSubControlByName(lay_ptr, LIST_COM_SELECT));
            chk_ptr->SetTag(i);

            CControlUI* ctrl_ptr = static_cast<CControlUI*>(m_PaintManager.FindSubControlByName(lay_ptr, LIST_COM_IMAGE));
            ctrl_ptr->SetBkImage(get_file_pic_path(*itor, eImageSmall).c_str());

            CLabelUI* lbl_ptr = static_cast<CLabelUI*>(m_PaintManager.FindSubControlByName(lay_ptr, LIST_COM_NAME));
            lbl_ptr->SetText(itor->file_name_.c_str());

            CEditUI* edt_ptr = static_cast<CEditUI*>(m_PaintManager.FindSubControlByName(lay_ptr, LIST_COM_CHANGE));
            edt_ptr->SetTag(i);
            edt_ptr->SetText(itor->file_name_.c_str());
            edt_ptr->OnNotify += MakeDelegate(this, &MainFrame::on_list_file_name_edt);

            lbl_ptr = static_cast<CLabelUI*>(m_PaintManager.FindSubControlByName(lay_ptr, LIST_COM_TIME));
            lbl_ptr->SetText(itor->modify_time_.c_str());

            lbl_ptr = static_cast<CLabelUI*>(m_PaintManager.FindSubControlByName(lay_ptr, LIST_COM_SIZE));
            kb = itor->file_size_;
            kb = kb / 1024;
            swprintf_s(tmp, L"%0.3f", kb);
            lbl_ptr->SetText(tmp);

            lbl_ptr = static_cast<CLabelUI*>(m_PaintManager.FindSubControlByName(lay_ptr, LIST_COM_PATH));
            lbl_ptr->SetText(itor->file_path_.c_str());
        }        
    }
    SIZE sz = { 0 };
    lst_right_list_ptr_->SetScrollPos(sz);
}

void MainFrame::on_msg_refresh_date_list()
{
    CDataFactory& factory = theDataFactory::instance();
    lock::Lock lock(&factory.file_lst_lock_);
    lay_right_date_content_ptr_->RemoveAll();
    SIZE sz = { 0 };
    lay_right_date_content_ptr_->SetScrollPos(sz);
}

void MainFrame::on_msg_refresh_thumbnail()
{
    CDataFactory& factory = theDataFactory::instance();
    lock::Lock lock(&factory.file_lst_lock_);
    curr_thumbnail_ = NULL;
    last_move_thumbnail_ = NULL;
    remove_thumbnail_lst_image();
    thumbnail_map_.clear();
    lay_right_thumbnail_content_ptr_->RemoveAll();

    if (NULL == factory.file_lst_)
    {
        return;
    }

    std::vector<PhoneFileInfo*> image_lst;
    int i = 0;
    double kb = 0.0;
    wchar_t tmp[MAX_PATH] = { 0 };
    std::wstring image_id;
    for (auto itor = factory.file_lst_->begin(); itor != factory.file_lst_->end(); ++itor, ++i)
    {
        if (!itor->is_del_)
        {
            CDialogBuilder dlg_build;
            CVerticalLayoutUI* lay_ptr = static_cast<CVerticalLayoutUI*>(dlg_build.Create(_T("thumbnail.xml"), (UINT)0, NULL, &m_PaintManager));
            lay_ptr->SetTag(i);
            if (itor->is_sync_)
            {
                lay_ptr->SetUserData(L"1");
            }
            lay_right_thumbnail_content_ptr_->Add(lay_ptr);

            CControlUI* ctrl_ptr = static_cast<CControlUI*>(m_PaintManager.FindSubControlByName(lay_ptr, THUMBNAIL_COM_PIC));
            ctrl_ptr->SetTag(itor->file_type_);

            image_id = get_thumbnail_image_id(get_file_pic_path(*itor, eImageBig), itor->thumbnail_width_, itor->thumbnail_height_);
            ctrl_ptr->SetBkImage(image_id.c_str());
            if (eFileDir == itor->file_type_)
            {
                swprintf_s(tmp, L"名称:%s\r\n修改时间:%s", itor->file_name_.c_str(), itor->modify_time_.c_str());
            }
            else
            {
                kb = itor->file_size_;
                kb = kb / 1024;
                swprintf_s(tmp, L"名称:%s\r\n大小:%0.3fKB\r\n修改时间:%s", itor->file_name_.c_str(), kb, itor->modify_time_.c_str());
            }
            ctrl_ptr->SetToolTip(tmp);

            CEditUI* edt_ptr = static_cast<CEditUI*>(m_PaintManager.FindSubControlByName(lay_ptr, THUMBNAIL_COM_NAME));
            edt_ptr->SetText(itor->file_name_.c_str());
            edt_ptr->OnNotify += MakeDelegate(this, &MainFrame::on_thumbnail_file_name_edt);
            edt_ptr->SetTag(i);

            thumbnail_map_.insert(std::make_pair(lay_ptr, ctrl_ptr));
            if (eFilePic == itor->file_type_ && itor->is_phone_)
            {
                PhoneFileInfo* image_info = new PhoneFileInfo(itor->file_name_, itor->file_path_, itor->serial_no_, lay_ptr);
                image_lst.push_back(image_info);
            }
        }        
    }
    SIZE sz = { 0 };
    lay_right_thumbnail_content_ptr_->SetScrollPos(sz);

    for (auto itor = image_lst.begin(); itor != image_lst.end(); ++itor, ++i)
    {
        theBusiness::instance().do_get_phone_file_image(*itor);
    }
}

bool MainFrame::on_thumbnail_file_name_edt(void* param)
{
    TNotifyUI* pMsg = (TNotifyUI*)param;
    if (NULL == pMsg)
    {
        return false;
    }
    
    if (_tcsicmp(pMsg->sType, DUI_MSGTYPE_KILLFOCUS) == 0)
    {
        pMsg->pSender->SetBorderSize(0);
        CControlUI* lay_ptr = pMsg->pSender->GetParent();
        pMsg->pSender->SetBkColor(lay_ptr->GetBkColor());

        std::wstring new_name = pMsg->pSender->GetText();
        if (!change_file_name_check(new_name))
        {
            pMsg->pSender->SetText(file_name_.c_str());
            return true;
        }

        std::wstring path = rename_file_in_file_list(pMsg->pSender->GetTag(), new_name);
        if (!path.empty())
        {
            theBusiness::instance().do_rename_file(file_name_, new_name, path, opt_phone_ptr_->IsSelected());
        }
    }
    else if (_tcsicmp(pMsg->sType, DUI_MSGTYPE_SETFOCUS) == 0)
    {
        pMsg->pSender->SetBorderSize(1);
        thumbnail_selected(pMsg->pSender->GetParent(), true);
        pMsg->pSender->SetBkColor(0xFFFFFFFF);
        file_name_ = pMsg->pSender->GetText().GetData();
    }
    else if (_tcsicmp(pMsg->sType, DUI_MSGTYPE_RETURN) == 0)
    {
        m_PaintManager.SetFocus(NULL);
    }

    return true;
}

std::wstring MainFrame::get_file_pic_path(const FileInfo& file_info, eImageMode image_mode)
{
    std::wstring path;
    if (eImageBig == image_mode)
    {
        path = L"pic/FileType/Big/";
    }
    else if (eImageMiddle == image_mode)
    {
        path = L"pic/FileType/Middle/";
    }
    else
    {
        path = L"pic/FileType/Small/";
    }

    switch (file_info.file_type_)
    {
        case eFileApk:
        {
            path.append(L"ApkType");
            break;
        }
        case eFileApps:
        {
            path.append(L"Apps");
            break;
        }
        case eFileCAD:
        {
            path.append(L"CADType");
            break;
        }
        case eFileDoc:
        {
            path.append(L"DocType");
            break;
        }
        case eFileEpub:
        {
            path.append(L"EpubType");
            break;
        }
        case eFileExe:
        {
            path.append(L"ExeType");
            break;
        }
        case eFileDir:
        {
            path.append(L"FolderType");
            break;
        }
        case eFileGeneral:
        {
            path.append(L"GeneralType");
            break;
        }
        case eFilePic:
        {
            if (eImageSmall != image_mode && !file_info.thumbnail_path_.empty())
            {
                path = file_info.thumbnail_path_;
                return std::move(path);
            }
            else
            {
                path.append(L"ImgType");
            }
            
            break;
        }
        case eFileIpa:
        {
            path.append(L"IpaType");
            break;
        }
        case eFileKey:
        {
            path.append(L"KeyType");
            break;
        }
        case eFileMusic:
        {
            path.append(L"MusicType");
            break;
        }
        case eFileNumbers:
        {
            path.append(L"NumbersType");
            break;
        }
        case eFileOther:
        {
            path.append(L"OtherType");
            break;
        }
        case eFilePages:
        {
            path.append(L"PagesType");
            break;
        }
        case eFilePdf:
        {
            path.append(L"PdfType");
            break;
        }
        case eFilePpt:
        {
            path.append(L"PptType");
            break;
        }
        case eFileRar:
        {
            path.append(L"RarType");
            break;
        }
        case eFileTorrent:
        {
            path.append(L"TorrentType");
            break;
        }
        case eFileTxt:
        {
            path.append(L"TxtType");
            break;
        }
        case eFileVideo:
        {
            path.append(L"VideoType");
            break;
        }
        case eFileVsd:
        {
            path.append(L"VsdType");
            break;
        }
        case eFileXls:
        {
            path.append(L"XlsType");
            break;
        }
        default:
        {
            path.append(L"OtherType");
        }
    }

    if (file_info.is_sync_)
    {
        path.append(L"_Sync.png");
    }
    else
    {
        path.append(L".png");
    }

    return std::move(path);
}

void MainFrame::thumbnail_selected(CControlUI* ctrl_ptr, bool selected)
{
    CEditUI* edt_ptr = static_cast<CEditUI*>(m_PaintManager.FindSubControlByName(ctrl_ptr, L"edt_file_name"));
    if (selected)
    {
        ctrl_ptr->SetBorderSize(1);
        ctrl_ptr->SetBkColor(0xFFDAF5FF);
        edt_ptr->SetBkColor(0xFFDAF5FF);
    }
    else
    {
        ctrl_ptr->SetBorderSize(0);
        ctrl_ptr->SetBkColor(0xFFFFFFFF);
        edt_ptr->SetBkColor(0xFFFFFFFF);
    }
}

void MainFrame::thumbnail_mouse_in(CControlUI* ctrl_ptr, bool mouse_in)
{
    CEditUI* edt_ptr = static_cast<CEditUI*>(m_PaintManager.FindSubControlByName(ctrl_ptr, L"edt_file_name"));
    if (mouse_in)
    {
        ctrl_ptr->SetBkColor(0xFFF2FAFF);
        edt_ptr->SetBkColor(0xFFF2FAFF);
    }
    else
    {
        ctrl_ptr->SetBkColor(0xFFFFFFFF);
        edt_ptr->SetBkColor(0xFFFFFFFF);
    }
}

CControlUI* MainFrame::get_thumbnail_by_mouse(POINT& pt)
{
    for (auto itor = thumbnail_map_.begin(); itor != thumbnail_map_.end(); ++itor)
    {
        const RECT& rc = itor->first->GetPos();
        if (::PtInRect(&rc, pt))
        {
            return itor->first;
        }        
    }
    return NULL;
}

void MainFrame::on_msg_mouse_lbuttondown(WPARAM& wParam, LPARAM& lParam)
{
    if (lay_right_thumbnail_ptr_->IsVisible())
    {
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        const RECT& rc = lay_right_thumbnail_content_ptr_->GetPos();
        if (::PtInRect(&rc, pt))
        {
            CControlUI* thumbnail = get_thumbnail_by_mouse(pt);
            if (NULL != thumbnail && curr_thumbnail_ == thumbnail)
            {
                return;
            }

            if (NULL != curr_thumbnail_)
            {
                thumbnail_selected(curr_thumbnail_, false);
            }
            curr_thumbnail_ = thumbnail;
            if (NULL != curr_thumbnail_)
            {
                thumbnail_selected(curr_thumbnail_, true);
            }
        }
    }
}

void MainFrame::on_msg_mouse_hover(WPARAM& wParam, LPARAM& lParam)
{
    if (lay_right_thumbnail_ptr_->IsVisible())
    {
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        const RECT& rc = lay_right_thumbnail_content_ptr_->GetPos();
        if (::PtInRect(&rc, pt))
        {
            CControlUI* thumbnail = get_thumbnail_by_mouse(pt);
            if (NULL != thumbnail && curr_thumbnail_ != thumbnail)
            {
                thumbnail_mouse_in(thumbnail, true);
            }
        }
    }
}

void MainFrame::on_msg_mouse_move(WPARAM& wParam, LPARAM& lParam)
{
    if (lay_right_thumbnail_ptr_->IsVisible())
    {
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        const RECT& rc = lay_right_thumbnail_content_ptr_->GetPos();
        if (::PtInRect(&rc, pt))
        {
            CControlUI* thumbnail = get_thumbnail_by_mouse(pt);
            if (thumbnail != last_move_thumbnail_)
            {
                if (NULL != last_move_thumbnail_ && curr_thumbnail_ != last_move_thumbnail_)
                {
                    thumbnail_mouse_in(last_move_thumbnail_, false);
                }

                last_move_thumbnail_ = thumbnail;
                if (NULL != last_move_thumbnail_  && curr_thumbnail_ != last_move_thumbnail_)
                {
                    thumbnail_mouse_in(last_move_thumbnail_, true);
                }
            }            
        }
    }
}

void MainFrame::on_msg_mouse_dbclick(WPARAM& wParam, LPARAM& lParam)
{
    if (lay_right_thumbnail_ptr_->IsVisible())
    {
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        const RECT& rc = lay_right_thumbnail_content_ptr_->GetPos();
        if (::PtInRect(&rc, pt))
        {
            CControlUI* thumbnail = get_thumbnail_by_mouse(pt);
            if (NULL != thumbnail)
            {
                ::PostMessage(this->GetHWND(), EMsgOpenFile, thumbnail->GetTag(), 0);
            }
        }
    }
}

void MainFrame::on_msg_open_file(WPARAM wParam)
{
    CDataFactory& factory = theDataFactory::instance();
    lock::Lock lock(&factory.file_lst_lock_);
    if (wParam >= factory.file_lst_->size())
    {
        return;
    }

    FileInfo& file_info = (*factory.file_lst_)[wParam];
    if (eFilePic == file_info.file_type_)
    {
        //新窗口打开图片
        open_pic(file_info);
    }
    else if (eFileVideo == file_info.file_type_)
    {
        //使用系统程序打开视频
    }
    else if (eFileMusic == file_info.file_type_)
    {
        //使用系统程序打开音频
    }
    else if (eFileDoc == file_info.file_type_)
    {
        //使用系统程序打开文档
    }
    else if (eFileOther == file_info.file_type_)
    {
        //使用系统程序打开其他类型文件
    }
    else if (eFileDir == file_info.file_type_)
    {
        //打开目录
        open_dir(file_info.file_name_);
    }
}

void MainFrame::open_dir(const std::wstring& file_name)
{
    ++curr_pos_;
    if (history_dir_lst_.size() > curr_pos_)
    {
        history_dir_lst_.resize(curr_pos_);
    }
    HistoryDirInfo dir_info(file_name);
    history_dir_lst_.push_back(dir_info);
    
    need_total_numbers_ = true;
    ::PostMessage(this->GetHWND(), EMsgGotoPath, 0, 0);
}

void MainFrame::open_pic(const FileInfo& file_info)
{
    ui::show_picture(file_info.is_phone_, file_info.file_name_, file_info.file_path_, btn_phone_name_ptr_->GetUserData().GetData());
}

void MainFrame::on_msg_path_forward()
{
    if ((curr_pos_+1) == history_dir_lst_.size())
    {
        return;
    }

    ++curr_pos_;
    need_total_numbers_ = false;
    HistoryDirInfo& dir_info = history_dir_lst_[curr_pos_];
    reset_page_item(dir_info.total_page_, dir_info.curr_page_, dir_info.page_pos_);
    ::PostMessage(this->GetHWND(), EMsgGotoPath, 0, 0);
}

void MainFrame::on_msg_path_back()
{
    if (0 == curr_pos_)
    {
        return;
    }
    --curr_pos_;
    need_total_numbers_ = false;
    HistoryDirInfo& dir_info = history_dir_lst_[curr_pos_];
    reset_page_item(dir_info.total_page_, dir_info.curr_page_, dir_info.page_pos_);
    ::PostMessage(this->GetHWND(), EMsgGotoPath, 0, 0);
}

void MainFrame::reset_curr_path_by_dir_lst()
{
    curr_path_.clear();
    for (int i = 0; i <= curr_pos_; ++i)
    {
        if (!history_dir_lst_[i].dir_name_.empty())
        {
            if (!curr_path_.empty())
            {
                curr_path_.append(L"/");
            }
            curr_path_.append(history_dir_lst_[i].dir_name_);
        }
    }

    int count = lay_right_dir_path_ptr_->GetCount() / 2;
    if (count < curr_pos_)
    {
        count = lay_right_dir_path_ptr_->GetCount();
        if (count > 0)
        {
            lay_right_dir_path_ptr_->GetItemAt(count - 1)->SetEnabled(true);
        }
        CButtonUI* path_folder_ptr = new CButtonUI();
        CButtonUI* path_folder_id_ptr = new CButtonUI();
        lay_right_dir_path_ptr_->Add(path_folder_ptr);
        lay_right_dir_path_ptr_->Add(path_folder_id_ptr);
        path_folder_ptr->SetText(history_dir_lst_[curr_pos_].dir_name_.c_str());
        path_folder_ptr->SetHotTextColor(0xFF06A8FF);
        path_folder_ptr->SetPushedTextColor(0xFF06A8FF);
        path_folder_ptr->SetTextColor(0xFF333333);
        path_folder_ptr->SetFixedHeight(16);
        path_folder_ptr->SetAttribute(L"autocalcwidth", L"true");
        path_folder_ptr->SetTag(curr_pos_);
        path_folder_ptr->OnNotify += MakeDelegate(this, &MainFrame::on_path_folder_click);
        path_folder_id_ptr->SetFixedWidth(30);
        path_folder_id_ptr->SetHotImage(L"file='pic/drop_right.png' source='16,0,32,16' dest='7,0,23,16'");
        path_folder_id_ptr->SetNormalImage(L"file='pic/drop_right.png' source='0,0,16,16' dest='7,0,23,16'");
        path_folder_id_ptr->SetPushedImage(L"file='pic/drop_right.png' source='16,0,32,16' dest='7,0,23,16'");
        path_folder_id_ptr->SetEnabled(false);
        path_folder_id_ptr->SetTag(path_folder_lst_.size());
        path_folder_id_ptr->SetUserData(L"0");
        path_folder_id_ptr->SetName(L"btn_path_folder_id");
        path_folder_id_ptr->OnNotify += MakeDelegate(this, &MainFrame::on_path_folder_id_click);
        path_folder_lst_.push_back(L"");
    }
    else if (count > curr_pos_)
    {
        count -= curr_pos_;
        for (int i = 0; i < count; ++i)
        {
            lay_right_dir_path_ptr_->RemoveAt(lay_right_dir_path_ptr_->GetCount() - 1);
            lay_right_dir_path_ptr_->RemoveAt(lay_right_dir_path_ptr_->GetCount() - 1);
        }        
        
        if (path_folder_lst_.size() >= count)
        {
            path_folder_lst_.resize(path_folder_lst_.size() - count);
        }
        count = lay_right_dir_path_ptr_->GetCount();
        if (count > 0)
        {
            lay_right_dir_path_ptr_->GetItemAt(count - 1)->SetEnabled(false);
        }
    }
    else//菜单选择最后一级时才出现此种情况
    {
        CControlUI* ctrl_ptr = lay_right_dir_path_ptr_->GetItemAt(lay_right_dir_path_ptr_->GetCount() - 2);
        std::wstring dir_name = history_dir_lst_[curr_pos_].dir_name_;
        if (0 != _tcsicmp(ctrl_ptr->GetText().GetData(), dir_name.c_str()))
        {
            ctrl_ptr->SetText(dir_name.c_str());
        }
    }
}

void MainFrame::visible_forward_back_btn()
{
    btn_right_back_ptr_->SetEnabled(true);
    btn_right_forward_ptr_->SetEnabled(true);
    if (0 == curr_pos_)
    {
        btn_right_back_ptr_->SetEnabled(false);
    }
    else if (history_dir_lst_.size() == (curr_pos_+1))
    {
        btn_right_forward_ptr_->SetEnabled(false);
    }
}

void MainFrame::add_path_folder()
{
    if (0 == path_folder_lst_.size())
    {
        return;
    }

    std::wstring& folder = path_folder_lst_[path_folder_lst_.size() - 1];
    folder.clear();
    CDataFactory& factory = theDataFactory::instance();
    lock::Lock lock(&factory.file_lst_lock_);
    for (auto itor = factory.file_lst_->begin(); itor != factory.file_lst_->end(); ++itor)
    {
        if (eFileDir == itor->file_type_)
        {
            if (!folder.empty())
            {
                folder.append(L"*");
            }
            folder.append(itor->file_name_);
        }
    }
}

bool MainFrame::on_path_folder_click(void* param)
{
    TNotifyUI* pMsg = (TNotifyUI*)param;
    if (NULL == pMsg)
    {
        return false;
    }

    if (_tcsicmp(pMsg->sType, DUI_MSGTYPE_CLICK) == 0)
    {
        need_total_numbers_ = true;
        curr_pos_ = pMsg->pSender->GetTag();
        history_dir_lst_[curr_pos_].curr_page_ = 1;
        ::PostMessage(this->GetHWND(), EMsgGotoPath, 0, 0);
    }
    return true;
}

bool MainFrame::on_path_folder_id_click(void* param)
{
    TNotifyUI* pMsg = (TNotifyUI*)param;
    if (NULL == pMsg)
    {
        return false;
    }

    if (_tcsicmp(pMsg->sType, DUI_MSGTYPE_CLICK) == 0)
    {
        int is_down = _wtoi(pMsg->pSender->GetUserData().GetData());
        if (!is_down)
        {
            pMsg->pSender->SetUserData(L"1");
            int tag = pMsg->pSender->GetTag();
            if (!path_folder_lst_[tag].empty())
            {
                ::PostMessage(this->GetHWND(), EMsgShowDirFolderMenu, (WPARAM)pMsg->pSender, tag);
            }            
        }
        else
        {
            pMsg->pSender->SetUserData(L"0");
        }
        set_path_folder_id_image((CButtonUI*)pMsg->pSender, !is_down);
    }
    else if (_tcsicmp(pMsg->sType, DUI_MSGTYPE_KILLFOCUS) == 0)
    {
        pMsg->pSender->SetUserData(L"0");
        set_path_folder_id_image((CButtonUI*)pMsg->pSender, 0);
    }
    return true;
}

void MainFrame::on_msg_goto_path()
{
    reset_curr_path_by_dir_lst();
    visible_forward_back_btn();
    ::PostMessage(this->GetHWND(), EMsgReloadData, 0, 0);
}

void MainFrame::set_path_folder_id_image(CButtonUI* btn_ptr, int is_drop)
{
    if (!is_drop)
    {
        btn_ptr->SetHotImage(L"file='pic/drop_right.png' source='16,0,32,16' dest='7,0,23,16'");
        btn_ptr->SetNormalImage(L"file='pic/drop_right.png' source='0,0,16,16' dest='7,0,23,16'");
        btn_ptr->SetPushedImage(L"file='pic/drop_right.png' source='16,0,32,16' dest='7,0,23,16'");
    }
    else
    {
        btn_ptr->SetHotImage(L"file='pic/drop_down.png' source='16,0,32,16' dest='7,0,23,16'");
        btn_ptr->SetNormalImage(L"file='pic/drop_down.png' source='0,0,16,16' dest='7,0,23,16'");
        btn_ptr->SetPushedImage(L"file='pic/drop_down.png' source='16,0,32,16' dest='7,0,23,16'");
    }    
}

void MainFrame::on_msg_refresh()
{
    show_loading(true);
    if (opt_local_ptr_->IsSelected())
    {
        theBusiness::instance().do_refresh_local_dir(curr_path_);
    }
    else
    {
        std::wstring phone_sn = btn_phone_name_ptr_->GetUserData().GetData();
        int curr_page = history_dir_lst_[curr_pos_].curr_page_;
        theBusiness::instance().do_get_dir_file(curr_path_, curr_page, phone_sn, false, need_total_numbers_);
    }
}

void MainFrame::on_msg_show_dir_folder_menu(WPARAM wParam, LPARAM lParam)
{
    CControlUI* ctrl_ptr = (CControlUI*)wParam;
    CMenuWnd* pMenu = new CMenuWnd(this->GetHWND());
    CPoint point;
    const RECT& rc = ctrl_ptr->GetPos();
    point.x = rc.left + 7;
    point.y = rc.bottom;
    ClientToScreen(m_hWnd, &point);
    STRINGorID xml(_T("folder_menu.xml"));
    std::pair<MainFrame*, int>* params = new std::pair<MainFrame*, int>(this, lParam);
    pMenu->SetModifyMenuFunc(add_path_menu_item, params);
    pMenu->SetMenuClickFunc(path_menu_item_click, this);
    pMenu->Init(NULL, NULL, point, xml);
}

void MainFrame::on_msg_init_page_item(WPARAM wParam, LPARAM lParam)
{
    int total = wParam;
    int curr_numbers = lParam;

    wchar_t tmp[16] = { 0 };
    swprintf_s(tmp, L"%d项", curr_numbers);
    lbl_right_item_count_ptr_->SetText(tmp);
    //只对总条数做处理
    if (-1 != total)
    {
        int total_pages = total / theDataFactory::instance().max_num_;
        if ((total % theDataFactory::instance().max_num_) != 0)
        {
            ++total_pages;
        }

        if (0 == total_pages)
        {
            ++total_pages;
        }
        history_dir_lst_[curr_pos_].total_page_ = total_pages;

        lay_right_page_ptr_->SetVisible(total_pages > 1);
        if (total_pages > 1)
        {
            CControlUI* ctrl = NULL;
            for (int i = 0; i < total_pages && i < 6; ++i)
            {
                swprintf_s(tmp, L"opt_page_%d", i + 1);
                ctrl = static_cast<COptionUI*>(m_PaintManager.FindSubControlByName(lay_right_page_ptr_, tmp));
                swprintf_s(tmp, L"%d", i + 1);
                ctrl->SetText(tmp);
                ctrl->SetVisible(true);
                ctrl->SetTag(i + 1);
            }

            for (int i = total_pages; i < 6; ++i)
            {
                swprintf_s(tmp, L"opt_page_%d", i + 1);
                ctrl = static_cast<COptionUI*>(m_PaintManager.FindSubControlByName(lay_right_page_ptr_, tmp));
                ctrl->SetVisible(false);
            }

            btn_page_forward_ptr_->SetVisible(true);
            btn_page_back_ptr_->SetVisible(true);
            btn_page_back_ptr_->SetEnabled(false);
            
            swprintf_s(tmp, L"共%d页", total_pages);
            lbl_page_total_ptr_->SetText(tmp);
            COptionUI* opt_ptr = static_cast<COptionUI*>(m_PaintManager.FindSubControlByName(lay_right_page_ptr_, L"opt_page_1"));
            opt_ptr->Selected(true);
        }
    }
}

void MainFrame::on_msg_goto_page(WPARAM wParam, LPARAM lParam)
{
    if (history_dir_lst_[curr_pos_].curr_page_ == wParam)
    {
        return;
    }

    btn_page_back_ptr_->SetEnabled(true);
    btn_page_forward_ptr_->SetEnabled(true);
    if (1 == wParam)
    {
        btn_page_back_ptr_->SetEnabled(false);
    }
    else if (history_dir_lst_[curr_pos_].total_page_ == wParam)
    {
        btn_page_forward_ptr_->SetEnabled(false);
    }

    need_total_numbers_ = false;
    history_dir_lst_[curr_pos_].curr_page_ = wParam;
    history_dir_lst_[curr_pos_].page_pos_ = lParam;
    ::PostMessage(this->GetHWND(), EMsgReloadData, 0, 0);
}

void MainFrame::on_page_back()
{
    wchar_t tmp[11] = { 0 };
    int page = history_dir_lst_[curr_pos_].curr_page_ - 1;

    COptionUI* opt_ptr = NULL;
    int i = 1;
    for (; i <= 6; ++i)
    {
        swprintf_s(tmp, L"opt_page_%d", i);
        opt_ptr = static_cast<COptionUI*>(m_PaintManager.FindSubControlByName(lay_right_page_ptr_, tmp));
        if (opt_ptr->GetTag() == page)
        {
            opt_ptr->Selected(true);
            return;
        }
    }

    i = 1;
    for (; i <= 6; ++i, ++page)
    {
        swprintf_s(tmp, L"opt_page_%d", i);
        opt_ptr = static_cast<COptionUI*>(m_PaintManager.FindSubControlByName(lay_right_page_ptr_, tmp));
        opt_ptr->SetTag(page);
        swprintf_s(tmp, L"%d", page);
        opt_ptr->SetText(tmp);
    }
    opt_ptr = static_cast<COptionUI*>(m_PaintManager.FindSubControlByName(lay_right_page_ptr_, L"opt_page_1"));
    ::PostMessage(this->GetHWND(), EMsgGotoPage, opt_ptr->GetTag(), 1);
}

void MainFrame::on_page_forward()
{
    wchar_t tmp[11] = { 0 };
    int page = history_dir_lst_[curr_pos_].curr_page_ + 1;

    COptionUI* opt_ptr = NULL;
    int i = 1;
    for (; i <= 6; ++i)
    {
        swprintf_s(tmp, L"opt_page_%d", i);
        opt_ptr = static_cast<COptionUI*>(m_PaintManager.FindSubControlByName(lay_right_page_ptr_, tmp));
        if (opt_ptr->GetTag() == page)
        {
            opt_ptr->Selected(true);
            return;
        }
    }

    i = 6;
    for (; i > 0; --i, --page)
    {
        swprintf_s(tmp, L"opt_page_%d", i);
        opt_ptr = static_cast<COptionUI*>(m_PaintManager.FindSubControlByName(lay_right_page_ptr_, tmp));
        opt_ptr->SetTag(page);
        swprintf_s(tmp, L"%d", page);
        opt_ptr->SetText(tmp);
    }
    
    opt_ptr = static_cast<COptionUI*>(m_PaintManager.FindSubControlByName(lay_right_page_ptr_, L"opt_page_6"));
    ::PostMessage(this->GetHWND(), EMsgGotoPage, opt_ptr->GetTag(), 6);
}

void MainFrame::on_msg_show_setting_win()
{
    CSetting setting;
    setting.Create(this->GetHWND(), _T("设置"), UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE);
    setting.CenterWindow();
    setting.ShowModal();
}

void MainFrame::on_msg_device_change(WPARAM wParam, LPARAM lParam)
{
    PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
    if (DBT_DEVICEARRIVAL == wParam || DBT_DEVICEREMOVECOMPLETE == wParam)
    {
        if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)
        {
            PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
            if (0 == lpdbv->dbcv_flags)//移动设备
            {
                SetTimer(eTimerGetDevices, 800);
            }
        }
    }
}

void MainFrame::remove_thumbnail_lst_image()
{
    CControlUI* ctrl_ptr = NULL;
    for (auto itor = thumbnail_map_.begin(); itor != thumbnail_map_.end(); ++itor)
    {
        ctrl_ptr = itor->second;
        if (eFilePic == ctrl_ptr->GetTag())
        {
            m_PaintManager.RemoveImage(ctrl_ptr->GetBkImage());
        }
    }
}

void MainFrame::show_loading(bool visible)
{
    gif_loading_ptr_->SetVisible(visible);
    if (visible)
    {
        gif_loading_ptr_->SetVisible(true);
        RECT rc;
        ::GetClientRect(this->GetHWND(), &rc);

        rc.left += (rc.right - rc.left - gif_loading_ptr_->GetFixedWidth()) / 2;
        rc.right = rc.left + gif_loading_ptr_->GetFixedWidth();
        rc.top += (rc.bottom - rc.top - gif_loading_ptr_->GetFixedHeight()) / 2;
        rc.bottom = rc.top + gif_loading_ptr_->GetFixedHeight();
        gif_loading_ptr_->SetPos(rc);
    }
}

void MainFrame::on_msg_module_option_change()
{
    remove_thumbnail_lst_image();
    lay_right_thumbnail_content_ptr_->RemoveAll();
    lst_right_list_ptr_->RemoveAll();
    lay_right_date_content_ptr_->RemoveAll();
    thumbnail_map_.clear();
    lay_right_dir_path_ptr_->RemoveAll();

    history_dir_lst_.clear();
    curr_pos_ = 0;
    history_dir_lst_.push_back(HistoryDirInfo(L""));

    curr_path_.clear();
    curr_thumbnail_ = NULL;
    last_move_thumbnail_ = NULL;
    path_folder_lst_.clear();
    btn_right_back_ptr_->SetEnabled(false);
    btn_right_forward_ptr_->SetEnabled(false);
    need_total_numbers_ = true;
}

void MainFrame::reset_page_item(int total_page, int curr_page, int page_pos)
{
    lay_right_page_ptr_->SetVisible((total_page > 1 ? true : false));
    if (total_page > 1)
    {
        int tmp_curr = 0;
        wchar_t tmp[16] = { 10 };
        COptionUI* ctrl = NULL;
        for (int i = 1; i <= 6; ++i)
        {
            swprintf_s(tmp, L"opt_page_%d", i);
            ctrl = static_cast<COptionUI*>(m_PaintManager.FindSubControlByName(lay_right_page_ptr_, tmp));

            if (i > page_pos)
            {
                tmp_curr = curr_page + i - page_pos;
                ctrl->SetVisible((tmp_curr > total_page) ? false : true);
            }
            else
            {
                tmp_curr = curr_page - (page_pos - i);
                ctrl->SetVisible(true);
            }
            swprintf_s(tmp, L"%d", tmp_curr);
            ctrl->SetText(tmp);
            ctrl->SetTag(tmp_curr);

            ctrl->Selected(i == page_pos);
        }

        btn_page_forward_ptr_->SetEnabled(true);
        btn_page_back_ptr_->SetEnabled(true);
        if (1 == page_pos)
        {
            btn_page_back_ptr_->SetEnabled(false);
        }
        else if (curr_page == total_page)
        {
            btn_page_forward_ptr_->SetEnabled(false);
        }

        swprintf_s(tmp, L"共%d页", total_page);
        lbl_page_total_ptr_->SetText(tmp);
    }
}

void MainFrame::on_msg_refresh_thumbnail_image(WPARAM wParam, LPARAM lParam)
{
    IplImage* image_ptr = (IplImage*)lParam;
    PhoneFileInfo* file_info = (PhoneFileInfo*)wParam;
    auto itor = thumbnail_map_.find((CControlUI*)file_info->ctrl_ptr_);
    if (thumbnail_map_.end() != itor)
    {
        std::wstring image_id = get_thumbnail_image_id(file_info->file_path_ + L"/" + file_info->file_name_, image_ptr->width, image_ptr->height);
        itor->second->SetBGRBkImage(image_id.c_str(), (unsigned char*)image_ptr->imageData, image_ptr->imageSize, image_ptr->width, image_ptr->height, false);
    }
    cvReleaseImage(&image_ptr);
    delete file_info;
}

std::wstring MainFrame::get_thumbnail_image_id(const std::wstring& image_name, int width, int height)
{
    if (0 == width)
    {
        width = 60; //文件类型图片宽度
    }

    if (0 == height)
    {
        height = 60; //文件类型图片高度
    }

    int left = (THUMBNAIL_IMAGE_WIDTH - width) / 2;
    int right = left + width;
    int top = (THUMBNAIL_IMAGE_HEIGHT - height) / 2;
    int bottown = top + height;

    std::wstring tmp = L"file='";
    tmp.append(image_name);
    tmp.append(L"' dest='");
    wchar_t dest[20] = { 0 };
    swprintf_s(dest, L"%d,%d,%d,%d'", left, top, right, bottown);
    tmp.append(dest);

    return tmp;
}

void MainFrame::on_msg_show_phone_list_menu()
{
    CMenuWnd* pMenu = new CMenuWnd(this->GetHWND());
    CPoint point;
    const RECT& rc = btn_phone_name_ptr_->GetPos();
    point.x = rc.left;
    point.y = rc.bottom;
    ClientToScreen(m_hWnd, &point);
    STRINGorID xml(_T("phone_menu.xml"));
    pMenu->SetModifyMenuFunc(add_phone_menu_item, NULL);
    pMenu->SetMenuClickFunc(phone_menu_item_click, this);
    pMenu->Init(NULL, NULL, point, xml);
}

void MainFrame::add_phone_menu_item(CMenuUI*  pMenu, void* params)
{
    auto phone_lst = theDataFactory::instance().get_phone_list();
    std::wstring image_path = L"file='pic/phone_menu.png' dest='4,3,15,21'";
    CMenuElementUI* item_ptr = NULL;
    for (auto itor = phone_lst.begin(); itor != phone_lst.end(); ++itor)
    {
        item_ptr = new CMenuElementUI();
        pMenu->Add(item_ptr);
        item_ptr->SetText(itor->product_.c_str());
        item_ptr->SetUserData(itor->serial_no_.c_str());
        item_ptr->SetFixedHeight(24);
        item_ptr->SetFixedWidth(150);
        item_ptr->SetToolTip(item_ptr->GetText().GetData());
        item_ptr->SetBkImage(image_path.c_str());
    }
    pMenu->Invalidate();
}

void MainFrame::phone_menu_item_click(CControlUI* ctrl_ptr, void* params)
{
    MainFrame* main_frame = (MainFrame*)params;
    if (ctrl_ptr->GetUserData() == main_frame->btn_phone_name_ptr_->GetUserData())
    {
        return;
    }

    main_frame->btn_phone_name_ptr_->SetText(ctrl_ptr->GetText().GetData());
    main_frame->btn_phone_name_ptr_->SetUserData(ctrl_ptr->GetUserData().GetData());
    theBusiness::instance().set_curr_phone(ctrl_ptr->GetUserData().GetData());
    if (main_frame->opt_phone_ptr_->IsSelected())
    {
        if (main_frame->opt_left_file_list_ptr_->IsSelected())
        {
            ::PostMessage(main_frame->GetHWND(), EMsgModuleOptionChange, 0, 0);
            ::PostMessage(main_frame->GetHWND(), EMsgReloadData, 0, 0);
        }
        else
        {
            main_frame->opt_left_file_list_ptr_->Selected(true);
        }
    }
}

void MainFrame::on_msg_mouse_rbuttondown(WPARAM wParam, LPARAM lParam)
{
    if (lay_right_thumbnail_ptr_->IsVisible())
    {
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        const RECT& rc = lay_right_thumbnail_content_ptr_->GetPos();
        if (::PtInRect(&rc, pt))
        {
            CControlUI* thumbnail = get_thumbnail_by_mouse(pt);
            if (NULL ==  thumbnail)
            {
                return;
            }

            if (thumbnail != curr_thumbnail_ && NULL != curr_thumbnail_)
            {
                thumbnail_selected(curr_thumbnail_, false);
            }

            curr_thumbnail_ = thumbnail;
            thumbnail_selected(curr_thumbnail_, true);

            int tag = thumbnail->GetTag();
            POINT* pt_ptr = new POINT;
            *pt_ptr = pt;
            ::PostMessage(this->GetHWND(), EMsgShowMainRightMenu, tag, (LPARAM)pt_ptr);
        }
    }
}

void MainFrame::on_msg_show_main_right_menu(WPARAM wParam, LPARAM lParam)
{
    POINT* pt = (POINT*)lParam;
    CMenuWnd* pMenu = new CMenuWnd(this->GetHWND());
    CPoint point;
    point.x = pt->x;
    point.y = pt->y;
    ClientToScreen(m_hWnd, &point);
    STRINGorID xml(_T("main_menu.xml"));
    std::pair<MainFrame*, int>* modify_param = new std::pair<MainFrame*, int>(this, wParam);
    pMenu->SetModifyMenuFunc(main_menu_modify_fun, modify_param);
    pMenu->SetMenuClickFunc(main_menu_item_click, this);
    pMenu->Init(NULL, NULL, point, xml);
    delete pt;
}

void MainFrame::main_menu_modify_fun(CMenuUI*  pMenu, void* params)
{
    std::pair<MainFrame*, int>* info = (std::pair<MainFrame*, int>*)params;
    CControlUI* delete_ptr = pMenu->GetItemAt(0);//删除
    CControlUI* rename_ptr = pMenu->GetItemAt(1);//重命名
    delete_ptr->SetTag(info->second);
    rename_ptr->SetTag(info->second);

    if (info->first->opt_right_switch_list_ptr_->IsSelected())
    {
        if (info->first->lst_right_list_ptr_->GetSelectedItemCount() != 1)
        {
            rename_ptr->SetEnabled(false);
        }        
    }
    
    CControlUI* sync_ptr = pMenu->GetItemAt(2);//同步
    CControlUI* cancel_sync_ptr = pMenu->GetItemAt(3);//取消同步
    sync_ptr->SetVisible(false);
    cancel_sync_ptr->SetVisible(false);
    sync_ptr->SetTag(info->second);
    cancel_sync_ptr->SetTag(info->second);

    if (info->first->opt_phone_ptr_->IsSelected())
    {
        CDataFactory& factory = theDataFactory::instance();
        lock::Lock lock(&factory.file_lst_lock_);
        if (factory.file_lst_->size() > info->second)
        {
            FileInfo& file_info = (*factory.file_lst_)[info->second];
            if (eFileDir == file_info.file_type_)
            {
                if (file_info.is_sync_)
                {
                    sync_ptr->SetVisible(false);
                    cancel_sync_ptr->SetVisible(true);
                }
                else
                {
                    sync_ptr->SetVisible(true);
                    cancel_sync_ptr->SetVisible(false);
                }
            }            
        }    
    }
    
    delete info;
}

void MainFrame::main_menu_item_click(CControlUI* ctrl_ptr, void* params)
{
    MainFrame* frame = (MainFrame*)params;
    int tag = ctrl_ptr->GetTag();

    if (_tcsicmp(ctrl_ptr->GetName(), MENU_ITEM_DELETE) == 0)
    {
        ::PostMessage(frame->GetHWND(), EMsgDeleteFile, (WPARAM)tag, 0);
    }
    else if (_tcsicmp(ctrl_ptr->GetName(), MENU_ITEM_RENAME) == 0)
    {
        ::PostMessage(frame->GetHWND(), EMsgRenameFile, (WPARAM)tag, 0);
    }
    else if (_tcsicmp(ctrl_ptr->GetName(), MENU_ITEM_SYNC) == 0)
    {
        ::PostMessage(frame->GetHWND(), EMsgAddPhoneSyncFile, (WPARAM)tag, 0);
    }
    else if (_tcsicmp(ctrl_ptr->GetName(), MENU_ITEM_CANCEL_SYNC) == 0)
    {
        ::PostMessage(frame->GetHWND(), EMsgDelPhoneSyncFile, (WPARAM)tag, 0);
    }
}

void MainFrame::on_msg_add_phone_sync_file(WPARAM wParam)
{
    CDataFactory& factory = theDataFactory::instance();
    lock::Lock lock(&factory.file_lst_lock_);
    if (factory.file_lst_->size() <= wParam)
    {
        return;
    }
    FileInfo& file_info = (*factory.file_lst_)[wParam];
    std::wstring path = file_info.file_path_;
    path.append(L"/");
    path.append(file_info.file_name_);
    theBusiness::instance().do_add_phone_sync_file(wParam, file_info.serial_no_, path, eFileDir==file_info.file_type_);
}

void MainFrame::on_msg_del_phone_sync_file(WPARAM wParam)
{
    CDataFactory& factory = theDataFactory::instance();
    lock::Lock lock(&factory.file_lst_lock_);
    if (factory.file_lst_->size() <= wParam)
    {
        return;
    }
    FileInfo& file_info = (*factory.file_lst_)[wParam];
    std::wstring path = file_info.file_path_;
    path.append(file_info.file_name_);
    theBusiness::instance().do_del_phone_sync_file(wParam, file_info.serial_no_, path);
}

void MainFrame::on_msg_add_phone_sync_file_result(WPARAM wParam, LPARAM lParam)
{
    if (2 == wParam)
    {
        CMsgBox::msg_box(this->GetHWND(), L"此目录已在同步目录中", L"", CMsgBox::EHints, MB_OK);
        return;
    }

    if (wParam)
    {
        CDataFactory& factory = theDataFactory::instance();
        lock::Lock lock(&factory.file_lst_lock_);
        (*factory.file_lst_)[lParam].is_sync_ = true;
        change_ctrl_sync_state(lParam, (*factory.file_lst_)[lParam]);
    }
    else
    {
        CMsgBox::msg_box(this->GetHWND(), L"添加同步目录失败", L"", CMsgBox::EError, MB_OK);
    }
}

void MainFrame::on_msg_del_phone_sync_file_result(WPARAM wParam, LPARAM lParam)
{
    if (wParam)
    {
        CDataFactory& factory = theDataFactory::instance();
        lock::Lock lock(&factory.file_lst_lock_);
        (*factory.file_lst_)[lParam].is_sync_ = false;
        change_ctrl_sync_state(lParam, (*factory.file_lst_)[lParam]);
    }
    else
    {
        CMsgBox::msg_box(this->GetHWND(), L"删除同步目录失败", L"", CMsgBox::EError, MB_OK);
    }
}

void MainFrame::change_ctrl_sync_state(int index, const FileInfo& file_info)
{
    if (lay_right_thumbnail_ptr_->IsVisible())
    {
        std::wstring bk_image = get_thumbnail_image_id(get_file_pic_path(file_info, eImageBig), file_info.thumbnail_width_, file_info.thumbnail_height_);
        CControlUI* lay_ptr = lay_right_thumbnail_content_ptr_->GetItemAt(index);
        CControlUI* ctrl_ptr = static_cast<CControlUI*>(m_PaintManager.FindSubControlByName(lay_ptr, THUMBNAIL_COM_PIC));
        ctrl_ptr->SetBkImage(bk_image.c_str());
    }
    else if (lst_right_list_ptr_->IsVisible())
    {
        std::wstring bk_image = get_file_pic_path(file_info, eImageSmall);
        CControlUI* lay_ptr = lst_right_list_ptr_->GetItemAt(index);
        CControlUI* ctrl_ptr = static_cast<CControlUI*>(m_PaintManager.FindSubControlByName(lay_ptr, LIST_COM_IMAGE));
        ctrl_ptr->SetBkImage(bk_image.c_str());
    }
}

LRESULT MainFrame::OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    EnterCriticalSection(&file_match_key_lock_);
    if (match_key_len_ >= 9)
    {
        wmemset(file_match_key_, 0, 10);
        match_key_len_ = 0;
    }
    file_match_key_[match_key_len_] = (wchar_t)wParam;
    ++match_key_len_;
    KillTimer(eTimerFileSearch);
    SetTimer(eTimerFileSearch, 400);
    LeaveCriticalSection(&file_match_key_lock_);
    return 0;
}

void MainFrame::on_msg_location_file(WPARAM wParam)
{
    int index = wParam;
    CDataFactory& factory = theDataFactory::instance();
    lock::Lock lock(&factory.file_lst_lock_);
    if (index < 0 || index >= factory.file_lst_->size())
    {
        return;
    }

    if (lay_right_thumbnail_ptr_->IsVisible())
    {
        CControlUI* ctrl_ptr = lay_right_thumbnail_content_ptr_->GetItemAt(index);

        const RECT& rc = ctrl_ptr->GetPos();
        int height = lay_right_thumbnail_content_ptr_->GetHeight();
        SIZE sz = lay_right_thumbnail_content_ptr_->GetScrollPos();
        if (rc.top < sz.cy || rc.bottom > (sz.cy+height))
        {
            sz.cx = rc.left;
            sz.cy = rc.top - THUMBNAIL_HEIGHT;
            lay_right_thumbnail_content_ptr_->SetScrollPos(sz);
        }
        
        if (NULL != curr_thumbnail_)
        {
            thumbnail_selected(curr_thumbnail_, false);
        }
        curr_thumbnail_ = ctrl_ptr;
        thumbnail_selected(ctrl_ptr, true);
    }
    else if (lst_right_list_ptr_->IsVisible())
    {
        lst_right_list_ptr_->SelectItem(index, true);
    }
}

void MainFrame::on_msg_delete_file(WPARAM wParam)
{
    int index = wParam;
    CDataFactory& factory = theDataFactory::instance();
    lock::Lock lock(&factory.file_lst_lock_);
    if (index < 0 || index >= factory.file_lst_->size())
    {
        return;
    }

    FileInfo& file_info = (*factory.file_lst_)[index];
    file_info.is_del_ = true;

    CControlUI* ctrl_ptr = NULL;
    if (opt_right_switch_list_ptr_->IsSelected())
    {
        for (int i = 0; i < lst_right_list_ptr_->GetCount(); ++i)
        {
            ctrl_ptr = lst_right_list_ptr_->GetItemAt(i);
            if (index == ctrl_ptr->GetTag())
            {
                lst_right_list_ptr_->Remove(ctrl_ptr);
                break;
            }
        }
    }
    else if (opt_right_switch_thumbnail_ptr_->IsSelected())
    {
        for (int i = 0; i < lay_right_thumbnail_content_ptr_->GetCount(); ++i)
        {
            ctrl_ptr = lay_right_thumbnail_content_ptr_->GetItemAt(i);
            if (index == ctrl_ptr->GetTag())
            {
                auto itor = thumbnail_map_.find(ctrl_ptr);
                if (thumbnail_map_.end() != itor)
                {
                    CControlUI* image_ptr = itor->second;
                    if (eFilePic == image_ptr->GetTag())
                    {
                        m_PaintManager.RemoveImage(image_ptr->GetBkImage());
                    }
                    thumbnail_map_.erase(itor);
                }
                lay_right_thumbnail_content_ptr_->Remove(ctrl_ptr);

                curr_thumbnail_ = NULL;
                last_move_thumbnail_ = NULL;
                break;
            }
        }
    }

    bool is_phone = opt_phone_ptr_->IsSelected();
    std::wstring phone_sn;
    if (is_phone)
    {
        phone_sn = btn_phone_name_ptr_->GetUserData().GetData();
    }

    std::wstring file = file_info.file_path_ + L"/" + file_info.file_name_;
    theBusiness::instance().do_delete_file(file, is_phone, phone_sn);
}

void MainFrame::on_msg_rename_file(WPARAM wParam)
{
    int index = wParam;
    CControlUI* ctrl_ptr = NULL;
    if (opt_right_switch_list_ptr_->IsSelected())
    {
        for (int i = 0; i < lst_right_list_ptr_->GetCount(); ++i)
        {
            ctrl_ptr = lst_right_list_ptr_->GetItemAt(i);
            if (index == ctrl_ptr->GetTag())
            {
                CLabelUI* lbl_ptr = static_cast<CLabelUI*>(m_PaintManager.FindSubControlByName(ctrl_ptr, LIST_COM_NAME));
                CEditUI* edt_ptr = static_cast<CEditUI*>(m_PaintManager.FindSubControlByName(ctrl_ptr, LIST_COM_CHANGE));
                lbl_ptr->SetVisible(false);
                edt_ptr->SetVisible(true);
                edt_ptr->SetFocus();
                edt_ptr->SetSelAll();
                break;
            }
        }
    }
    else if (opt_right_switch_thumbnail_ptr_->IsSelected())
    {
        for (int i = 0; i < lay_right_thumbnail_content_ptr_->GetCount(); ++i)
        {
            ctrl_ptr = lay_right_thumbnail_content_ptr_->GetItemAt(i);
            if (index == ctrl_ptr->GetTag())
            {
                CEditUI* edt_ptr = static_cast<CEditUI*>(m_PaintManager.FindSubControlByName(ctrl_ptr, THUMBNAIL_COM_NAME));
                edt_ptr->SetFocus();
                edt_ptr->SetSelAll();
                break;
            }
        }
    }
}

bool MainFrame::on_list_file_name_edt(void* param)
{
    TNotifyUI* pMsg = (TNotifyUI*)param;
    if (NULL == pMsg)
    {
        return false;
    }

    if (_tcsicmp(pMsg->sType, DUI_MSGTYPE_KILLFOCUS) == 0 )
    {
        if (!pMsg->pSender->IsVisible())
        {
            return true;
        }
        pMsg->pSender->SetBorderSize(0);
        CControlUI* lay_ptr = pMsg->pSender->GetParent();
        CLabelUI* lbl_ptr = static_cast<CLabelUI*>(m_PaintManager.FindSubControlByName(lay_ptr, LIST_COM_NAME));
        lbl_ptr->SetVisible(true);
        pMsg->pSender->SetVisible(false);

        std::wstring new_name = pMsg->pSender->GetText();
        if (!change_file_name_check(new_name))
        {
            pMsg->pSender->SetText(file_name_.c_str());
            return true;
        }

        lbl_ptr->SetText(new_name.c_str());

        std::wstring path = rename_file_in_file_list(pMsg->pSender->GetTag(), new_name);
        if (!path.empty())
        {
            theBusiness::instance().do_rename_file(file_name_, new_name, path, opt_phone_ptr_->IsSelected());
        }        
    }
    else if (_tcsicmp(pMsg->sType, DUI_MSGTYPE_SETFOCUS) == 0)
    {
        pMsg->pSender->SetBorderSize(1);
        file_name_ = pMsg->pSender->GetText().GetData();
    }
    else if (_tcsicmp(pMsg->sType, DUI_MSGTYPE_RETURN) == 0)
    {
        m_PaintManager.SetFocus(NULL);
    }

    return true;
}

bool MainFrame::change_file_name_check(const std::wstring& new_name)
{
    if (new_name.empty() || _tcsicmp(new_name.c_str(), file_name_.c_str()) == 0)
    {
        return false;
    }

    CDataFactory& factory = theDataFactory::instance();
    lock::Lock lock(&factory.file_lst_lock_);
    for (auto itor = factory.file_lst_->begin(); itor != factory.file_lst_->end(); ++itor)
    {
        if (itor->is_del_)
        {
            continue;
        }

        if (0 == _tcsicmp(new_name.c_str(), itor->file_name_.c_str()))
        {
            CMsgBox::msg_box(this->GetHWND(), L"文件名已经存在", L"", CMsgBox::EHints, MB_OK, false, 1);
            return false;
        }
    }
    return true;
}

std::wstring MainFrame::rename_file_in_file_list(int index, const std::wstring& new_name)
{
    std::wstring path;
    CDataFactory& factory = theDataFactory::instance();
    lock::Lock lock(&factory.file_lst_lock_);
    if (index >= 0 && index < factory.file_lst_->size())
    {
        (*factory.file_lst_)[index].file_name_ = new_name;
        path = (*factory.file_lst_)[index].file_path_;
    }
    return std::move(path);
}



