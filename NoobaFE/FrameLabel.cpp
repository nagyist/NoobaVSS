/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* FrameLabel.cpp                                                       */
/*                                                                      */
/* Nick D'Ademo <nickdademo@gmail.com>                                  */
/* Modified to fit NoobaVSS by                                          */
/*      D.A.U.Nanayakkara <daun07@gmail.com>                            */
/*                                                                      */
/* Copyright (c) 2012-2013 Nick D'Ademo                                 */
/*                                                                      */
/* Permission is hereby granted, free of charge, to any person          */
/* obtaining a copy of this software and associated documentation       */
/* files (the "Software"), to deal in the Software without restriction, */
/* including without limitation the rights to use, copy, modify, merge, */
/* publish, distribute, sublicense, and/or sell copies of the Software, */
/* and to permit persons to whom the Software is furnished to do so,    */
/* subject to the following conditions:                                 */
/*                                                                      */
/* The above copyright notice and this permission notice shall be       */
/* included in all copies or substantial portions of the Software.      */
/*                                                                      */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,      */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF   */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                */
/* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS  */
/* BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN   */
/* ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN    */
/* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE     */
/* SOFTWARE.                                                            */
/*                                                                      */
/************************************************************************/

#include "FrameLabel.h"
// Qt
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <QDebug>

FrameLabel::FrameLabel(QWidget *parent) :
    QLabel(parent),
    _showSketches(true),
    _color(Qt::transparent)
{
    setMouseTracking(true);
//    setScaledContents(true);
    startPointOrg.setX(0);
    startPointOrg.setY(0);
    mouseCursorPos.setX(0);
    mouseCursorPos.setY(0);
    _draw=false;
    mouseData.leftButtonRelease=false;
    mouseData.rightButtonRelease=false;
    createContextMenu();
    setFocusPolicy(Qt::StrongFocus);
    _color = QColor(Qt::transparent);
}

void FrameLabel::mouseMoveEvent(QMouseEvent *ev)
{
    if(_image.isNull())
        return;
    // Save mouse cursor position
    QPointF tmpPoint = ev->pos();
    setMouseCursorPos(tmpPoint);

    // Update box width and height if box drawing is in progress
    if(_draw)
    {
        switch(_drawMode)
        {
        case nooba::lineDraw:
        {
            drawingLine.setP2(toOriginalImage(tmpPoint));
            break;
        }
        default:
            break;
        }
    }
    // Inform main window of mouse move event
    emit onMouseMoveEvent();
}

void FrameLabel::setMouseCursorPos(QPointF input)
{
    mouseCursorPos=input;
}

QPointF FrameLabel::getMouseCursorPos()
{
    return mouseCursorPos;
}

void FrameLabel::setDrawMode(const QString &varName, const QColor &color, nooba::DrawMode drawMode)
{
    _varName = varName;
    _drawMode = drawMode;
    _color  = color;
}

void FrameLabel::setImage(const QImage& image)
{
    _image = image;
    _pixmap = QPixmap::fromImage(_image);
    setPixmap(_pixmap.scaled(size(), Qt::KeepAspectRatio));
    update();
}

QSize FrameLabel::sizeHint()
{
    if(_image.isNull())
        return QSize();

    return QSize(_image.width(), _image.height());
}

void FrameLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    if(_image.isNull())
        return;
    // Update cursor position
    setMouseCursorPos(ev->pos());
    qDebug() << getMouseCursorPos() << Q_FUNC_INFO;
    // On left mouse button release
    if(ev->button()==Qt::LeftButton)
    {
        // Set leftButtonRelease flag to TRUE
        mouseData.leftButtonRelease=true;
        if(_draw)
        {
            // Stop drawing box
            _draw=false;
        }
    }
    // On right mouse button release
    else if(ev->button()==Qt::RightButton)
    {
        _draw=false;
        _varName.clear();
        _drawMode = nooba::noDraw;
        drawingLine = QLineF();
        _color = QColor(Qt::transparent);
    }
}

void FrameLabel::mousePressEvent(QMouseEvent *ev)
{
    if(_image.isNull())
        return;
    // Update cursor position
    setMouseCursorPos(ev->pos());;

    if(ev->button()==Qt::LeftButton)
    {
        // Start drawing box
        if(_varName.isEmpty())
            return;
        QPointF p1 = ev->pos();

        // start point relative to original image.
        startPointOrg= toOriginalImage(p1);
        qDebug() << p1 << Q_FUNC_INFO;
        qDebug() << "org" << startPointOrg << Q_FUNC_INFO;
        switch(_drawMode)
        {
        case nooba::lineDraw:
        {
            _S_Line s_l = _lineMap.take(_varName);
            if(s_l._line.isNull())
            {
                if(drawingLine.isNull())
                {
                    drawingLine = QLineF(startPointOrg, startPointOrg);
                }
              }
            else
            {
                drawingLine =s_l._line;
            }
            drawingLine.setPoints(startPointOrg, startPointOrg);
            _draw=true;
            break;
        }
        case nooba::boxDraw:
        {
            box = new QRect(startPointOrg.x(),startPointOrg.y(),0,0);
            _draw = true;
            break;
        }
        default:
            break;
        }
    }
}

void FrameLabel::paintEvent(QPaintEvent *ev)
{
    QLabel::paintEvent(ev);
    if(_image.isNull())
        return;
    QPainter painter(this);

    if(_showSketches)
    {
        foreach(_S_Line s_l, _lineMap)
        {
            painter.setPen(s_l._color);
            painter.drawLine(toCurrentImage(s_l._line));
        }
    }
    painter.setPen(_color);
    painter.drawLine(toCurrentImage(drawingLine));
}

void FrameLabel::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Return)
    {
        switch(_drawMode)
        {
        case nooba::lineDraw:
        {
            _lineMap.insert(_varName, _S_Line(drawingLine, _color));
            _varName.clear();
            emit lineUpdated(drawingLine.toLine());
            drawingLine = QLineF();
            _color = QColor(Qt::transparent);
        }
        default:
            break;
        }
        _drawMode = nooba::noDraw;
        _draw = false;
    }
    else
    {
        QLabel::keyReleaseEvent(event);
    }
}

void FrameLabel::resizeEvent(QResizeEvent *event)
{
    if(_pixmap.isNull())
        return;

    setPixmap(_pixmap.scaled(event->size(), Qt::KeepAspectRatio));
    update();
}

void FrameLabel::createContextMenu()
{

}

QPointF FrameLabel::toOriginalImage(const QPointF &c_p)
{
    return QPointF((c_p.x() * _image.width())/ width(), (c_p.y() * _image.height())/ height());

}

QPointF FrameLabel::toCurrentImage(const QPointF& o_p)
{
    return QPointF((o_p.x() * width()) / _image.width(), (o_p.y() * height()) / _image.height());
}

QLineF FrameLabel::toCurrentImage(const QLineF &o_l)
{
    return QLineF(toCurrentImage(o_l.p1()), toCurrentImage(o_l.p2()));
}
