#include "draggableMenuBar.h"
#include <QMouseEvent>


DraggableMenuBar::DraggableMenuBar(QWidget* parent) : QMenuBar(parent)
{

}

#ifndef Q_OS_WIN
void DraggableMenuBar::mousePressEvent(QMouseEvent* event)
{
    if (actionAt(event->pos()))
    {
        actionActivated = true;
        QMenuBar::mousePressEvent(event);
        return;
    }
    if (actionActivated)
    {
        actionActivated = false;
        QMenuBar::mouseReleaseEvent(event);
        return;
    }

    QWidget* parent = parentWidget();
    assert(parent);

    mousePressed = true;
    mousePos = event->globalPos();
    wndPos = parentWidget()->pos();
}

void DraggableMenuBar::mouseMoveEvent(QMouseEvent* event)
{
    QWidget* parent = parentWidget();
    assert(parent);
    if (parent && mousePressed && !(parent->isFullScreen() || parent->isMaximized()))
        parent->move(wndPos + (event->globalPos() - mousePos));
}

void DraggableMenuBar::mouseReleaseEvent(QMouseEvent* event)
{
    if (actionActivated)
    {
        actionActivated = false;
        QMenuBar::mouseReleaseEvent(event);
        return;
    }
    mousePressed = false;
}

void DraggableMenuBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    emit DoubleClicked();
}
#endif