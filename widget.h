#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "socketthr.h"
#include "SynReader.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_checkBox_toggled(bool checked);
    void on_pushButton_clicked();
    void slotInfo(const QString);
    void slotErr(const QString);
    void slotPic(const QPixmap);
    void slotShowIdCard(void);
    void slotShowimage(const char*);

private:
    Ui::Widget *ui;
    SocketThr *thr;
};

#endif // WIDGET_H
