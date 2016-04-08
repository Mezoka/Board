#include <cstdio>
#include <cstring>
#include <cctype>
#include <fstream>

#include "board.h"

void Board::Clear()
{
    std::vector<Node>().swap(Tree);
    std::vector<Node>().swap(Tree2);

    Node node;
    node.Turn = BLACK;
    node.Pass = 0;
    Tree.push_back(node);

    Turn = BLACK;
    Mode = 3; // play black and white
    Index = 0;
    Index2 = -1;
    Path = 0;
    Pass = 0;

    memset(Point, 0, sizeof(Point));
    memset(Mark, 0, sizeof(Point));
    memset(Area, 0, sizeof(Point));
}

void Board::Reset(int x, int y)
{
    if (x > MAX_BOARD) x = MAX_BOARD;
    else if (x < MIN_BOARD) x = MIN_BOARD;
    if (y > MAX_BOARD) y = MAX_BOARD;
    else if (y < MIN_BOARD) y = MIN_BOARD;

    Size = Width = x;
    Height = y;

    int a = 3;
    int b = Size - 4;
    int c = (Size - 1) / 2;

    Star[0] = a; Star[1] = a;
    Star[2] = b; Star[3] = a;
    Star[4] = a; Star[5] = b;
    Star[6] = b; Star[7] = b;
    Star[8] = c; Star[9] = c;
    Star[10] = c; Star[11] = a;
    Star[12] = a; Star[13] = c;
    Star[14] = b; Star[15] = c;
    Star[16] = c; Star[17] = b;

    Clear();
}

int Board::CheckBound(int x, int y)
{
    return (x >= 0 && y >= 0 && x < Width && y < Height);
}

int Board::GetColor(int x, int y)
{
    return Point[y][x];
}

void Board::Print()
{
    char Color[3] = {'.', 'X', 'O'};

    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {
            printf("%c ", Color[GetColor(x, y)]);
        }
        printf("\n");
    }
    printf("\n");
}

int Board::OtherColor(int color)
{
    return (color ^ 3);
}

Board::Property Board::MakeProperty(int label, int value, int x, int y)
{
    Property Prop;

    Prop.Label = label;
    Prop.Value = value;
    Prop.Row = x;
    Prop.Col = y;

    return Prop;
}

void Board::Capture(int x, int y, int color, Node *n)
{
    if (CheckBound(x, y) && GetColor(x, y) == color) {
        Point[y][x] = 0;
        n->Prop.push_back(MakeProperty(TOKEN_TAKE, color, x, y));

        Capture(x - 1, y, color, n);
        Capture(x + 1, y, color, n);
        Capture(x, y - 1, color, n);
        Capture(x, y + 1, color, n);
    }
}

int Board::CheckLiberty(int x, int y, int color)
{
    if (CheckBound(x, y))
    {
        int k = GetColor(x, y);
        if (k == EMPTY) return 1;
        if (k != color || Mark[y][x] == Path) return 0;

        // visit //
        Mark[y][x] = Path;

        // found liberty //
        if (CheckLiberty(x - 1, y, color)) return 1;
        if (CheckLiberty(x + 1, y, color)) return 1;
        if (CheckLiberty(x, y - 1, color)) return 1;
        if (CheckLiberty(x, y + 1, color)) return 1;
    }

    return 0;
}

// check for capture //
void Board::Check(int x, int y, int color, Node *node)
{
    if (CheckBound(x, y) && GetColor(x, y) == color) {
        Path = Path + 1;
        if (CheckLiberty(x, y, color) == 0)
            Capture(x, y, color, node);
    }
}

void Board::CheckAll(int x, int y, int color, Node *node)
{
    int color2 = OtherColor(color);

    // capture other //
    Check(x - 1, y, color2, node);
    Check(x + 1, y, color2, node);
    Check(x, y - 1, color2, node);
    Check(x, y + 1, color2, node);

    // capture this //
    Check(x, y, color, node);
}

// test play color //
int Board::GetAlpha(int x, int y)
{
    if (Index2 >= 0) {
        for (int i = Index2 + 1; i <= Index; i++) {
            Property Prop = Tree[i].Prop[0];
            if (Prop.Label == TOKEN_PLAY) {
                if (x == Prop.Row && y == Prop.Col) {
                    if (Prop.Value == BLACK) return 224;
                    return 192;
                }
            }
        }
    }

    return 255;
}

int Board::Add(int x, int y, int color)
{
    if (CheckBound(x, y) && GetColor(x, y) == EMPTY) {
        Point[y][x] = color;

        Node *node = &Tree[Index];
        node->Prop.push_back(MakeProperty(TOKEN_ADD, color, x, y));
        CheckAll(x, y, color, node);
        return 1;
    }

    return 0;
}

void Board::TreeCut()
{
    if (int n = Tree.size() - Index - 1) {
        if (Mode == 3 && Index2 < 0 && !BOARD_FILE.isEmpty()) { // backup for test play
            Tree2 = Tree;
            Index2 = Index;
        }
        while (n--) Tree.pop_back();
    }
}

