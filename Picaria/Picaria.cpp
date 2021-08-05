#include "Picaria.h"
#include "ui_Picaria.h"

#include <QDebug>
#include <QMessageBox>
#include <QActionGroup>
#include <QSignalMapper>

#include <cstdbool>
#include <cstdio>

// Adjacent Positions to each button
// false: not-available; true: adjacent;
int adjacentHoles_09HolesMode[13][13]= {
         {false, true,  false, false, false, true,  true,  false, false, false, false, false, false}, // Hole 01
         {true,  false, true,  false, false, true,  true,  true,  false, false, false, false, false}, // Hole 02
         {false, true,  false, false, false, false, true,  true,  false, false, false, false, false}, // Hole 03
         {false, false, false, false, false, false, false, false, false, false, false, false, false}, // Hole 04
         {false, false, false, false, false, false, false, false, false, false, false, false, false}, // Hole 05
         {true,  true,  false, false, false, false, true,  false, false, false, true,  true,  false}, // Hole 06
         {true,  true,  true,  false, false, true,  false, true,  false, false, true,  true,  true }, // Hole 07
         {false, true,  true,  false, false, false, true,  false, false, false, false, true,  true }, // Hole 08
         {false, false, false, false, false, false, false, false, false, false, false, false, false}, // Hole 09
         {false, false, false, false, false, false, false, false, false, false, false, false, false}, // Hole 10
         {false, false, false, false, false, true,  true,  false, false, false, false, true,  false}, // Hole 11
         {false, false, false, false, false, true,  true,  true,  false, false, true, false,  true }, // Hole 12
         {false, false, false, false, false, false, true,  true,  false, false, false, true,  false}, // Hole 13
        };
int adjacentHoles_13HolesMode[13][13]= {
         {false, true,  false, true,  false, true,  false, false, false, false, false, false, false}, // Hole 01
         {true,  false, true,  true,  true,  false, false, false, false, false, false, false, false}, // Hole 02
         {false, true,  false, false, true,  false, false, true,  false, false, false, false, false}, // Hole 03
         {true,  true,  false, false, false, true,  true,  false, false, false, false, false, false}, // Hole 04
         {false, true,  true,  false, false, false, true,  true,  false, false, false, false, false}, // Hole 05
         {true,  false, false, true,  false, false, true,  false, true,  false, true,  false, false}, // Hole 06
         {false, true,  false, true,  true,  true,  true,  true,  true,  true,  false, true,  false}, // Hole 07
         {false, false, true,  false, true,  false, true,  false, false, true,  false, false, true }, // Hole 08
         {false, false, false, false, false, true,  true,  false, false, false, true,  true,  false}, // Hole 09
         {false, false, false, false, false, false, true,  true,  false, false, false, true,  true }, // Hole 10
         {false, false, false, false, false, true,  false, false, true,  false, false, true,  false}, // Hole 11
         {false, false, false, false, false, false, false, false, true,  true,  true, false,  true }, // Hole 12
         {false, false, false, false, false, false, false, true,  false, true,  false, true,  false}, // Hole 13
        };
int previousButtonID = -1; // Store the button position to move pieces on phase 2

Picaria::Player state2player(Hole::State state) {
    switch (state) {
        case Hole::RedState:
            return Picaria::RedPlayer;
        case Hole::BlueState:
            return Picaria::BluePlayer;
        default:
            Q_UNREACHABLE();
    }
}

Hole::State player2state(Picaria::Player player) {
    return player == Picaria::RedPlayer ? Hole::RedState : Hole::BlueState;
}

Picaria::Picaria(QWidget *parent) : QMainWindow(parent), ui(new Ui::Picaria), m_mode(Picaria::NineHoles), m_player(Picaria::RedPlayer), m_phase(Picaria::DropPhase) {
    ui->setupUi(this);

    QActionGroup* modeGroup = new QActionGroup(this);
    modeGroup->setExclusive(true);
    modeGroup->addAction(ui->action9holes);
    modeGroup->addAction(ui->action13holes);

    QObject::connect(ui->actionNew, SIGNAL(triggered(bool)), this, SLOT(reset()));
    QObject::connect(ui->actionQuit, SIGNAL(triggered(bool)), qApp, SLOT(quit()));
    QObject::connect(modeGroup, SIGNAL(triggered(QAction*)), this, SLOT(updateMode(QAction*)));
    QObject::connect(this, SIGNAL(modeChanged(Picaria::Mode)), this, SLOT(reset()));
    QObject::connect(ui->actionAbout, SIGNAL(triggered(bool)), this, SLOT(showAbout()));

    QSignalMapper* map = new QSignalMapper(this);
    for (int id = 0; id < 13; ++id) {
        QString holeName = QString("hole%1").arg(id+1, 2, 10, QChar('0'));
        Hole* hole = this->findChild<Hole*>(holeName);
        Q_ASSERT(hole != nullptr);

        m_holes[id] = hole;
        map->setMapping(hole, id);
        QObject::connect(hole, SIGNAL(clicked(bool)), map, SLOT(map()));
    }

    #if QT_VERSION < QT_VERSION_CHECK(6,0,0)
        QObject::connect(map, SIGNAL(mapped(int)), this, SLOT(play(int)));
    #else
        QObject::connect(map, SIGNAL(mappedInt(int)), this, SLOT(play(int)));
    #endif

    this->reset();

    this->adjustSize();
    this->setFixedSize(this->size());
}

