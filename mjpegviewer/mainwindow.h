#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include <QTcpSocket>
#include <QPixmap>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void readTcpData(void);

private:
    bool seqCompare(const unsigned char *seq, unsigned int seq_len,
                    const unsigned char *data, unsigned int data_len,
                    unsigned int *seq_idx, unsigned int *buf_idx);
    void proto(unsigned char *buf, unsigned int bufSize);

    Ui::MainWindow *ui;
    QTcpSocket *socket;

    enum {
        eStateInit,
        eStateHttpHdr,
        eStateJpgHdr,
        eStateJpgLen,
        eStateJpgLenTermination,
        eStateJpgData,
        eStateJpgReady
    } state;

    unsigned int seqIdx;

    char          jpgLenBuf[20];
    unsigned int  jpgLenIdx;
    unsigned int  jpgLen;

    unsigned char jpgBuf[100000];
    unsigned int  jpgIdx;

    int count;

};

#endif // MAINWINDOW_H
