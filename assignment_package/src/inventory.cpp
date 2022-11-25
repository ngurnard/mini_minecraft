#include "inventory.h"
#include "ui_inventory.h" // had to make the inventory.ui file under forms for this to be recognized

Inventory::Inventory(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Inventory)
{
    ui->setupUi(this);
}

Inventory::~Inventory()
{
    delete ui;
}

void Inventory::slot_setGrass(QString s) {
    ui->grassLabel->setText(s);
}

void Inventory::slot_setDirt(QString s) {
    ui->dirtLabel->setText(s);
}

void Inventory::slot_setStone(QString s) {
    ui->stoneLabel->setText(s);
}

void Inventory::slot_setWater(QString s) {
    ui->waterLabel->setText(s);
}

void Inventory::slot_setLava(QString s) {
    ui->lavaLabel->setText(s);
}
void Inventory::slot_setIce(QString s) {
    ui->iceLabel->setText(s);
}

void Inventory::slot_setSnow(QString s) {
    ui->snowLabel->setText(s);
}

void Inventory::slot_setSand(QString s) {
    ui->sandLabel->setText(s);
}

