#include "ui/MainWindow.h"

#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("Workpiece Inspector"));
    resize(960, 640);

    auto *label = new QLabel(QStringLiteral("Workpiece Inspector"), this);
    label->setAlignment(Qt::AlignCenter);
    setCentralWidget(label);
}
