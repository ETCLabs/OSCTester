// Copyright (c) 2019 Electronic Theatre Controls, Inc., http://www.etcconnect.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef PARADIGMOSCTESTER_H
#define PARADIGMOSCTESTER_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QUdpSocket>
#include "OSCParser.h"

class QComboBox;

namespace Ui {
class OSCTesterMainWin;
}

static QStringList OSC_TYPE_NAMES {
    "Invalid", "char", "int32", "int64", "float32", "float64", "string", "blob", "time", "rgba32", "midi", "true", "false", "null", "infinity" };

class OSCTesterMainWin : public QMainWindow
{
    Q_OBJECT

public:
    explicit OSCTesterMainWin(QWidget *parent = nullptr);
    ~OSCTesterMainWin();
private slots:
    void updateOscPacket();
    void on_btnConnect_pressed();
    void on_btnSend_pressed();
    void on_btnAddOscData_pressed();
    void on_btnDelOscData_pressed();
    void on_coMode_currentIndexChanged(int index);
    void argumentComboChanged(int index);

    // TCP
    void tcpSocketConnected();
    void tcpSocketDisconnected();
    void tcpReadyRead();
    void tcpError(QAbstractSocket::SocketError error);

    // UDP
    void udpReadyRead();

private:
    Ui::OSCTesterMainWin *ui;
    QAbstractSocket *m_socket = Q_NULLPTR;
    QByteArray m_command;
    QList <QComboBox *> m_argumentCombos;
    OSCStream m_tcpStream;

    void handleOscIn(char *data, size_t size);

};

#endif // PARADIGMOSCTESTER_H
