#include "device.h"
#include "mainwindow.h"
#include "dialogsettings.h"
#include "ui_mainwindow.h"
#include <QThread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    /* Settings dialog */
    connect(ui->actionConfiguration, &QAction::triggered, this, [this](){
        DialogSettings* dlg = new DialogSettings(this);
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->show();
    });

    /* Create device */
    Device* dev = new Device("/dev/nxp_simtemp", "/sys/class/misc/nxp_simtemp");

    /* Create new thread for reading */
    QThread* readThread = new QThread(this);
    dev->moveToThread(readThread);

    /* Read loop */
    connect(readThread, &QThread::started, [dev]() {
        while (true) {
            dev->read();
        }
    });

    /* Display read */
    connect(dev, &Device::readSignal, this, [this,dev](const nxp_simtemp_sample_t &sample){
        /* Show temp */
        double tempC = sample.temp_mC / 1000.0;
        ui->temp_mC_line->setText(QString::number(tempC));
        ui->temp_mC_dial->setValue(static_cast<int>(tempC));
        /* Show threshold flag */
        ui->th_checkBox->setChecked(sample.flags & EVENT_MASK_TH);
        /* Show time stamp in ns */
        ui->timestamp_ns_line->setText(QString::number(sample.timestamp_ns));

        /* Read sysfs from driver */
        dev->read_attr("mode");
        dev->read_attr("threshold_mC");
        dev->read_attr("sampling_ms");

        /* Get sysfs values */
        int mode = dev->get_attr("mode");
        int threshold_mC = dev->get_attr("threshold_mC");
        int sampling_ms = dev->get_attr("sampling_ms");

        /* Show mode */
        ui->norm_checkBox->setChecked(mode == 0);
        ui->noisy_checkBox->setChecked(mode == 1);
        ui->ramp_checkBox->setChecked(mode == 2);
        /* Show threshold */
        ui->threshold_mC_line->setText(QString::number(threshold_mC / 1000.0));
        /* Show sampling */
        ui->threshold_mC_line->setText(QString::number(sampling_ms));
    });

    /* Start reading thread */
    readThread->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

