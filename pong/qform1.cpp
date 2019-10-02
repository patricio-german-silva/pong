#include "qform1.h"
#include "ui_qform1.h"

QForm1::QForm1(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QForm1)
{
    ui->setupUi(this);

    QPaintBox1 = new QPaintBox(0, 0, ui->frame);
    QSerial1 = new QSerial("ttyACM0", 9600, true, true);
    QSerial1->open();

    connect(QPaintBox1, &QPaintBox::paintBoxMouseMove, this, &QForm1::OnQPaintBox1MouseMove);
    connect(QPaintBox1, &QPaintBox::paintBoxMouseUp, this, &QForm1::OnQPaintBox1MouseUp);
    connect(QPaintBox1, &QPaintBox::paintBoxMouseDown, this, &QForm1::OnQPaintBox1MouseDown);

    Qtimer1 = new QTimer(this);

    connect(Qtimer1, &QTimer::timeout, this, &QForm1::onQtimer1);

    // intervalo de ejecución del Timer
    timerQuanto = 25;
    /* El color de la bola */
    ballColor = Qt::red;
    // cantidad de pixeles que representan un metro físico
    pixelsPorMetro = 100;
    // Posicion actual de la bola
    posx = 0;
    posy = 0;
    // Velocidad actual de la bola
    velx = 0.0;
    vely = 0.0;
    // Aceleraciravedad Inicial
    gravedad = 9.8;
    // Aceleracion ajustada a la escala
    G = gravedad * pixelsPorMetro;
    perdida = 0.25;
    ballRadius = 40;
    timerSleep = false;
    // Pixels cada 200ms
    avance200ms = 10;

    buttonPressed = Qt::NoButton;

    /* True si se presionó el boton derecho sobre la bola */
    isSelected = false;

    Qtimer1->start(timerQuanto);
    //El tamaño minimo de frame es el tamaño de la bola
    ui->frame->setMinimumSize(QSize(2*QForm1::ballRadius, 2*QForm1::ballRadius));
}

QForm1::~QForm1()
{
    QSerial1->close();
    delete QSerial1;
    delete ui;
}

void QForm1::OnQPaintBox1MouseMove(Qt::MouseButton button, int x, int y){
    if(QForm1::isSelected){
        QForm1::statusBar()->showMessage("Dragged " + QString().number(QForm1::buttonPressed) + ", on the ball!!");
        // El mouse se mantiene en el medio de la bola
        QForm1::ballMove(x - QForm1::ballRadius, y - QForm1::ballRadius, false);
    }else {
        QForm1::statusBar()->showMessage("Dragged " + QString().number(QForm1::buttonPressed));
    }
}

void QForm1::OnQPaintBox1MouseDown(Qt::MouseButton button, int x, int y){
    QForm1::buttonPressed = button;
    QForm1::statusBar()->showMessage("Pressed " + QString().number(QForm1::buttonPressed));
    if(QForm1::buttonPressed == Qt::LeftButton){
        if(QPaintBox1->getCanvas()->toImage().pixelColor(x, y) == QForm1::ballColor){
            QForm1::isSelected = true;
            QForm1::statusBar()->showMessage("Pressed " + QString().number(QForm1::buttonPressed) + ", on the ball!!");
        }
    } else {
            QForm1::isSelected = false;
            QForm1::statusBar()->showMessage("Pressed " + QString().number(QForm1::buttonPressed));
    }
}


void QForm1::OnQPaintBox1MouseUp(Qt::MouseButton button, int x, int y){
    if(QForm1::isSelected){
        // Reinicializo los valores de posicion y velocidad para el recalculo de movimiento
        QForm1::posx = (qreal)(x - QForm1::ballRadius);
        QForm1::posy = (qreal)(y - QForm1::ballRadius);
    }
    QForm1::buttonPressed = Qt::NoButton;
    QForm1::isSelected = false;
    QForm1::statusBar()->showMessage("Relesed " + QString().number(QForm1::buttonPressed));
}

// Borro y redibujo la bola
void QForm1::ballMove(int x, int y, bool dot){
    QForm1::ballErase();
    QForm1::posx = x;
    QForm1::posy = y;
    QForm1::ballWrite(x, y, dot);
    ui->spinBoxPosX->setValue((int)QForm1::posx);
    ui->spinBoxPosY->setValue((int)QForm1::posy);
}

