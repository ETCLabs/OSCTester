#include "OSCTesterWidgets.h"
#include <QHBoxLayout>
#include <QColorDialog>
#include <QKeyEvent>

/***************************************************** BlobWidget ******************************************************/

static QList<int> ACCEPTED_KEYS =
{
    Qt::Key_0,
    Qt::Key_1,
    Qt::Key_2,
    Qt::Key_3,
    Qt::Key_4,
    Qt::Key_5,
    Qt::Key_6,
    Qt::Key_7,
    Qt::Key_8,
    Qt::Key_9,
    Qt::Key_A,
    Qt::Key_B,
    Qt::Key_C,
    Qt::Key_D,
    Qt::Key_E,
    Qt::Key_F,
    Qt::Key_Delete,
    Qt::Key_Backspace,
    Qt::Key_Space
};

BlobWidget::BlobWidget(QWidget *parent) : QLineEdit(parent)
{
    m_valid = true;
    setAutoFillBackground(true);
}

void BlobWidget::keyPressEvent(QKeyEvent * event)
{
    if(!ACCEPTED_KEYS.contains(event->key()))
    {
        event->setAccepted(false);
        return;
    }
    QLineEdit::keyPressEvent(event);
    if(event->key()!=Qt::Key_Backspace && event->key()!=Qt::Key_Delete)
        setText(fixupHex(text()));
}

QString BlobWidget::fixupHex(const QString &input)
{
    QString hex = input.toUpper();
    QString result;
    hex = hex.replace(QString(" "), QString());
    for(int i=0; i<hex.length(); i++)
    {
        result += hex.at(i);
        if(i%2 && i<hex.length()-1) result += QString(" ");
    }
    return result;
}

QByteArray BlobWidget::currentValue()
{
    QByteArray result;
    QString hex = text().replace(QString(" "), QString());
    for(int i=0; i<hex.length(); i+=2)
    {
        QString value = hex.at(i);
        if(i==hex.length()-1)
            break;
        else
            value.append(hex.at(i+1));
        result.append(static_cast<char>(value.toInt(Q_NULLPTR, 16)));
    }
    return result;
}


/***************************************************** MIDIWidget ******************************************************/

MIDIWidget::MIDIWidget(QWidget *parent) : QWidget(parent)
{
    m_port = 0;
    m_status = 0;
    m_data1 = 0;
    m_data2 = 0;

    QHBoxLayout *layout = new QHBoxLayout;
    for(int i=0; i<4; i++)
    {
        QSpinBox *s = new QSpinBox(this);
        m_spinboxes << s;
        s->setDisplayIntegerBase(16);
        s->setRange(0, 0xFF);
        layout->addWidget(s);
        connect(s, SIGNAL(valueChanged(int)), this, SLOT(spinBoxChanged()));
    }
    m_spinboxes[0]->setPrefix(tr("Port ID: "));
    m_spinboxes[1]->setPrefix(tr("Status: "));
    m_spinboxes[2]->setPrefix(tr("Data 1: "));
    m_spinboxes[3]->setPrefix(tr("Data 2: "));

    layout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(layout);
    setAutoFillBackground(true);
}

void MIDIWidget::spinBoxChanged()
{
    m_port      = 0xFF & m_spinboxes[0]->value();
    m_status    = 0xFF & m_spinboxes[1]->value();
    m_data1     = 0xFF & m_spinboxes[2]->value();
    m_data2     = 0xFF & m_spinboxes[3]->value();
    emit valueChanged();
}

quint32 MIDIWidget::value()
{
    quint32 result = 0;
    result |= static_cast<quint8>(m_port << 24);
    result |= static_cast<quint8>(m_status << 16);
    result |= static_cast<quint8>(m_data1 << 8);
    result |= m_data2;

    return result;
}

/***************************************************** RGBAWidget ******************************************************/

RGBAWidget::RGBAWidget(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout;
    m_chip = new QWidget(this);
    m_chip->setMinimumWidth(50);
    layout->addWidget(m_chip);

    m_redSpin = new QSpinBox(this);
    m_redSpin->setRange(0, 255);
    m_redSpin->setPrefix(tr("Red :"));
    connect(m_redSpin, SIGNAL(valueChanged(int)), this, SLOT(setRedValue(int)));
    layout->addWidget(m_redSpin);

    m_greenSpin = new QSpinBox(this);
    m_greenSpin->setRange(0, 255);
    m_greenSpin->setPrefix(tr("Green :"));
    connect(m_greenSpin, SIGNAL(valueChanged(int)), this, SLOT(setGreenValue(int)));
    layout->addWidget(m_greenSpin);

    m_blueSpin = new QSpinBox(this);
    m_blueSpin->setRange(0, 255);
    m_blueSpin->setPrefix(tr("Blue :"));
    connect(m_blueSpin, SIGNAL(valueChanged(int)), this, SLOT(setBlueValue(int)));
    layout->addWidget(m_blueSpin);

    m_alphaSpin = new QSpinBox(this);
    m_alphaSpin->setRange(0, 255);
    m_alphaSpin->setPrefix(tr("Alpha :"));
    connect(m_alphaSpin, SIGNAL(valueChanged(int)), this, SLOT(setAlphaValue(int)));
    layout->addWidget(m_alphaSpin);

    m_editButton = new QPushButton(this);
    m_editButton->setText("...");
    connect(m_editButton, SIGNAL(pressed()), this, SLOT(openEditor()));
    layout->addWidget(m_editButton);

    layout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(layout);
    setAutoFillBackground(true);
}


void RGBAWidget::openEditor()
{
    QColorDialog dialog;
    dialog.setOption(QColorDialog::ShowAlphaChannel, true);
    dialog.setCurrentColor(m_value);
    if(dialog.exec() == QDialog::Accepted)
    {
        m_value = dialog.currentColor();
        m_chip->setStyleSheet(QString("background-color : %1").arg(m_value.name(QColor::HexArgb)));
        blockSignals(true);
        m_redSpin->setValue(m_value.red());
        m_greenSpin->setValue(m_value.green());
        m_blueSpin->setValue(m_value.blue());
        m_alphaSpin->setValue(m_value.alpha());
        blockSignals(false);

        emit valueChanged();
    }
}

void RGBAWidget::setRedValue(int red)
{
    m_value.setRed(red);
    m_chip->setStyleSheet(QString("background-color : %1").arg(m_value.name(QColor::HexArgb)));
    emit valueChanged();
}

void RGBAWidget::setBlueValue(int blue)
{
    m_value.setBlue(blue);
    m_chip->setStyleSheet(QString("background-color : %1").arg(m_value.name(QColor::HexArgb)));
    emit valueChanged();
}

void RGBAWidget::setGreenValue(int green)
{
    m_value.setGreen(green);
    m_chip->setStyleSheet(QString("background-color : %1").arg(m_value.name(QColor::HexArgb)));
    emit valueChanged();
}

void RGBAWidget::setAlphaValue(int alpha)
{
    m_value.setAlpha(alpha);
    m_chip->setStyleSheet(QString("background-color : %1").arg(m_value.name(QColor::HexArgb)));
    emit valueChanged();
}
