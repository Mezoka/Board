#include "Window.h"

Player::Player(QWidget *parent, int color) : QObject(parent)
{
    Process = NULL;
    TextEdit = NULL;
    Side = color;
    Wait = 0;

    Task.append("name\r\n");
    Task.append("version\r\n");
}

void Player::Setup(QString str, QString arg)
{
    Process = new QProcess;
//  process->setReadChannel(QProcess::StandardOutput);
    connect(Process, SIGNAL(readyReadStandardOutput()), this, SLOT(readStandardOutput()));
    connect(Process, SIGNAL(readyReadStandardError()), this, SLOT(readStandardError()));

    Board *Widget = (Board*) parent();
    Task.append(QString("boardsize %1\r\n").arg(Widget->Child.Size));
    Task.append("komi " + Widget->Child.BOARD_KOMI + "\r\n");

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
    if (!Task.isEmpty() && Process && Wait == 0) {
        if ((((Board*) parent())->View & Board::PLAY_PAUSE) == 0) {
            Process->write(Task[0].toLatin1());
            Wait = 1;
        }
    }
}

void Player::Remove()
{
    Task.removeFirst();
    Wait = 0;
    Send();
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

void Player::Play(QString str)
{
    if (Process) {
        if (Side == BLACK) {
//          Task.append("play white " + str + "\r\n");
            Task.append("play W " + str + "\r\n");
            Task.append("genmove black\r\n");
        }
        else {
//          Task.append("play black " + str + "\r\n");
            Task.append("play B " + str + "\r\n");
            Task.append("genmove white\r\n");
        }

        Send();
    }
}

void Player::Play(int x, int y, int size)
{
    Play((x < 0 ? "PASS" : QChar('A' + (x < 8 ? x : ++x))
        + QString::number(size - y)));
}

void Player::readStandardOutput()
{
    Board *Widget = (Board*) parent();
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
            Remove();
        }
        else if (!respond.isEmpty()) {
            if (Task[0].startsWith("name")) {
                if (Side == BLACK) Widget->Child.PLAYER_BLACK = respond;
                else Widget->Child.PLAYER_WHITE = respond;
                Remove();
            }
            else if (Task[0].startsWith("version")) {
                if (Side == BLACK) {
                    Widget->Child.BLACK_LEVEL = respond;
                    TextEdit->parentWidget()->setWindowTitle(Widget->Child.PLAYER_BLACK + " " + respond);
                }
                else {
                    Widget->Child.WHITE_LEVEL = respond;
                    TextEdit->parentWidget()->setWindowTitle(Widget->Child.PLAYER_WHITE + " " + respond);
                }
                Remove();
            }
            else if ((Task[0].startsWith("genmove black") && Side == BLACK && Widget->Child.Tree.back().Turn == BLACK) ||
                    (Task[0].startsWith("genmove white") && Side == WHITE && Widget->Child.Tree.back().Turn == WHITE)) {

                if (QString::compare(respond, "resign", Qt::CaseInsensitive) == 0) {
                    Process->write("quit\r\n");
                    if (Other->Process)
                        Other->Process->write("quit\r\n");
                    Widget->Child.BOARD_RESULT = QString(Side == BLACK ? "W" : "B") + "+Resign";
                    return;
                }

                int Score = !((Widget->View & Board::SHOW_SCORE) ||
                    (Widget->View & Board::SHOW_GRID_SCORE));

                if (QString::compare(respond, "pass", Qt::CaseInsensitive) == 0) {
                    if (Widget->Child.Stat.Pass > 0) {
                        Process->write("quit\r\n");
                        if (Other->Process)
                            Other->Process->write("quit\r\n");
                        return;
                    }

                    Widget->Child.Append(-1, -1, Side, Score);
                    Widget->repaint();
                }
                else if (respond.size() == 2 || respond.size() == 3) {
                    int y, x = respond[0].toUpper().toLatin1() - 'A';
                    if (x > 8) x--;
                    if (respond.size() == 2) y = respond[1].digitValue();
                    else y = respond.mid(1, 2).toInt();

                    Widget->Child.Append(x, Widget->Child.Size - y, Side, Score);
                    Widget->repaint();
                }

                Remove();
                Other->Play(respond.toUpper());
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

Board::Board(QWidget *parent) : QWidget(parent)
{
    setMouseTracking(true);
    setAcceptDrops(true);

    TextEdit = NULL;
    Table = NULL;

    Child.Reset();
    Child.BOARD_DATE = QDate::currentDate().toString("yyyy-MM-dd");

    Mode = 0;
    View = 0;
    Side = 3; // play black & white

}

int Board::Read(const QString &file, int k)
{
    if (Child.Read(file, k)) {
        ((Window*) parent())->SetTitle(QFileInfo(file).fileName());
        Mode = MODE_FILE;
        ShowTable(1);
        activateWindow();
        setFocus();
        return 1;
    }

    return 0;
}

void Board::ShowTable(int init)
{
    if (Table) {
        if (init) {
            Table->clear();
            Table->setColumnCount(2);
            Table->setRowCount(6);
            Table->horizontalHeader()->setStretchLastSection(true);
            Table->horizontalHeader()->hide();
            Table->verticalHeader()->hide();

            Table->setItem(0, 0, new QTableWidgetItem("Event"));
            Table->setItem(1, 0, new QTableWidgetItem("Date"));
            Table->setItem(2, 0, new QTableWidgetItem("Black"));
            Table->setItem(3, 0, new QTableWidgetItem("White"));
            Table->setItem(4, 0, new QTableWidgetItem("Komi"));
            Table->setItem(5, 0, new QTableWidgetItem("Result"));

            Table->setItem(0, 1, new QTableWidgetItem(Child.BOARD_EVENT));
            Table->setItem(1, 1, new QTableWidgetItem(Child.BOARD_DATE));
            Table->setItem(2, 1, new QTableWidgetItem(Child.PLAYER_BLACK));
            Table->setItem(3, 1, new QTableWidgetItem(Child.PLAYER_WHITE));
            Table->setItem(4, 1, new QTableWidgetItem(Child.BOARD_KOMI));
            Table->setItem(5, 1, new QTableWidgetItem(Child.BOARD_RESULT));

        }
        if (Child.Index > 0) {
            if (Table->rowCount() == 6) {
                Table->insertRow(6);
                Table->setItem(6, 0, new QTableWidgetItem("Move"));
            }
            Table->setItem(6, 1, new QTableWidgetItem(tr("%1").arg(Child.Index)));
        }
        else {
            if (Table->rowCount() > 6) {
                Table->removeRow(6);
            }
        }
    }

    if (TextEdit) {
        TextEdit->setText(Child.Tree[Child.Index].Text);
    }

    View &= ~SHOW_SCORE;
    View &= ~SHOW_GRID_SCORE;

    repaint();
}

void Board::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls() && Side == 3)
        event->accept();
}

void Board::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        if (Mode == 0 || Mode == MODE_FILE) {
            Read(event->mimeData()->urls().at(0).toLocalFile());
        }
    }
}