// Borro la bola de la posicion actual
void QForm1::ballErase(){
    QPainter paint(QPaintBox1->getCanvas());
        QForm1::brush.setColor(Qt::black);
    brush.setStyle(Qt::SolidPattern);
    paint.setBrush(brush);
    paint.drawEllipse((int)QForm1::posx, (int)QForm1::posy, 2 * QForm1::ballRadius, 2 * QForm1::ballRadius);
}

// redibujo la bola con esquina superior derecha en x,y
void QForm1::ballWrite(int x, int y, bool dot){
    QPainter paint(QPaintBox1->getCanvas());
    QForm1::brush.setColor(QForm1::ballColor);
    brush.setStyle(Qt::SolidPattern);
    paint.setBrush(brush);
    paint.drawEllipse(x, y, 2 * QForm1::ballRadius, 2 * QForm1::ballRadius);
    QPaintBox1->update();
}

/* Redibuja y reinicia la bola desde la posicion seteada por posx, posy, velx y vely */
void QForm1::ballStop(){
    QForm1::timerSleep = false;
    QForm1::ballErase();
}

void QForm1::ballStart(){
    QForm1::ballWrite((int)QForm1::posx, (int)QForm1::posy, false);
    QForm1::timerSleep = false;
}


void QForm1::paintEvent(QPaintEvent *event){
    static bool first = false;
    if(!first){
        QPaintBox1->resize(ui->frame->size());

        // Inicializo la posicion de la bola, al centro del canvas
        QForm1::posx = (ui->frame->size().width()/2) - QForm1::ballRadius;
        QForm1::posy = (ui->frame->size().height()/2) - QForm1::ballRadius;
        ui->spinBoxPosX->setMaximum(ui->frame->width());
        ui->spinBoxPosY->setMaximum(ui->frame->height());
        QForm1::ballWrite((int)QForm1::posx, (int)QForm1::posy, false);
        first = true;
    }
}

void QForm1::resizeEvent(QResizeEvent *event){
    QPaintBox1->resize(ui->frame->size());
    // Mantengo la bola dentro del frame
    if (QForm1::posx > ui->frame->size().width() - QForm1::ballRadius) QForm1::posx = ui->frame->size().width() - QForm1::ballRadius;
    if (QForm1::posy > ui->frame->size().height() - QForm1::ballRadius) QForm1::posy = ui->frame->size().height() - QForm1::ballRadius;
    // Impido que la posicion pueda ser seteada fuera del frame
    ui->spinBoxPosX->setMaximum(ui->frame->width());
    ui->spinBoxPosY->setMaximum(ui->frame->height());
}

