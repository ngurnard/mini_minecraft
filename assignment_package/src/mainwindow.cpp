#include "mainwindow.h"
#include <ui_mainwindow.h>
#include "cameracontrolshelp.h"
#include <QResizeEvent>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), cHelp()
{
    ui->setupUi(this);
    ui->mygl->setFocus();
    this->playerInfoWindow.show();
    playerInfoWindow.move(QGuiApplication::primaryScreen()->availableGeometry().center() - this->rect().center() + QPoint(this->width() * 0.75, 0));

    connect(ui->mygl, SIGNAL(sig_sendPlayerPos(QString)), &playerInfoWindow, SLOT(slot_setPosText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendPlayerVel(QString)), &playerInfoWindow, SLOT(slot_setVelText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendPlayerAcc(QString)), &playerInfoWindow, SLOT(slot_setAccText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendPlayerLook(QString)), &playerInfoWindow, SLOT(slot_setLookText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendPlayerChunk(QString)), &playerInfoWindow, SLOT(slot_setChunkText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendPlayerTerrainZone(QString)), &playerInfoWindow, SLOT(slot_setZoneText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendLiquidBool(QString)), &playerInfoWindow, SLOT(slot_setLiquidText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendGroundBool(QString)), &playerInfoWindow, SLOT(slot_setGroundText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendCamBlock(QString)), &playerInfoWindow, SLOT(slot_setcamBlockText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendCurrentHoldingBlock(QString)), &playerInfoWindow, SLOT(slot_setCurrentHoldingBlock(QString)));

    // Inventory specific
    connect(ui->mygl, SIGNAL(sig_showInventory(bool)), this, SLOT(slot_showInventory(bool)));
    connect(ui->mygl, SIGNAL(sig_sendGrassCount(int)), &inventoryWindow, SLOT(slot_setGrass(int)));
    connect(ui->mygl, SIGNAL(sig_sendDirtCount(int)), &inventoryWindow, SLOT(slot_setDirt(int)));
    connect(ui->mygl, SIGNAL(sig_sendStoneCount(int)), &inventoryWindow, SLOT(slot_setStone(int)));
    connect(ui->mygl, SIGNAL(sig_sendWaterCount(int)), &inventoryWindow, SLOT(slot_setWater(int)));
    connect(ui->mygl, SIGNAL(sig_sendLavaCount(int)), &inventoryWindow, SLOT(slot_setLava(int)));
    connect(ui->mygl, SIGNAL(sig_sendIceCount(int)), &inventoryWindow, SLOT(slot_setIce(int)));
    connect(ui->mygl, SIGNAL(sig_sendSnowCount(int)), &inventoryWindow, SLOT(slot_setSnow(int)));
    connect(ui->mygl, SIGNAL(sig_sendSandCount(int)), &inventoryWindow, SLOT(slot_setSand(int)));

    connect(ui->mygl, SIGNAL(sig_sendPlayer(Player*)), &inventoryWindow, SLOT(slot_setHoldingBlock(Player*)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::exit();
}

void MainWindow::on_actionCamera_Controls_triggered()
{
    cHelp.show();
}

void MainWindow::slot_showInventory(bool show) {
    if (show) {
        inventoryWindow.show();
    } else {
        inventoryWindow.close();
    }
}
