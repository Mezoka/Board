#ifndef __WINDOW_H
#define __WINDOW_H

#include <QMainWindow>
#include <QProcess>

#include "board.h"

QT_BEGIN_NAMESPACE
class QTextEdit;
QT_END_NAMESPACE

class Player : public QObject
{
    Q_OBJECT

public:

    Player(QWidget *parent, int color);
    ~Player();

    void Setup(QString str, QString arg);
    void PlayMove(QString str);
    void PlayMove(int x, int y, int size);
    void Send();

    int Play;

    QProcess *Process;
    QTextEdit *TextEdit;
    QStringList Task;

    Player *Other;

public slots:

    void readStandardOutput();
    void readStandardError();

protected:

    QString GetRespond(QString str);

};

class BoardView : public QWidget
{
public:

    BoardView(QWidget *parent = 0);
    void Read(QString file);

    Board board;
    Player *Other;

protected:

    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void wheelEvent(QWheelEvent *event);

    QPoint Cursor = QPoint(-1, -1);
    QPoint Cursor2 = QPoint(-1, -1);

    int Dist;
    int x1, y1, x2, y2;

    int ShowLabel = 0;
    int ShowText = 0;

};

class Window : public QMainWindow
{
public:

    Window();
    void CreateDock();

    BoardView *Widget;

    QTextEdit *TextEdit;
    QTextEdit *TextEdit2;

    Player *Player1;
    Player *Player2;

};

#endif
