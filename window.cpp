#include <QApplication>
#include <QtWidgets>

#include "window.h"

BoardView::BoardView(QWidget *parent) : QWidget(parent)
{
    setStyleSheet("background-color: rgb(216, 172, 76);");
    setMouseTracking(true);
    setAcceptDrops(true);

    board.Reset();
    board.BOARD_KOMI = "7.5";
}

void BoardView::Read(QString file)
{
    if (board.Read(file)) {
        activateWindow();
        repaint();
        setFocus();
    }
}

void BoardView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Right || event->key() == Qt::Key_Down) {
        if (board.NextNode()) repaint();
    }
    else if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Up) {
        if (board.PrevNode()) repaint();
    }
    else if (event->key() == Qt::Key_Home) {
        if (board.FirstNode()) repaint();
    }
    else if (event->key() == Qt::Key_End) {
        if (board.LastNode()) repaint();
    }
    else if (event->key() == Qt::Key_PageUp) {
        if (board.PrevNode(10)) repaint();
    }
    else if (event->key() == Qt::Key_PageDown) {
        if (board.NextNode(10)) repaint();
    }
    else if (event->key() == Qt::Key_Space) {
        ShowLabel = ! ShowLabel;
        repaint();
    }
    else if (event->key() == Qt::Key_Escape) {
        if (board.Index2 >= 0) {
            while (board.Index2 >= 0)
                board.PrevNode();
            repaint();
        }
    }
    else if (event->key() == Qt::Key_S) {
        board.BOARD_FILE = "001.SGF";
        if (board.Write(board.BOARD_FILE)) board.BOARD_FILE.clear();
        else repaint();
    }
    else if (event->key() == Qt::Key_P) { // pass
        if ((board.Mode & board.Turn) && (board.Mode & board.Tree.back().Turn)) {
            board.Play(-1, -1, board.Turn);
            repaint();
            if (board.Mode != 3)
                Other->PlayMove(-1, -1, 0);
        }
    }
}

void BoardView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls() && board.Mode == 3)
        event->accept();
}

void BoardView::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls())
        Read(event->mimeData()->urls().at(0).toLocalFile());
}

void BoardView::wheelEvent(QWheelEvent *event)
{
    if (event->delta() < 0) {
        if (board.NextNode()) repaint();
    }
    else if (event->delta() > 0) {
        if (board.PrevNode()) repaint();
    }
}

void BoardView::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::NoButton) {
        Cursor2 = QPoint(-1, -1);

        int Dist2 = Dist / 2;
        int i2, i = x1 - Dist2;
        int j2, j = y1 - Dist2;

        for (int x = 0; x < board.Size; x++) {
            i2 = i + Dist;
            if (event->x() > i && event->x() <= i2) {
                Cursor2.setX(x);
                break;
            }
            i = i2;
        }

        for (int y = 0; y < board.Size; y++) {
            j2 = j + Dist;
            if (event->y() > j && event->y() <= j2) {
                Cursor2.setY(y);
                break;
            }
            j = j2;
        }

        if ((Cursor.x() < 0 || Cursor.y() < 0) &&
            (Cursor2.x() < 0 || Cursor2.y() < 0)) return;

        if (Cursor != Cursor2) {
            Cursor = Cursor2;
            repaint();
        }
    }
}

void BoardView::mousePressEvent(QMouseEvent *event)
{
    setFocus();

    if (event->buttons() == Qt::LeftButton && (board.Mode & board.Turn) && (board.Mode & board.Tree.back().Turn)) {
        if (Cursor.x() >= 0 && Cursor.y() >= 0 && Dist > 6) {
            int n = board.Tree.size() - board.Index - 1;
            if (board.Play(Cursor.x(), Cursor.y(), board.Turn)) {
                repaint();
                if (board.Mode != 3) {
                    while (n--) Other->Task.append("undo\r\n");
                    Other->PlayMove(Cursor.x(), Cursor.y(), board.Size);
                }
            }
        }
    }
}

