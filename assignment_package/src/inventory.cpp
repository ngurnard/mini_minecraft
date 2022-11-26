#include "inventory.h"
#include "ui_inventory.h" // had to make the inventory.ui file under forms for this to be recognized

Inventory::Inventory(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Inventory)
{
    ui->setupUi(this);

    connect(ui->grassButton, SIGNAL(clicked()), this, SLOT(slot_getHoldingBlockGrass()));
    connect(ui->dirtButton, SIGNAL(clicked()), this, SLOT(slot_getHoldingBlockDirt()));
    connect(ui->stoneButton, SIGNAL(clicked()), this, SLOT(slot_getHoldingBlockStone()));
    connect(ui->waterButton, SIGNAL(clicked()), this, SLOT(slot_getHoldingBlockWater()));
    connect(ui->lavaButton, SIGNAL(clicked()), this, SLOT(slot_getHoldingBlockLava()));
    connect(ui->iceButton, SIGNAL(clicked()), this, SLOT(slot_getHoldingBlockIce()));
    connect(ui->snowButton, SIGNAL(clicked()), this, SLOT(slot_getHoldingBlockSnow()));
    connect(ui->sandButton, SIGNAL(clicked()), this, SLOT(slot_getHoldingBlockSand()));
}

Inventory::~Inventory()
{
    delete ui;
}

void Inventory::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_I) {
        this->close(); // close the inventory window (widget)
    }
}

void Inventory::slot_setGrass(int count) {
    if (count != 0) {
        // display the grass block
        QPixmap grassPix(":/textures/grass_block.png");
        ui->grassPic->setPixmap(grassPix);
        ui->grassPic->setScaledContents(true);

        ui->grassCount->setText(QString::number(count));
    } else {
        // undisplay the grass block
        QPixmap grassPix(":/textures/blank.png");
        ui->grassPic->setPixmap(grassPix);
        ui->grassPic->setScaledContents(true);

        ui->grassCount->clear();
    }
}

void Inventory::slot_setDirt(int count) {
    if (count != 0) {
        // display the dirt block
        QPixmap dirtPix(":/textures/dirt_block.png");
        ui->dirtPic->setPixmap(dirtPix);
        ui->dirtPic->setScaledContents(true);

        ui->dirtCount->setText(QString::number(count));
    } else {
        // undisplay the dirt block
        QPixmap dirtPix(":/textures/blank.png");
        ui->dirtPic->setPixmap(dirtPix);
        ui->dirtPic->setScaledContents(true);

        ui->dirtCount->clear();
    }
}

void Inventory::slot_setStone(int count) {
    if (count != 0) {
        // display the stone block
        QPixmap stonePix(":/textures/stone_block.png");
        ui->stonePic->setPixmap(stonePix);
        ui->stonePic->setScaledContents(true);

        ui->stoneCount->setText(QString::number(count));
    } else {
        // undisplay the stone block
        QPixmap stonePix(":/textures/blank.png");
        ui->stonePic->setPixmap(stonePix);
        ui->stonePic->setScaledContents(true);

        ui->stoneCount->clear();
    }
}

void Inventory::slot_setWater(int count) {
    if (count != 0) {
        // display the water block
        QPixmap waterPix(":/textures/water_block.png");
        ui->waterPic->setPixmap(waterPix);
        ui->waterPic->setScaledContents(true);

        ui->waterCount->setText(QString::number(count));
    } else {
        // undisplay the water block
        QPixmap waterPix(":/textures/blank.png");
        ui->waterPic->setPixmap(waterPix);
        ui->waterPic->setScaledContents(true);

        ui->waterCount->clear();
    }
}