int Board::Play(int x, int y, int color)
{
    Node node;
    int color2 = OtherColor(color);

    if (CheckBound(x, y)) {
        if (GetColor(x, y) == EMPTY) {
            TreeCut();
            Point[y][x] = color;
            Index++;
            node.Prop.push_back(MakeProperty(TOKEN_PLAY, color, x, y));
            CheckAll(x, y, color, &node);

            Turn = color2;
            Pass = 0;
            node.Turn = Turn;
            node.Pass = Pass;
            Tree.push_back(node);
            return 1;
        }
    }
    else {
        TreeCut();
        Index++;
        Pass++;
        node.Prop.push_back(MakeProperty(TOKEN_PASS, color, 0, 0));
        Turn = color2;
        node.Turn = Turn;
        node.Pass = Pass;
        Tree.push_back(node);
        return 1;
    }

    return 0;
}

int Board::Append(int x, int y, int color)
{
    if (Index == Tree.size() - 1)
        return Play(x, y, color);

    int k = Index;
    LastNode();
    Play(x, y, color);
    while (k < Index) PrevNode();
    return 0;
}

int Board::FirstNode()
{
    if (Index > 0) {
        while(PrevNode()) {

        }
        return 1;
    }

    return 0;
}

int Board::LastNode()
{
    if (Index < Tree.size() - 1) {
        while (NextNode()) {

        }
        return 1;
    }

    return 0;
}

int Board::PrevNode(int k)
{
    if (Index <= 0) return 0;

    while (Index && k) {
        int total = Tree[Index].Prop.size();

        for (int i = 0; i < total; i++) {
            Property Prop = Tree[Index].Prop[i];
            if (Prop.Label == TOKEN_PLAY || Prop.Label == TOKEN_ADD)
                Point[Prop.Col][Prop.Row] = 0;
            else if (Prop.Label == TOKEN_TAKE)
                Point[Prop.Col][Prop.Row] = Prop.Value;
        }

        Index--; k--;
        Turn = Tree[Index].Turn;
        Pass = Tree[Index].Pass;

        if (Index == Index2) {
            Tree = Tree2; // restore from test play
            Index2 = -1;
        }
    }

    return 1;
}

int Board::NextNode(int k)
{
    int last = Tree.size() - 1;
    if (Index >= last) return 0;

    while (Index < last && k) {
        Index++;
        int total = Tree[Index].Prop.size();
        Turn = Tree[Index].Turn;
        Pass = Tree[Index].Pass;

        for (int i = 0; i < total; i++) {
            Property Prop = Tree[Index].Prop[i];
            if (Prop.Label == TOKEN_PLAY || Prop.Label == TOKEN_ADD)
                Point[Prop.Col][Prop.Row] = Prop.Value;
            else if (Prop.Label == TOKEN_TAKE)
                Point[Prop.Col][Prop.Row] = 0;
        }

        k--;
    }

    return 1;
}

QString Board::GetText(QString &buf, int i, int j)
{
    return buf.mid(i + 1, j - i - 1);
}

int Board::GetProperty(QString &buf, int i, int j)
{
    QString str = buf.mid(i, j - i);
    str = str.toUpper();

    if (str == "B")
        return TOKEN_BLACK;
    if (str == "W")
        return TOKEN_WHITE;
    if (str == "C")
        return TOKEN_COMMENT;
    if (str == "SZ")
        return TOKEN_SIZE;
    if (str == "PB")
        return TOKEN_PLAY_BLACK;
    if (str == "PW")
        return TOKEN_PLAY_WHITE;
    if (str == "AB")
        return TOKEN_ADD_BLACK;
    if (str == "AW")
        return TOKEN_ADD_WHITE;
    if (str == "EV" || str == "TE")
        return TOKEN_EVENT;
    if (str == "LB")
        return TOKEN_LABEL;
    if (str == "MA")
        return TOKEN_MARK;
    if (str == "TR")
        return TOKEN_TRIANGLE;
    if (str == "CR")
        return TOKEN_CIRCLE;
    if (str == "SQ")
        return TOKEN_SQUARE;
    if (str == "DT" || str == "RD")
        return TOKEN_DATE;
    if (str == "KM" || str == "KO")
        return TOKEN_KOMI;
    if (str == "RE")
        return TOKEN_RESULT;

    return TOKEN_NONE;
}

int Board::GetNumber(QString &buf, int i, int j)
{
    return buf.mid(i + 1, j - i - 1).toInt();
}

