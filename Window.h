#ifndef __WINDOW_H_
#define __WINDOW_H_

#include <QtWidgets>
#include "Go.h"

enum
{
    MODE_FILE = 1,
    MODE_PLAY
};

class Player : public QObject
{
    Q_OBJECT

public:

    Player(QWidget *parent, int color);
    ~Player();

    void Setup(QString str, QString arg);
    void Play(QString str);
    void Play(int x, int y, int size);
    void Send();
    void Remove();

    int Side;
    int Wait;

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

class Board : public QWidget
{
public:

    Board(QWidget *parent = NULL);
    int Read(const QString &file, int k = 0);
    void ShowTable(int init = 0);

    Go Child;
    Player *Other;

    enum
    {
        SHOW_TEXT = 1,
        SHOW_LABEL = 2,
        SHOW_GRID_LABEL = 4,
        SHOW_GRID_SCORE = 8,
        SHOW_SCORE = 16,
        PLAY_PAUSE = 32
    };

    int Mode, View;
    int Side;

    QString Title;
    QTextEdit *TextEdit;
    QTableWidget *Table;

protected:

    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void wheelEvent(QWheelEvent *event);

    QPoint Cursor[2];

    int GridSize;

    int BOARD_TOP, BOARD_LEFT;
    int BOARD_LOWER, BOARD_RIGHT;
    int TABLE_TOP, TABLE_LEFT;
    int TABLE_WIDTH;

};

class Window : public QMainWindow
{
public:

    Window();
    void CreateDock();
    void SetTitle(QString str = "");

    Board *Child;

    QTextEdit *TextEdit;
    QTextEdit *TextEdit2;
    QString Title;

    Player *Player1;
    Player *Player2;

protected:

    void paintEvent(QPaintEvent *event);

};

#endif
