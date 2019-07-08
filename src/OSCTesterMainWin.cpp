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

#include "OSCTesterMainWin.h"
#include "ui_OSCTesterMainWin.h"
#include "OSCParser.h"
#include <QMessageBox>
#include <limits>
#include <QDebug>
#include <QNetworkDatagram>
#include <QDateTimeEdit>
#include <QDateTime>
#include <QSettings>
#include "OSCTesterWidgets.h"


qint64 qDateTimeToOSCTime(const QDateTime &time)
{
    qint64 result = 0;
    QDateTime epoch;
    epoch.setDate(QDate(1900, 1, 1));
    epoch.setTime(QTime(0, 0, 0));
    quint32 firstPart = static_cast<quint32>(epoch.secsTo(time));
    result |= (firstPart << 6);

    // TODO handle sub-second precision
    return result;

}

QString oscTypeToString(const int type)
{
    switch(type)
    {
    case OSCArgument::OSC_TYPE_INVALID:
        return QString("Invalid");
    case OSCArgument::OSC_TYPE_CHAR:
        return QString("Char");
    case OSCArgument::OSC_TYPE_INT32:
        return QString("Int32");
    case OSCArgument::OSC_TYPE_INT64:
        return QString("Int64");
    case OSCArgument::OSC_TYPE_FLOAT32:
        return QString("Float32");
    case OSCArgument::OSC_TYPE_FLOAT64:
        return QString("Flot64");
    case OSCArgument::OSC_TYPE_STRING:
        return QString("String");
    case OSCArgument::OSC_TYPE_BLOB:
        return QString("Blob");
    case OSCArgument::OSC_TYPE_TIME:
        return QString("Time");
    case OSCArgument::OSC_TYPE_RGBA32:
        return QString("RGBA32");
    case OSCArgument::OSC_TYPE_MIDI:
        return QString("MIDI");
    case OSCArgument::OSC_TYPE_TRUE:
        return QString("Bool (True)");
    case OSCArgument::OSC_TYPE_FALSE:
        return QString("Bool (False)");
    case OSCArgument::OSC_TYPE_NULL:
        return QString("Null");
    case OSCArgument::OSC_TYPE_INFINITY:
        return QString("Infinity");
    }

    return QString("Unknown (%1)").arg(type);
}

QString prettifyHex(const QByteArray &data)
{
    QString result;
    for(int line=0; line<data.length(); line+=16)
    {
        QString text = QString("%1 ").arg(line, 4, 16, QChar('0')).toUpper();
        for(int i=0; i<16; i++)
            if(line+i<data.length())
                text.append(QString(" %1").arg(static_cast<unsigned char>( data[line+i]), 2, 16, QChar('0')).toUpper());

        // Fill out the last line
        if(text.length() < 52)
            text += QString(53 - text.length(), ' ');

        text.append("  ");

        for(int i=0; i<16; i++)
            if(line+i<data.length())
            {
                QChar c(data[line+i]);
                if(c.isPrint())
                    text.append(QString("%1").arg(c));
                else
                    text.append(QString("."));
            }

        text += "\r\n";
        result += text;
    }
    return result;
}

OSCTesterMainWin::OSCTesterMainWin(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::OSCTesterMainWin),
    m_tcpStream(OSCStream::FRAME_MODE_DEFAULT)
{
    ui->setupUi(this);

    // Load Saved Settings
    QSettings s;

    ui->leTargetIp->setText(s.value("ip", QVariant("10.101.10.10")).toString());
    ui->sbPort->setValue(s.value("port", QVariant(3031)).toInt());

    connect(ui->leOscPath, SIGNAL(textChanged(const QString &)), this, SLOT(updateOscPacket()));
    connect(ui->cbSlip, SIGNAL(toggled(bool)), this, SLOT(updateOscPacket()));
    connect(ui->twDataFields, SIGNAL(itemChanged(QTableWidgetItem *)), this, SLOT(updateOscPacket()));

}