void QForm1::onQtimer1(){
    // Leo comandos por puerto serie
    leerCMD();
    if(QForm1::timerSleep) return;

    static int ultimaPosX, ultimaPosY = 0;
    int framex = ui->frame->width();
    int framey = ui->frame->height();
    qreal x, y, sy, sx, At, At1, vtotal, vdir = qreal(0);
    At=(qreal)QForm1::timerQuanto/1000;
    if(!QForm1::isSelected){
        At=(qreal)QForm1::timerQuanto/1000;
        //Calculo la posicion de la bola en en instante actual
        x = QForm1::posx + (QForm1::velx * At);
        // Tanto aceleracion como velocidades son lineales, por lo que puedo hacer la equivalencia de pixeles a metros aumentando en esa proporcion el valor de G
        y = ((0.5 * QForm1::G * qPow(At, 2)) + (QForm1::vely * At)) + QForm1::posy;
        sy = (QForm1::G * At + QForm1::vely);
        sx = QForm1::velx;
        // Calculo vector velocidad y direccion
        vtotal = qSqrt(qPow(sy, 2) + qPow(sx, 2));
        vdir = qAtan2(sy, sx);
        if (vdir < 0) vdir = (2*3.14159265358979323846+vdir);

        /* Controlo impacto contra los bordes
         * Las perdidas de velocidad se aplica a la componente perpendicular al impacto
         * para ello obtengo instante del impacto, que será menor a At y luego calculo
         * el desplazamiento con la velocidad nueva, ya aplicada la perdida y con las
         * coordenadas de la pared como valor inicial
         */
        if(y > framey-(2*QForm1::ballRadius)){
            At1 = (-(QForm1::vely) + qSqrt(qPow(QForm1::vely, 2) - 4 * 0.5 * QForm1::G * (QForm1::posy - (framey-(2*QForm1::ballRadius)))))/(2 * 0.5 * QForm1::G);
            sy = -((QForm1::G * At1 + QForm1::vely)) * (1-perdida);
            y = ((0.5 * QForm1::G * qPow(At-At1, 2)) + (sy * (At-At1))) + (framey-(2*QForm1::ballRadius));
            sy = (QForm1::G * (At-At1) + sy);
            if(y > framey-(2*QForm1::ballRadius)){
                y = framey-(2*QForm1::ballRadius);
                sy = 0;
                // Está rodando por el piso, pierde velocidad en x a 10% de lo seteado en perdida
                sx = sx * (1-(perdida/10));
            }else
                QForm1::blinkLed(LED_BOTTOM);
        }else if (y < 0) {
            At1 = (-(QForm1::vely) - qSqrt(qPow(QForm1::vely, 2) - 4 * 0.5 * QForm1::G * QForm1::posy))/(2 * 0.5 * QForm1::G);
            sy = -((QForm1::G * At1 + QForm1::vely)) * (1-perdida);
            y = ((0.5 * QForm1::G * qPow(At-At1, 2)) + (sy * (At-At1)));
            sy = (QForm1::G * (At-At1) + sy);
            if(y < 0){
                y = 0;
                sy = 0;
                // Está rodando por el techo, pierde velocidad en x a 10% de lo seteado en perdida
                sx = sx * (1-(perdida/10));
            }else
                QForm1::blinkLed(LED_TOP);
        }
        if (x > framex-(2*QForm1::ballRadius)){
            At1 = ((framex-(2*QForm1::ballRadius)) - QForm1::posx) / QForm1::velx;
            sx = -sx * (1-perdida);
            if(qAbs(sx) < 1) sx = 0;  //para que pare en algun momento
            x = (framex-(2*QForm1::ballRadius)) + (sx * (At-At1));
            QForm1::blinkLed(LED_LEFT);
       }else if (x < 0) {
            At1 = -QForm1::posx / QForm1::velx;
            sx = -sx * (1-perdida);
            if(qAbs(sx) < 1) sx = 0; //para que pare en algun momento
            x = (sx * (At-At1));
            QForm1::blinkLed(LED_RIGTH);
        }
        QForm1::ballMove(x, y, false);

        // Almaceno los nuevos valores en las variables globales
        QForm1::posx = x;
        QForm1::posy = y;
        QForm1::vely = sy;
        QForm1::velx = sx;
        QForm1::statusBar()->showMessage("Velocidad : " + QString().number(vtotal/QForm1::pixelsPorMetro) + ", Direccion: " + QString().number((int)qRadiansToDegrees(vdir)) + "°");
        ui->doubleSpinBoxVel->setValue(vtotal/QForm1::pixelsPorMetro);
        ui->doubleSpinBoxGrados->setValue(qRadiansToDegrees(vdir));
    } else {
        //Mientras el mouse arrastra la bola, calculo su velocidad para inicializar cuando sea liberada
        QForm1::velx=(QForm1::posx - ultimaPosX)/(At);
        QForm1::vely=(QForm1::posy - ultimaPosY)/(At);
        ultimaPosX = (int)QForm1::posx;
        ultimaPosY = (int)QForm1::posy;
    }
}

void QForm1::blinkLed(int l){
    quint8 msg[] = {0xC1,0x00};
    switch(l){
    case LED_TOP:
        msg[1] |= 17;
        break;
    case LED_BOTTOM:
        msg[1] |= 17<<1;
        break;
    case LED_LEFT:
        msg[1] |= 17<<2;
        break;
    case LED_RIGTH:
        msg[1] |= 17<<3;
        break;
    }
    QSerial1->serialSend(msg, 2);
}