void Board::wheelEvent(QWheelEvent *event)
{
    if (event->delta() < 0) {
        if (Child.Forward()) ShowTable();
    }
    else if (event->delta() > 0) {
        if (Child.Undo()) ShowTable();
    }
}

void Board::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::NoButton) {
        Cursor2 = QPoint(-1, -1);

        int Half = GridSize / 2;
        int i2, i = x1 - Half;
        int j2, j = y1 - Half;

        for (int x = 0; x < Child.Size; x++) {
            i2 = i + GridSize;
            if (event->x() > i && event->x() <= i2) {
                Cursor2.setX(x);
                break;
            }
            i = i2;
        }

        for (int y = 0; y < Child.Size; y++) {
            j2 = j + GridSize;
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

void Board::mousePressEvent(QMouseEvent *event)
{
    setFocus();

    if (Cursor.x() >= 0 && Cursor.y() >= 0 && GridSize > 6) {
        if (Mode == MODE_PLAY) {
            if ((Side & Child.Stat.Turn) && (Side & Child.Tree.back().Turn)) {
                if (event->buttons() == Qt::LeftButton) {
                    int n = Child.Tree.size() - Child.Index - 1;
                    if (Child.Play(Cursor.x(), Cursor.y(), Child.Stat.Turn)) {
                        repaint();
                        if (Side != 3) {
                            while (n--) Other->Task.append("undo\r\n");
                            Other->Play(Cursor.x(), Cursor.y(), Child.Size);
                        }
                    }
                }
                else if (event->buttons() == Qt::RightButton) {

                }
            }
        }
        else {
            if (event->buttons() == Qt::LeftButton) {
                if (Child.Play(Cursor.x(), Cursor.y(), Child.Stat.Turn, Mode == MODE_FILE)) {
                    repaint();
                }
            }
        }
    }
}

void Board::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_Right: case Qt::Key_Down:
            if (Child.Forward()) ShowTable();
            break;
        case Qt::Key_Left: case Qt::Key_Up:
            if (Child.Undo()) ShowTable();
            break;
        case Qt::Key_Home:
            if (Child.Start()) ShowTable();
            break;
        case Qt::Key_End:
            if (Child.End()) ShowTable();
            break;
        case Qt::Key_PageUp:
            if (Child.Undo(10)) ShowTable();
            break;
        case Qt::Key_PageDown:
            if (Child.Forward(10)) ShowTable();
            break;
        case Qt::Key_Space: // Grid Label
            if (View & SHOW_SCORE) {
                View &= ~SHOW_SCORE;
                View |= SHOW_GRID_SCORE;
                View |= SHOW_GRID_LABEL;
            }
            else if (View & SHOW_GRID_SCORE) {
                View |= SHOW_SCORE;
                View &= ~SHOW_GRID_SCORE;
                View &= ~SHOW_GRID_LABEL;
            }
            else View ^= SHOW_GRID_LABEL;
            repaint();
            break;
        case Qt::Key_A: // Score
            if (View & SHOW_GRID_LABEL) {
                if (View & SHOW_SCORE) {
                    View &= ~SHOW_SCORE;
                    View |= SHOW_GRID_SCORE;
                }
                else if (View & SHOW_GRID_SCORE) View &= ~SHOW_GRID_SCORE;
                else View |= SHOW_SCORE;
            }
            else {
                if (View & SHOW_GRID_SCORE) View &= ~SHOW_GRID_SCORE;
                else View ^= SHOW_SCORE;
            }
            if (View & SHOW_SCORE) Child.Rand(2000);
            repaint();
            break;
        case Qt::Key_Escape:
            if (Child.Index2 >= 0) {
                while (Child.Index2 >= 0) Child.Undo();
                repaint();
            }
            break;
        case Qt::Key_S: // Save
            Child.BOARD_FILE = "001.SGF";
            if (Child.Write(Child.BOARD_FILE)) {
                Child.BOARD_FILE.clear();
                Title = " - SAVE";
                parentWidget()->setWindowTitle(((Window*) parent())->Title + Title);
                return;
            }
            break;
        case Qt::Key_T: // Pause
            if (Mode == MODE_PLAY) {
                View ^= PLAY_PAUSE;
                if ((View & PLAY_PAUSE) == 0) {
                    Player *Player = ((Window*) parent())->Player1;
                    if (Player) Player->Send();
                    Player = ((Window*) parent())->Player2;
                    if (Player) Player->Send();
                    Title.clear();
                }
                else {
                    Title = " - PAUSE";
                }
                parentWidget()->setWindowTitle(((Window*) parent())->Title + Title);
                repaint();
                return;
            }
            break;
        case Qt::Key_P: // Pass
            if (Mode == MODE_PLAY) {
                if ((Side & Child.Stat.Turn) && (Side & Child.Tree.back().Turn)) {
                    int n = Child.Tree.size() - Child.Index - 1;
                    Child.Play(-1, -1, Child.Stat.Turn);
                    repaint();
                    if (Side != 3) {
                        while (n--) Other->Task.append("undo\r\n");
                        Other->Play(-1, -1, Child.Size);
                    }
                }
            }
            else {
                Child.Play(-1, -1, Child.Stat.Turn, Mode == MODE_FILE);
                repaint();
            }
            break;        
    }

    if (!Title.isEmpty()) {
        Title.clear();
        ((Window*) parent())->SetTitle();
    }
}