OSCTesterMainWin::~OSCTesterMainWin()
{
    // Save Settings
    QSettings s;
    s.setValue("ip", QVariant(ui->leTargetIp->text()));
    s.setValue("port", QVariant(ui->sbPort->value()));

    delete ui;
}

void OSCTesterMainWin::updateOscPacket()
{
    OSCPacketWriter writer(ui->leOscPath->text().toStdString());

    int row = 0;
    foreach(QComboBox *combo, m_argumentCombos)
    {
        OSCArgument::EnumArgumentTypes type = static_cast<OSCArgument::EnumArgumentTypes>(combo->currentIndex()+1);
        switch(type)
        {
        case OSCArgument::OSC_TYPE_CHAR:
        {
            QSpinBox *spin = dynamic_cast<QSpinBox *>(ui->twDataFields->cellWidget(row, 1));
            writer.AddChar(static_cast<char>(spin->value()));
        }
            break;
        case OSCArgument::OSC_TYPE_INT32:
        {
            QSpinBox *spin = dynamic_cast<QSpinBox *>(ui->twDataFields->cellWidget(row, 1));
            writer.AddInt32(static_cast<qint32>(spin->value()));
        }
            break;
        case OSCArgument::OSC_TYPE_INT64:
        {
            QSpinBox *spin = dynamic_cast<QSpinBox *>(ui->twDataFields->cellWidget(row, 1));
            writer.AddInt64(static_cast<qint64>(spin->value()));
        }
            break;
        case OSCArgument::OSC_TYPE_FLOAT32:
        {
            QDoubleSpinBox *spin = dynamic_cast<QDoubleSpinBox *>(ui->twDataFields->cellWidget(row, 1));
            writer.AddFloat32(spin->value());
        }
            break;
        case OSCArgument::OSC_TYPE_FLOAT64:
        {
            QDoubleSpinBox *spin = dynamic_cast<QDoubleSpinBox *>(ui->twDataFields->cellWidget(row, 1));
            writer.AddFloat64(spin->value());
        }
            break;
        case OSCArgument::OSC_TYPE_STRING:
        {
            QString value = ui->twDataFields->item(row, 1)->text();
            writer.AddString(value.toStdString());
        }
            break;
        case OSCArgument::OSC_TYPE_BLOB:			// b
        {
            BlobWidget *edit = dynamic_cast<BlobWidget *>(ui->twDataFields->cellWidget(row, 1));
            writer.AddBlob(edit->currentValue().constData(), edit->currentValue().size());
        }
            break;
        case OSCArgument::OSC_TYPE_TIME:			// t (OSC-timetag)
        {
            QDateTimeEdit *edit = dynamic_cast<QDateTimeEdit *>(ui->twDataFields->cellWidget(row, 1));
            writer.AddTime(qDateTimeToOSCTime(edit->dateTime()));
        }
            break;
        case OSCArgument::OSC_TYPE_RGBA32:		// r
        {
            RGBAWidget *edit = dynamic_cast<RGBAWidget *>(ui->twDataFields->cellWidget(row, 1));
            QColor color = edit->getValue();
            OSCArgument::sRGBA value;
            value.r = static_cast<quint8>(color.red());
            value.g = static_cast<quint8>(color.green());
            value.b = static_cast<quint8>(color.blue());
            value.a = static_cast<quint8>(color.alpha());
            writer.AddRGBA(value);
        }
            break;
        case OSCArgument::OSC_TYPE_MIDI:		// m (4 byte MIDI message. Bytes from MSB to LSB are: port id, status byte, data1, data2)
        {
            MIDIWidget *edit = dynamic_cast<MIDIWidget *>(ui->twDataFields->cellWidget(row, 1));
            int32_t value = edit->value();
            writer.AddMidi(value);
        }
            break;
        case OSCArgument::OSC_TYPE_TRUE:			// T (True, 0 bytes)
            writer.AddTrue();
            break;
        case OSCArgument::OSC_TYPE_FALSE:			// F (False, 0 bytes)
            writer.AddFalse();
            break;
        case OSCArgument::OSC_TYPE_NULL:			// N (Null, 0 bytes
            writer.AddNull();
            break;
        case OSCArgument::OSC_TYPE_INFINITY:
            writer.AddInfinity();
            break;
        }
        row++;
    }


    size_t packetSize = writer.ComputeSize();
    m_command = QByteArray(packetSize, '0');

    writer.Write(m_command.data(), packetSize);


    if(ui->cbSlip->isChecked())
    {
        OSCStream stream(OSCStream::FRAME_MODE_DEFAULT);
        stream.Add(m_command.constData(), m_command.length());

        size_t size = m_command.length();
        char *packedData = stream.CreateFrame(m_command.data(), size);
        m_command = QByteArray(packedData, size);
    }

    ui->teDataToSend->setPlainText(prettifyHex(m_command));
}


