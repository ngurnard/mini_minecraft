#ifndef INVENTORY_H
#define INVENTORY_H

#include <QWidget> // make the gui a widget similar to player info
                   // otherwise would have to make frame buffer object to be like minecraft and that is hard
                   // would have to map clicks on spaces of the screen and stuff... this qt way is easier
#include <ui_mainwindow.h>

namespace Ui {
    class Inventory;
}

class Inventory: public QWidget {
    Q_OBJECT

public:
    explicit Inventory(QWidget *parent = nullptr);
    ~Inventory();

    Ui::MainWindow *ui_mainwindow; // so we can hide the main window when we open the inventory

public slots:
    void slot_setGrass(QString);
    void slot_setDirt(QString);
    void slot_setStone(QString);
    void slot_setWater(QString);
    void slot_setLava(QString);
    void slot_setIce(QString);
    void slot_setSnow(QString s);
    void slot_setSand(QString s);

private:
    Ui::Inventory *ui;
};

// For reference: the block types we have
// EMPTY, GRASS, DIRT, STONE, WATER, LAVA, ICE, SNOW, SAND, UNCERTAIN, BEDROCK

#endif // INVENTORY_H
