#ifndef OSCTESTERWIDGETS_H
#define OSCTESTERWIDGETS_H

#include <QWidget>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QColor>
#include <QLineEdit>

class BlobWidget : public QLineEdit
{
    Q_OBJECT
public:
    BlobWidget(QWidget *parent = Q_NULLPTR);
    QByteArray currentValue();
protected:
    virtual void keyPressEvent(QKeyEvent * event);
private:
    QString fixupHex(const QString &input);
    bool m_valid;
};


class MIDIWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MIDIWidget(QWidget *parent = nullptr);
    quint32 value();
signals:
    void valueChanged();
private slots:
    void spinBoxChanged();
private:
    QList<QSpinBox *>m_spinboxes;
    quint8 m_port, m_status, m_data1, m_data2;
};

class RGBAWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RGBAWidget(QWidget *parent = nullptr);
    QColor getValue() const {return m_value;}
signals:
    void valueChanged();
private slots:
    void openEditor();
    void setRedValue(int red);
    void setBlueValue(int blue);
    void setGreenValue(int green);
    void setAlphaValue(int alpha);
private:
    QWidget *m_chip;
    QSpinBox *m_redSpin;
    QSpinBox *m_greenSpin;
    QSpinBox *m_blueSpin;
    QSpinBox *m_alphaSpin;
    QPushButton *m_editButton;
    QColor m_value;
};

#endif // OSCTESTERWIDGETS_H
