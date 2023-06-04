#include "framelesswindow.h"
#include <QHBoxLayout>
#include <QRect>


WindowCtrlButton::WindowCtrlButton(QWidget* parent) : QPushButton(parent)
{

}

QSize WindowCtrlButton::sizeHint() const
{
    return QSize(20, 20);
}


void FramelessWindow::SetupWindowCtrlButtons()
{
    auto closeButton = new WindowCtrlButton();
    auto minimizeButton = new WindowCtrlButton();
    auto maximizeButton = new WindowCtrlButton();
    maximizeButton->setCheckable(true);

    connect(minimizeButton, &QPushButton::pressed, this, &QMainWindow::showMinimized);
    connect(closeButton, &QPushButton::pressed, this, &QMainWindow::close);
    connect(maximizeButton, &QPushButton::pressed, [this, maximizeButton]() {
        if (isMaximized())
        {
            showNormal();
            maximizeButton->setChecked(false);
        }
        else
        {
            showMaximized();
            maximizeButton->setChecked(true);
        }
        });

    QWidget* ctrlButtonsWrapper = new QWidget();
    ctrlButtonsWrapper->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    QHBoxLayout* menuWidgetLayout = new QHBoxLayout(ctrlButtonsWrapper);
    menuWidgetLayout->setContentsMargins(0, 2, 2, 0);
    ui.qMenuBarMain->setCornerWidget(ctrlButtonsWrapper);
    menuWidgetLayout->addWidget(minimizeButton);
    menuWidgetLayout->addWidget(maximizeButton);
    menuWidgetLayout->addWidget(closeButton);
}

#if (defined (_WIN32) || defined (_WIN64))
#include <windows.h>
#include <WinUser.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <gdiplus.h>
#pragma comment (lib,"Dwmapi.lib")
#include <QWindow>

FramelessWindow::FramelessWindow(QWidget* parent) :
    QMainWindow(parent, Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint | Qt::FramelessWindowHint),
    borderWidth(5),
    justMaximized(false)
{
    ui.setupUi(this);
    SetupWindowCtrlButtons();
    wndDescriptor = (HWND)winId();

    const MARGINS marginsForShadow = { 1, 1, 1, 1 };
    DwmExtendFrameIntoClientArea(wndDescriptor, &marginsForShadow);

    SetWindowLongPtr(wndDescriptor, GWL_STYLE, WS_POPUPWINDOW | WS_CAPTION | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_CLIPCHILDREN);

    connect(window()->windowHandle(), &QWindow::screenChanged, [this](QScreen*) {
        SetWindowPos(wndDescriptor, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE);
        });
}

bool FramelessWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
{
    MSG* msg = static_cast<MSG*>(message);
    QPoint globalCursorPos = QCursor::pos();

    switch (msg->message)
    {
    case WM_NCCALCSIZE:
    {
        NCCALCSIZE_PARAMS& params = *reinterpret_cast<NCCALCSIZE_PARAMS*>(msg->lParam);
        if (params.rgrc[0].top != 0)
            params.rgrc[0].top -= 1;

        return true;
    }
    case WM_NCHITTEST:
    {
        *result = 0;

        const LONG border_width = borderWidth;
        RECT winrect;
        GetWindowRect(wndDescriptor, &winrect);

        long x = GET_X_LPARAM(msg->lParam);
        long y = GET_Y_LPARAM(msg->lParam);

        bool resizeWidth = minimumWidth() != maximumWidth();
        bool resizeHeight = minimumHeight() != maximumHeight();

        if (resizeWidth)
        {
            //left border
            if (x >= winrect.left && x < winrect.left + border_width)
                *result = HTLEFT;
            //right border
            else if (x < winrect.right && x >= winrect.right - border_width)
                *result = HTRIGHT;
        }
        if (resizeHeight)
        {
            if (y < winrect.bottom && y >= winrect.bottom - border_width)
                *result = HTBOTTOM;
            //top border
            else if (y >= winrect.top && y < winrect.top + border_width)
                *result = HTTOP;
        }
        if (resizeWidth && resizeHeight)
        {
            //bottom left corner
            if (x >= winrect.left && x < winrect.left + border_width && y < winrect.bottom && y >= winrect.bottom - border_width)
                *result = HTBOTTOMLEFT;
            //bottom right corner
            else if (x < winrect.right && x >= winrect.right - border_width && y < winrect.bottom && y >= winrect.bottom - border_width)
                *result = HTBOTTOMRIGHT;
            //top left corner
            else if (x >= winrect.left && x < winrect.left + border_width && y >= winrect.top && y < winrect.top + border_width)
                *result = HTTOPLEFT;
            //top right corner
            else if (x < winrect.right && x >= winrect.right - border_width && y >= winrect.top && y < winrect.top + border_width)
                *result = HTTOPRIGHT;
        }

        if (*result)
            return true;

        if (!ui.qMenuBarMain)
            break;

        QPoint pos = ui.qMenuBarMain->mapFromGlobal(globalCursorPos);
        auto menuGeom = ui.qMenuBarMain->geometry().translated(ui.qMenuBarMain->mapToGlobal(QPoint(0, 0)));

        if (!menuGeom.contains(globalCursorPos) || ui.qMenuBarMain->childAt(pos) || ui.qMenuBarMain->actionAt(pos))
            break;

        *result = HTCAPTION;
        return true;
    }
    case WM_GETMINMAXINFO:
    {
        if (::IsZoomed(msg->hwnd))
        {
            RECT frame = { 0, 0, 0, 0 };
            AdjustWindowRectEx(&frame, WS_OVERLAPPEDWINDOW, FALSE, 0);
            auto dpr = devicePixelRatioF();

            frames.setLeft(abs(frame.left) / dpr + 0.5);
            frames.setTop(abs(frame.bottom) / dpr + 0.5);
            frames.setRight(abs(frame.right) / dpr + 0.5);
            frames.setBottom(abs(frame.bottom) / dpr + 0.5);

            setContentsMargins(frames.left(), frames.top(), frames.right(), frames.bottom());
            justMaximized = true;
        }
        else
        {
            if (justMaximized)
            {
                setContentsMargins(margins);
                frames = QMargins();
                justMaximized = false;
            }
        }
    }
    default:
    {
        break;
    }
    }

    return false;
}