void QForm1::leerCMD(){
    if (QSerial1->serialAvailable() > 0){
        bool flag = false;
        quint8 *buf = new quint8[QSerial1->serialAvailable()];
        QSerial1->serialReceive(buf);
        // Calculo vector direccion, el angulo es para toda la circunferencia de 0 a 360
        qreal vtotal = qSqrt(qPow(QForm1::vely, 2) + qPow(QForm1::velx, 2));
        qreal vdir = qAtan2(QForm1::vely, QForm1::velx);
        if (vdir < 0) vdir = (2*3.14159265358979323846+vdir);

        switch (buf[0]){
            case 0xC0:
                // Respuesta a un seteo de los leds, lo ignoro por ahora
                break;
            case 0xC1:
                // Respuesta a un rebote en la pared, lo ignoro por ahora
                break;
            case 0xC2:
                // Respuesta a un pedido de estado de los botones
                break;
            case 0xC3:
                // Boton presionado por menos de 150ms
                QForm1::ballErase();
                if((buf[1] & (1<<0)) == (1<<0) && QForm1::posy > 1) QForm1::posy -= 1;
                if((buf[1] & (1<<1)) == (1<<1) && QForm1::posy+1 < (ui->frame->size().height()-(QForm1::ballRadius*2))) QForm1::posy += 1;
                if((buf[1] & (1<<2)) == (1<<2) && QForm1::posx > 1) QForm1::posx -= 1;
                if((buf[1] & (1<<3)) == (1<<3) && QForm1::posx+1 < (ui->frame->size().width()-(QForm1::ballRadius*2))) QForm1::posx += 1;
                QForm1::ballWrite(QForm1::posx, QForm1::posy, false);
                break;
            case 0xC4:
                // Boton presionado por mas de 200ms, movimiento solo si se presiona solo un boton
                switch(buf[1]){
                case (0x01): //0001
                    QForm1::ballErase();
                    if(QForm1::posy > QForm1::avance200ms){
                        QForm1::posy -= QForm1::avance200ms;
                    }else{
                        QForm1::posy = 0;
                    }
                    QForm1::ballWrite(QForm1::posx, QForm1::posy, false);
                    break;
                case (0x02): //0010
                    QForm1::ballErase();
                    if(QForm1::posy+QForm1::avance200ms < (ui->frame->size().height()-(QForm1::ballRadius*2))){
                        QForm1::posy += QForm1::avance200ms;
                    }else{
                        QForm1::posy = ui->frame->size().height()-(QForm1::ballRadius*2);
                    }
                    QForm1::ballWrite(QForm1::posx, QForm1::posy, false);
                    break;
                case (0x04): //0100
                    QForm1::ballErase();
                    if(QForm1::posx > QForm1::avance200ms){
                        QForm1::posx -= QForm1::avance200ms;
                    }else{
                        QForm1::posx = 0;
                    }
                    QForm1::ballWrite(QForm1::posx, QForm1::posy, false);
                    break;
                case (0x08): //1000
                    QForm1::ballErase();
                    if(QForm1::posx+QForm1::avance200ms < (ui->frame->size().width()-(QForm1::ballRadius*2))){
                        QForm1::posx += QForm1::avance200ms;
                    }else{
                        QForm1::posx = ui->frame->size().width()-(QForm1::ballRadius*2);
                    }
                    QForm1::ballWrite(QForm1::posx, QForm1::posy, false);
                    break;
                case(0x09): //1001 Aumenta angulo de tiro
                    vdir+=0.017453; // vdir está en radianes
                    QForm1::vely = vtotal*qSin(qDegreesToRadians(vdir));
                    QForm1::velx = vtotal*qCos(qDegreesToRadians(vdir));
                    flag = true;
                    break;
                case(0x0a): //1010 Reduce angulo de tiro
                    vdir-=0.017453; // vdir está en radianes
                    QForm1::vely = vtotal*qSin(qDegreesToRadians(vdir));
                    QForm1::velx = vtotal*qCos(qDegreesToRadians(vdir));
                    flag = true;
                    break;
                case(0x05): //0101 Aumenta velocidad inicial
                    vtotal+=(0.5*QForm1::pixelsPorMetro);
                    QForm1::vely = vtotal*qSin(vdir);
                    QForm1::velx = vtotal*qCos(vdir);
                    flag = true;
                    break;
                case(0x06): //0110 Reduce velocidad inicial
                    vtotal-=(0.5*QForm1::pixelsPorMetro);
                    if (vtotal<0) vtotal = 0;
                    QForm1::vely = vtotal*qSin(vdir);
                    QForm1::velx = vtotal*qCos(vdir);
                    flag = true;
                    break;
                }
        }
        delete [] buf;
        // Actualizo los valores de los lineEdit si es necesario
        if(flag){
            ui->doubleSpinBoxVel->setValue(vtotal/QForm1::pixelsPorMetro);
            ui->doubleSpinBoxGrados->setValue(qRadiansToDegrees(vdir));
        }
    }
}
void QForm1::on_pushButton_clicked()
{
    if(!QForm1::timerSleep){
        QForm1::timerSleep = true;
        ui->pushButton->setText("Continuar");
    }else{
        QForm1::timerSleep = false;
        ui->pushButton->setText("Detener");
    }
}