Picaria::~Picaria() {
    delete ui;
}

void Picaria::setMode(Picaria::Mode mode) {
    if (m_mode != mode) {
        m_mode = mode;
        emit modeChanged(mode);
    }
}

void Picaria::switchPlayer() {
    m_player = m_player == Picaria::RedPlayer ? Picaria::BluePlayer : Picaria::RedPlayer;
    this->updateStatusBar();
}

void Picaria::play(int id) {
    Hole* hole = m_holes[id];

    // Phase 1
    if (m_phase == Picaria::DropPhase) {
        if (bluePieces < 3 || redPieces < 3) {
            if (this->m_player == BluePlayer && bluePieces < 3 && hole->state() == hole->EmptyState){
                this->bluePieces++;
            } else if (this->m_player == RedPlayer && redPieces < 3 && hole->state() == hole->EmptyState) {
                this->redPieces++;
            } else {
                return;
            }

            // EmptyState or SelectableState
            if(hole->state() == 0 || hole->state() == 3){
                qDebug() << "clicked on: " << hole->objectName();

                hole->setState(player2state(m_player));

                if (bluePieces == 3 && redPieces == 3){
                    m_phase = Picaria::MovePhase; // change to phase 2
                }

                this->switchPlayer();
            }
        }
    } else /* Phase 2 */{

        // Select piece to move
        if ( (hole->state() == hole->BlueState && m_player == Picaria::BluePlayer) ||
             (hole->state() == hole->RedState && m_player == Picaria::RedPlayer) ){

            // Clean old selectables
            for(int i = 0; i < 13; i++){
                Hole* currentHole = m_holes[i];
                if (currentHole->state() == currentHole->SelectableState){
                    currentHole->setState(currentHole->EmptyState);
                }
            }

            // Mark current selectables
            for(int i = 0; i < 13; i++){
                Hole* currentHole = m_holes[i];

                if(m_mode == Picaria::Mode::NineHoles){
                    if(adjacentHoles_09HolesMode[id][i] && currentHole->state() == currentHole->EmptyState){
                        currentHole->setState(hole->SelectableState);
                        previousButtonID = id;
                    }
                } else {
                    if(adjacentHoles_13HolesMode[id][i] && currentHole->state() == currentHole->EmptyState){
                        currentHole->setState(hole->SelectableState);
                        previousButtonID = id;
                    }
                }
            }

            return;
        } else if (hole->state() == hole->SelectableState){
            // Set new position
            if (m_player == Picaria::RedPlayer){
                hole->setState(hole->RedState);
            } else {
                hole->setState(hole->BlueState);
            }

            // Clean old position
            Hole* oldHole = m_holes[previousButtonID];
            oldHole->setState(oldHole->EmptyState);

            // Clean selectable states
            for (int i = 0; i < 13; i++){
                Hole* currentHole = m_holes[i];
                if(currentHole->state() == currentHole->SelectableState){
                    currentHole->setState(currentHole->EmptyState);
                }
            }

            previousButtonID = -1;
            this->switchPlayer();
        } else {
            return;
        }

    }

}

void Picaria::reset() {
    // Reset pieces counter
    this->bluePieces = 0;
    this->redPieces = 0;

    // Reset each hole.
    for (int id = 0; id < 13; ++id) {
        Hole* hole = m_holes[id];
        hole->reset();

        // Set the hole visibility according to the board mode.
        switch (id) {
            case 3:
            case 4:
            case 8:
            case 9:
                hole->setVisible(m_mode == Picaria::ThirteenHoles);
                break;
            default:
                break;
        }
    }

    // Reset the player and phase.
    m_player = Picaria::RedPlayer;
    m_phase = Picaria::DropPhase;

    // Finally, update the status bar.
    this->updateStatusBar();
}

void Picaria::showAbout() {
    QMessageBox::information(this, tr("About"), tr("Picaria\n\nPedro Henrique Estevam Vaz de Melo - vazconnected@gmail.com\nAna Julia Velasque Rodrigues - anajvelasque@gmail.com"));
}

void Picaria::updateMode(QAction* action) {
    this->reset();
    if (action == ui->action9holes)
        this->setMode(Picaria::NineHoles);
    else if (action == ui->action13holes)
        this->setMode(Picaria::ThirteenHoles);
    else
        Q_UNREACHABLE();
}

void Picaria::updateStatusBar() {
    QString player(m_player == Picaria::RedPlayer ? "vermelho" : "azul");
    QString phase(m_phase == Picaria::DropPhase ? "colocar" : "mover");

    ui->statusbar->showMessage(tr("Fase de %1: vez do jogador %2").arg(phase).arg(player));
}
