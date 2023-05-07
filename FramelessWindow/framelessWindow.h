#pragma once
#include "ui_framelesswindow.h"
#include <QMainWindow>
#include <QPushButton>
#include <QMouseEvent>
#include <QMargins>


class WindowCtrlButton : public QPushButton
{
    Q_OBJECT

public:
    WindowCtrlButton(QWidget* parent = Q_NULLPTR);
    ~WindowCtrlButton() = default;

public:
    QSize sizeHint() const override;
};

class FramelessWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit FramelessWindow(QWidget* parent = Q_NULLPTR);
    ~FramelessWindow() = default;

private:
    void SetupWindowCtrlButtons();

private:
    Ui::FramelessWindow ui;
    const quint8 dragBorderWidth = 5;

#ifdef Q_OS_WIN
public:
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;

private:
    int borderWidth;
    QMargins margins;
    QMargins frames;
    bool justMaximized;
    HWND wndDescriptor;

#elif Q_OS_UNIX
protected:
    virtual void CheckBorderDragging(QMouseEvent* event);

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    bool LeftBorderHit(const QPoint& pos);
    bool RightBorderHit(const QPoint& pos);
    bool TopBorderHit(const QPoint& pos);
    bool BottomBorderHit(const QPoint& pos);

private:
    QRect startGeometry;
    bool mousePressed    = false;
    bool dragTop         = false;
    bool dragLeft        = false;
    bool dragRight       = false;
    bool dragBottom      = false;
#endif
};
