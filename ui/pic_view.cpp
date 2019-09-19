
#include "../stdafx.h"
#include <windows.h>
#if !defined(UNDER_CE)
#include <shellapi.h>
#endif
#include "../resource.h"

#include "pic_view.h"
#include <ShlObj.h>
#include "msg_def.h"
#include <opencv/highgui.h>
#include "../business/BusinessInterface.h"
#include "msg_box.h"
#include "../data/DataFactory.h"
#include <character_cvt.hpp>

#define THUMBNAIL_MAX_WIDTH 76
#define THUMBNAIL_MAX_HEIGHT 58

#define LEFT_RIGHT_LAY_WIDTH 114
#define TOP_LAY_HEIGHT 60
#define BOTTOM_LAY_HEIGHT 56
#define THUMBNAIL_LAY_HEIGHT 66

CPicView::CPicView() :ModuleLog("PicView")
, lay_main_ptr_(nullptr)
, lay_top_ptr_(nullptr)
, lbl_image_name_ptr_(nullptr)
, minbtn_ptr_(nullptr)
, maxbtn_ptr_(nullptr)
, restorebtn_ptr_(nullptr)
, closebtn_ptr_(nullptr)
, ctrl_image_ptr_(nullptr)
, lay_thumbnail_ptr_(nullptr)
, btn_last_page_ptr_(nullptr)
, lay_thumbnail_list_ptr_(nullptr)
, btn_next_page_ptr_(nullptr)
, lay_left_ptr_(nullptr)
, btn_last_one_ptr_(nullptr)
, lay_right_ptr_(nullptr)
, btn_next_one_ptr_(nullptr)
, lay_bottom_ptr_(nullptr)
, btn_rotate_ptr_(nullptr)
, btn_download_ptr_(nullptr)
, btn_delete_ptr_(nullptr)
, btn_show_thumbnail_ptr_(nullptr)
, btn_hide_thumbnail_ptr_(nullptr)
, gif_loading_ptr_(nullptr)

, is_phone_(false)
, pic_list_(nullptr)
, is_init_(false)
, rotate_(0)
, curr_sel_option_(nullptr)
{
}

CPicView::~CPicView()
{
}

LPCTSTR CPicView::GetWindowClassName() const
{
	return _T("CPicView");
}

CDuiString CPicView::GetSkinFile()
{
	return _T("pic_view.xml");
}

CDuiString CPicView::GetSkinFolder()
{
	return  _T("skin\\PhotoManager");
}

void CPicView::OnClick(TNotifyUI& msg)
{
    if (msg.pSender == closebtn_ptr_)
    {
        ShowWindow(false);
        return;
    }
    else if (msg.pSender == btn_show_thumbnail_ptr_)
    {
        on_show_thumbnail();
    }
    else if (msg.pSender == btn_hide_thumbnail_ptr_)
    {
        on_hide_thumbnail();
    }
    else if (msg.pSender == btn_rotate_ptr_)
    {
        on_rotate();
    }
    else if (msg.pSender == btn_last_page_ptr_)
    {
        lay_thumbnail_list_ptr_->PageLeft();
    }
    else if (msg.pSender == btn_next_page_ptr_)
    {
        lay_thumbnail_list_ptr_->PageRight();
    }
    else if (msg.pSender == btn_last_one_ptr_)
    {
        on_last_image();
    }
    else if (msg.pSender == btn_next_one_ptr_)
    {
        on_next_image();
    }
    else if (msg.pSender == btn_download_ptr_)
    {
        on_download();
    }
    else if (msg.pSender == btn_delete_ptr_)
    {
        on_delete();
    }
	__super::OnClick(msg);
}