void OSCTesterMainWin::on_coMode_currentIndexChanged(int index)
{
    // By default want SLIP for TCP not UDP
    ui->cbSlip->setChecked(index==0);
}

void OSCTesterMainWin::on_btnConnect_pressed()
{
    if(ui->coMode->currentIndex()==0)
    {
        // TCP
        m_socket = new QTcpSocket(this);
        m_socket->connectToHost(ui->leTargetIp->text(), ui->sbPort->value());
        connect(m_socket, &QIODevice::readyRead, this, &OSCTesterMainWin::tcpReadyRead);
        connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &OSCTesterMainWin::tcpError);
        connect(m_socket, SIGNAL(connected()), this, SLOT(tcpSocketConnected()));
        connect(m_socket, SIGNAL(disconnected()), this, SLOT(tcpSocketDisconnected()));
        ui->btnConnect->setEnabled(false);

        ui->lblConnStatus->setText("Connecting...");
    }
    else {
        // UDP
        m_socket = new QUdpSocket(this);
        bool ok = m_socket->bind(QHostAddress::Any, ui->sbPort->value());
        if(!ok)
        {
            QMessageBox::warning(this, tr("Couldn't Bind"), tr("Unable to bind UDP socket"));
            return;
        }
        m_socket->connectToHost(ui->leTargetIp->text(), ui->sbPort->value());
        connect(m_socket, SIGNAL(readyRead()), this, SLOT(udpReadyRead()));
        ui->btnConnect->setEnabled(false);
    }
}


void OSCTesterMainWin::tcpReadyRead()
{
    QTcpSocket *socket = static_cast<QTcpSocket *>(m_socket);
    QByteArray data = socket->readAll();

    ui->teDataRx->setPlainText(prettifyHex(data));

    m_tcpStream.Add(data.constData(), data.length());

    size_t size = OSCStream::DEFAULT_MAX_FRAME_SIZE;
    char *decodedData = m_tcpStream.GetNextFrame(size);
    while(decodedData)
    {
        handleOscIn(decodedData, size);
        decodedData = m_tcpStream.GetNextFrame(size);
    }
}

void OSCTesterMainWin::udpReadyRead()
{
    QUdpSocket *socket = static_cast<QUdpSocket *>(m_socket);

    while(socket->hasPendingDatagrams())
    {
        QNetworkDatagram datagram = socket->receiveDatagram();
        QByteArray data = datagram.data();
        handleOscIn(data.data(), data.length());
    }
}

void OSCTesterMainWin::handleOscIn(char *data, size_t size)
{
    OSCPacketReader reader;
    reader.Parse(data, size);
    if(reader.IsValid())
        ui->swDecodedOsc->setCurrentIndex(0);
    else {
        // Not valid OSC
        ui->swDecodedOsc->setCurrentIndex(1);
        return;
    }

    ui->leRxOscPath->setText(QString::fromStdString(reader.GetPath()));

    ui->twParsedOsc->setRowCount(reader.ArgumentCount());
    for(int i=0; i<reader.ArgumentCount(); i++)
    {
        OSCArgument a = reader.GetArgument(i);
        std::string s;
        a.GetString(s);
        QString desc = QString::fromStdString(s);
        ui->twParsedOsc->setItem(i, 0, new QTableWidgetItem(oscTypeToString(a.GetType())));
        ui->twParsedOsc->setItem(i, 1, new QTableWidgetItem(desc));
    }

    QString historyString = QDateTime::currentDateTime().toString();
    historyString.append(" - ");
    historyString.append(QString::fromStdString(reader.GetPath()));
    ui->lwHistory->addItem(historyString);

}