void QForm1::on_spinBox_valueChanged(const QString &arg1)
{
    QForm1::ballStop();
    QForm1::ballRadius = ui->spinBox->value();
    QForm1::ballStart();
    ui->frame->setMinimumSize(QSize(2*QForm1::ballRadius, 2*QForm1::ballRadius));
}

void QForm1::on_spinBox_2_valueChanged(const QString &arg1)
{
    QForm1::pixelsPorMetro = ui->spinBox_2->value();
    QForm1::G = QForm1::gravedad * QForm1::pixelsPorMetro;
}

void QForm1::on_doubleSpinBox_valueChanged(const QString &arg1)
{
    QForm1::gravedad = ui->doubleSpinBox->value();
    QForm1::G = QForm1::gravedad * QForm1::pixelsPorMetro;
}

void QForm1::on_doubleSpinBox_2_valueChanged(const QString &arg1)
{
    QForm1::perdida = ui->doubleSpinBox_2->value()/100;
}

void QForm1::on_spinBoxPosX_valueChanged(const QString &arg1)
{
    if(QForm1::timerSleep){
        QForm1::ballErase();
        QForm1::posx = ui->spinBoxPosX->value();
        if (QForm1::posx < 0) QForm1::posx = 0;
        if (QForm1::posx > ui->frame->width()-(2*QForm1::ballRadius)) QForm1::posx = ui->frame->width()-(2*QForm1::ballRadius);
        QForm1::ballWrite(QForm1::posx, QForm1::posy, false);
    }
}

void QForm1::on_spinBoxPosY_valueChanged(const QString &arg1)
{
    if(QForm1::timerSleep){
        QForm1::ballErase();
        QForm1::posy = ui->spinBoxPosY->value();
        if (QForm1::posy < 0) QForm1::posy = 0;
        if (QForm1::posy > ui->frame->height()-(2*QForm1::ballRadius)) QForm1::posy = ui->frame->height()-(2*QForm1::ballRadius);
        QForm1::ballWrite(QForm1::posx, QForm1::posy, false);
    }
}

void QForm1::on_doubleSpinBoxGrados_valueChanged(const QString &arg1)
{
    if(QForm1::timerSleep){
        // Calculo velocidad vectorial actual y aplico la nueva direccion
        qreal vtotal = qSqrt(qPow(QForm1::vely, 2) + qPow(QForm1::velx, 2));
        QForm1::vely = vtotal*qSin(qDegreesToRadians(ui->doubleSpinBoxGrados->value()));
        QForm1::velx = vtotal*qCos(qDegreesToRadians(ui->doubleSpinBoxGrados->value()));

    }
}

void QForm1::on_doubleSpinBoxVel_valueChanged(const QString &arg1)
{
    if(QForm1::timerSleep){
        // Calculo la direccion actual
        qreal vdir = qAtan2(QForm1::vely, QForm1::velx);
        if (vdir < 0) vdir = (2*3.14159265358979323846+vdir);
        QForm1::vely = ui->doubleSpinBoxVel->value()*QForm1::pixelsPorMetro*qSin(vdir);
        QForm1::velx = ui->doubleSpinBoxVel->value()*QForm1::pixelsPorMetro*qCos(vdir);
    }
}
