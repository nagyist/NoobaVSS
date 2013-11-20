#include "FrameViewer.h"
#include "ui_FrameViewer.h"
// Qt
#include <QCloseEvent>
#include <QMdiSubWindow>

FrameViewer::FrameViewer(const QString &title, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FrameViewer)
{
    ui->setupUi(this);
    ui->canvas->setProperty("canvas", true);    // for use with style sheet styling
    setWindowTitle(title);
    // show video in center
    ui->canvas->setAlignment(Qt::AlignCenter);
    ui->canvas->setAlignment(Qt::AlignCenter);
    ui->canvas->setMinimumSize(1,1);
}

FrameViewer::~FrameViewer()
{
    delete ui;
}

void FrameViewer::closeEvent(QCloseEvent * event)
{
    event->ignore();
}

bool FrameViewer::updateFrame(QImage in)
{
    if(in.isNull())
        return false;

    _pixmap = QPixmap::fromImage(in);
    ui->canvas->setPixmap(_pixmap.scaled(ui->canvas->size(), Qt::KeepAspectRatio));
    return true;
}

void FrameViewer::resizeEvent(QResizeEvent *event)
{
    if(_pixmap.isNull())
        return;

    ui->canvas->setPixmap(_pixmap.scaled(event->size(), Qt::KeepAspectRatio));
}

void FrameViewer::setVisibility(bool isVisible)
{
    _mdiSubWindow->setVisible(isVisible);
    return;
}