void BoardView::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    Dist = std::min(size().width(), size().height()) / (board.Size + 2);

    int width = (board.Size - 1) * Dist;
    int Dist2 = Dist * 2;
    double Font = Dist / 2.8;
    double radius = Dist * 0.4823 - 0.5869;

    x1 = (size().width() - width) / 2;
    y1 = (size().height() - width) / 2;
    x2 = x1 + width;
    y2 = y1 + width;

    if (Dist > 6) {
        painter.setPen(QPen(QColor(48, 48, 48), 0, Qt::SolidLine, Qt::FlatCap));

        for (int i = 0, d = 0; i < board.Size; i++, d += Dist) {
            painter.drawLine(x1 + d, y1, x1 + d, y2);
            painter.drawLine(x1, y1 + d, x2 + 1, y1 + d);
        }

        painter.setRenderHint(QPainter::Antialiasing);
        //painter.setRenderHint(QPainter::HighQualityAntialiasing);

        // Board Label //
        if (ShowLabel && Font >= 6) {
            painter.setPen(Qt::black);
            painter.setFont(QFont("Arial", Font));

            int k1 = y1 - Dist * 1.05;
            int k2 = y2 + Dist * 1.05;

            for (int i = 0, x = x1; i < board.Size; i++, x += Dist) {
                QChar Value = QChar('A' + i + (i < 8 ? 0 : 1));
                painter.drawText(x - Dist + 1, k1 - Dist + 1, Dist2, Dist2, Qt::AlignCenter, Value);
                painter.drawText(x - Dist + 1, k2 - Dist + 1, Dist2, Dist2, Qt::AlignCenter, Value);
            }

            k1 = x1 - Dist * 1.18;
            k2 = x2 + Dist * 1.17;

            for (int i = 0, y = y1; i < board.Size; i++, y += Dist) {
                QString Value = QString::number(board.Size - i);
                painter.drawText(k1 - Dist - 0, y - Dist + 1, Dist2, Dist2, Qt::AlignCenter, Value);
                painter.drawText(k2 - Dist - 0, y - Dist + 1, Dist2, Dist2, Qt::AlignCenter, Value);
            }
        }

        // Star //
        if (board.Size >= 13) {
            painter.setPen(QPen(QColor(15, 15, 15), 1.2, Qt::SolidLine));
            painter.setBrush(Qt::black);

            for (int i = 0, k = 0; i < 9; i++, k += 2) {
                if (board.Size & 1 == 0 && i >= 4) break;
                painter.drawEllipse(x1 + board.Star[k] * Dist - 2, y1 + board.Star[k + 1] * Dist - 2, 5, 5);
            }
        }

        if (radius >= 0) {
            for (int j = 0, y = y1; j < board.Size; j++, y += Dist) {
                for (int i = 0, x = x1; i < board.Size; i++, x += Dist) {
                    int color = board.GetColor(i, j);
                    int alpha = board.GetAlpha(i, j);

                    if (color == BLACK) {
                        painter.setPen(QPen(Qt::black, 1.02, Qt::SolidLine));
                        painter.setBrush(QColor(0, 0, 0, alpha));
                        painter.drawEllipse(QPointF(x + 0.5, y + 0.5), radius, radius);
                    }
                    else if (color == WHITE) {
                        painter.setPen(QPen(QColor(48, 48, 48), 1.01, Qt::SolidLine));
                        painter.setBrush(QColor(240, 240, 240, alpha));
                        painter.drawEllipse(QPointF(x + 0.5, y + 0.5), radius, radius);
                    }

                    // Area //
                    color = board.Area[j][i];
                    if (color != EMPTY) {
                        painter.setPen(Qt::NoPen);
                        if (color == BLACK) painter.setBrush(QBrush(QColor(0, 0, 0), Qt::BDiagPattern));
                        else painter.setBrush(QBrush(QColor(255, 255, 255), Qt::Dense5Pattern));
                        int half = int (radius);
                        int size = half * 2 + 1;
                        painter.drawRect(x - half, y - half, size, size);
                    }
                }
            }

            // Label //
            int total = board.Tree[board.Index].Prop.size();
            for (int i = 0; i < total; i++) {
                Board::Property Prop = board.Tree[board.Index].Prop[i];
                int color = board.Point[Prop.Col][Prop.Row];
                int x = x1 + Prop.Row * Dist;
                int y = y1 + Prop.Col * Dist;

                if (Prop.Label == Board::TOKEN_PLAY || Prop.Label == Board::TOKEN_CIRCLE) {
                    QColor color2 = (color == BLACK ? QColor(240, 240, 240) : Qt::black);
                    int half = double (Dist) / 12;

                    painter.setPen(QPen(color2, 1.04, Qt::SolidLine));
                    painter.setBrush(color2);
                    painter.drawEllipse(QPointF(x + 0.5, y + 0.5), half, half);
                }
                else if (Prop.Label == Board::TOKEN_TRIANGLE) {
                    double size = Dist / 4.2;
                    double poly[3][2] = {0, -1 * size, 0.866 * size, 0.5 * size, -0.866 * size, 0.5 * size};
                    painter.setPen(QPen(color == BLACK ? Qt::white : Qt::black, 1.2, Qt::SolidLine));
                    painter.drawLine(x + 1 + poly[1][0], y + 1.3 + poly[1][1], x + 1 + poly[2][0], y + 1.3 + poly[2][1]);
                    painter.drawLine(x + 1 + poly[0][0], y + 1.3 + poly[0][1], x + 1 + poly[1][0], y + 1.3 + poly[1][1]);
                    painter.drawLine(x + 1 + poly[0][0], y + 1.3 + poly[0][1], x + 1 + poly[2][0], y + 1.3 + poly[2][1]);
                }
                else if (Prop.Label == Board::TOKEN_MARK) {
                    int size = Dist / 4.8;
                    int poly[4][2] = {-1 * size, -1 * size, 1 * size, -1 * size, -1 * size, 1 * size, 1 * size, 1 * size};
                    painter.setPen(QPen(color == BLACK ? Qt::white : Qt::black, 1.2, Qt::SolidLine));
                    painter.drawLine(x + 1 + poly[0][0], y + 1 + poly[0][1], x + 1 + poly[3][0], y + 1 + poly[3][1]);
                    painter.drawLine(x + 1 + poly[1][0], y + 1 + poly[1][1], x + 1 + poly[2][0], y + 1 + poly[2][1]);
                }
                else if (Prop.Label == Board::TOKEN_SQUARE) {
                    int half = Dist / 6;
                    int size = half * 2;
                    painter.setPen(QPen(color == BLACK ? QColor(240, 240, 240) : Qt::black, 1, Qt::SolidLine));
                    painter.setRenderHint(QPainter::Antialiasing, false);
                    painter.drawRect(x - half , y - half, size, size);
                    painter.setRenderHint(QPainter::Antialiasing);
                }
                else if (Prop.Label == Board::TOKEN_LABEL || Prop.Label == Board::TOKEN_NUMBER) {
                    if (Font >= 6) {
                        if (color == EMPTY) painter.setBackgroundMode(Qt::OpaqueMode);
                        painter.setPen(QPen(color == BLACK ? Qt::white : Qt::black, 1, Qt::SolidLine));
                        painter.setFont(QFont("Arial", Font));
                        painter.drawText(x - Dist + 1, y - Dist + 1, Dist2, Dist2, Qt::AlignCenter,
                            Prop.Label == Board::TOKEN_LABEL ? QChar(Prop.Value) : QString::number(Prop.Value));
                        if (color == EMPTY) painter.setBackgroundMode(Qt::TransparentMode);
                    }
                }
            }

            // Text //
            if (!board.BOARD_FILE.isEmpty()) {
                painter.setPen(Qt::black);

                painter.setFont(QFont("Arial", 12));
                painter.drawText(10, 20, board.BOARD_EVENT);
                painter.drawText(10, 40, board.BOARD_DATE);
                painter.drawText(10, 60, board.PLAYER_BLACK);
                painter.drawText(10, 80, board.PLAYER_WHITE);
                painter.drawText(10, 100, board.BOARD_KOMI);
                painter.drawText(10, 120, board.BOARD_RESULT);
            }

            // Cursor //
            if (Cursor.x() >=0 && Cursor.y() >= 0) {
                QColor color = board.Index2 >= 0 ? Qt::green : Qt::red;
                painter.setPen(QPen(color, 2, Qt::SolidLine));
                painter.setBrush(color);
                painter.drawEllipse(x1 + Cursor.x() * Dist - 2, y1 + Cursor.y() * Dist - 2, 5, 5);
            }
        }
    }
}

