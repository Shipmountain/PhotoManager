#ifndef MSG_DEF_H
#define MSG_DEF_H

enum EWindowMsg
{
    EMsgReloadData = WM_USER + 1000,    //���¼�������
    EMsgExchangeDataMode,               //�л�����ģʽ
    EMsgRefreshResult,                  //ˢ�½��
    EMsgRefreshThumbnail,               //ˢ������ͼ
    EMsgRefreshList,                    //ˢ���б�
    EMsgRefreshDateList,                //ˢ��ʱ���б�
    EMsgOpenFile,                       //���ļ�
    EMsgPathForward,                    //ǰ��
    EMsgPathBack,                       //����
    EMsgRefresh,                        //ˢ��
    EMsgGotoPath,                       //��ת��ָ��·��
    EMsgShowDirFolderMenu,              //��ʾĿ¼�µ��ļ��в˵�
    EMsgInitPageItem,                   //��ʼ����ҳ��
    EMsgGotoPage,                       //��ת��ָ��ҳ
    EMsgShowSettingWin,                 //��ʾ���ô���
    EMsgModuleOptionChange,             //ѡ��ģ��ı�
    EMsgRefreshThumbnailImage,          //ˢ������ͼͼƬ
    EMsgShowPhoneListMenu,              //��ʾ�ֻ��б�˵�
    EMsgRefreshPhoneDevice,             //ˢ���ֻ��豸
    EMsgShowPic,                        //��ʾͼƬ
    EMsgShowMainRightMenu,              //��ʾ�������Ҽ��˵�
    EMsgAddPhoneSyncFile,               //����ֻ�ͬ���ļ�
    EMsgDelPhoneSyncFile,               //ȡ���ֻ�ͬ���ļ�
    EMsgAddPhoneSyncFileResult,         //����ֻ�ͬ���ļ����
    EMsgDelPhoneSyncFileResult,         //ȡ���ֻ�ͬ���ļ����
    EMsgLocationFile,                   //��λ�ļ�
    EMsgRefreshViewBigImage,            //��ʾͼƬ
    EMsgRefreshViewImageList,           //��ʾͼƬ�б�
    EMsgRefreshViewThumbnail,           //ˢ����ʾ����ͼ
    EMsgDeleteFile,                     //ɾ���ļ�
    EMsgRenameFile,                     //�������ļ�
};

struct ViewThumbnailParam 
{
    ViewThumbnailParam(int index, void* image, void* dark_image):
        index_(index), image_(image), dark_image_(dark_image){}

    int     index_;
    void*   image_;
    void*   dark_image_;
};

#endif //MSG_DEF_H


