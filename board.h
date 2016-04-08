#ifndef __BOARD_H_
#define __BOARD_H_

#include <QtCore>
#include <vector>

#define EMPTY 0
#define BLACK 1
#define WHITE 2

#define MIN_BOARD  2
#define MAX_BOARD  19

class Board
{
public:

    enum
    {
        TOKEN_NONE, TOKEN_BLACK, TOKEN_WHITE, TOKEN_ADD, TOKEN_ADD_BLACK, TOKEN_ADD_WHITE, TOKEN_PLAY,
        TOKEN_TAKE, TOKEN_PASS, TOKEN_LABEL, TOKEN_NUMBER, TOKEN_MARK, TOKEN_TRIANGLE, TOKEN_CIRCLE,
        TOKEN_SQUARE, TOKEN_PLAY_BLACK, TOKEN_PLAY_WHITE, TOKEN_COMMENT, TOKEN_TIME, TOKEN_EVENT,
        TOKEN_PLACE, TOKEN_DATE, TOKEN_RESULT, TOKEN_SIZE, TOKEN_KOMI, TOKEN_HANDICAP
    };

    struct Property
    {
        int Label, Value;
        int Row, Col;
    };

    struct Node
    {
        std::vector<Property> Prop;
        int Turn, Pass, Ko;
        QString Text;
    };

    void Clear();
    void Reset(int x = 19, int y = 19);
    void Print();

    int OtherColor(int color);
    int Play(int x, int y, int color);
    int Add(int x, int y, int color);
    int Append(int x, int y, int color);
    int Read(QString Path, int k = 0);
    int Write(QString Path);
    int CheckBound(int x, int y);
    int GetColor(int x, int y);

    void Capture(int x, int y, int color, Node *node);
    void Check(int x, int y, int color, Node *node);
    void CheckAll(int x, int y, int color, Node *node);
    int CheckLiberty(int x, int y, int color);
    void TreeCut();

    int GetAlpha(int x, int y);
    int GetProperty(QString &buf, int i, int j);
    int GetNumber(QString &buf, int i, int j);

    QString GetText(QString &buf, int i, int j);
    Property MakeProperty(int label, int value, int x, int y);

    int FirstNode();
    int LastNode();
    int PrevNode(int k = 1);
    int NextNode(int k = 1);

    int Point[MAX_BOARD][MAX_BOARD];
    int Mark[MAX_BOARD][MAX_BOARD];
    int Area[MAX_BOARD][MAX_BOARD];

    int Width, Height, Size;
    int Turn, Pass, Mode, Path;
    int Index, Index2;
    int Star[18];

    std::vector<Node> Tree, Tree2;

    QString BOARD_EVENT, BOARD_DATE;
    QString PLAYER_BLACK, PLAYER_WHITE;
    QString BLACK_LEVEL, WHITE_LEVEL;
    QString BOARD_HANDICAP, BOARD_KOMI;
    QString BOARD_RESULT, BOARD_FILE;

};

#endif