void CPicView::InitWindow()
{
    lay_main_ptr_ = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("lay_main")));
    lay_top_ptr_ = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("lay_top")));
    lbl_image_name_ptr_ = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lbl_image_name")));
    minbtn_ptr_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("minbtn")));
    maxbtn_ptr_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("maxbtn")));
    restorebtn_ptr_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("restorebtn")));
    closebtn_ptr_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("closebtn")));
    ctrl_image_ptr_ = static_cast<CControlUI*>(m_PaintManager.FindControl(_T("ctrl_image")));
    lay_thumbnail_ptr_ = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("lay_thumbnail")));
    btn_last_page_ptr_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_last_page")));
    lay_thumbnail_list_ptr_ = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("lay_thumbnail_list")));
    btn_next_page_ptr_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_next_page")));
    lay_left_ptr_ = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("lay_left")));
    btn_last_one_ptr_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_last_one")));
    lay_right_ptr_ = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("lay_right")));
    btn_next_one_ptr_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_next_one")));
    lay_bottom_ptr_ = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("lay_bottom")));
    btn_rotate_ptr_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_rotate")));
    btn_download_ptr_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_download")));
    btn_delete_ptr_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_delete")));
    btn_show_thumbnail_ptr_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_show_thumbnail")));
    btn_hide_thumbnail_ptr_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_hide_thumbnail")));
    gif_loading_ptr_ = static_cast<CGifUI*>(m_PaintManager.FindControl(_T("gif_loading")));
}

void CPicView::OnPrepare(TNotifyUI& msg)
{
    ::GetClientRect(this->GetHWND(), &win_max_rc_);
    SetIcon(IDI_ICON_PROGRAM16);
    ShowWindow(false);
}

void CPicView::Notify(TNotifyUI& msg)
{
    if (_tcsicmp(msg.sType, DUI_MSGTYPE_WINDOWINIT) == 0)
	{
		OnPrepare(msg);
	}
    else if (_tcsicmp(msg.sType, DUI_MSGTYPE_ITEMSELECT) == 0)
    {
        OnItemSelect(msg);
    }

	WindowImplBase::Notify(msg);
}

UILIB_RESOURCETYPE CPicView::GetResourceType() const
{
#ifdef _DEBUG
	return  UILIB_FILE;
#else
	return UILIB_ZIPRESOURCE;
#endif
}

LPCTSTR CPicView::GetResourceID() const
{
	return MAKEINTRESOURCE(IDR_ZIPRES);
}

LRESULT CPicView::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (EMsgShowPic == uMsg)
    {
        on_msg_show_pic(wParam);
    }
    else if (WM_MOUSEHOVER == uMsg)
    {
        on_msg_mouse_hover(wParam, lParam);
    }
    else if (WM_MOUSEMOVE == uMsg)
    {
        on_msg_mouse_move(wParam, lParam);
    }
    else if (EMsgRefreshViewBigImage == uMsg)
    {
        on_msg_refresh_view_big_image(wParam, lParam);
    }
    else if (EMsgRefreshViewImageList == uMsg)
    {
        on_msg_refresh_view_image_list(wParam);
    }
    else if (EMsgRefreshViewThumbnail == uMsg)
    {
        on_msg_refresh_view_thumbnail(wParam);
    }

//     wchar_t tmp[50] = { 0 };
//     wsprintf(tmp, _T("\r\n uMsg:%d wParam:%d lParam:%d"), uMsg, wParam, lParam);
//     OutputDebugString(tmp);

    return __super::HandleMessage(uMsg, wParam, lParam);
}

void CPicView::OnItemSelect(TNotifyUI& msg)
{
}

LRESULT CPicView::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (eTimerDelayInitData == wParam)
    {
        KillTimer(eTimerDelayInitData);
        int width = 0;
        int height = 0;
        get_big_image_max_size(width, height);
        theBusiness::instance().do_get_view_big_image(pic_name_, pic_path_, is_phone_, width, height, rotate_, phone_sn_);
        theBusiness::instance().do_get_view_image_list(pic_path_, is_phone_);
    }
    else if (eTimerDelayGetBigImage == wParam)
    {
        KillTimer(eTimerDelayGetBigImage);
        if (nullptr != curr_sel_option_)
        {
            int index = curr_sel_option_->GetTag();
            int width = 0;
            int height = 0;
            get_big_image_max_size(width, height);
            theBusiness::instance().do_get_view_big_image((*pic_list_)[index].first, (*pic_list_)[index].second, is_phone_, width, height, rotate_, phone_sn_);
        }        
    }

    bHandled = TRUE;
    return 0;
}