void OSCTesterMainWin::tcpError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    QTcpSocket *socket = static_cast<QTcpSocket *>(m_socket);
    QMessageBox::warning(this, tr("TCP Socket Error"),
                         tr("Couldn't connect to %1, error %2").arg(ui->leTargetIp->text(), socket->errorString()));
    ui->btnConnect->setEnabled(true);
    ui->lblConnStatus->setText("Not Connected");
}

void OSCTesterMainWin::on_btnSend_pressed()
{
    if(m_socket)
    {
        m_socket->write(m_command);
    }
}


void OSCTesterMainWin::on_btnAddOscData_pressed()
{
    ui->twDataFields->blockSignals(true);
    ui->twDataFields->setRowCount(ui->twDataFields->rowCount()+1);

    QComboBox *combo = new QComboBox(ui->twDataFields);
    for(int i=1; i<OSC_TYPE_NAMES.count(); i++) // Omit INVALID value
        combo->addItem(OSC_TYPE_NAMES.at(i), QVariant(i));

    connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(argumentComboChanged(int)));

    ui->twDataFields->setCellWidget(ui->twDataFields->rowCount()-1, 0, combo);
    m_argumentCombos << combo;

    ui->twDataFields->setItem(ui->twDataFields->rowCount()-1, 1, new QTableWidgetItem());

    combo->setCurrentIndex(OSCArgument::OSC_TYPE_STRING - 1);

    ui->twDataFields->blockSignals(false);

    updateOscPacket();
}

void OSCTesterMainWin::on_btnDelOscData_pressed()
{
    int row = ui->twDataFields->currentRow();
    if(row<0) return;
    ui->twDataFields->removeRow(row);
    m_argumentCombos.removeAt(row);
    updateOscPacket();
}