Player::Player(QWidget *parent, int color) : QObject(parent)
{
    Process = NULL;
    TextEdit = NULL;
    Play = color;

    Task.append("name\r\n");
    Task.append("version\r\n");
}

void Player::Setup(QString str, QString arg)
{
    Process = new QProcess;
    //process->setReadChannel(QProcess::StandardOutput);
    connect(Process, SIGNAL(readyReadStandardOutput()), this, SLOT(readStandardOutput()));
    connect(Process, SIGNAL(readyReadStandardError()), this, SLOT(readStandardError()));

    BoardView *Widget = (BoardView*) parent();
    Task.append(QString("boardsize %1\r\n").arg(Widget->board.Size));
    Task.append("komi " + Widget->board.BOARD_KOMI + "\r\n");

    QStringList args = arg.split(' ', QString::SkipEmptyParts);
    QFileInfo Info(str);
    Process->setWorkingDirectory(Info.absolutePath());
    Process->start("\"" + str + "\"", args);
}

Player::~Player()
{
    if (Process)
        Process->kill();
}

void Player::Send()
{
    if (!Task.isEmpty() && Process)
        Process->write(Task[0].toLatin1());
}

QString Player::GetRespond(QString str)
{
    QString result;

    if (str.startsWith("=")) {
        str.remove(0, str.indexOf(" ") + 1);
        result = str.trimmed();
    }

    return result;
}

