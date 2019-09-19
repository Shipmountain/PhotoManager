#ifndef PIC_VIEW_HPP
#define PIC_VIEW_HPP
#include "../../DuiLib/Utils/WinImplBase.h"
#include <log.h>

using namespace DuiLib;


class CPicView : public WindowImplBase, Log::ModuleLog
{
private:
    enum eViewTimer
    {
        eTimerDelayInitData = 20,   //延迟获取初始数据
        eTimerDelayGetBigImage,     //延迟获取大图
    };
public:

    CPicView();
    ~CPicView();

public:

	LPCTSTR GetWindowClassName() const;	
	virtual void InitWindow();
	virtual CDuiString GetSkinFile();
	virtual CDuiString GetSkinFolder();
	virtual UILIB_RESOURCETYPE GetResourceType() const;
	LPCTSTR GetResourceID() const;
    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
protected:
	void Notify(TNotifyUI& msg);
	void OnPrepare(TNotifyUI& msg);
	virtual void OnClick(TNotifyUI& msg);
    void OnItemSelect(TNotifyUI& msg);

private:
    void on_msg_show_pic(WPARAM wParam);
    void on_msg_mouse_hover(WPARAM& wParam, LPARAM& lParam);
    void on_msg_mouse_move(WPARAM& wParam, LPARAM& lParam);
    void on_msg_refresh_view_big_image(WPARAM& wParam, LPARAM& lParam);
    void on_msg_refresh_view_image_list(WPARAM& wParam);
    void on_msg_refresh_view_thumbnail(WPARAM& wParam);

    void remove_thumbnail_pic();
    bool on_thumbnail_select(void* param);
    std::wstring get_image_dest(int left, int top, int right, int bottom, int width, int height);
    void set_bottom_lay_pos(const RECT& rc);
    RECT get_big_image_rect();
    void get_big_image_max_size(int& width, int& height);
    void reset_hide_thumbnail_text();
    void show_loading(bool visible);
    void on_show_thumbnail();
    void on_hide_thumbnail();
    void on_rotate();
    void on_last_image();
    void on_next_image();
    void on_delete();
    void on_download();
private:
    bool is_phone_;
    std::string pic_name_;
    std::string pic_path_;
    std::string phone_sn_;
    std::vector<std::pair<std::string, std::string>>* pic_list_;
    bool is_init_;
    int  rotate_;//旋转度
    COptionUI*  curr_sel_option_;
    RECT win_max_rc_;
private:
    CVerticalLayoutUI*          lay_main_ptr_;

    CHorizontalLayoutUI*        lay_top_ptr_;
    CLabelUI*                   lbl_image_name_ptr_;
    CButtonUI*                  minbtn_ptr_;
    CButtonUI*                  maxbtn_ptr_;
    CButtonUI*                  restorebtn_ptr_;
    CButtonUI*                  closebtn_ptr_;

    CControlUI*                 ctrl_image_ptr_;

    CHorizontalLayoutUI*        lay_thumbnail_ptr_;
    CButtonUI*                  btn_last_page_ptr_;
    CHorizontalLayoutUI*        lay_thumbnail_list_ptr_;
    CButtonUI*                  btn_next_page_ptr_;

    CVerticalLayoutUI*          lay_left_ptr_;
    CButtonUI*                  btn_last_one_ptr_;

    CVerticalLayoutUI*          lay_right_ptr_;
    CButtonUI*                  btn_next_one_ptr_;

    CHorizontalLayoutUI*        lay_bottom_ptr_;
    CButtonUI*                  btn_rotate_ptr_;
    CButtonUI*                  btn_download_ptr_;
    CButtonUI*                  btn_delete_ptr_;
    CButtonUI*                  btn_show_thumbnail_ptr_;
    CButtonUI*                  btn_hide_thumbnail_ptr_;

    CGifUI*                     gif_loading_ptr_;
};

#endif // PIC_VIEW_HPP