LRESULT CPicView::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (1 != wParam)//最小化
    {
        RECT rc;
        RECT rc1;
        RECT rc2;
        GetClientRect(this->GetHWND(), &rc);
        if (lay_top_ptr_->IsVisible())
        {
            rc1 = { rc.left, rc.top, rc.right, 70 };
            lay_top_ptr_->SetPos(rc1);
            set_bottom_lay_pos(rc);
        }

        if (lay_left_ptr_->IsVisible())
        {
            rc1.left = rc.left;
            rc1.top = rc.top + 70;
            rc1.right = 114;
            rc1.bottom = rc.bottom - 70;

            rc2.left = rc.right - 114;
            rc2.top = rc.top + 70;
            rc2.right = rc.right;
            rc2.bottom = rc.bottom - 70;
            lay_left_ptr_->SetPos(rc1);
            lay_right_ptr_->SetPos(rc2);
        }

        if (!pic_path_.empty())
        {
            if (is_init_)
            {
                SetTimer(eTimerDelayInitData, 500);
            }
            else
            {
                SetTimer(eTimerDelayGetBigImage, 500);
            }
            is_init_ = false;
        }
    }
    return 0;
}

void CPicView::on_msg_show_pic(WPARAM wParam)
{
    ShowPicParam* params = (ShowPicParam*)wParam;
    is_phone_ = params->is_phone_pic_;
    pic_name_ = character::unicode_to_mbyte(params->pic_name_);
    pic_path_ = character::unicode_to_mbyte(params->pic_path_);
    phone_sn_ = character::unicode_to_mbyte(params->phone_sn_);

    show_loading(true);
    rotate_ = 0;
    curr_sel_option_ = nullptr;
    m_PaintManager.RemoveImage(ctrl_image_ptr_->GetBkImage());
    remove_thumbnail_pic();

    //设置默认图片
    std::wstring dest = get_image_dest(win_max_rc_.left, win_max_rc_.top, win_max_rc_.right, win_max_rc_.bottom, 152, 124);
    std::wstring pic_id = L"file='pic/PicViewDlg/image_defualt.png'";
    pic_id.append(dest);
    ctrl_image_ptr_->SetBkImage(pic_id.c_str());
    btn_download_ptr_->SetVisible(is_phone_);
    btn_hide_thumbnail_ptr_->SetText(L"第0/0张");

    if (!::IsZoomed(this->GetHWND()))
    {
        is_init_ = true;
        ::ShowWindow(this->GetHWND(), SW_SHOWMAXIMIZED);
        maxbtn_ptr_->SetVisible(false);
        restorebtn_ptr_->SetVisible(true);
    }
    else if (!::IsWindowVisible(this->GetHWND()))
    {
        ::ShowWindow(this->GetHWND(), SW_SHOWMAXIMIZED);
        SetTimer(eTimerDelayInitData, 500);
    }
    else
    {
        ::SetForegroundWindow(this->GetHWND());
        int width = 0;
        int height = 0;
        get_big_image_max_size(width, height);
        theBusiness::instance().do_get_view_big_image(pic_name_, pic_path_, is_phone_, width, height, rotate_, phone_sn_);
        theBusiness::instance().do_get_view_image_list(pic_path_, is_phone_);
    }

    delete params;
}


