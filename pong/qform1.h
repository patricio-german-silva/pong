#ifndef QFORM1_H
#define QFORM1_H

#define LED_TOP 0
#define LED_BOTTOM 1
#define LED_LEFT 2
#define LED_RIGTH 3


#include <QMainWindow>
#include <QTimer>
#include <QtMath>
#include "qpaintbox.h"
#include "qserial.h"


namespace Ui {
class QForm1;
}

class QForm1 : public QMainWindow
{
    Q_OBJECT

public:
    explicit QForm1(QWidget *parent = nullptr);
    ~QForm1();

private slots:
    void OnQPaintBox1MouseMove(Qt::MouseButton button, int x, int y);

    void OnQPaintBox1MouseUp(Qt::MouseButton button, int x, int y);

    void OnQPaintBox1MouseDown(Qt::MouseButton button, int x, int y);

    void onQtimer1();

    void paintEvent(QPaintEvent *event);

    void resizeEvent(QResizeEvent *event);

    void on_pushButton_clicked();


    void on_spinBox_valueChanged(const QString &arg1);

    void on_spinBox_2_valueChanged(const QString &arg1);

    void on_doubleSpinBox_valueChanged(const QString &arg1);

    void on_doubleSpinBox_2_valueChanged(const QString &arg1);

    void on_spinBoxPosX_valueChanged(const QString &arg1);

    void on_spinBoxPosY_valueChanged(const QString &arg1);

    void on_doubleSpinBoxGrados_valueChanged(const QString &arg1);

    void on_doubleSpinBoxVel_valueChanged(const QString &arg1);

private:
    Ui::QForm1 *ui;
    QTimer *Qtimer1;
    QPaintBox *QPaintBox1;
    QSerial *QSerial1;
    QPen pen;
    QBrush brush;
    Qt::MouseButton buttonPressed;
    bool isSelected;
    Qt::GlobalColor ballColor;
    int ballRadius;
    int timerQuanto;
    int avance200ms;
    bool timerSleep;
    // Velocidad y posicion de la bola real
    qreal velx;
    qreal vely;
    qreal posx;
    qreal posy;
    qreal gravedad;
    qreal G;
    qreal perdida;
    qreal pixelsPorMetro;
    void ballMove(int x, int y, bool dot);
    void ballErase();
    void ballWrite(int x, int y, bool dot);
    void ballStop();
    void ballStart();
    void blinkLed(int);
    void leerCMD();
};
#endif // QFORM1_H
