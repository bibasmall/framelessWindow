#pragma once
#include <QMenuBar>


class DraggableMenuBar : public QMenuBar
{
    Q_OBJECT

public:
    explicit DraggableMenuBar(QWidget* parent = Q_NULLPTR);
    ~DraggableMenuBar() = default;

#if (defined (LINUX) || defined (__linux__))
signals:
    void DoubleClicked();

public:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    QPoint mousePos;
    QPoint wndPos;

    bool mousePressed    = false;
    bool actionActivated = false;
#endif
};