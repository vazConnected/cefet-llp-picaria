#ifndef PICARIA_H
#define PICARIA_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
    class Picaria;
}
QT_END_NAMESPACE

class Hole;

class Picaria : public QMainWindow {
    Q_OBJECT
    Q_PROPERTY(Picaria::Mode mode READ mode WRITE setMode NOTIFY modeChanged)

    public:
        enum Mode {
            NineHoles,
            ThirteenHoles
        };
        Q_ENUM(Mode)

        enum Player {
            RedPlayer,
            BluePlayer
        };
        Q_ENUM(Player)

        enum Phase {
            DropPhase,
            MovePhase
        };
        Q_ENUM(Phase)

        Picaria(QWidget *parent = nullptr);
        virtual ~Picaria();

        Picaria::Mode mode() const { return m_mode; }
        void setMode(Picaria::Mode mode);

    signals:
        void modeChanged(Picaria::Mode mode);

    private:
        Ui::Picaria *ui;    // user interface
        Hole* m_holes[13];  // posicoes no tabuleiro
        Mode m_mode;
        Player m_player;
        Phase m_phase;
        int redPieces = 0;      // Peças vermelha (jogador 1)
        int bluePieces = 0;     // Peças azuis (jogador 2)

        void switchPlayer();

    private slots:
        void play(int id);
        void reset();

        void showAbout();

        void updateMode(QAction* action);
        void updateStatusBar();

};

#endif // PICARIA_H
