
#include "mainwindow.h"
#include "ui_mainwindow.h"

const unsigned char crlfcrlf[] = { '\r', '\n', '\r', '\n'};
const unsigned char lfcrlf[] = { '\n', '\r', '\n'};
const unsigned char jpgHdr[] = { '-',  '-',  'B',  'o',  'u',  'n',  'd',  'a',
                                 'r',  'y',  'S',  't',  'r',  'i',  'n',  'g',
                                 '\r', '\n', 'C',  'o',  'n',  't',  'e',  'n',
                                 't',  '-',  't',  'y',  'p',  'e',  ':',  ' ',
                                 'i',  'm',  'a',  'g',  'e',  '/',  'j',  'p',
                                 'e',  'g',  '\r', '\n', 'C',  'o',  'n',  't',
                                 'e',  'n',  't',  '-',  'L',  'e',  'n',  'g',
                                 't',  'h',  ':'
                               };

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);

    socket = new QTcpSocket(this);
    connect(socket, SIGNAL(readyRead()), SLOT(readTcpData()));

    state = eStateInit;
    socket->connectToHost("10.0.0.203", 8081);

    count = 0;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readTcpData(void) {

    QByteArray data = socket->readAll();

//    printf("received %d\n", data.count());

    if (0 /*data.count() > 10000*/) {
        proto((unsigned char*)(data.data()), 200);
        proto((unsigned char*)(data.data()) + 200, 1448);
        proto((unsigned char*)(data.data()) + 200 + 1448, 1448);
        proto((unsigned char*)(data.data()) + 200 + 1448 * 2, 1448);



    }

    proto((unsigned char*)(data.data()), data.count());
//    proto((unsigned char*)(data.data()) + 198, data.count() - 198);
}

bool MainWindow::seqCompare(
    const unsigned char *seq, unsigned int seqLen,
    const unsigned char *data, unsigned int dataLen,
    unsigned int *seqIdx, unsigned int *bufIdx) {

    unsigned int i;
    unsigned int j = *seqIdx;
//unsigned char ch_d;
//unsigned char ch_s;

    for (i = 0; (i < dataLen) && (j < seqLen); i++) {
//ch_d = data[i];
//ch_s = seq[j];

       if (seq[j] == data[i]) {
          j++;
       } else {
          j = 0;
       }
    }
    *bufIdx += i;
    if (j == seqLen) {
        return true;
    } else {
        *seqIdx = j;
        return false;
    }
}

void MainWindow::proto(unsigned char *buf, unsigned int bufSize) {

    unsigned int bufIdx = 0;
    bool         again = false;
    char         *endPtr;

    do {
        switch (state) {
        case eStateInit:
            seqIdx = 0;
            state = eStateHttpHdr;
            again = true;
            break;
        case eStateHttpHdr:
            if (seqCompare(crlfcrlf, sizeof(crlfcrlf), buf + bufIdx,
                           bufSize - bufIdx, &seqIdx, &bufIdx)) {
                state = eStateJpgHdr;
                seqIdx = 0;
                again = true;
            } else {
                again = false;
            }
            break;
        case eStateJpgHdr:
            if (seqCompare(jpgHdr, sizeof(jpgHdr), buf + bufIdx,
                           bufSize - bufIdx, &seqIdx, &bufIdx)) {
                state = eStateJpgLen;
                seqIdx = 0;
                jpgLenIdx = 0;
                again = true;
            } else {
                again = false;
            }
            break;
        case eStateJpgLen:
            while ((jpgLenIdx < sizeof(jpgLenBuf)) &&
                   (bufIdx < bufSize) &&
                   (buf[bufIdx] != '\r')) {
                jpgLenBuf[jpgLenIdx] = buf[bufIdx];
                jpgLenIdx++;
                bufIdx++;
            }
            if ((buf[bufIdx] == '\r')) {
                jpgLenBuf[jpgLenIdx] = '\0';
                jpgLen = strtoul(jpgLenBuf, &endPtr, 0);
                state = eStateJpgLenTermination;
                again = true;
            } else {
                again = false;
            }
            break;
        case eStateJpgLenTermination:
            if (seqCompare(lfcrlf, sizeof(lfcrlf), buf + bufIdx,
                           bufSize - bufIdx, &seqIdx, &bufIdx)) {
                state = eStateJpgData;
                seqIdx = 0;
                jpgIdx = 0;
                again = true;
            } else {
                again = false;
            }
            break;
        case eStateJpgData:
            while ((bufIdx < bufSize) &&
                   (jpgIdx < jpgLen) &&
                   (jpgIdx < sizeof(jpgBuf))) {
                jpgBuf[jpgIdx] = buf[bufIdx];
                jpgIdx++;
                bufIdx++;
            }
            if (jpgIdx == jpgLen) {
                if (count == 3) {
                    QPixmap pic;
                    pic.loadFromData(jpgBuf, jpgLen, "JPG");
                    QPixmap small;
                    small = pic.scaledToWidth(480);
                    ui->label->setPixmap(small);
                    count = 0;
                }
                count++;
                /* next jpg */
                state = eStateJpgHdr;
                again = true;
            } else {
                again = false;
            }
            break;
        default:
            break;
        }
    } while (again);

}