void CPicView::on_msg_mouse_hover(WPARAM& wParam, LPARAM& lParam)
{
    RECT rc;
    GetClientRect(this->GetHWND(), &rc);
    POINT pt;
    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);

    RECT rc1 = { rc.left, rc.top, rc.right, TOP_LAY_HEIGHT };
    RECT rc2 = { rc.left, rc.bottom-BOTTOM_LAY_HEIGHT, rc.right, rc.bottom };
    if (lay_thumbnail_ptr_->IsVisible())
    {
        rc2.top -= THUMBNAIL_LAY_HEIGHT;
        rc2.bottom -= THUMBNAIL_LAY_HEIGHT;
    }
    if (!::PtInRect(&rc1, pt) && !::PtInRect(&rc2, pt))
    {
        lay_top_ptr_->SetVisible(false);
        lay_bottom_ptr_->SetVisible(false);
    }

    rc1.left = rc.left;
    rc1.top = rc.top + TOP_LAY_HEIGHT;
    rc1.right = LEFT_RIGHT_LAY_WIDTH;
    rc1.bottom = rc.bottom - BOTTOM_LAY_HEIGHT;

    rc2.left = rc.right - LEFT_RIGHT_LAY_WIDTH;
    rc2.top = rc.top + TOP_LAY_HEIGHT;
    rc2.right = rc.right;
    rc2.bottom = rc.bottom - BOTTOM_LAY_HEIGHT;

    if (!::PtInRect(&rc1, pt) && !::PtInRect(&rc2, pt))
    {
        lay_left_ptr_->SetVisible(false);
        lay_right_ptr_->SetVisible(false);
    }
}

void CPicView::on_msg_mouse_move(WPARAM& wParam, LPARAM& lParam)
{
    RECT rc;
    GetClientRect(this->GetHWND(), &rc);
    POINT pt;
    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);

    RECT rc1 = { rc.left, rc.top, rc.right, TOP_LAY_HEIGHT };
    RECT rc2 = { rc.left, rc.bottom - BOTTOM_LAY_HEIGHT, rc.right, rc.bottom };
    if (lay_thumbnail_ptr_->IsVisible())
    {
        rc2.top -= THUMBNAIL_LAY_HEIGHT;
        rc2.bottom -= THUMBNAIL_LAY_HEIGHT;
    }

    if (::PtInRect(&rc1, pt) || ::PtInRect(&rc2, pt))
    {
        lay_top_ptr_->SetVisible(true);
        if (lay_top_ptr_->GetWidth() != (rc1.right - rc1.left))
        {
            lay_top_ptr_->SetPos(rc1);
        }

        lay_bottom_ptr_->SetVisible(true);
        if (lay_bottom_ptr_->GetWidth() != (rc2.right - rc2.left))
        {
            lay_bottom_ptr_->SetPos(rc2);
        }
    }

    rc1.left = rc.left;
    rc1.top = rc.top + TOP_LAY_HEIGHT;
    rc1.right = LEFT_RIGHT_LAY_WIDTH;
    rc1.bottom = rc.bottom - BOTTOM_LAY_HEIGHT;

    rc2.left = rc.right - LEFT_RIGHT_LAY_WIDTH;
    rc2.top = rc.top + TOP_LAY_HEIGHT;
    rc2.right = rc.right;
    rc2.bottom = rc.bottom - BOTTOM_LAY_HEIGHT;
    if (::PtInRect(&rc1, pt) || ::PtInRect(&rc2, pt))
    {
        lay_left_ptr_->SetVisible(true);
        if (lay_left_ptr_->GetHeight() != (rc1.bottom - rc1.top))
        {
            lay_left_ptr_->SetPos(rc1);
        }

        lay_right_ptr_->SetVisible(true);
        if (lay_right_ptr_->GetHeight() != (rc2.bottom - rc2.top))
        {
            lay_right_ptr_->SetPos(rc2);
        }
    }
}

