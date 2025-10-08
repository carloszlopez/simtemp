#include "device.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QThread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);



    /* Create device */
    Device* dev = new Device("/dev/nxp_simtemp", "/sys/class/misc/nxp_simtemp");

    /* change mode */
    ui->mode_comboBox->addItem("NORMAL");
    ui->mode_comboBox->addItem("NOISY");
    ui->mode_comboBox->addItem("RAMP");
    connect(ui->mode_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [dev](int index){
            dev->write_attr("mode", std::to_string(index));
    });

    /* Create new thread for reading */
    QThread* readThread = new QThread(this);
    dev->moveToThread(readThread);

    /* Read loop */
    connect(readThread, &QThread::started, [dev]() {
        while (true) {
            dev->read();           // runs in readThread
            QThread::msleep(100);  // small delay to avoid spinning too fast
        }
    });

    /* Display temperature */
    connect(dev, &Device::readSignal, this, [this](const nxp_simtemp_sample_t &sample){
        double tempC = sample.temp_mC / 1000.0;
    ui->temp_label->setText(QString::number(tempC, 'f', 2) + " °C");
    });

    /* Start reading thread */
    readThread->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

