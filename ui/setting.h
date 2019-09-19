#ifndef SETTING_HPP
#define SETTING_HPP
#include "../../DuiLib/Utils/WinImplBase.h"
#include <log.h>

using namespace DuiLib;


class CSetting : public WindowImplBase, Log::ModuleLog
{
public:

    CSetting();
    ~CSetting();

public:

	LPCTSTR GetWindowClassName() const;	
	virtual void InitWindow();
	virtual CDuiString GetSkinFile();
	virtual CDuiString GetSkinFolder();
	virtual UILIB_RESOURCETYPE GetResourceType() const;
	LPCTSTR GetResourceID() const;
    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
protected:
	void Notify(TNotifyUI& msg);
	void OnPrepare(TNotifyUI& msg);
	virtual void OnClick(TNotifyUI& msg);
    void OnSelectChange(TNotifyUI& msg);
    void OnRButtonDown(TNotifyUI& msg);
    void OnMenuItemClick(TNotifyUI& msg);
    void OnItemSelect(TNotifyUI& msg);
    //滑块事件处理函数
    bool on_text_changed(void* param);
private:
    void on_ok();
private:
    CLabelUI*           lbl_title_ptr_;
    CButtonUI*          closebtn_ptr_;
    CEditUI*            edt_local_store_path_ptr_;
    CButtonUI*          btn_ok_ptr_;
    CButtonUI*          btn_cancel_ptr_;
};

#endif // SETTING_HPP