#elif (defined (LINUX) || defined (__linux__))
FramelessWindow::FramelessWindow(QWidget* parent) : QMainWindow(parent)
{
    ui.setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setMouseTracking(true);
    startGeometry = geometry();
    QApplication::instance()->installEventFilter(this);
    SetupWindowCtrlButtons();
    connect(ui.qMenuBarMain, &DraggableMenuBar::DoubleClicked, [this]() { isMaximized() ? showNormal() : showMaximized(); });
}

void FramelessWindow::CheckBorderDragging(QMouseEvent* event)
{
    if (isMaximized())
        return;

    QPoint globalMousePos = event->globalPos();
    if (mousePressed)
    {
        if (dragTop && dragLeft)
        {
            int diff = globalMousePos.y() - startGeometry.y();
            int newy = startGeometry.y() + diff;
            diff = globalMousePos.x() - startGeometry.x();
            int newx = startGeometry.x() + diff;
            if (newy > 0 && newx > 0)
            {
                QRect newg = startGeometry;
                newg.setY(newy);
                newg.setX(newx);
                setGeometry(newg);
            }
        }
        else if (dragBottom && dragLeft)
        {
            int diff = globalMousePos.y() - (startGeometry.y() + startGeometry.height());
            int newh = startGeometry.height() + diff;
            diff = globalMousePos.x() - startGeometry.x();
            int newx = startGeometry.x() + diff;
            if (newh > 0 && newx > 0)
            {
                QRect newg = startGeometry;
                newg.setX(newx);
                newg.setHeight(newh);
                setGeometry(newg);
            }
        }
        else if (dragBottom && dragRight)
        {
            int diff = globalMousePos.y() - (startGeometry.y() + startGeometry.height());
            int newh = startGeometry.height() + diff;
            diff = globalMousePos.x() - (startGeometry.x() + startGeometry.width());
            int neww = startGeometry.width() + diff;
            if (newh > 0 && neww > 0)
            {
                QRect newg = startGeometry;
                newg.setWidth(neww);
                newg.setHeight(newh);
                setGeometry(newg);
            }
        }
        else if (dragTop)
        {
            int diff = globalMousePos.y() - startGeometry.y();
            int newy = startGeometry.y() + diff;
            if (newy > 0)
            {
                QRect newg = startGeometry;
                newg.setY(newy);
                setGeometry(newg);
            }
        }
        else if (dragLeft)
        {
            int diff = globalMousePos.x() - startGeometry.x();
            int newx = startGeometry.x() + diff;
            if (newx > 0)
            {
                QRect newg = startGeometry;
                newg.setX(newx);
                setGeometry(newg);
            }
        }
        else if (dragRight)
        {
            int diff = globalMousePos.x() - (startGeometry.x() + startGeometry.width());
            int neww = startGeometry.width() + diff;
            if (neww > 0)
            {
                QRect newg = startGeometry;
                newg.setWidth(neww);
                newg.setX(startGeometry.x());
                setGeometry(newg);
            }
        }
        else if (dragBottom)
        {
            int diff = globalMousePos.y() - (startGeometry.y() + startGeometry.height());
            int newh = startGeometry.height() + diff;
            if (newh > 0)
            {
                QRect newg = startGeometry;
                newg.setHeight(newh);
                newg.setY(startGeometry.y());
                setGeometry(newg);
            }
        }
    }
    else
    {
        if (LeftBorderHit(globalMousePos) && TopBorderHit(globalMousePos))
            setCursor(Qt::SizeFDiagCursor);
        else if (LeftBorderHit(globalMousePos) && BottomBorderHit(globalMousePos))
            setCursor(Qt::SizeBDiagCursor);
        else if (RightBorderHit(globalMousePos) && BottomBorderHit(globalMousePos))
            setCursor(Qt::SizeFDiagCursor);
        else
        {
            if (TopBorderHit(globalMousePos) || BottomBorderHit(globalMousePos))
                setCursor(Qt::SizeVerCursor);
            else if (LeftBorderHit(globalMousePos) || RightBorderHit(globalMousePos))
                setCursor(Qt::SizeHorCursor);
            else
            {
                dragTop = false;
                dragLeft = false;
                dragRight = false;
                dragBottom = false;
                setCursor(Qt::ArrowCursor);
            }
        }
    }
}