void Board::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    GridSize = std::min(size().width(), size().height()) / (Child.Size + 2.5);

    int d = (600 - 24 * GridSize) / 14 + 24;
    QColor ColorBorder(d, d, d);
    QColor ColorBack(216, 172, 76);

    painter.setBackground(QBrush(ColorBack));
    painter.fillRect(0, 0, size().width(), size().height(), ColorBack);

    int Width = (Child.Size - 1) * GridSize;
    int Grid = GridSize * 2;
    double Font = GridSize / 2.8;
    double Radius = GridSize * 0.4823 - 0.5869;

    x1 = (size().width() - Width) / 2;
    y1 = (size().height() - Width) / 2;
    x2 = x1 + Width;
    y2 = y1 + Width;

    if (GridSize > 6) {
        painter.setPen(QPen(QColor(48, 48, 48), 0, Qt::SolidLine, Qt::FlatCap));

        for (int i = 0, d = 0; i < Child.Size; i++, d += GridSize) {
            painter.drawLine(x1 + d, y1, x1 + d, y2);
            painter.drawLine(x1, y1 + d, x2 + 1, y1 + d);
        }

        painter.setRenderHint(QPainter::Antialiasing);
//      painter.setRenderHint(QPainter::HighQualityAntialiasing);
//      painter.setBackgroundMode(Qt::TransparentMode);

        // Grid Label //
        if (Font >= 6) {
            painter.setPen(Qt::black);
            painter.setFont(QFont("Arial", Font));

            if ((View & SHOW_GRID_LABEL) && (View & SHOW_SCORE) == 0) {
                int k1 = y1 - GridSize * 1.05;
                int k2 = y2 + GridSize * 1.05;

                for (int i = 0, x = x1; i < Child.Size; i++, x += GridSize) {
                    QChar Value = QChar('A' + i + (i < 8 ? 0 : 1));
                    painter.drawText(x - GridSize + 1, k1 - GridSize + 1, Grid, Grid, Qt::AlignCenter, Value);
                    painter.drawText(x - GridSize + 1, k2 - GridSize + 1, Grid, Grid, Qt::AlignCenter, Value);
                }

                k1 = x1 - GridSize * 1.18;
                k2 = x2 + GridSize * 1.17;

                for (int i = 0, y = y1; i < Child.Size; i++, y += GridSize) {
                    QString Value = QString::number(Child.Size - i);
                    painter.drawText(k1 - GridSize - 0, y - GridSize + 1, Grid, Grid, Qt::AlignCenter, Value);
                    painter.drawText(k2 - GridSize - 0, y - GridSize + 1, Grid, Grid, Qt::AlignCenter, Value);
                }
            }
            if (View & SHOW_SCORE) {
                painter.drawText(0, y1 - GridSize * 1.05 - GridSize + 1, size().width(), Grid, Qt::AlignCenter, Child.BOARD_SCORE);
            }
        }

        // Star //
        if (Child.Size >= 13) {
            painter.setPen(QPen(QColor(15, 15, 15), 1.2, Qt::SolidLine));
            painter.setBrush(Qt::black);

            for (int i = 0, k = 0; i < 9; i++, k += 2) {
                if (Child.Size & 1 == 0 && i >= 4) break;
                painter.drawEllipse(x1 + Child.Star[k] * GridSize - 2, y1 + Child.Star[k + 1] * GridSize - 2, 5, 5);
            }
        }

        if (Radius >= 0) {
            int size = GridSize / 4.8;
            QPolygonF TriPoly, MarkPoly;
            MarkPoly << QPoint(-1 * size, -1 * size) << QPoint(1 * size, -1 * size) << QPoint(-1 * size, 1 * size) << QPoint(1 * size, 1 * size);
            double dsize = GridSize / 4.2;
            TriPoly << QPoint(0, -1 * dsize) << QPoint(0.866 * dsize, 0.5 * dsize) << QPoint(-0.866 * dsize, 0.5 * dsize);

            for (int j = 0, y = y1; j < Child.Size; j++, y += GridSize) {
                for (int i = 0, x = x1; i < Child.Size; i++, x += GridSize) {
                    int k = Child.GetPoint(i, j);
                    int color = Child.Stat.Point[k];
                    int color2 = Child.Area[k];

                    // Area //
                    if (((View & SHOW_SCORE) || (View & SHOW_GRID_SCORE)) && color2 != BOTH_AREA) {
                        painter.setPen(Qt::NoPen);
                        if (color2 == BLACK_AREA || color2 == WHITE_DEAD) painter.setBrush(QBrush(QColor(0, 0, 0), Qt::BDiagPattern));
                        else painter.setBrush(QBrush(QColor(255, 255, 255), Qt::Dense5Pattern));
                        int half = int (Radius);
                        int size = half * 2 + 1;
                        painter.drawRect(x - half, y - half, size, size);
                    }

                    if (color == BLACK) {
                        painter.setPen(QPen(QColor(24, 24, 24), 1, Qt::SolidLine));
                        painter.setBrush(QColor(0, 0, 0));
                        painter.drawEllipse(QPointF(x + 0.5, y + 0.5), Radius, Radius);
                    }
                    else if (color == WHITE) {
                        painter.setPen(QPen(ColorBorder, 1, Qt::SolidLine));
                        painter.setBrush(QColor(240, 240, 240));
                        painter.drawEllipse(QPointF(x + 0.5, y + 0.5), Radius, Radius);
                    }

                    // Mark //
                    if (((View & SHOW_SCORE) || (View & SHOW_GRID_SCORE)) && color2 != BOTH_AREA) {
                        if (color2 == BLACK_DEAD || color2 == WHITE_DEAD) {
                            QPolygonF Poly = MarkPoly.translated(x + 0.55, y + 1);
                            painter.setPen(QPen(color == BLACK ? Qt::white : Qt::black, 1.1, Qt::SolidLine));
                            painter.drawLine(Poly[0], Poly[3]);
                            painter.drawLine(Poly[1], Poly[2]);
                        }
                    }
                }
            }

            // Label //
            int total = Child.Tree[Child.Index].Prop.size();
            for (int i = 0; i < total; i++) {
                Go::GoProp Prop = Child.Tree[Child.Index].Prop[i];
                int color = Child.GetColor(Prop.Col, Prop.Row);
                int x = x1 + Prop.Col * GridSize;
                int y = y1 + Prop.Row * GridSize;

                if (Prop.Label == Go::TOKEN_PLAY || Prop.Label == Go::TOKEN_CIRCLE) {
                    QColor color2 = (color == BLACK ? QColor(240, 240, 240) : Qt::black);
                    int Half = double (GridSize) / 12;
                    painter.setPen(QPen(color2, 1.04, Qt::SolidLine));
                    painter.setBrush(color2);
                    painter.drawEllipse(QPointF(x + 0.5, y + 0.5), Half, Half);
                }
                else if (Prop.Label == Go::TOKEN_TRIANGLE) {
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(QBrush(color == BLACK ? Qt::white : Qt::black));
                    painter.drawPolygon(TriPoly.translated(x + 0.65, y + 1.4));
                }
                else if (Prop.Label == Go::TOKEN_MARK) {
                    QPolygonF Poly = MarkPoly.translated(x + 0.55, y + 1);
                    painter.setPen(QPen(color == BLACK ? Qt::white : Qt::black, 1.2, Qt::SolidLine));
                    painter.drawLine(Poly[0], Poly[3]);
                    painter.drawLine(Poly[1], Poly[2]);
                }
                else if (Prop.Label == Go::TOKEN_SQUARE) {
                    int Half = GridSize / 6;
                    int size = Half * 2;
                    painter.setBrush(Qt::NoBrush);
                    painter.setPen(QPen(color == BLACK ? QColor(240, 240, 240) : Qt::black, 1, Qt::SolidLine));
                    painter.setRenderHint(QPainter::Antialiasing, false);
                    painter.drawRect(x - Half , y - Half, size, size);
                    painter.setRenderHint(QPainter::Antialiasing);
                }
                else if (Prop.Label == Go::TOKEN_LABEL || Prop.Label == Go::TOKEN_NUMBER) {
                    if (Font >= 6) {
                        if (color == EMPTY) painter.setBackgroundMode(Qt::OpaqueMode);
                        painter.setPen(QPen(color == BLACK ? Qt::white : Qt::black, 1, Qt::SolidLine));
                        painter.setFont(QFont("Arial", Font));
                        painter.drawText(x - GridSize + 1, y - GridSize + 1, Grid, Grid, Qt::AlignCenter,
                            Prop.Label == Go::TOKEN_LABEL ? QChar(Prop.Value) : QString::number(Prop.Value));
                        if (color == EMPTY) painter.setBackgroundMode(Qt::TransparentMode);
                    }
                }
            }

            // Cursor //
            if (Cursor.x() >=0 && Cursor.y() >= 0) {
                QColor color = Child.Index2 < 0 ? QColor(0, 224, 0) : Qt::red;
                painter.setPen(QPen(color, 2, Qt::SolidLine));
                painter.setBrush(color);
                painter.drawEllipse(x1 + Cursor.x() * GridSize - 2, y1 + Cursor.y() * GridSize - 2, 5, 5);
            }
        }
    }

}

Window::Window()
{
    Child = new Board(this);
    SetTitle(Child->Child.BOARD_DATE);
    setCentralWidget(Child);
    resize(800, 700);
}

void Window::SetTitle(QString str)
{
    if(!str.isEmpty()) Title = str;
    setWindowTitle(Title);
}

void Window::paintEvent(QPaintEvent *event)
{

}

void Window::CreateDock()
{
    if (Child->Mode == 0 || Child->Mode == MODE_FILE) {

        QDockWidget *dock = new QDockWidget("", this);
        dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
        Child->Table = new QTableWidget(dock);
        dock->setWidget(Child->Table);
        addDockWidget(Qt::LeftDockWidgetArea, dock);

        Child->Table->setFrameStyle(QFrame::NoFrame);
        Child->Table->setFont(QFont("MingLiU", 12));

        dock = new QDockWidget("", this);
        dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
        Child->TextEdit = new QTextEdit(dock);
        Child->TextEdit->setFontPointSize(10);
        dock->setWidget(Child->TextEdit);
        addDockWidget(Qt::LeftDockWidgetArea, dock);
    }
    else if (Child->Mode == MODE_PLAY) {
        Player1 = new Player(Child, BLACK);
        Player2 = new Player(Child, WHITE);

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
}
