#include "device.h"
#include "mainwindow.h"
#include "dialogsettings.h"
#include "ui_mainwindow.h"
#include <QThread>
#include <QSpinBox>
#include <QComboBox>
#include <QDialogButtonBox>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    /* Create device */
    Device* dev = new Device("/dev/nxp_simtemp", "/sys/class/misc/nxp_simtemp");

    /* Settings dialog */
    connect(ui->actionConfiguration, &QAction::triggered, this, [this, dev]() {
        DialogSettings* dlg = new DialogSettings(this);
        dlg->setAttribute(Qt::WA_DeleteOnClose);

        /* Read current device settings */
        dev->read_attr("mode");
        dev->read_attr("threshold_mC");
        dev->read_attr("sampling_ms");

        int mode = dev->get_attr("mode");
        int threshold_mC = dev->get_attr("threshold_mC");
        int sampling_ms = dev->get_attr("sampling_ms");

        /* Show current device settings */
        if (auto modeCombo = dlg->findChild<QComboBox*>("mode_comboBox"))
            modeCombo->setCurrentIndex(mode);
        if (auto thSpin = dlg->findChild<QSpinBox*>("threshold_spinBox"))
            thSpin->setValue(threshold_mC);
        if (auto sampSpin = dlg->findChild<QSpinBox*>("sampling_spinBox"))
            sampSpin->setValue(sampling_ms);

        /* Write user settings */
        connect(dlg->findChild<QDialogButtonBox*>("buttonBox"), &QDialogButtonBox::accepted,
                this, [dlg, dev]() {
            /* Get user input */
            int mode = dlg->findChild<QComboBox*>("mode_comboBox")->currentIndex();
            int threshold_mC = dlg->findChild<QSpinBox*>("threshold_spinBox")->value();
            int sampling_ms  = dlg->findChild<QSpinBox*>("sampling_spinBox")->value();

            /* Write the attributes to the driver */
            dev->write_attr("mode", std::to_string(mode));
            dev->write_attr("threshold_mC", std::to_string(threshold_mC));
            dev->write_attr("sampling_ms", std::to_string(sampling_ms));

            dlg->accept();
        });
        dlg->show();
    });

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
        ui->th_checkBox->setChecked(sample.flags & THRESHOLD_EVENT);
        /* Show time stamp in ns */
        ui->timestamp_ns_line->setText(QString::number(sample.timestamp_ns));
    });

    /* Start reading thread */
    readThread->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