void FramelessWindow::mousePressEvent(QMouseEvent* event)
{
    if (isMaximized())
        return;

    mousePressed = true;
    startGeometry = geometry();

    QPoint globalMousePos = mapToGlobal(QPoint(event->x(), event->y()));

    if (LeftBorderHit(globalMousePos) && TopBorderHit(globalMousePos))
    {
        dragTop = true;
        dragLeft = true;
        setCursor(Qt::SizeFDiagCursor);
    }
    else if (LeftBorderHit(globalMousePos) && BottomBorderHit(globalMousePos))
    {
        dragLeft = true;
        dragBottom = true;
        setCursor(Qt::SizeBDiagCursor);
    }
    else if (RightBorderHit(globalMousePos) && BottomBorderHit(globalMousePos))
    {
        dragRight = true;
        dragBottom = true;
        setCursor(Qt::SizeFDiagCursor);
    }
    else
    {
        if (TopBorderHit(globalMousePos))
        {
            dragTop = true;
            setCursor(Qt::SizeVerCursor);
        }
        else if (LeftBorderHit(globalMousePos))
        {
            dragLeft = true;
            setCursor(Qt::SizeHorCursor);
        }
        else if (RightBorderHit(globalMousePos))
        {
            dragRight = true;
            setCursor(Qt::SizeHorCursor);
        }
        else if (BottomBorderHit(globalMousePos))
        {
            dragBottom = true;
            setCursor(Qt::SizeVerCursor);
        }
    }
}

void FramelessWindow::mouseReleaseEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    if (isMaximized())
        return;

    mousePressed = false;
    bool cursorSwitchNeeded = dragTop || dragLeft || dragRight || dragBottom;
    dragTop = false;
    dragLeft = false;
    dragRight = false;
    dragBottom = false;
    if (cursorSwitchNeeded)
        setCursor(Qt::ArrowCursor);
}

bool FramelessWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (isMaximized())
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent* pMouse = dynamic_cast<QMouseEvent*>(event);
            if (pMouse)
            {
                auto pMenuBar = qobject_cast<DraggableMenuBar*>(obj);
                if (pMenuBar && !pMenuBar->actionAt(pMouse->pos()))
                    return true;
            }
        }

        return QWidget::eventFilter(obj, event);
    }

    if (event->type() == QEvent::MouseMove)
    {
        QMouseEvent* pMouse = dynamic_cast<QMouseEvent*>(event);
        if (pMouse)
            CheckBorderDragging(pMouse);
    }
    else if (event->type() == QEvent::MouseButtonPress && obj == this)
    {
        QMouseEvent* pMouse = dynamic_cast<QMouseEvent*>(event);
        if (pMouse)
            mousePressEvent(pMouse);
    }
    else if (event->type() == QEvent::MouseButtonPress && qobject_cast<DraggableMenuBar*>(obj))
    {
        QMouseEvent* pMouse = dynamic_cast<QMouseEvent*>(event);
        if (pMouse && pMouse->pos().y() < dragBorderWidth)
            mousePressEvent(pMouse);
    }
    else if (event->type() == QEvent::MouseButtonRelease)
    {
        if (mousePressed)
        {
            QMouseEvent* pMouse = dynamic_cast<QMouseEvent*>(event);
            if (pMouse)
                mouseReleaseEvent(pMouse);
        }
    }

    return QWidget::eventFilter(obj, event);
}

bool FramelessWindow::LeftBorderHit(const QPoint& pos)
{
    const QRect& rect = geometry();
    if (pos.x() >= rect.x() && pos.x() <= rect.x() + dragBorderWidth)
        return true;

    return false;
}

bool FramelessWindow::RightBorderHit(const QPoint& pos)
{
    const QRect& rect = geometry();
    int tmp = rect.x() + rect.width();
    if (pos.x() <= tmp && pos.x() >= (tmp - dragBorderWidth))
        return true;

    return false;
}

bool FramelessWindow::TopBorderHit(const QPoint& pos)
{
    const QRect& rect = geometry();
    if (pos.y() >= rect.y() && pos.y() <= rect.y() + dragBorderWidth)
        return true;

    return false;
}

bool FramelessWindow::BottomBorderHit(const QPoint& pos)
{
    const QRect& rect = geometry();
    int tmp = rect.y() + rect.height();
    if (pos.y() <= tmp && pos.y() >= (tmp - dragBorderWidth))
        return true;

    return false;
}
#endif
