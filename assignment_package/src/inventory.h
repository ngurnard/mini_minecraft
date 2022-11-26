#ifndef INVENTORY_H
#define INVENTORY_H

#include <QWidget> // make the gui a widget similar to player info
                   // otherwise would have to make frame buffer object to be like minecraft and that is hard
                   // would have to map clicks on spaces of the screen and stuff... this qt way is easier
#include <QKeyEvent> // need this in order to close the inventory
#include <QPixmap>
#include "scene/player.h"

namespace Ui {
    class Inventory;
}

class Inventory: public QWidget {
    Q_OBJECT

public:
    explicit Inventory(QWidget *parent = nullptr);
    ~Inventory();

    // Need to get the keypress to close since we switch from mainwindow
    void keyPressEvent(QKeyEvent *e);
    void setInventoryPlayer(Player &player);

public slots:
    void slot_setGrass(int);
    void slot_setDirt(int);
    void slot_setStone(int);
    void slot_setWater(int);
    void slot_setLava(int);
    void slot_setIce(int);
    void slot_setSnow(int);
    void slot_setSand(int);

//    void slot_setHoldingBlock(Player &player);


private:
    Ui::Inventory *ui;
};

// For reference: the block types we have
// EMPTY, GRASS, DIRT, STONE, WATER, LAVA, ICE, SNOW, SAND, UNCERTAIN, BEDROCK

#endif // INVENTORY_H
