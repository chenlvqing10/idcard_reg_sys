#ifndef SOCKETTHR_H
#define SOCKETTHR_H
#include <QThread>
#include <QTcpSocket>
#include <QHostAddress>
#include <QtEndian> //提供大小端转换的函数
#include <QPixmap>
#include <QByteArray>
#include <qlibrary.h>
#include <QDir>
#include "SynReader.h"
#define __IPV4
#define SERVERPORT  66691
#define __FCU2302__ARM64

//定义结构体
typedef struct IDCardData {
    char name[30];                 //姓名          30字节  0-29
    char gender[4];                //性别          4字节   30-33
    char national[14];              //民族          12字节   34-37
    char birthday[16];             //生日          16字节  38-53
    char address[70];              //地址          70字节  54-123
    char idnumber[36];             //身份证号      36字节   124-159
    char maker[50];                //签证机关      30字节   160-189
    char start_date[16];           //有效起始期限  16字节    190-205
    char end_date[16];             //有效结束期限  16字节    206-221
    char reserved[36];             //保留字节      36字节   222-258
} St_IDCardData;


class SocketThr : public QThread
{
    Q_OBJECT
public:
    explicit SocketThr(QObject *parent = nullptr):
        QThread(parent){}
    ~SocketThr();

signals:
    void sigErr(const QString);
    void sigInfo(const QString);
    void sigShowIdCard(void);
    void sigShowimage(const char*);
    void sigPic(const QPixmap);

public slots:
    void setIP(const QString &ipstr);
    void exitthr();
private:
    void run(){
        qDebug("-------------start----------------");
        //定义一个流式套接字对象
        QTcpSocket s;
        //发起连接服务器
        static int count = 0;
         qDebug() << "server ip:" << ipstr << Qt::endl;
        s.connectToHost(QHostAddress(ipstr), (quint16)SERVERPORT);
        //阻塞等连接成功
        while(!s.waitForConnected(5000)){
            count++;
            qDebug() << "server connect error is:" << s.error() << Qt::endl;
            //触发自定义信号，参数是错误信息：连接失败
            s.connectToHost(QHostAddress(ipstr), (quint16)SERVERPORT);
            if(count>=3)
            {
                emit sigErr("连接失败："+s.errorString());
                isRun = false;
                return;
            }
        }

        //触发自定义信号：参数连接成功信息
        emit sigInfo("连接成功");
        isRun = true;

        while(isRun){
            //从缓冲区读数据
            //阻塞等待接收缓存是否有数据可读
            if(!s.waitForReadyRead(~0)){
                emit sigErr("连接断开："+s.errorString());
                goto ERR_STEP;
            }

            int len_info = sizeof(idcard_st);
            char revbuf[1024] = {0};
            s.read(revbuf,len_info);
            //for(int i=0;i<len_info;i++)
              //  qDebug() << "revbuf[" << i << "]:" << revbuf[i] << Qt::endl;
            //将接收到的字符转换为
            memcpy(&idcard_st,revbuf,len_info);
            qDebug() << "idnumber:" << idcard_st.idnumber << Qt::endl;


            if(!strcmp(old_idnumber,idcard_st.idnumber)) {
                 qDebug() << "same same same smae same same----------------------" << Qt::endl;
                 flag_same =true;
            }
            else {
                strcpy(old_idnumber , idcard_st.idnumber);
                flag_same = false;
            }

            qDebug() << "old_idnumber:" << old_idnumber << Qt::endl;


            emit sigShowIdCard();

#ifdef __FCU2302__ARM64    /* if the platform is ARM64,use the 1024byte image data because of having no 64_bits lib ,and using x64 lib for creating image file*/
        if(s.waitForReadyRead(~0)){
            char revbuf[1024] = {0};
            s.read(revbuf,1024);
            for(int i=0;i<1024;i++)
            {
                idcard_bitmap.append( revbuf[i]);
            }
            qDebug() << "idcard_bitmap:" << idcard_bitmap.toHex()<< Qt::endl;
            loadlib(lib_SynReader);
        }
#else
            int len = 0;
            if(s.waitForReadyRead(~0)){
                //接收图片信息
                //接收协议头:4字节整型，文件大小
                s.read(reinterpret_cast<char *>(&len), 4);
                //从网络字节序转为本地字节序
                len = qFromBigEndian(len);
                qDebug("文件大小：%d", len);
            }

            //定义一个数组，缓存大小是len字节，初值是0  1.bmp文件大小 75535字节  716KB
            QByteArray arr(len,0);
            //得到数组的首地址
            char *ptr = arr.data();
            int size = len;
            while(size){ //有一点收一点，直到收够
                //阻塞等待接收缓存是否有数据可读
                if(!s.waitForReadyRead(~0)){
                    emit sigErr("连接断开："+s.errorString());
                    goto ERR_STEP;
                }
                //从接收缓存读出size字节到ptr指向的缓存
                //实际读出的字节数为num  最多1KB=1024字节
                qint64 num = s.read(ptr, size);
                //计算剩余字节数
                size -= num;
                //调整缓存位置
                ptr += num;
                 qDebug() << "size:" << size << Qt::endl;
            }

            QPixmap pix;
            //把数组中的文件内容装到pix对象
            pix.loadFromData(arr);
            emit sigPic(pix);
#endif
        }//end isRun == true
        emit sigErr("线程退出");

ERR_STEP:
        isRun = false;
    }//end run
private:
    bool isRun;
    QString ipstr;
public:
    St_IDCardData idcard_st;
private:
    QByteArray idcard_bitmap;
    char old_idnumber[30];
    bool flag_same = false;
private:
    QLibrary *lib_SynReader = NULL;
    void loadlib(QLibrary *lib);
    int  getBmpCount(const char *path);
 };

#endif // SOCKETTHR_H