void CPicView::on_msg_refresh_view_big_image(WPARAM& wParam, LPARAM& lParam)
{
    show_loading(false);
    std::string* name = (std::string*)wParam;
    std::wstring file_name = character::mbyte_to_unicode(*name);
    lbl_image_name_ptr_->SetText(file_name.c_str());

    RECT rc = get_big_image_rect();
    GetClientRect(this->GetHWND(), &rc);
    IplImage* image_ptr = (IplImage*)lParam;
    if (NULL != image_ptr)
    {
        std::wstring dest = get_image_dest(rc.left, rc.top, rc.right, rc.bottom, image_ptr->width, image_ptr->height);
        std::wstring pic_id = L"file='view_big_image'";
        pic_id.append(dest);

        ctrl_image_ptr_->SetBGRBkImage(pic_id.c_str(), (unsigned char*)image_ptr->imageData, image_ptr->imageSize, image_ptr->width, image_ptr->height, false);
        
        cvReleaseImage(&image_ptr);
    }
    else
    {
        std::wstring dest = get_image_dest(rc.left, rc.top, rc.right, rc.bottom, 86, 78);
        std::wstring pic_id = L"file='pic/PicViewDlg/image_failed.png'";
        pic_id.append(dest);
        ctrl_image_ptr_->SetBkImage(pic_id.c_str());
    }
    delete name;
}

void CPicView::on_msg_refresh_view_image_list(WPARAM& wParam)
{
    if (nullptr != pic_list_)
    {
        delete pic_list_;
        pic_list_ = nullptr;
    }

    pic_list_ = (std::vector<std::pair<std::string, std::string>>*)wParam;
    if (NULL == pic_list_)
    {
        return;
    }

    COptionUI* curr_option = NULL;;

    CControlUI* ctrl_ptr = NULL;
    int i = 0;
    for (auto itor = pic_list_->begin(); itor != pic_list_->end(); ++itor, ++i)
    {
        COptionUI* opt_ptr = new COptionUI();
        lay_thumbnail_list_ptr_->Add(opt_ptr);
        opt_ptr->SetFixedWidth(THUMBNAIL_MAX_WIDTH);
        opt_ptr->SetFixedHeight(THUMBNAIL_MAX_HEIGHT);
        opt_ptr->SetTag(i);
        opt_ptr->SetGroup(L"thumbnail_pic_group");
        opt_ptr->SetBorderColor(0xFF0586FF);
        opt_ptr->SetBkColor(0xFF7A7C7E);
        opt_ptr->SetNormalImage(L"file='pic/PicViewDlg/thumbnail_def.png' dest='17,12,59,46'");
        opt_ptr->SetHotImage(L"file='pic/PicViewDlg/thumbnail_def.png' dest='17,12,59,46'");
        opt_ptr->SetSelectedImage(L"file='pic/PicViewDlg/thumbnail_def.png' dest='17,12,59,46'");
        opt_ptr->OnNotify += MakeDelegate(this, &CPicView::on_thumbnail_select);

        ctrl_ptr = new CControlUI;
        ctrl_ptr->SetTag(-1);
        ctrl_ptr->SetFixedWidth(8);
        lay_thumbnail_list_ptr_->Add(ctrl_ptr);
        

        if (itor->first == pic_name_)
        {
            curr_option = opt_ptr;
        }
        theBusiness::instance().do_get_view_thumbnail(itor->first, itor->second, is_phone_, THUMBNAIL_MAX_WIDTH, THUMBNAIL_MAX_HEIGHT, i, phone_sn_);
    }
    if (NULL != curr_option)
    {
        curr_option->Selected(true);
    }
}

void CPicView::remove_thumbnail_pic()
{
    int count = lay_thumbnail_list_ptr_->GetCount();
    for (int i = 0; i < count; ++i)
    {
        CControlUI* item_ptr = lay_thumbnail_list_ptr_->GetItemAt(i);
        if (-1 != item_ptr->GetTag())
        {
            COptionUI* opt_ptr = (COptionUI*)item_ptr;
            m_PaintManager.RemoveImage(opt_ptr->GetNormalImage());
            m_PaintManager.RemoveImage(opt_ptr->GetHotImage());
            m_PaintManager.RemoveImage(opt_ptr->GetSelectedImage());
        }        
    }
    lay_thumbnail_list_ptr_->RemoveAll();
}