void OSCTesterMainWin::argumentComboChanged(int index)
{
    QComboBox *combo = dynamic_cast<QComboBox*>(sender());
    if(!combo) return;

    auto listPos = m_argumentCombos.indexOf(combo);
    if(listPos<0) return;

    // Set up appropriate editors
    switch(index+1)
    {
    case OSCArgument::OSC_TYPE_CHAR:
    {
        QSpinBox *sb = new QSpinBox(ui->twDataFields);
        sb->setMinimum(0);
        sb->setMaximum(255);
        ui->twDataFields->setCellWidget(listPos, 1, sb);
        connect(sb, SIGNAL(valueChanged(int)), this, SLOT(updateOscPacket()));
    }
        break;
    case OSCArgument::OSC_TYPE_INT32:
    {
        QSpinBox *sb = new QSpinBox(ui->twDataFields);
        sb->setMinimum(0);
        sb->setMaximum(std::numeric_limits<int>::max());
        ui->twDataFields->setCellWidget(listPos, 1, sb);
        connect(sb, SIGNAL(valueChanged(int)), this, SLOT(updateOscPacket()));
    }
        break;
    case OSCArgument::OSC_TYPE_INT64:
    {
        QSpinBox *sb = new QSpinBox(ui->twDataFields);
        sb->setMinimum(0);
        sb->setMaximum(std::numeric_limits<int>::max()); // TODO: QSpinBox may not work for values >32bit max
        ui->twDataFields->setCellWidget(listPos, 1, sb);
       connect(sb, SIGNAL(valueChanged(int)), this, SLOT(updateOscPacket()));
    }
        break;
    case OSCArgument::OSC_TYPE_FLOAT32:
    {
        QDoubleSpinBox *sb = new QDoubleSpinBox(ui->twDataFields);
        sb->setMinimum(0);
        ui->twDataFields->setCellWidget(listPos, 1, sb);
        connect(sb, SIGNAL(valueChanged(double)), this, SLOT(updateOscPacket()));
    }
        break;
    case OSCArgument::OSC_TYPE_FLOAT64:
    {
        QDoubleSpinBox *sb = new QDoubleSpinBox(ui->twDataFields);
        sb->setMinimum(0);
        ui->twDataFields->setCellWidget(listPos, 1, sb);
        connect(sb, SIGNAL(valueChanged(double)), this, SLOT(updateOscPacket()));
    }
        break;
    case OSCArgument::OSC_TYPE_STRING:
    {
        delete ui->twDataFields->cellWidget(listPos, 1);
    }
        break;
    case OSCArgument::OSC_TYPE_BLOB:
    {
        BlobWidget *w = new BlobWidget(ui->twDataFields);
        ui->twDataFields->setCellWidget(listPos, 1, w);
        connect(w, SIGNAL(textChanged(QString)), this, SLOT(updateOscPacket()));
    }
        break;
    case OSCArgument::OSC_TYPE_TIME:
    {
        QDateTimeEdit *dt = new QDateTimeEdit(ui->twDataFields);
        ui->twDataFields->setCellWidget(listPos, 1, dt);
        connect(dt, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(updateOscPacket()));
    }
        break;
    case OSCArgument::OSC_TYPE_RGBA32:
    {
        RGBAWidget *w = new RGBAWidget(ui->twDataFields);
        ui->twDataFields->setCellWidget(listPos, 1, w);
        connect(w, SIGNAL(valueChanged()), this, SLOT(updateOscPacket()));
    }
        break;
    case OSCArgument::OSC_TYPE_MIDI:
    {
        MIDIWidget *w = new MIDIWidget(ui->twDataFields);
        ui->twDataFields->setCellWidget(listPos, 1, w);
        connect(w, SIGNAL(valueChanged()), this, SLOT(updateOscPacket()));
    }
        break;
    case OSCArgument::OSC_TYPE_TRUE:
    {
        delete ui->twDataFields->cellWidget(listPos, 1);
        QTableWidgetItem *item = new QTableWidgetItem();
        item->setText(tr("TRUE"));
        item->setFlags(Qt::ItemIsEnabled);
        item->setTextAlignment(Qt::AlignHCenter);
        ui->twDataFields->setItem(listPos, 1, item);
    }
        break;
    case OSCArgument::OSC_TYPE_FALSE:
    {
        delete ui->twDataFields->cellWidget(listPos, 1);
        QTableWidgetItem *item = new QTableWidgetItem();
        item->setText(tr("FALSE"));
        item->setFlags(Qt::ItemIsEnabled);
        item->setTextAlignment(Qt::AlignHCenter);
        ui->twDataFields->setItem(listPos, 1, item);
    }
        break;
    case OSCArgument::OSC_TYPE_NULL:
    {
        delete ui->twDataFields->cellWidget(listPos, 1);
        QTableWidgetItem *item = new QTableWidgetItem();
        item->setText(tr("NULL"));
        item->setFlags(Qt::ItemIsEnabled);
        item->setTextAlignment(Qt::AlignHCenter);
        ui->twDataFields->setItem(listPos, 1, item);
    }
        break;
    case OSCArgument::OSC_TYPE_INFINITY:
    {
        delete ui->twDataFields->cellWidget(listPos, 1);
        QTableWidgetItem *item = new QTableWidgetItem();
        item->setText(tr("INFINITY"));
        item->setFlags(Qt::ItemIsEnabled);
        item->setTextAlignment(Qt::AlignHCenter);
        ui->twDataFields->setItem(listPos, 1, item);
    }
        break;
    default:
        break;
    }
}


void OSCTesterMainWin::tcpSocketConnected()
{
    ui->lblConnStatus->setText(tr("Connected"));
}

void OSCTesterMainWin::tcpSocketDisconnected()
{
    ui->lblConnStatus->setText(tr("Disconnected"));
    ui->btnConnect->setEnabled(true);
}
