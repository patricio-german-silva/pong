#ifndef QPAINTBOX_H
#define QPAINTBOX_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>

class QPaintBox : public QWidget
{
    Q_OBJECT
public:
    explicit QPaintBox(int aWidth = 200, int aHeigth = 200, QWidget *parent = nullptr);

    void setWidth(int awidth);
    void setHeight(int aheight);
    QPixmap *getCanvas(void);

signals:
    void paintBoxMouseMove(Qt::MouseButton button, int x, int y);
    void paintBoxMouseDown(Qt::MouseButton button, int x, int y);
    void paintBoxMouseUp(Qt::MouseButton button, int x, int y);

public slots:

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
    QPixmap *pixelCanvas;

};

#endif // QPAINTBOX_H