void CPicView::on_msg_refresh_view_thumbnail(WPARAM& wParam)
{
    ViewThumbnailParam* param = (ViewThumbnailParam*)wParam;
    int index = param->index_ * 2;
    if (index < lay_thumbnail_list_ptr_->GetCount())
    {
        COptionUI* ctrl_ptr = (COptionUI*)lay_thumbnail_list_ptr_->GetItemAt(index);
        wchar_t tmp[10] = { 0 };
        swprintf_s(tmp, L"%d", ctrl_ptr->GetTag());

        if (NULL != param->dark_image_)
        {
            IplImage* image_ptr = (IplImage*)param->dark_image_;
            std::wstring pic_id = L"file='view_thumbnail_nor_";
            pic_id.append(tmp);
            pic_id.append(L"'");
            std::wstring dest = get_image_dest(0, 0, THUMBNAIL_MAX_WIDTH, THUMBNAIL_MAX_HEIGHT, image_ptr->width, image_ptr->height);
            pic_id.append(dest);
            ctrl_ptr->SetBGRNormalImage(pic_id.c_str(), (unsigned char*)image_ptr->imageData, image_ptr->imageSize, image_ptr->width, image_ptr->height, false);
            cvReleaseImage(&image_ptr);
        }

        if (NULL != param->image_)
        {
            IplImage* image_ptr = (IplImage*)param->image_;
            std::wstring pic_id = L"file='view_thumbnail_sel_";
            pic_id.append(tmp);
            pic_id.append(L"'");
            std::wstring dest = get_image_dest(0, 0, THUMBNAIL_MAX_WIDTH, THUMBNAIL_MAX_HEIGHT, image_ptr->width, image_ptr->height);
            pic_id.append(dest);
            ctrl_ptr->SetBGRSelectedImage(pic_id.c_str(), (unsigned char*)image_ptr->imageData, image_ptr->imageSize, image_ptr->width, image_ptr->height, false);

            pic_id = L"file='view_thumbnail_hot_";
            pic_id.append(tmp);
            pic_id.append(L"'");
            pic_id.append(dest);
            ctrl_ptr->SetBGRHotImage(pic_id.c_str(), (unsigned char*)image_ptr->imageData, image_ptr->imageSize, image_ptr->width, image_ptr->height, false);

            cvReleaseImage(&image_ptr);
        }
    }

    delete param;
}

bool CPicView::on_thumbnail_select(void* param)
{
    TNotifyUI* pMsg = (TNotifyUI*)param;
    if (nullptr == pMsg)
    {
        return false;
    }

    if (pMsg->sType == DUI_MSGTYPE_SELECTCHANGED)
    {
        if (nullptr != curr_sel_option_ && curr_sel_option_ != pMsg->pSender)
        {
            show_loading(true);
            curr_sel_option_->SetBorderSize(0);
            rotate_ = 0;
            int index = pMsg->pSender->GetTag();
            int width = 0;
            int height = 0;
            get_big_image_max_size(width, height);
            theBusiness::instance().do_get_view_big_image((*pic_list_)[index].first, (*pic_list_)[index].second, is_phone_, width, height, rotate_, phone_sn_);
        }
        curr_sel_option_ = (COptionUI*)pMsg->pSender;
        curr_sel_option_->SetBorderSize(2);
        reset_hide_thumbnail_text();
    }
    return true;
}

std::wstring CPicView::get_image_dest(int left, int top, int right, int bottom, int width, int height)
{
    if (bottom - top > height)
    {
        top = (bottom - top - height) / 2;
        bottom = top + height;
    }

    if (right - left > width)
    {
        left = (right - left - width) / 2;
        right = left + width;
    }

    wchar_t tmp[40] = { 0 };
    swprintf_s(tmp, L" dest='%d,%d,%d,%d'", left, top, right, bottom);
    std::wstring dest = tmp;
    return std::move(dest);
}

