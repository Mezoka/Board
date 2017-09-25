#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <QtWidgets>
#include "Board.h"

enum
{
    BOARD_FILE = 1,
    BOARD_PLAY
};

class Player : public QObject
{
    Q_OBJECT

public:

    Player(QWidget *parent, int color, int size = 0, double komi = 0);
    ~Player();

    void Setup(const QString &str, const QString &arg);
    int CheckTask(const QString &str);
    void Append(const QString &str);
    void Play(QString str, int color = 0);
    void Play(int x, int y, int size, int color = 0);
    void Send();
    void Remove();

    int Side;
    int Wait;

    QProcess *Process;
    QTextEdit *TextEdit;
    QStringList Task;
    QString Respond;

public slots:

    void readStandardOutput();
    void readStandardError();

};

class Widget : public QWidget
{
public:

    Widget(QWidget *parent = NULL);

    void ShowTable(int init = 0);
    void SetTitle(const QString &str);
    void GetRespond(const QString &str, int color);

    int Read(const QString &file, int k = 0);

    Board Child;
    Player *Play[3];

    enum
    {
        VIEW_LABEL = 1,
        VIEW_SCORE = 2,
        VIEW_MARK = 4,
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
    int BOARD_DOWN, BOARD_RIGHT;
    int TABLE_TOP, TABLE_LEFT;
    int TABLE_DOWN, TABLE_RIGHT;
    int TABLE_WIDTH;

};

class Window : public QMainWindow
{
public:

    Window();

    void CreateDock();
    void CreateMenu();
    void SetTitle(QString str = "");

    Widget *Child;
    QString Title;

protected:

    void paintEvent(QPaintEvent *event);

};

#endif
