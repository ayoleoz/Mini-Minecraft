#ifndef INVENTORY_H
#define INVENTORY_H

#include <QWidget>
#include <QPushButton>
#include <ui_mainwindow.h>

namespace Ui {
class Inventory;
}

class Inventory: public QWidget
{
    Q_OBJECT

public:
    explicit Inventory(QWidget *parent = nullptr);
    ~Inventory();

    void keyPressEvent(QKeyEvent *e);
    Ui::MainWindow *ui_main;

public slots:
    void slot_setNumGrass(int);
    void slot_setNumDirt(int);
    void slot_setNumStone(int);
    void slot_setNumSnow(int);
    void slot_setNumBedrock(int);
    void slot_setNumWater(int);
    void slot_setNumLava(int);

    void slot_setCurrBlockToGrass();
    void slot_setCurrBlockToDirt();
    void slot_setCurrBlockToStone();
    void slot_setCurrBlockToSnow();
    void slot_setCurrBlockToBedrock();
    void slot_setCurrBlockToWater();
    void slot_setCurrBlockToLava();

private:
    Ui::Inventory *ui;
};

#endif // INVENTORY_H
