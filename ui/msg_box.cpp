
#include "../stdafx.h"
#include <windows.h>
#if !defined(UNDER_CE)
#include <shellapi.h>
#endif
#include "../resource.h"

#include "msg_box.h"
#include "msg_def.h"

static CMsgBox*     k_msg_box = NULL;

CMsgBox::CMsgBox(const std::wstring& content, const std::wstring& title, ShowType show_type, int btn_type, bool wait) :ModuleLog("MsgBox")
, lbl_title_ptr_(NULL)
, closebtn_ptr_(NULL)
, ctl_image_ptr_(NULL)
, txt_msg_ptr_(NULL)
, btn_ok_ptr_(NULL)
, btn_cancel_ptr_(NULL)

, msg_(content)
, title_(title)
, show_type_(show_type)
, btn_type_(btn_type)
, wait_(wait)
{
}

CMsgBox::~CMsgBox()
{
}

LPCTSTR CMsgBox::GetWindowClassName() const
{
	return _T("CMsgBox");
}

CDuiString CMsgBox::GetSkinFile()
{
	return _T("msg_box.xml");
}

CDuiString CMsgBox::GetSkinFolder()
{
	return  _T("skin\\PhotoManager");
}

void CMsgBox::OnClick(TNotifyUI& msg)
{
	if (msg.pSender == btn_ok_ptr_)
	{
        if (wait_)
        {
            Close(IDOK);
        }
        else
        {
            ShowWindow(false);
        }
        
	}
    else if (msg.pSender == btn_cancel_ptr_)
    {
        if (wait_)
        {
            Close(IDCANCEL);
        }
        else
        {
            this->ShowWindow(false);
        }
        
    }
    else if (msg.pSender == closebtn_ptr_ )
	{
        if (wait_)
        {
            Close(IDCLOSE);
        } 
        else
        {
            this->ShowWindow(false);
        }		
	}

	__super::OnClick(msg);
}

void CMsgBox::InitWindow()
{
    lbl_title_ptr_ = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lbl_title")));
	closebtn_ptr_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("closebtn")));
    ctl_image_ptr_ = static_cast<CControlUI*>(m_PaintManager.FindControl(_T("ctl_image")));
    txt_msg_ptr_ = static_cast<CTextUI*>(m_PaintManager.FindControl(_T("txt_msg")));
	btn_ok_ptr_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_ok")));
	btn_cancel_ptr_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_cancel")));
}

void CMsgBox::OnPrepare(TNotifyUI& msg)
{
    set(msg_, title_, show_type_, btn_type_);
}

void CMsgBox::Notify(TNotifyUI& msg)
{
	if (_tcsicmp(msg.sType, _T("windowinit")) == 0)
	{
		OnPrepare(msg);
	}

	WindowImplBase::Notify(msg);
}

UILIB_RESOURCETYPE CMsgBox::GetResourceType() const
{
#ifdef _DEBUG
	return  UILIB_FILE;
#else
	return UILIB_ZIPRESOURCE;
#endif
}

LPCTSTR CMsgBox::GetResourceID() const
{
	return MAKEINTRESOURCE(IDR_ZIPRES);
}

LRESULT CMsgBox::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return __super::HandleMessage(uMsg, wParam, lParam);
}

LRESULT CMsgBox::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    ShowWindow(false);
    KillTimer(0);
    bHandled = FALSE;
    return 0;
}

int CMsgBox::msg_box(HWND parent_hwnd, const std::wstring& content, const std::wstring& title, ShowType show_type, int btn_type, bool wait/* = true*/, int show_time/* = -1*/)
{
    if (wait)
    {
        CMsgBox msg_box(content, title, show_type, btn_type, wait);
        msg_box.Create(parent_hwnd, _T(""), UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE);
        msg_box.CenterWindow();
        return msg_box.ShowModal();
    }
    else
    {
        if (NULL == k_msg_box)
        {
            k_msg_box = new CMsgBox(content, title, show_type, btn_type, wait);
            k_msg_box->Create(parent_hwnd, _T(""), UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE);
        }
        else
        {
            k_msg_box->set(content, title, show_type, btn_type);
        }
        k_msg_box->CenterWindow();
        k_msg_box->ShowWindow(true);
        if (-1 != show_time)
        {
            k_msg_box->SetTimer(0, show_time * 1000);
        }
    }
    return 0;
}

void CMsgBox::set(const std::wstring& content, const std::wstring& title, ShowType show_type, int btn_type)
{
    msg_ = content;
    title_ = title;
    show_type_ = show_type;
    btn_type_ = btn_type;
    if (title_.empty())
    {
        if (EHints == show_type)
        {
            title_ = L"提示";
        }
        else if (EWarning == show_type)
        {
            title_ = L"警告";
        }
        else if (EError == show_type)
        {
            title_ = L"错误";
        }
        else if (EQuestion == show_type)
        {
            title_ = L"疑问";
        }
    }

    if (EOK == show_type)
    {
        ctl_image_ptr_->SetBkImage(_T("pic/msg_ok.png"));
    }
    else if (EHints == show_type)
    {
        ctl_image_ptr_->SetBkImage(_T("pic/msg_hints.png"));
    }
    else if (EWarning == show_type)
    {
        ctl_image_ptr_->SetBkImage(_T("pic/msg_warning.png"));
    }
    else if (EError == show_type)
    {
        ctl_image_ptr_->SetBkImage(_T("pic/msg_error.png"));
    }
    else if (EQuestion == show_type)
    {
        ctl_image_ptr_->SetBkImage(_T("pic/msg_question.png"));
    }

    if (MB_OK == btn_type_)
    {
        btn_cancel_ptr_->SetVisible(false);
    }
    else
    {
        btn_cancel_ptr_->SetVisible(true);
    }

    if (MB_OK == btn_type_ || MB_OKCANCEL == btn_type_)
    {
        btn_ok_ptr_->SetText(_T("确定"));
        btn_cancel_ptr_->SetText(_T("取消"));
    }
    else if (MB_YESNOCANCEL == btn_type_ || MB_YESNO == btn_type_)
    {
        btn_ok_ptr_->SetText(_T("是"));
        btn_cancel_ptr_->SetText(_T("否"));
    }


    lbl_title_ptr_->SetText(title_.c_str());
    txt_msg_ptr_->SetText(msg_.c_str());
}

