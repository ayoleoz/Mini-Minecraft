#include <QKeyEvent>
#include <QPixmap>
#include "inventory.h"
#include "ui_inventory.h"

Inventory::Inventory(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Inventory)
{
    ui->setupUi(this);

    connect(ui->grassRadioButton, SIGNAL(clicked()), this, SLOT(slot_setCurrBlockToGrass()));
    connect(ui->dirtRadioButton, SIGNAL(clicked()), this, SLOT(slot_setCurrBlockToDirt()));
    connect(ui->stoneRadioButton, SIGNAL(clicked()), this, SLOT(slot_setCurrBlockToStone()));
    connect(ui->snowRadioButton, SIGNAL(clicked()), this, SLOT(slot_setCurrBlockToSnow()));
    connect(ui->bedrockRadioButton, SIGNAL(clicked()), this, SLOT(slot_setCurrBlockToBedRock()));
    connect(ui->lavaRadioButton, SIGNAL(clicked()), this, SLOT(slot_setCurrBlockToLava()));
    connect(ui->waterRadioButton, SIGNAL(clicked()), this, SLOT(slot_setCurrBlockToWater()));
}

Inventory::~Inventory() {
    delete ui;
}

// close the inventory
void Inventory::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_I) {
        this->close();
    }
}

// update quantities
void Inventory::slot_setNumGrass(int n) {
    ui->numGrass->display(n);
}

void Inventory::slot_setNumDirt(int n) {
    ui->numDirt->display(n);
}

void Inventory::slot_setNumStone(int n) {
    ui->numStone->display(n);
}

void Inventory::slot_setNumSnow(int n) {
    ui->numSnow->display(n);
}

void Inventory::slot_setNumBedrock(int n) {
    ui->numBedrock->display(n);
}

void Inventory::slot_setNumWater(int n) {
    ui->numWater->display(n);
}

void Inventory::slot_setNumLava(int n) {
    ui->numLava->display(n);
}

// set current block
void Inventory::slot_setCurrBlockToGrass() {
    ui_main->mygl->currBlockType = GRASS;
}

void Inventory::slot_setCurrBlockToDirt() {
    ui_main->mygl->currBlockType = DIRT;
}

void Inventory::slot_setCurrBlockToStone() {
    ui_main->mygl->currBlockType = STONE;
}

void Inventory::slot_setCurrBlockToSnow() {
    ui_main->mygl->currBlockType = SNOW;
}

void Inventory::slot_setCurrBlockToBedrock() {
    ui_main->mygl->currBlockType = BEDROCK;
}

void Inventory::slot_setCurrBlockToWater() {
    ui_main->mygl->currBlockType = WATER;
}

void Inventory::slot_setCurrBlockToLava() {
    ui_main->mygl->currBlockType = LAVA;
}