void CPicView::set_bottom_lay_pos(const RECT& rc)
{
    RECT rc2 = { rc.left, rc.bottom - BOTTOM_LAY_HEIGHT, rc.right, rc.bottom };
    if (lay_thumbnail_ptr_->IsVisible())
    {
        rc2.top -= THUMBNAIL_LAY_HEIGHT;
        rc2.bottom -= THUMBNAIL_LAY_HEIGHT;
    }
    lay_bottom_ptr_->SetVisible(true);
    lay_bottom_ptr_->SetPos(rc2);
}

RECT  CPicView::get_big_image_rect()
{
    RECT rc;
    GetClientRect(this->GetHWND(), &rc);
    if (lay_thumbnail_ptr_->IsVisible())
    {
        rc.bottom -= THUMBNAIL_LAY_HEIGHT;
    }

    return std::move(rc);
}

void CPicView::get_big_image_max_size(int& width, int& height)
{
    RECT rc = get_big_image_rect();
    width = rc.right - rc.left;
    height = rc.bottom - rc.top;
}

void CPicView::reset_hide_thumbnail_text()
{
    if (nullptr != curr_sel_option_)
    {
        int index = lay_thumbnail_list_ptr_->GetItemIndex(curr_sel_option_);
        index = index / 2 + 1;
        wchar_t tmp[100] = { 0 };
        swprintf_s(tmp, L"第%d/%d张", index, lay_thumbnail_list_ptr_->GetCount()/2);
        btn_hide_thumbnail_ptr_->SetText(tmp);
    }    
}

void CPicView::show_loading(bool visible)
{
    gif_loading_ptr_->SetVisible(visible);
    if (visible)
    {
        RECT rc = { 0 };
        ::GetClientRect(this->GetHWND(), &rc);
        rc.left = (rc.right - rc.left - 36) / 2 + rc.left;
        rc.right = rc.left + 36;
        rc.top = (rc.bottom - rc.top - 36) / 2 + rc.top;
        rc.bottom = rc.top + 36;
        gif_loading_ptr_->SetPos(rc);
    }    
}

void CPicView::on_show_thumbnail()
{
    lay_thumbnail_ptr_->SetVisible(true);
    btn_show_thumbnail_ptr_->SetVisible(false);
    btn_hide_thumbnail_ptr_->SetVisible(true);
    RECT rc;
    ::GetClientRect(this->GetHWND(), &rc);
    set_bottom_lay_pos(rc);
    SetTimer(eTimerDelayGetBigImage, 500);
}

void CPicView::on_hide_thumbnail()
{
    lay_thumbnail_ptr_->SetVisible(false);
    btn_show_thumbnail_ptr_->SetVisible(true);
    btn_hide_thumbnail_ptr_->SetVisible(false);
    RECT rc;
    ::GetClientRect(this->GetHWND(), &rc);
    set_bottom_lay_pos(rc);
    SetTimer(eTimerDelayGetBigImage, 500);
}

void CPicView::on_rotate()
{
    rotate_ += 90;
    if (rotate_ >= 360)
    {
        rotate_ = rotate_ % 360;
    }
    int width = 0;
    int height = 0;
    get_big_image_max_size(width, height);
    theBusiness::instance().do_get_view_big_image(pic_name_, pic_path_, is_phone_, width, height, rotate_, phone_sn_);
}

void CPicView::on_last_image()
{
    if (nullptr != curr_sel_option_)
    {
        int index = lay_thumbnail_list_ptr_->GetItemIndex(curr_sel_option_);
        if (index <= 0)
        {
            CMsgBox::msg_box(this->GetHWND(), L"已是第一张！", L"", CMsgBox::EHints, MB_OK, false, 1);
        }
        else
        {
            index -= 2;
            COptionUI* opt_ptr = (COptionUI*)lay_thumbnail_list_ptr_->GetItemAt(index);
            opt_ptr->Selected(true);
        }
    }
}