void Inventory::slot_setLava(int count) {
    if (count != 0) {
        // display the lava block
        QPixmap lavaPix(":/textures/lava_block.png");
        ui->lavaPic->setPixmap(lavaPix);
        ui->lavaPic->setScaledContents(true);

        ui->lavaCount->setText(QString::number(count));
    } else {
        // undisplay the lava block
        QPixmap lavaPix(":/textures/blank.png");
        ui->lavaPic->setPixmap(lavaPix);
        ui->lavaPic->setScaledContents(true);

        ui->lavaCount->clear();
    }
}
void Inventory::slot_setIce(int count) {
    if (count != 0) {
        // display the ice block
        QPixmap icePix(":/textures/ice_block.png");
        ui->icePic->setPixmap(icePix);
        ui->icePic->setScaledContents(true);

        ui->iceCount->setText(QString::number(count));
    } else {
        // undisplay the ice block
        QPixmap icePix(":/textures/blank.png");
        ui->icePic->setPixmap(icePix);
        ui->icePic->setScaledContents(true);

        ui->iceCount->clear();
    }
}

void Inventory::slot_setSnow(int count) {
    if (count != 0) {
        // display the snow block
        QPixmap snowPix(":/textures/snow_block.png");
        ui->snowPic->setPixmap(snowPix);
        ui->snowPic->setScaledContents(true);

        ui->snowCount->setText(QString::number(count));
    } else {
        // undisplay the snow block
        QPixmap snowPix(":/textures/blank.png");
        ui->snowPic->setPixmap(snowPix);
        ui->snowPic->setScaledContents(true);

        ui->snowCount->clear();
    }
}

void Inventory::slot_setSand(int count) {
    if (count != 0) {
        // display the sand block
        QPixmap sandPix(":/textures/sand_block.png");
        ui->sandPic->setPixmap(sandPix);
        ui->sandPic->setScaledContents(true);

        ui->sandCount->setText(QString::number(count));
    } else {
        // undisplay the sand block
        QPixmap sandPix(":/textures/blank.png");
        ui->sandPic->setPixmap(sandPix);
        ui->sandPic->setScaledContents(true);

        ui->sandCount->clear();
    }
}

void Inventory::slot_getHoldingBlockGrass() {
    this->holding = GRASS;
}

void Inventory::slot_getHoldingBlockDirt() {
    this->holding = DIRT;
}

void Inventory::slot_getHoldingBlockStone() {
    this->holding = STONE;
}

void Inventory::slot_getHoldingBlockWater() {
    this->holding = WATER;
}

void Inventory::slot_getHoldingBlockLava() {
    this->holding = LAVA;
}

void Inventory::slot_getHoldingBlockIce() {
    this->holding = ICE;
}

void Inventory::slot_getHoldingBlockSnow() {
    this->holding = SNOW;
}

void Inventory::slot_getHoldingBlockSand() {
    this->holding = SAND;
}

void Inventory::slot_setHoldingBlock(Player *player) {
//    QString printMe = type_enum_to_string.at(player->holdingBlock);
//    if (this->holding != EMPTY) {
//        player->holdingBlock = this->holding;
////        std::cout << "holding changed in inventory to: " << printMe.toStdString() << std::endl;
//    }

    // Don't change block type if you don't have any left
    if (this->holding == GRASS && player->grassCount != 0) {
        player->holdingBlock = this->holding;
    } else if (this->holding == DIRT && player->dirtCount != 0) {
        player->holdingBlock = this->holding;
    } else if (this->holding == STONE && player->stoneCount != 0) {
        player->holdingBlock = this->holding;
    } else if (this->holding == WATER && player->waterCount != 0) {
        player->holdingBlock = this->holding;
    } else if (this->holding == LAVA && player->lavaCount != 0) {
        player->holdingBlock = this->holding;
    } else if (this->holding == ICE && player->iceCount != 0) {
        player->holdingBlock = this->holding;
    } else if (this->holding == SNOW && player->snowCount != 0) {
        player->holdingBlock = this->holding;
    } else if (this->holding == SAND && player->sandCount != 0) {
        player->holdingBlock = this->holding;
    } else {
        player->holdingBlock = EMPTY;
    }
}


