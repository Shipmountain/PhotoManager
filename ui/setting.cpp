
#include "../stdafx.h"
#include <windows.h>
#if !defined(UNDER_CE)
#include <shellapi.h>
#endif
#include "../resource.h"

#include "setting.h"
#include "../data/DataFactory.h"
#include <character_cvt.hpp>


CSetting::CSetting() :ModuleLog("Setting")
, lbl_title_ptr_(NULL)
, closebtn_ptr_(NULL)
, edt_local_store_path_ptr_(NULL)
, btn_ok_ptr_(NULL)
, btn_cancel_ptr_(NULL)
{
}

CSetting::~CSetting()
{
}

LPCTSTR CSetting::GetWindowClassName() const
{
	return _T("CSetting");
}

CDuiString CSetting::GetSkinFile()
{
	return _T("setting.xml");
}

CDuiString CSetting::GetSkinFolder()
{
	return  _T("skin\\PhotoManager");
}

void CSetting::OnClick(TNotifyUI& msg)
{
    if (msg.pSender == btn_ok_ptr_)
    {
        on_ok();
    }
    else if (msg.pSender == btn_cancel_ptr_)
    {
        Close(IDCANCEL);
    }

	__super::OnClick(msg);
}

void CSetting::InitWindow()
{
    lbl_title_ptr_ = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lbl_title")));
    closebtn_ptr_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("closebtn")));
    edt_local_store_path_ptr_ = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edt_local_store_path")));
    btn_ok_ptr_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_ok")));
    btn_cancel_ptr_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_cancel")));
}

void CSetting::OnPrepare(TNotifyUI& msg)
{
    edt_local_store_path_ptr_->SetText(character::mbyte_to_unicode(theDataFactory::instance().local_storage_path_).c_str());
    edt_local_store_path_ptr_->SetToolTip(edt_local_store_path_ptr_->GetToolTip().GetData());
}

void CSetting::Notify(TNotifyUI& msg)
{
    if (_tcsicmp(msg.sType, DUI_MSGTYPE_WINDOWINIT) == 0)
	{
		OnPrepare(msg);
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
    else if (_tcsicmp(msg.sType, DUI_MSGTYPE_ITEMSELECT) == 0)
    {
        OnItemSelect(msg);
    }

	WindowImplBase::Notify(msg);
}

UILIB_RESOURCETYPE CSetting::GetResourceType() const
{
#ifdef _DEBUG
	return  UILIB_FILE;
#else
	return UILIB_ZIPRESOURCE;
#endif
}

LPCTSTR CSetting::GetResourceID() const
{
	return MAKEINTRESOURCE(IDR_ZIPRES);
}

LRESULT CSetting::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return __super::HandleMessage(uMsg, wParam, lParam);
}

bool CSetting::on_text_changed(void* param)
{
    TNotifyUI* pMsg = (TNotifyUI*)param;
    if (nullptr == pMsg)
    {
        return false;
    }

    if (pMsg->sType == DUI_MSGTYPE_TEXTCHANGED)
    {
       
    }

    return true;
}

void CSetting::OnSelectChange(TNotifyUI& msg)
{
    
}

void CSetting::OnMenuItemClick(TNotifyUI& msg)
{
  
}

void CSetting::OnRButtonDown(TNotifyUI& msg)
{
  
}

void CSetting::OnItemSelect(TNotifyUI& msg)
{
}

void CSetting::on_ok()
{
    if (edt_local_store_path_ptr_->GetText().IsEmpty())
    {
        return;
    }

    theDataFactory::instance().local_storage_path_ = character::unicode_to_mbyte(std::wstring(edt_local_store_path_ptr_->GetText().GetData()));
    Close(IDOK);
}

