#ifndef HOLE_H
#define HOLE_H

#include <QObject>
#include <QPushButton>

class Hole : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(State state READ state WRITE setState NOTIFY stateChanged)

    public:
        enum State {
            EmptyState, // 0
            RedState,   // 1
            BlueState,  // 2
            SelectableState // 4
        };
        Q_ENUM(State)

        explicit Hole(QWidget *parent = nullptr);
        virtual ~Hole();

        State state() const {
            return m_state;
        }
        void setState(State State);

    public slots:
        void reset();

    signals:
        void stateChanged(State State);

    private:
        State m_state;
        static QPixmap stateToPixmap(State state);

    private slots:
        void updateHole(State state);

};

#endif // HOLE_H