void Player::PlayMove(QString str)
{
    if (Process) {
        if (Play == BLACK) {
            Task.append("play white " + str + "\r\n");
            Task.append("genmove black\r\n");
        }
        else {
            Task.append("play black " + str + "\r\n");
            Task.append("genmove white\r\n");
        }

        Send();
    }
}

void Player::PlayMove(int x, int y, int size)
{
    PlayMove((x < 0 ? "PASS" :
        QChar('A' + (x < 8 ? x : ++x)) + QString::number(size - y)));
}

void Player::readStandardOutput()
{
    BoardView *Widget = (BoardView*) parent();
    QString respond, line;

    while (Process->canReadLine()) {
        line = Process->readLine();
        respond = GetRespond(line);

        if (TextEdit) {
            int ScrollDown = TextEdit->verticalScrollBar()->value() == TextEdit->verticalScrollBar()->maximum();
            TextEdit->insertPlainText(line);
            if (ScrollDown) TextEdit->verticalScrollBar()->setValue(TextEdit->verticalScrollBar()->maximum());
        }
        if (!respond.isNull()) break;
    }

    if (!respond.isNull() && !Task.isEmpty()) {
        if (Task[0].startsWith("boardsize") || Task[0].startsWith("komi") ||
                Task[0].startsWith("play") || Task[0].startsWith("undo")) {
            Task.removeFirst();
            Send();
        }
        else if (!respond.isEmpty()) {
            if (Task[0].startsWith("name")) {
                if (Play == BLACK) Widget->board.PLAYER_BLACK = respond;
                else Widget->board.PLAYER_WHITE = respond;
                Task.removeFirst();
                Send();
            }
            else if (Task[0].startsWith("version")) {
                if (Play == BLACK) Widget->board.BLACK_LEVEL = respond;
                else Widget->board.WHITE_LEVEL = respond;
                Task.removeFirst();
                Send();
            }
            else if ((Task[0].startsWith("genmove black") && Play == BLACK && Widget->board.Tree.back().Turn == BLACK) ||
                    (Task[0].startsWith("genmove white") && Play == WHITE && Widget->board.Tree.back().Turn == WHITE)) {
                Task.removeFirst();

                if (QString::compare(respond, "resign", Qt::CaseInsensitive) == 0) {
                    Process->write("quit\r\n");
                    if (Other->Process)
                        Other->Process->write("quit\r\n");
                    return;
                }

                if (QString::compare(respond, "pass", Qt::CaseInsensitive) == 0) {
                    if (Widget->board.Pass > 0) {
                        Process->write("quit\r\n");
                        if (Other->Process)
                            Other->Process->write("quit\r\n");
                        return;
                    }

                    Widget->board.Append(-1, -1, Play);
                    Widget->repaint();
                }
                else if (respond.size() == 2 || respond.size() == 3) {
                    int y, x = respond[0].toUpper().toLatin1() - 'A';
                    if (x > 8) x--;
                    if (respond.size() == 2) y = respond[1].digitValue();
                    else y = respond.mid(1, 2).toInt();

                    Widget->board.Append(x, Widget->board.Size - y, Play);
                    Widget->repaint();
                }

                Other->PlayMove(respond);
            }
        }
    }
}

