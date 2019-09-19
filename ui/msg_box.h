#ifndef MSG_BOX_HPP
#define MSG_BOX_HPP
#include "../../DuiLib/Utils/WinImplBase.h"
#include <log.h>

using namespace DuiLib;


class CMsgBox : public WindowImplBase, Log::ModuleLog
{
public:
    enum ShowType
    {
        EOK,
        EHints,
        EWarning,
        EError,
        EQuestion,
    };
private:
    CMsgBox(const std::wstring& content, const std::wstring& title, ShowType show_type, int btn_type, bool wait);
    ~CMsgBox();

public:
    static int msg_box(HWND parent_hwnd, const std::wstring& content, const std::wstring& title, ShowType show_type, int btn_type, bool wait = true, int show_time=-1);

public:

	LPCTSTR GetWindowClassName() const;	
	virtual void InitWindow();
	virtual CDuiString GetSkinFile();
	virtual CDuiString GetSkinFolder();
	virtual UILIB_RESOURCETYPE GetResourceType() const;
	LPCTSTR GetResourceID() const;
    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
protected:
	void Notify(TNotifyUI& msg);
	void OnPrepare(TNotifyUI& msg);
	virtual void OnClick(TNotifyUI& msg);

private:
    void set(const std::wstring& content, const std::wstring& title, ShowType show_type, int btn_type);
private:
    std::wstring msg_;
    std::wstring title_;
    int btn_type_;
    ShowType show_type_;
    bool wait_;
private:
    CLabelUI*               lbl_title_ptr_;
	CButtonUI*				closebtn_ptr_;
    CControlUI*             ctl_image_ptr_;
    CTextUI*                txt_msg_ptr_;
	CButtonUI*				btn_ok_ptr_;
	CButtonUI*				btn_cancel_ptr_;
};

#endif // MSG_BOX_HPP