#include "widget.h"
#include "ui_widget.h"
#include "socketthr.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    thr = new SocketThr();//create children thread object

    connect(thr, SIGNAL(sigInfo(const QString)),
            this, SLOT(slotInfo(const QString)));
    connect(thr, SIGNAL(sigErr(const QString)),
            this, SLOT(slotErr(const QString)));
    connect(thr, SIGNAL(sigPic(const QPixmap)),
            this, SLOT(slotPic(const QPixmap)));

    connect(thr, SIGNAL(sigShowIdCard(void)),
            this, SLOT(slotShowIdCard(void)));
    connect(thr, SIGNAL(sigShowimage(const char*)),
            this, SLOT(slotShowimage(const char*)));
   // connect( thr, SIGNAL(finished()), this, SLOT(deleteLater()) );
   // connect( thr, SIGNAL(finished()), thr, SLOT(deleteLater()) );
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_checkBox_toggled(bool checked)
{
    if(checked){
        ui->showLB->setScaledContents(true);
    }else{
        ui->showLB->setScaledContents(false);
    }
}

void Widget::on_pushButton_clicked()
{

    ui->pushButton->setEnabled(false);

    if("连接(&L)" == ui->pushButton->text()){
        ui->pushButton->setText("断开(&L)");
        //设置服务IP
        QString ipstr = ui->lineEdit->text();
        thr->setIP(ipstr);
        //开启线程
        thr->start();

    }else{
        ui->pushButton->setText("连接(&L)");
        thr->exitthr();
    }
}

void Widget::slotInfo(const QString text)
{
    setWindowTitle(text);
    ui->pushButton->setText("断开(&L)");
    ui->pushButton->setEnabled(true);
}

void Widget::slotErr(const QString text)
{
    setWindowTitle(text);
    ui->pushButton->setText("连接(&L)");
    ui->pushButton->setEnabled(true);
}

void Widget::slotPic(const QPixmap pix)
{
    ui->showLB->setPixmap(pix);
}

void Widget::slotShowIdCard()
{
    QString name(thr->idcard_st.name);
    ui->lb_name_show->setText(name);

    QString gender(thr->idcard_st.gender);
    ui->lb_gender_show->setText(gender);

    QString national(thr->idcard_st.national);
    ui->lb_national_show->setText(national);

    QString birthday(thr->idcard_st.birthday);
    ui->lb_birthday_show->setText(birthday);

    QString address(thr->idcard_st.address);
    ui->lb_address_show->setText(address);

    QString idnumber(thr->idcard_st.idnumber);
    ui->lb_idnumber_show->setText(idnumber);

    QString maker(thr->idcard_st.maker);
    ui->lb_maker_show->setText(maker);

    QString start_date(thr->idcard_st.start_date);
    ui->lb_startdate_show->setText(start_date);

    QString end_date(thr->idcard_st.end_date);
    ui->lb_enddate_show->setText(end_date);
}

void Widget::slotShowimage(const char* filename)
{
    QImage* img=new QImage;

    if(! ( img->load(filename) ) ) //加载图像
    {
        delete img;
        return;
    }
     qDebug() << "aaaa" << Qt::endl;

    ui->showLB->setPixmap(QPixmap::fromImage(*img));
}