void Player::readStandardError()
{
    QByteArray bArray = Process->readAllStandardError();
    QString Info = bArray;

    if (TextEdit) {
        int ScrollDown = TextEdit->verticalScrollBar()->value() == TextEdit->verticalScrollBar()->maximum();
        TextEdit->insertPlainText(Info);
        if (ScrollDown) TextEdit->verticalScrollBar()->setValue(TextEdit->verticalScrollBar()->maximum());
    }
}

Window::Window()
{
    Widget = new BoardView(this);
    setCentralWidget(Widget);
    setWindowTitle(" ");
    resize(800, 550);
    Widget->board.BOARD_DATE = QDate::currentDate().toString("yyyy-MM-dd");
}

void Window::CreateDock()
{
    Player1 = new Player(Widget, BLACK);
    Player2 = new Player(Widget, WHITE);

    Player1->Other = Player2;
    Player2->Other = Player1;

    QDockWidget *dock = new QDockWidget(" ", this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    Player1->TextEdit = new QTextEdit(dock);
    Player1->TextEdit->setFontPointSize(10);
    dock->setWidget(Player1->TextEdit);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    dock = new QDockWidget(" ", this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    Player2->TextEdit = new QTextEdit(dock);
    Player2->TextEdit->setFontPointSize(10);
    dock->setWidget(Player2->TextEdit);
    addDockWidget(Qt::RightDockWidgetArea, dock);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QStringList args = QCoreApplication::arguments();
    Window Win;

    if (argc == 2) {
        Win.Widget->Read(args[1]);
    }
    else if (argc > 2) {
        QString str[2], arg[2];
        int color = EMPTY;

        for (int i = 1; i < argc; i++) {
            if (args[i] == "-black") {
                color = BLACK;
            }
            else if (args[i] == "-white") {
                color = WHITE;
            }
            else if (args[i] == "-size") {
                if (i + 1 < argc) {
                    int size = args[i + 1].toInt();
                    Win.Widget->board.Reset(size, size);
                    i++;
                }
            }
            else if (args[i] == "-komi") {
                if (i + 1 < argc) {
                    Win.Widget->board.BOARD_KOMI = args[i + 1];
                    i++;
                }
            }
            else {
                if (color == BLACK) {
                    if (str[0].isEmpty()) str[0] = args[i];
                    else arg[0] = args[i];
                }
                else if (color == WHITE) {
                    if (str[1].isEmpty()) str[1] = args[i];
                    else arg[1] = args[i];
                }
            }
        }

        // Start //
        if (color != EMPTY) {
            Win.CreateDock();

            if (!str[0].isEmpty()) {
                Win.Widget->board.Mode &= 2; // human not black
                Win.Widget->Other = Win.Player1;
                Win.Player1->Setup(str[0], arg[0]);
            }
            if (!str[1].isEmpty()) {
                Win.Widget->board.Mode &= 1; // human not white
                Win.Widget->Other = Win.Player2;
                Win.Player2->Setup(str[1], arg[1]);
            }

            Win.Player1->Task.append("genmove black\r\n");
            Win.Player1->Send();
            Win.Player2->Send();
        }
    }

    Win.showMaximized();
    Win.Widget->setFocus();

    return app.exec();
}