int Board::Write(QString Path)
{
    QFile file(Path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return 1;
    QTextStream Stream(&file);

    Stream << QString("(;SZ[%1]KM[%2]PB[%3]PW[%4]DT[%5]RE[%6]").arg(QString::number(Size),
        BOARD_KOMI, PLAYER_BLACK, PLAYER_WHITE, BOARD_DATE, BOARD_RESULT);

    for (int i = 0; i < Tree.size(); i++) {
        for (int k = 0; k < Tree[i].Prop.size(); k++) {
            Property Prop = Tree[i].Prop[k];
            if (Prop.Label == TOKEN_PLAY || Prop.Label == TOKEN_ADD) {
                Stream << ";" << (Prop.Label == TOKEN_ADD ? "A" : "") << (Prop.Value == BLACK ? "B" : "W") <<
                    QString("[%1%2]").arg(QChar('a' + Prop.Row), QChar('a' + Prop.Col));
            }
        }
    }

    Stream << ")";
    file.close();
    return 0;
}

int Board::Read(QString Path, int k)
{
    QFile file(Path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return 0;
    QTextStream Stream(&file);
    QString buf = Stream.readAll();
    file.close();

    int fsize = buf.size();
    if (fsize <= 0) return 0;

    int token = TOKEN_NONE;
    int v1 = -1, v2 = 0;
    int p = -1, q = -2;

    Reset();
    BOARD_FILE = Path;
    BOARD_EVENT.clear(); BOARD_DATE.clear();
    PLAYER_BLACK.clear(); PLAYER_WHITE.clear();
    BLACK_LEVEL.clear(); WHITE_LEVEL.clear();
    BOARD_HANDICAP.clear(); BOARD_KOMI.clear();
    BOARD_RESULT.clear();

    for (int i = 0; i < fsize; i++) {
        QChar c = buf[i];

        if (c == '[' && v1 < 0) { // '[' not in []
            v1 = i;
        }
        else if (c == ')' && v1 < 0) { // ')' not in []
            break;
        }
        else if (c == '\\') {
            if (q >= 0 && i == q + 1) q = -2; // double slashes, ignore it. if q = -1 then i == q + 1 may be true
            else q = i;                       // position of '\'
        }
        else if (c == ']' && i != q + 1 && v1 >= 0) { // not '\]' and have '['
            v2 = i;
            if (p >= 0) token = GetProperty(buf, p, v1); // use previous token for second property

            if (token == TOKEN_BLACK || token == TOKEN_WHITE) {
                int color = (token == TOKEN_BLACK ? BLACK : WHITE);
                if (v2 - v1 == 3) {
                    Play(buf[v1 + 1].toLower().toLatin1() - 'a', buf[v1 + 2].toLower().toLatin1() - 'a', color);
                }
                else Play(-1, -1, color); // pass
                if (--k == 0) return 1;
            }
            else if (token == TOKEN_ADD_BLACK || token == TOKEN_ADD_WHITE) {
                int color = (token == TOKEN_ADD_BLACK ? BLACK : WHITE);
                if (v2 - v1 == 3) {
                    Add(buf[v1 + 1].toLower().toLatin1() - 'a', buf[v1 + 2].toLower().toLatin1() - 'a', color);
                }
            }
            else if (token == TOKEN_LABEL) {
                if (v2 - v1 >= 5 && buf[v1 + 3] == ':') {
                    Property Prop;
                    Prop.Label = TOKEN_LABEL;
                    Prop.Value = buf[v1 + 4].toLatin1();
                    Prop.Row = buf[v1 + 1].toLower().toLatin1() - 'a';
                    Prop.Col = buf[v1 + 2].toLower().toLatin1() - 'a';
                    Tree[Index].Prop.push_back(Prop);
                }
            }
            else if (token == TOKEN_MARK || token == TOKEN_TRIANGLE || token == TOKEN_CIRCLE || token == TOKEN_SQUARE) {
                if (v2 - v1 == 3) {
                    Property Prop;
                    Prop.Label = token;
                    Prop.Row = buf[v1 + 1].toLower().toLatin1() - 'a';
                    Prop.Col = buf[v1 + 2].toLower().toLatin1() - 'a';
                    Tree[Index].Prop.push_back(Prop);
                }
            }
            else if (token == TOKEN_SIZE) {
                Size = GetNumber(buf, v1, v2);
                Reset(Size, Size);
            }
            else if (token == TOKEN_COMMENT)
                Tree[Index].Text += GetText(buf, v1, v2);
            else if (token == TOKEN_EVENT)
                BOARD_EVENT = GetText(buf, v1, v2);
            else if (token == TOKEN_DATE)
                BOARD_DATE = GetText(buf, v1, v2);
            else if (token == TOKEN_KOMI)
                BOARD_KOMI = GetText(buf, v1, v2);
            else if (token == TOKEN_RESULT)
                BOARD_RESULT = GetText(buf, v1, v2);
            else if (token == TOKEN_PLAY_BLACK)
                PLAYER_BLACK = GetText(buf, v1, v2);
            else if (token == TOKEN_PLAY_WHITE)
                PLAYER_WHITE = GetText(buf, v1, v2);

            p = v1 = -1; // reset
        }
        else if (c == ' ' || c == '(' || c == ')' || c == ';' || c == '\n' || c == '\r' || c == '\f') {

        }
        else {
            if (p < 0 && v1 < 0) { // is reset
               p = i;
            }
        }
    }

    FirstNode();
    return 1;
}
