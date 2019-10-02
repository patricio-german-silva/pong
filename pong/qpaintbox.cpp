#include "qpaintbox.h"

//QPaintBox::QPaintBox(QWidget *parent) : QWidget(parent)
QPaintBox::QPaintBox(int aWidth, int aHeight, QWidget *parent) : QWidget(parent)
{
    if(parent){
        aWidth = parent->width();
        aHeight = parent->height();
    }

    pixelCanvas = new QPixmap(aWidth, aHeight);
    this->resize(aWidth, aHeight);
    pixelCanvas->fill(Qt::black);
}
//------------------------------------------------------------------------------
void QPaintBox::paintEvent(QPaintEvent * /* event */){
    QPainter Canvas(this);

    Canvas.drawPixmap(0,0,*pixelCanvas);

}
//------------------------------------------------------------------------------
void QPaintBox::resizeEvent(QResizeEvent */* event */){
    QPixmap aux(this->width(), this->height());

    aux.copy(pixelCanvas->rect());
    pixelCanvas->scaled(aux.width(), aux.height());
    pixelCanvas->swap(aux);
}
//------------------------------------------------------------------------------
void QPaintBox::setWidth(int aWidth){
    pixelCanvas->size().setWidth(aWidth);
    this->resize(aWidth, this->height());
}
//------------------------------------------------------------------------------
void QPaintBox::setHeight(int aHeight){
    pixelCanvas->size().setWidth(aHeight);
    this->resize(this->width(), aHeight);
}
//------------------------------------------------------------------------------
QPixmap *QPaintBox::getCanvas(){
    return pixelCanvas;
}

//------------------------------------------------------------------------------
void QPaintBox::mousePressEvent(QMouseEvent *event){
    emit (paintBoxMouseDown(event->button(),event->pos().x(), event->pos().y()));
}

//------------------------------------------------------------------------------
void QPaintBox::mouseReleaseEvent(QMouseEvent *event){
    emit (paintBoxMouseUp(event->button(),event->pos().x(), event->pos().y()));
}

//------------------------------------------------------------------------------
void QPaintBox::mouseMoveEvent(QMouseEvent *event){
    emit (paintBoxMouseMove(event->button(),event->pos().x(), event->pos().y()));
}
