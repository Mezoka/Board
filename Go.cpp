#include "Go.h"

void Go::Reset(int x, int y)
{
    Size = Width = x;
    Height = y;

    DATA_WIDTH = Width + 1;
    DATA_HEIGHT = Height + 2;

    DATA_START = Width + 2;
    DATA_END = (Width + 1) * (Height + 1) - 1;

    Side[0] = 1; Side[1] = DATA_WIDTH;
    Side[2] = -1; Side[3] = -DATA_WIDTH;

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

void Go::Clear()
{
    Stat.Point[DATA_WIDTH * DATA_HEIGHT] = OUT;
    Stat.Ko = 0, Stat.Pass = 0, Stat.Turn = BLACK;
    BOARD_KOMI = "7.5", Komi = 7.5, Index = 0, Index2 = -1;

    ClearMark();

    for (int i = 1, k = 0; i <= DATA_HEIGHT; i++) {
        for (int j = 1; j <= DATA_WIDTH; j++, k++) {
            if (i == 1 || i == DATA_HEIGHT || j == 1) Stat.Point[k] = OUT;
            else Stat.Point[k] = EMPTY;
        }
    }

    std::vector<Node>().swap(Tree);

    Node node;
    node.Turn = BLACK;
    node.Pass = 0;
    Tree.push_back(node);
}

void Go::ClearMark()
{
    memset(Mark, 0 , sizeof(Mark));
    Path = 0;
}

int Go::GetPoint(int x, int y)
{
    return (y + 1) * DATA_WIDTH + x + 1;
}

int Go::GetColor(int x, int y)
{
    return Stat.Point[GetPoint(x, y)];
}

void Go::PrintHeader()
{
    fprintf(stdout, "  ");
    for (int j = 0; j < Width; j++)
        fprintf(stdout, " %c", 'A' + j + (j >= 8));
}

void Go::ShowBoard()
{
    int color[3] = {'.', 'X', 'O'};
    PrintHeader();
  
    for (int i = 1, k = 0; i <= DATA_HEIGHT; i++) {
        if (i != 1 && i != DATA_HEIGHT) fprintf(stdout, "\n%2d", Height - i + 2);

        for (int j = 1; j <= DATA_WIDTH; j++, k++) {
            if (i != 1 && i != DATA_HEIGHT && j != 1) {
                fprintf(stdout, " %c", color[Stat.Point[k]]);
            }
        }
        if (i != 1 && i != DATA_HEIGHT) fprintf(stdout, " %d", Height - i + 2);
    }

    fprintf(stdout, "\n"); PrintHeader();
    fprintf(stdout, "\n\n");
}

void Go::PrintMove(int z)
{
    int y = z / DATA_WIDTH;
    int x = z - y * DATA_WIDTH;

    if (x >= 9) x++;
    printf("%c%d ", 'A' + x - 1, Height - y + 1);
}

void Go::ShowBlock()
{
    ClearMark();

    for (int i = DATA_START; i <= DATA_END; i++) {
        int k = Stat.Point[i];
        if ((k == BLACK || k == WHITE) && Mark[i] == 0) { // not visit
            Path++;
            Mark[i] = Path;
            SLIST_CLEAR(Block);
            SLIST_PUSH(Block, i); PrintMove(i);

            for (int n = 1; n <= Block[0]; n++) {
                for (int d = 0; d < 4; d++) {
                    int i2 = Block[n] + Side[d];
                    int k2 = Stat.Point[i2];
                    if (k2 == k && Mark[i2] == 0) { // not vist
                        Mark[i2] = Path;
                        SLIST_PUSH(Block, i2); PrintMove(i2);
                    }
                }
            }

            printf("\n");
        }
    }

    printf("\n");
}

int Go::CheckBound(int x, int y)
{
    return (x >= 0 && x < Width && y >= 0 && y < Height);
}

int Go::Add(int x, int y, int color)
{
    int z = 0;
    if (CheckBound(x, y)) {
        z = GetPoint(x, y);
        if (Stat.Point[z] != EMPTY) return 0;
    }
    else return 0;

    Node node;
    int color2 = OtherColor(color);
    int cp = 0;

    Stat.Point[z] = color;
    node.Prop.push_back(MakeProp(TOKEN_ADD, color, x, y));

    for (int i = 0; i < 4; i++) {
        int k = z + Side[i];
        if (Stat.Point[k] == color2) {
            cp += Remove(k, color2, node);
        }
    }

    if (cp == 0)
        Remove(z, color, node);

    return 1;
}

// Play Board at z for color with bound, ko and empty check
int Go::Play(int x, int y, int color, int mode)
{
    int z = 0;

    if (CheckBound(x, y)) {
        z = GetPoint(x, y);
        if (Stat.Point[z] != EMPTY || z == Stat.Ko) return 0;
    }

    if (mode && Index2 < 0) { // backup for test play
        Tree2 = Tree;
        Index2 = Index;
    }

    Cut();

    Node node;
    int color2 = OtherColor(color);
    Stat.Ko = 0;

    if (z == 0) {
        Stat.Pass++;
        node.Prop.push_back(MakeProp(TOKEN_PASS, color, 0, 0));
    }
    else {
        Stat.Pass = 0;
        Stat.Point[z] = color;
        node.Prop.push_back(MakeProp(TOKEN_PLAY, color, x, y));
        int r[4] = { 0, 0, 0, 0 };
        int size, Ko = 0, cp = 0;

        for (int i = 0; i < 4; i++) // round check
            r[Stat.Point[z + Side[i]]] = 1;

        for (int i = 0; i < 4; i++) {
            int k = z + Side[i];
            if (Stat.Point[k] == color2) {
                size = Remove(k, color2, node);
                if (size == 1) Ko = k;
                cp = cp + size;
            }
        }

        if (cp == 1 && r[color] == 0 && r[0] == 0)
            Stat.Ko = Ko;
        if (cp == 0)
            Remove(z, color, node);
    }

    Stat.Turn = color2;
    node.Turn = Stat.Turn;
    node.Pass = Stat.Pass;
    node.Ko = Stat.Ko;
    Tree.push_back(node);
    Index++;
    return 1;
}

int Go::Append(int x, int y, int color, int i)
{
    if (i && (Index == Tree.size() - 1))
        return Play(x, y, color);

    int k = Index;
    End();
    int result = Play(x, y, color);
    Undo(Index - k);
    return result;
}

int Go::Remove(int z, int color, Node &node)
{
    Path++;
    Mark[z] = Path;

    SLIST_CLEAR(Block);
    SLIST_PUSH(Block, z); // point to check

    for (int n = 1; n <= Block[0]; n++) {
        int k = Block[n];
        for (int i = 0; i < 4; i++) {
            int k2 = k + Side[i];
            int p = Stat.Point[k2];
            if (p == EMPTY) return 0; // liberty
            if (p == color && Mark[k2] != Path) {
                Mark[k2] = Path;
                SLIST_PUSH(Block, k2);
            }
        }
    }

    for (int n = 1; n <= Block[0]; n++) {
        int k = Block[n];
        Stat.Point[k] = EMPTY;
        int y = k / DATA_WIDTH;
        int x = k - y * DATA_WIDTH;
        node.Prop.push_back(MakeProp(TOKEN_TAKE, color, x - 1, y - 1));
    }

    return Block[0];
}

int Go::CheckRemove(int z, int color)
{
    Path++;
    Mark[z] = Path;

    SLIST_CLEAR(Block);
    SLIST_PUSH(Block, z); // point to check

    for (int n = 1; n <= Block[0]; n++) {
        int k = Block[n];
        for (int i = 0; i < 4; i++) {
            int k2 = k + Side[i];
            int p = Stat.Point[k2];
            if (p == EMPTY) return 0; // liberty
            if (p == color && Mark[k2] != Path) {
                Mark[k2] = Path;
                SLIST_PUSH(Block, k2);
            }
        }
    }

    for (int n = 1; n <= Block[0]; n++) {
        int k = Block[n];
        Stat.Point[k] = EMPTY;
        SLIST_PUSH(Move, k);
    }

    return Block[0];
}

// Play Board at z and returns 1 if no ko, or fill eye
int Go::CheckPlay(int z, int color, int color2)
{
    if (z == Stat.Ko) return 1;
    int r[4] = { 0, 0, 0, 0 };
    for (int i = 0; i < 4; i++) // round check
        r[Stat.Point[z + Side[i]]] = 1;
    if (r[0] == 0 && r[color2] == 0) // fill eye
        return 1;

    Stat.Point[z] = color;
    int size, Ko = 0, cp = 0;

    for (int i = 0; i < 4; i++) {
        int k = z + Side[i];
        if (Stat.Point[k] == color2) {
            size = CheckRemove(k, color2);
            if (size == 1) {
                Ko = 1;
                Stat.Ko = k; // may be ko
            }
            cp = cp | size;
        }
    }

    if (cp == 0) {
        if (r[0] == 0 && r[color] == 0) { // fill eye and suicde
            Stat.Point[z] = EMPTY;
            return 1;
        }
        CheckRemove(z, color);
    }

    Stat.Pass = 0;
    if (Ko == 0) Stat.Ko = 0;
    return 0;
}

void Go::Rand()
{
    int color = Stat.Turn;
    int color2 = OtherColor(color);
    int n = MAX_LEN;

    int Move2[MOVE_MAX];

    ClearMark();

    SLIST_CLEAR(Move);
    SLIST_CLEAR(Move2);

    for (int i = DATA_START; i <= DATA_END; i++)
        if (Stat.Point[i] == EMPTY) SLIST_PUSH(Move, i); // empty moves

    Stat.Pass = 0;

    while (Stat.Pass < 2 && n--) {
        if (Move[0] == 0) {
            Stat.Pass++;
            Stat.Ko = 0;
        }
        else {
            int i = SLIST_RAND(Move);
            if (CheckPlay(Move[i], color, color2)) {
                SLIST_PUSH(Move2, Move[i]);
                SLIST_DELETE(Move, i);
                continue;
            }

            SLIST_DELETE(Move, i);
        }

        SLIST_MERGE(Move, Move2);
        SLIST_CLEAR(Move2);
        color2 = color;
        color = OtherColor(color);
    }
}

void Go::Rand(int total)
{
    State Stat2 = Stat;
    int Result[BOARD_MAX];
    memset(Result, 0, sizeof(Mark));

    for (int n = 0; n < total; n++) {
        Rand();
        for (int i = DATA_START; i <= DATA_END; i++) { // update result
            int k = Stat.Point[i];
            if (k == BLACK) Result[i]++;
            else if (k == WHITE) Result[i]--;
            else if (k == EMPTY) {
                int k2 = Stat.Point[i+1];
                if (k2 == BLACK) Result[i]++;
                else if (k2 == WHITE) Result[i]--;
                else {
                    k2 = Stat.Point[i-1];
                    if (k2 == BLACK) Result[i]++;
                    else Result[i]--;
                }
            }
        }
        Stat = Stat2;
    }

    // Find largest value in block and set block to that value
    ClearMark();

    for (int i = DATA_START; i <= DATA_END; i++) {
        int k = Stat.Point[i];
        if ((k == BLACK || k == WHITE) && Mark[i] == 0) { // not visit
            Path++;
            Mark[i] = Path;
            SLIST_CLEAR(Block);
            SLIST_PUSH(Block, i);
            int Value = (k == BLACK ? -total : total);

            for (int n = 1; n <= Block[0]; n++) {
                int score = Result[Block[n]];
                if ((k == BLACK && score > Value) ||
                    (k == WHITE && score < Value)) Value = score;
                for (int d = 0; d < 4; d++) {
                    int i2 = Block[n] + Side[d];
                    int k2 = Stat.Point[i2];
                    if (k2 == k && Mark[i2] == 0) { // not vist
                        Mark[i2] = Path;
                        SLIST_PUSH(Block, i2);
                    }
                }
            }

            for (int n = 1; n <= Block[0]; n++) {
                Result[Block[n]] = Value;
            }
        }
    }

    BLACK_SCORE = 0;
    WHITE_SCORE = 0;

    for (int j = 1, k = 0; j <= DATA_HEIGHT; j++) {
        for (int i = 1; i <= DATA_WIDTH; i++, k++) {
            if (j != 1 && j != DATA_HEIGHT && i != 1) {
                double d = (double) Result[k] / total;
                if (d >= 0.1) {
                    Area[k] = (Stat.Point[k] == WHITE ? WHITE_DEAD : BLACK_AREA);
                    BLACK_SCORE++;
                }
                else if (d <= -0.1) {
                    Area[k] = (Stat.Point[k] == BLACK ? BLACK_DEAD : WHITE_AREA);
                    WHITE_SCORE++;
                }
                else Area[k] = BOTH_AREA;
            }
        }
    }

    FINAL_SCORE = BLACK_SCORE - WHITE_SCORE - Komi;
    BOARD_SCORE = QString("B %1 W %2 + %3 = %4 RE : %5%6").arg(BLACK_SCORE).arg(WHITE_SCORE).arg(BOARD_KOMI).arg((double) WHITE_SCORE + Komi).arg((FINAL_SCORE > 0 ? "B + " : (FINAL_SCORE < 0 ? "W + " : ""))).arg(fabs(FINAL_SCORE));
}

int Go::Start()
{
    if (Index > 0) {
        while(Undo()) { }
        return 1;
    }

    return 0;
}

int Go::End()
{
    if (Index < Tree.size() - 1) {
        while (Forward()) { }
        return 1;
    }

    return 0;
}

int Go::Undo(int k)
{
    if (Index <= 0 || k <= 0) return 0;

    while (Index && k) {
        int total = Tree[Index].Prop.size();

        for (int i = 0; i < total; i++) {
            GoProp Prop = Tree[Index].Prop[i];
            if (Prop.Label == TOKEN_PLAY || Prop.Label == TOKEN_ADD)
                Stat.Point[GetPoint(Prop.Col, Prop.Row)] = 0;
            else if (Prop.Label == TOKEN_TAKE)
                Stat.Point[GetPoint(Prop.Col, Prop.Row)] = Prop.Value;
        }

        Index--; k--;
        Stat.Turn = Tree[Index].Turn;
        Stat.Pass = Tree[Index].Pass;

        if (Index == Index2) {
            Tree = Tree2; // restore from test play
            Index2 = -1;
        }
    }

    return 1;
}

int Go::Forward(int k)
{
    int last = Tree.size() - 1;
    if (Index >= last) return 0;

    while (Index < last && k) {
        Index++;
        int total = Tree[Index].Prop.size();
        Stat.Turn = Tree[Index].Turn;
        Stat.Pass = Tree[Index].Pass;

        for (int i = 0; i < total; i++) {
            GoProp Prop = Tree[Index].Prop[i];
            if (Prop.Label == TOKEN_PLAY || Prop.Label == TOKEN_ADD)
                Stat.Point[GetPoint(Prop.Col, Prop.Row)] = Prop.Value;
            else if (Prop.Label == TOKEN_TAKE)
                Stat.Point[GetPoint(Prop.Col, Prop.Row)] = 0;
        }

        k--;
    }

    return 1;
}

void Go::Cut()
{
    int n = Tree.size() - Index - 1;
    while (n--) Tree.pop_back();
}

Go::GoProp Go::MakeProp(int label, int value, int x, int y)
{
    GoProp Prop;

    Prop.Label = label;
    Prop.Value = value;
    Prop.Col = x;
    Prop.Row = y;

    return Prop;
}


QString Go::GetText(QString &buf, int i, int j)
{
    return buf.mid(i + 1, j - i - 1);
}

int Go::GetProp(QString &buf, int i, int j)
{
    QString str = buf.mid(i, j - i);
    str = str.toUpper();

    if (str == "B")
        return TOKEN_BLACK;
    if (str == "W")
        return TOKEN_WHITE;
    if (str == "C" || str == "TC")
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

int Go::GetNumber(QString &buf, int i, int j)
{
    return buf.mid(i + 1, j - i - 1).toInt();
}

int Go::Read(const QString &Path, int k)
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
            if (p >= 0) token = GetProp(buf, p, v1); // use previous token for second property

            if (token == TOKEN_BLACK || token == TOKEN_WHITE) {
                int color = (token == TOKEN_BLACK ? BLACK : WHITE);
                if (v2 - v1 == 3) {
                    Play(buf[v1 + 1].toLower().toLatin1() - 'a', buf[v1 + 2].toLower().toLatin1() - 'a', color);
                }
                else Play(-1, -1, color); // pass
                // if (--k == 0) return 1;
            }
            else if (token == TOKEN_ADD_BLACK || token == TOKEN_ADD_WHITE) {
                int color = (token == TOKEN_ADD_BLACK ? BLACK : WHITE);
                if (v2 - v1 == 3) {
                    Add(buf[v1 + 1].toLower().toLatin1() - 'a', buf[v1 + 2].toLower().toLatin1() - 'a', color);
                }
            }
            else if (token == TOKEN_LABEL) {
                if (v2 - v1 >= 5 && buf[v1 + 3] == ':') {
                    GoProp Prop;
                    Prop.Label = TOKEN_LABEL;
                    Prop.Value = buf[v1 + 4].toLatin1();
                    Prop.Col = buf[v1 + 1].toLower().toLatin1() - 'a';
                    Prop.Row = buf[v1 + 2].toLower().toLatin1() - 'a';
                    Tree[Index].Prop.push_back(Prop);
                }
            }
            else if (token == TOKEN_MARK || token == TOKEN_TRIANGLE || token == TOKEN_CIRCLE || token == TOKEN_SQUARE) {
                if (v2 - v1 == 3) {
                    GoProp Prop;
                    Prop.Label = token;
                    Prop.Col = buf[v1 + 1].toLower().toLatin1() - 'a';
                    Prop.Row = buf[v1 + 2].toLower().toLatin1() - 'a';
                    Tree[Index].Prop.push_back(Prop);
                }
            }
            else if (token == TOKEN_SIZE) {
                Size = GetNumber(buf, v1, v2);
                Reset(Size, Size);
            }
            else if (token == TOKEN_COMMENT) {
                QString Text = GetText(buf, v1, v2);
                Text.replace("\\\\", "\\");
                Text.replace("\\]", "]");
                Tree[Index].Text += Text;
            }
            else if (token == TOKEN_EVENT)
                BOARD_EVENT = GetText(buf, v1, v2);
            else if (token == TOKEN_DATE)
                BOARD_DATE = GetText(buf, v1, v2);
            else if (token == TOKEN_RESULT)
                BOARD_RESULT = GetText(buf, v1, v2);
            else if (token == TOKEN_PLAY_BLACK)
                PLAYER_BLACK = GetText(buf, v1, v2);
            else if (token == TOKEN_PLAY_WHITE)
                PLAYER_WHITE = GetText(buf, v1, v2);
            else if (token == TOKEN_KOMI) {
                BOARD_KOMI = GetText(buf, v1, v2);
                Komi = BOARD_KOMI.toDouble();
            }

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

    if (k > 0) while (k < Index) Undo();
    else Start();
    return 1;
}

int Go::Write(const QString &Path)
{
    QFile file(Path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return 0;
    QTextStream Stream(&file);

    Stream << QString("(;SZ[%1]KM[%2]PB[%3]PW[%4]DT[%5]RE[%6]").arg(QString::number(Size),
        BOARD_KOMI, PLAYER_BLACK, PLAYER_WHITE, BOARD_DATE, BOARD_RESULT);

    for (int i = 0; i < Tree.size(); i++) {
        for (int k = 0; k < Tree[i].Prop.size(); k++) {
            GoProp Prop = Tree[i].Prop[k];
            if (Prop.Label == TOKEN_PLAY || Prop.Label == TOKEN_ADD) {
                Stream << ";" << (Prop.Label == TOKEN_ADD ? "A" : "") << (Prop.Value == BLACK ? "B" : "W") <<
                    QString("[%1%2]").arg(QChar('a' + Prop.Col), QChar('a' + Prop.Row));
            }
            else if (Prop.Label == TOKEN_PASS) {
                Stream << ";" << (Prop.Value == BLACK ? "B" : "W") << "[]";
            }
        }
    }

    Stream << ")";
    file.close();
    return 1;
}
