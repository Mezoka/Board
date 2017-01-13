#ifndef __BOARD_H__
#define __BOARD_H__

#include <QtCore>

#define MIN_SIZE 2
#define MAX_SIZE 19

#define BOARD_MAX (MAX_SIZE + 1) * (MAX_SIZE + 2) + 1
#define MOVE_MAX  MAX_SIZE * MAX_SIZE + 2
#define MAX_LEN MAX_SIZE * MAX_SIZE * 2

// Simple List
inline void SLIST_CLEAR(int *ls) { ls[0] = 0; }
inline void SLIST_PUSH(int *ls, int i) { ls[++ls[0]] = i; }
inline int SLIST_POP(int *ls) { return ls[ls[0]--]; }
inline void SLIST_ADD(int *ls, int i) {
    for (int k = 1; k <= ls[0]; k++) if (ls[k] == i) return;
    ls[++ls[0]] = i; }
inline void SLIST_DELETE(int *ls, int i) { ls[i] = ls[ls[0]--]; }
inline int SLIST_RAND(int *ls) { return rand() % ls[0] + 1; }
inline void SLIST_MERGE(int *ls, int *ls2) {
    for (int k = 1; k <= ls2[0]; k++) ls[++ls[0]] = ls2[k]; }
inline void SLIST_PRINT(int *ls) {
    for (int k = 1; k <= ls[0]; k++) printf("%d ", ls[k]); printf("\n"); }

enum Color
{
    EMPTY, BLACK, WHITE, OUT
};

enum
{
    BOTH_AREA, BLACK_AREA, WHITE_AREA,
    BLACK_DEAD, WHITE_DEAD
};

class Go
{
    public:

    enum
    {
        PASS, RESIGN
    };

    enum
    {
        TOKEN_NONE, TOKEN_BLACK, TOKEN_WHITE, TOKEN_ADD, TOKEN_ADD_BLACK, TOKEN_ADD_WHITE, TOKEN_PLAY,
        TOKEN_TAKE, TOKEN_PASS, TOKEN_LABEL, TOKEN_NUMBER, TOKEN_MARK, TOKEN_TRIANGLE, TOKEN_CIRCLE,
        TOKEN_SQUARE, TOKEN_PLAY_BLACK, TOKEN_PLAY_WHITE, TOKEN_COMMENT, TOKEN_TIME, TOKEN_EVENT,
        TOKEN_PLACE, TOKEN_DATE, TOKEN_RESULT, TOKEN_SIZE, TOKEN_KOMI, TOKEN_HANDICAP
    };

    inline static int OtherColor(int color)
    {
        return color ^ 3;
    }

    struct GoProp
    {
        int Label, Value;
        int Col, Row;
    };

    struct Node // Record
    {
        std::vector<GoProp> Prop;
        QString Text;
        int Ko, Turn, Pass;
    };

    struct State
    {
        int Point[BOARD_MAX];
        int Ko, Turn, Pass;
    };

    void Clear();
    void ClearMark();
    void Reset(int x = 19, int y = 19);

    void ShowBoard();
    void PrintHeader();
    void PrintMove(int z);

    int Read(const QString &Path, int k = 0);
    int Write(const QString &Path);

    int GetColor(int x, int y);
    int GetPoint(int x, int y);
    int CheckBound(int x, int y);
    int Play(int x, int y, int color, int mode = 0);
    int Append(int x, int y, int color, int i = 1);
    int Add(int x, int y, int color);
    int Remove(int z, int color, Node &node);
    int CheckPlay(int z, int color, int color2);
    int CheckRemove(int z, int color);

    int GetProp(QString &buf, int i, int j);
    int GetNumber(QString &buf, int i, int j);

    QString GetText(QString &buf, int i, int j);
    GoProp MakeProp(int label, int value, int x, int y);

    int Start();
    int Forward(int k = 1);
    int End();
    int Undo(int k = 1);

    void Cut();
    void Rand();
    void Rand(int total);
    void ShowBlock();

    int Width, Height, Size;
    int Index, Index2, Path;

    int Mark[BOARD_MAX];
    int Area[BOARD_MAX];
    int Move[MOVE_MAX];
    int Block[MOVE_MAX];
    int Side[8];
    int Star[18];

    int BLACK_SCORE;
    int WHITE_SCORE;

    double FINAL_SCORE;
    double Komi;

    int DATA_WIDTH, DATA_HEIGHT;
    int DATA_START, DATA_END;

    State Stat;

    std::vector<Node> Tree, Tree2;

    QString BOARD_EVENT, BOARD_DATE;
    QString PLAYER_BLACK, PLAYER_WHITE;
    QString BLACK_LEVEL, WHITE_LEVEL;
    QString BOARD_HANDICAP, BOARD_KOMI;
    QString BOARD_RESULT, BOARD_FILE;
    QString BOARD_SCORE;

};

#endif