void CPicView::on_next_image()
{
    if (nullptr != curr_sel_option_)
    {
        int index = lay_thumbnail_list_ptr_->GetItemIndex(curr_sel_option_);
        index += 2;
        if (index >= lay_thumbnail_list_ptr_->GetCount())
        {
            CMsgBox::msg_box(this->GetHWND(), L"已是最后一张！", L"", CMsgBox::EHints, MB_OK, false, 1);
        }
        else
        {
            COptionUI* opt_ptr = (COptionUI*)lay_thumbnail_list_ptr_->GetItemAt(index);
            opt_ptr->Selected(true);
        }
    }
}

void CPicView::on_delete()
{
    if (nullptr == curr_sel_option_)
    {
        return;
    }
    int index = curr_sel_option_->GetTag();
    std::string path = (*pic_list_)[index].second + "/" + (*pic_list_)[index].first;

    int new_index = 0;
    index = lay_thumbnail_list_ptr_->GetItemIndex(curr_sel_option_);
    if ((index + 2) >= lay_thumbnail_list_ptr_->GetCount())
    {
        new_index = index - 2;
    }
    else
    {
        new_index = index;
    }

    m_PaintManager.RemoveImage(curr_sel_option_->GetNormalImage());
    m_PaintManager.RemoveImage(curr_sel_option_->GetHotImage());
    m_PaintManager.RemoveImage(curr_sel_option_->GetSelectedImage());
    CControlUI* ctrl_ptr = lay_thumbnail_list_ptr_->GetItemAt(index + 1);
    lay_thumbnail_list_ptr_->Remove(ctrl_ptr);
    lay_thumbnail_list_ptr_->Remove(curr_sel_option_);
    
    if (new_index >= 0)
    {
        COptionUI* opt_ptr = (COptionUI*)lay_thumbnail_list_ptr_->GetItemAt(new_index);
        opt_ptr->Selected(true);
    }
    else
    {
        m_PaintManager.RemoveImage(ctrl_image_ptr_->GetBkImage());
        std::wstring dest = get_image_dest(win_max_rc_.left, win_max_rc_.top, win_max_rc_.right, win_max_rc_.bottom, 152, 124);
        std::wstring pic_id = L"file='pic/PicViewDlg/image_defualt.png'";
        pic_id.append(dest);
        ctrl_image_ptr_->SetBkImage(pic_id.c_str());
        btn_hide_thumbnail_ptr_->SetText(L"第0/0张");
    }
    theBusiness::instance().do_delete_file(character::mbyte_to_unicode(path), is_phone_, character::mbyte_to_unicode(phone_sn_));
}

void CPicView::on_download()
{
    if (nullptr == curr_sel_option_)
    {
        return;
    }

    CDataFactory& factory = theDataFactory::instance();
    if (factory.save_as_path_.empty())
    {
        char szBuffer[MAX_PATH] = { 0 };
        BROWSEINFOA bi;
        ZeroMemory(&bi, sizeof(BROWSEINFOA));
        bi.hwndOwner = this->GetHWND();
        bi.pszDisplayName = szBuffer;
        bi.ulFlags = BIF_BROWSEINCLUDEFILES | BIF_RETURNONLYFSDIRS | BIF_EDITBOX;
        LPITEMIDLIST idl = ::SHBrowseForFolderA(&bi);
        if (NULL != idl)
        {
            ::SHGetPathFromIDListA(idl, szBuffer);
        }
        factory.save_as_path_ = szBuffer;
    }

    int index = curr_sel_option_->GetTag();

    std::string phone_file = (*pic_list_)[index].second;
    phone_file.append("/");
    phone_file.append((*pic_list_)[index].first);

    std::string store_path = factory.save_as_path_ + (*pic_list_)[index].first;
    theBusiness::instance().do_download_phone_file(phone_sn_, phone_file, store_path);
}


