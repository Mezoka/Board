#include "Window.h"

Player::Player(QWidget *parent, int color, int size, double komi) : QObject(parent)
{
    Process = NULL;
    TextEdit = NULL;
    Side = color;
    Wait = 0;

    Append("name");
    Append("version");

    if (size) Append(QString("boardsize %1").arg(size));
    if (komi) Append(QString("komi %1").arg(komi));
}

Player::~Player()
{
    if (Process)
        Process->kill();
}

void Player::Setup(const QString &str, const QString &arg)
{
    if (Process == NULL) {
        Process = new QProcess;
//      process->setReadChannel(QProcess::StandardOutput);
        connect(Process, SIGNAL(readyReadStandardOutput()), this, SLOT(readStandardOutput()));
        connect(Process, SIGNAL(readyReadStandardError()), this, SLOT(readStandardError()));
    }

    QStringList args = arg.split(' ', QString::SkipEmptyParts);
    QFileInfo Info(str);
    Process->setWorkingDirectory(Info.absolutePath());
    Process->start("\"" + str + "\"", args);
}

int Player::CheckTask(const QString &str)
{
    if (!Task.isEmpty())
        return Task[0].startsWith(str);
    return false;
}

void Player::Send()
{
    if (!Task.isEmpty() && Process && Wait == 0) {
        if ((((Widget*) parent())->View & Widget::PLAY_PAUSE) == 0) {
            Process->write(Task[0].toLatin1());
            Respond.clear();
            Wait = 1;
        }
    }
}

void Player::Append(const QString &str)
{
    Task.append(str + "\r\n");
}

void Player::Remove()
{
    Wait = 0;

    if (!Task.isEmpty()) {
        Task.removeFirst();
        Send();
    }
}

void Player::Play(QString str, int color)
{
    if (Process) {
        if (color == WHITE || (color == 0 && Side == BLACK)) {
            Append("play W " + str);
            if (color == 0) Append("genmove b"); // not handicap
        }
        else {
            Append("play B " + str);
            if (color == 0) Append("genmove w");
        }

        Send();
    }
}

void Player::Play(int x, int y, int size, int color)
{
    Play((x < 0 ? "PASS" : QChar('A' + (x < 8 ? x : ++x))
        + QString::number(size - y)), color);
}

void Player::readStandardOutput()
{
    QString line;

    while (Process->canReadLine()) {
        line = Process->readLine();
        line.replace("\r\n", "\n");

        if (TextEdit) {
            if ((Side == BLACK || Side == WHITE) || !CheckTask("play")) { // special requirement
                int ScrollDown = TextEdit->verticalScrollBar()->value() == TextEdit->verticalScrollBar()->maximum();
                TextEdit->insertPlainText(line);
                if (ScrollDown) TextEdit->verticalScrollBar()->setValue(TextEdit->verticalScrollBar()->maximum());
            }
        }

        if (Respond.isEmpty()) {
            if (line.startsWith("="))
                Respond = line.mid(line.indexOf(" ") + 1);
        }
        else Respond += line;
    }

    if (Respond.endsWith("\n\n")) {
        ((Widget*) parent())->GetRespond(Respond.trimmed(), Side);
    }
}

void Player::readStandardError()
{
    QByteArray bArray = Process->readAllStandardError();
    QString Info = bArray;

    if (TextEdit) {
        Info.replace("\r\r\n", "\r\n"); // Python Windows Bug
        int ScrollDown = TextEdit->verticalScrollBar()->value() == TextEdit->verticalScrollBar()->maximum();
        TextEdit->insertPlainText(Info);
        if (ScrollDown) TextEdit->verticalScrollBar()->setValue(TextEdit->verticalScrollBar()->maximum());
    }
}

Widget::Widget(QWidget *parent) : QWidget(parent)
{
    setMouseTracking(true);
    setAcceptDrops(true);

    TextEdit = NULL;
    Table = NULL;

    Play[1] = Play[2] = NULL;

    Child.Reset();
    Child.BOARD_DATE = QDate::currentDate().toString("yyyy-MM-dd");

    Mode = 0;
    View = 0;
    Side = 0; // play black & white

    Cursor[0] = QPoint(-1, -1);
    Cursor[1] = QPoint(-1, -1);
}

int Widget::Read(const QString &str, int k)
{
    if (Mode == BOARD_PLAY) {

    }
    else if (Child.Read(str, k)) {
        ((Window*) parent())->SetTitle(QFileInfo(str).fileName());
        Mode = BOARD_FILE;
        ShowTable(1);
        activateWindow();
        setFocus();
        return 1;
    }

    return 0;
}

void Widget::SetTitle(const QString &str)
{
    Title = str;
    if (str.isEmpty()) parentWidget()->setWindowTitle(((Window*) parent())->Title);
    else parentWidget()->setWindowTitle(((Window*) parent())->Title + " - " + Title);
}

void Widget::ShowTable(int init)
{
    if (Table) {
        int n = Child.BOARD_HANDICAP.isEmpty() ? 6 : 7;

        if (init) {
            Table->clear();
            Table->setColumnCount(2);
            Table->setRowCount(n);
            Table->horizontalHeader()->setStretchLastSection(true);
            Table->horizontalHeader()->hide();
            Table->verticalHeader()->hide();

            Table->setItem(0, 0, new QTableWidgetItem("Event"));
            Table->setItem(1, 0, new QTableWidgetItem("Date"));
            Table->setItem(2, 0, new QTableWidgetItem("Black"));
            Table->setItem(3, 0, new QTableWidgetItem("White"));
            Table->setItem(4, 0, new QTableWidgetItem("Komi"));

            if (n == 6)
                Table->setItem(5, 0, new QTableWidgetItem("Result"));
            else {
                Table->setItem(5, 0, new QTableWidgetItem("Handicap"));
                Table->setItem(6, 0, new QTableWidgetItem("Result"));
            }

            Table->setItem(0, 1, new QTableWidgetItem(Child.BOARD_EVENT));
            Table->setItem(1, 1, new QTableWidgetItem(Child.BOARD_DATE));
            Table->setItem(2, 1, new QTableWidgetItem(Child.PLAYER_BLACK));
            Table->setItem(3, 1, new QTableWidgetItem(Child.PLAYER_WHITE));
            Table->setItem(4, 1, new QTableWidgetItem(Child.BOARD_KOMI));

            if (n == 6)
                Table->setItem(5, 1, new QTableWidgetItem(Child.BOARD_RESULT));
            else {
                Table->setItem(5, 1, new QTableWidgetItem(Child.BOARD_HANDICAP));
                Table->setItem(6, 1, new QTableWidgetItem(Child.BOARD_RESULT));
            }
        }
        if (Child.Record.Index > 0) {
            if (Table->rowCount() == n) {
                Table->insertRow(n);
                Table->setItem(n, 0, new QTableWidgetItem("Move"));
            }
            Table->setItem(n, 1, new QTableWidgetItem(tr("%1").arg(Child.Record.Index)));
        }
        else {
            if (Table->rowCount() > n) {
                Table->removeRow(n);
            }
        }
    }

    if (TextEdit) {
        TextEdit->setPlainText(Child.Record.Node[Child.Record.Index].Text);
    }

    View &= ~VIEW_SCORE;
    repaint();
}

void Widget::GetRespond(const QString &str, int color)
{
    if (color == BLACK || color == WHITE) {
        if (Play[color]->CheckTask("genmove b") && color == BLACK && Child.Record.Node.back().Turn == BLACK ||
            Play[color]->CheckTask("genmove w") && color == WHITE && Child.Record.Node.back().Turn == WHITE)
        {
            int other = Child.OtherColor(color);
            int i = !(View & VIEW_SCORE);

            if (QString::compare(str, "resign", Qt::CaseInsensitive) == 0) {
                Play[color]->Process->write("quit\r\n");
                if (Play[other] && Play[other]->Process)
                    Play[other]->Process->write("quit\r\n");
                Child.BOARD_RESULT = QString(color == BLACK ? "W" : "B") + "+Resign";
                return;
            }
            if (QString::compare(str, "pass", Qt::CaseInsensitive) == 0) {
                Child.Append(-1, -1, color, i);
                repaint();

                if (Child.State.Pass >= 2) {
                    Play[color]->Process->write("quit\r\n");
                    if (Play[other] && Play[other]->Process)
                        Play[other]->Process->write("quit\r\n");
                    return;
                }
            }
            else if (str.size() == 2 || str.size() == 3) {
                int y, x = str[0].toUpper().toLatin1() - 'A';
                if (x > 8) x--;
                if (str.size() == 2) y = str[1].digitValue();
                else y = str.mid(1, 2).toInt();

                Child.Append(x, Child.Size - y, color, i);
                repaint();
            }

            Play[color]->Remove();
            if (Play[other]) Play[other]->Play(str.toUpper());
        }
        else {
            if (Play[color]->CheckTask("name")) {
                if (color == BLACK) Child.PLAYER_BLACK = str;
                else Child.PLAYER_WHITE = str;
            }
            else if (Play[color]->CheckTask("version")) {
                if (color == BLACK) {
                    Child.BLACK_LEVEL = str;
                    if (Play[color]->TextEdit) Play[color]->TextEdit->parentWidget()->
                            setWindowTitle(Child.PLAYER_BLACK + " " + str);
                }
                else {
                    Child.WHITE_LEVEL = str;
                    if (Play[color]->TextEdit) Play[color]->TextEdit->parentWidget()->
                            setWindowTitle(Child.PLAYER_WHITE + " " + str);
                }
            }

            Play[color]->Remove();
        }
    }
}

void Widget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls() && Side == 0)
        event->accept();
}

void Widget::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        if (Mode == 0 || Mode == BOARD_FILE) {
            Read(event->mimeData()->urls().at(0).toLocalFile());
        }
    }
}

void Widget::wheelEvent(QWheelEvent *event)
{
    if (event->delta() < 0) {
        if (Child.Forward()) ShowTable();
    }
    else if (event->delta() > 0) {
        if (Child.Undo()) ShowTable();
    }
}

void Widget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::NoButton) {
        Cursor[1] = QPoint(-1, -1);

        int Half = GridSize / 2;
        int i2, i = BOARD_LEFT - Half;
        int j2, j = BOARD_TOP - Half;

        for (int x = 0; x < Child.Size; x++) {
            i2 = i + GridSize;
            if (event->x() > i && event->x() <= i2) {
                Cursor[1].setX(x);
                break;
            }
            i = i2;
        }

        for (int y = 0; y < Child.Size; y++) {
            j2 = j + GridSize;
            if (event->y() > j && event->y() <= j2) {
                Cursor[1].setY(y);
                break;
            }
            j = j2;
        }

        if ((Cursor[0].x() < 0 || Cursor[0].y() < 0) &&
            (Cursor[1].x() < 0 || Cursor[1].y() < 0)) return;

        if (Cursor[0] != Cursor[1]) {
            Cursor[0] = Cursor[1];
            repaint();
        }
    }
}

void Widget::mousePressEvent(QMouseEvent *event)
{
    setFocus();

    if (Cursor[0].x() >= 0 && Cursor[0].y() >= 0 && GridSize > 6) {
        if (Mode == BOARD_PLAY) {
            if (((Side ^ 3) & Child.State.Turn) && ((Side ^ 3) & Child.Record.Node.back().Turn)) {
                if (event->buttons() == Qt::LeftButton) {
                    int n = Child.Record.Node.size() - Child.Record.Index - 1;
                    if (Child.Play(Cursor[0].x(), Cursor[0].y(), Child.State.Turn)) {
                        repaint();
                        if (Side) {
                            while (n--) Play[Side]->Append("undo");
                            Play[Side]->Play(Cursor[0].x(), Cursor[0].y(), Child.Size);
                        }
                    }
                }
                else if (event->buttons() == Qt::RightButton) {

                }
            }
        }
        else {
            if (event->buttons() == Qt::LeftButton) {
                if (Child.Play(Cursor[0].x(), Cursor[0].y(), Child.State.Turn, Mode == BOARD_FILE)) {
                    repaint();
                }
            }
        }
    }
}

void Widget::keyPressEvent(QKeyEvent *event)
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
        case Qt::Key_F1: // Grid Coord
            View ^= VIEW_LABEL;
            repaint();
            break;
        case Qt::Key_F2: // Score
            View ^= VIEW_SCORE;
            if (View & VIEW_SCORE) Child.Rand(2000);
            repaint();
            break;
        case Qt::Key_Escape:
            if (View & VIEW_SCORE) View ^= VIEW_SCORE;
            if (Child.Record2.Index >= 0) {
                while (Child.Record2.Index >= 0) Child.Undo();
            }
            repaint();
            break;
        case Qt::Key_S: // Save
            Child.BOARD_FILE = "001.SGF";
            if (Child.Write(Child.BOARD_FILE)) {
                Child.BOARD_FILE.clear();
                SetTitle("SAVE");
                return;
            }
            break;
        case Qt::Key_T: // Pause
            if (Mode == BOARD_PLAY) {
                View ^= PLAY_PAUSE;
                if ((View & PLAY_PAUSE) == 0) {
                    if (Play[BLACK]) Play[BLACK]->Send();
                    if (Play[WHITE]) Play[WHITE]->Send();
                    SetTitle("");
                    repaint();
                }
                else SetTitle("PAUSE");
                return;
            }
            break;
        case Qt::Key_P: // Pass
            if (Mode == BOARD_PLAY) {
                if (((Side ^ 3) & Child.State.Turn) && ((Side ^ 3) & Child.Record.Node.back().Turn)) {
                    int n = Child.Record.Node.size() - Child.Record.Index - 1;
                    Child.Play(-1, -1, Child.State.Turn);
                    repaint();
                    if (Side) {
                        while (n--) Play[Side]->Task.append("undo\r\n");
                        Play[Side]->Play(-1, -1, Child.Size);
                    }
                    SetTitle("PASS");
                    return;
                }
            }
            else {
                Child.Play(-1, -1, Child.State.Turn, Mode == BOARD_FILE);
                repaint();
            }
            break;
    }

    if (!Title.isEmpty() && event->key() != Qt::Key_Control && event->key() != Qt::Key_Alt)
        SetTitle("");
}

void Widget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    GridSize = std::min(size().width(), size().height()) / (Child.Size + 2.5);
    int theme = 0;

    QColor ColorBack(216, 172, 76);
    painter.setBackground(QBrush(ColorBack));
    if (theme) painter.drawTiledPixmap(0, 0, size().width(), size().height(), QPixmap(":/RESOURCE/Wood.png"));
    else painter.fillRect(0, 0, size().width(), size().height(), ColorBack);

    int Width = (Child.Size - 1) * GridSize;
    int Grid = GridSize * 2;
    double Font = GridSize / 2.8;
    double Radius = GridSize * 0.4823 - 0.5869;

    BOARD_LEFT = (size().width() - Width) / 2;
    BOARD_TOP = (size().height() - Width) / 2;
    BOARD_RIGHT = BOARD_LEFT + Width;
    BOARD_DOWN = BOARD_TOP + Width;

    if (GridSize > 6) {
        painter.setPen(QPen(QColor(48, 48, 48), 0, Qt::SolidLine, Qt::FlatCap));

        for (int i = 0, d = 0; i < Child.Size; i++, d += GridSize) {
            painter.drawLine(BOARD_LEFT + d, BOARD_TOP, BOARD_LEFT + d, BOARD_DOWN);
            painter.drawLine(BOARD_LEFT, BOARD_TOP + d, BOARD_RIGHT + 1, BOARD_TOP + d);
        }

        painter.setRenderHint(QPainter::Antialiasing);
//      painter.setRenderHint(QPainter::HighQualityAntialiasing);
//      painter.setBackgroundMode(Qt::TransparentMode);

        if (Font >= 6) {
            painter.setPen(Qt::black);
            painter.setFont(QFont("Arial", Font));

            // Grid Coord //
            if (View & VIEW_LABEL) {
                int k1 = BOARD_TOP - GridSize * 1.05;
                int k2 = BOARD_DOWN + GridSize * 1.05;

                for (int i = 0, x = BOARD_LEFT; i < Child.Size; i++, x += GridSize) {
                    QChar Value = QChar('A' + i + (i < 8 ? 0 : 1));
                    if (!(View & VIEW_SCORE)) painter.drawText(x - GridSize + 1, k1 - GridSize + 1, Grid, Grid, Qt::AlignCenter, Value);
                    painter.drawText(x - GridSize + 1, k2 - GridSize + 1, Grid, Grid, Qt::AlignCenter, Value);
                }

                k1 = BOARD_LEFT - GridSize * 1.18;
                k2 = BOARD_RIGHT + GridSize * 1.17;

                for (int i = 0, y = BOARD_TOP; i < Child.Size; i++, y += GridSize) {
                    QString Value = QString::number(Child.Size - i);
                    painter.drawText(k1 - GridSize - 0, y - GridSize + 1, Grid, Grid, Qt::AlignCenter, Value);
                    painter.drawText(k2 - GridSize - 0, y - GridSize + 1, Grid, Grid, Qt::AlignCenter, Value);
                }
            }
            if (View & VIEW_SCORE) {
                painter.setFont(QFont("Arial", Font + 1));
                painter.drawText(0, BOARD_TOP - GridSize * 1.2 - GridSize + 1, size().width(), Grid, Qt::AlignCenter, Child.BOARD_SCORE);
            }
        }

        // Star //
        if (Child.Size >= 13) {
            painter.setPen(QPen(QColor(15, 15, 15), 1.2, Qt::SolidLine));
            painter.setBrush(Qt::black);

            for (int i = 0, k = 0; i < 9; i++, k += 2) {
                if ((Child.Size & 1) == 0 && i >= 4) break;
                painter.drawEllipse(BOARD_LEFT + Child.Star[k] * GridSize - 2, BOARD_TOP + Child.Star[k + 1] * GridSize - 2, 5, 5);
            }
        }

        if (Radius >= 0) {
            int size = GridSize / 4.8;
            QPolygonF Mark, Triangle;
            Mark << QPoint(-1 * size, -1 * size) << QPoint(1 * size, -1 * size) << QPoint(-1 * size, 1 * size) << QPoint(1 * size, 1 * size);
            double dsize = GridSize / 4.2;
            Triangle << QPoint(0, -1 * dsize) << QPoint(0.866 * dsize, 0.5 * dsize) << QPoint(-0.866 * dsize, 0.5 * dsize);

            for (int j = 0, y = BOARD_TOP; j < Child.Size; j++, y += GridSize) {
                for (int i = 0, x = BOARD_LEFT; i < Child.Size; i++, x += GridSize) {
                    int k = Child.GetPoint(i, j);
                    int color = Child.State.Table[k];
                    int label = Child.Status[k].Label;

                    // Area //
                    if ((View & VIEW_SCORE) && label != BOTH_AREA) {
                        painter.setPen(Qt::NoPen);
                        if (label == BLACK_AREA || label == WHITE_DEAD) {
                            if (theme) painter.setBrush(QBrush(Child.Status[k].Color));
                            else painter.setBrush(QBrush(QColor(0, 0, 0), Qt::BDiagPattern));
                        }
                        else {
                            if (theme) painter.setBrush(QBrush(Child.Status[k].Color));
                            else painter.setBrush(QBrush(QColor(255, 255, 255), Qt::Dense5Pattern));
                        }
                        int half = int (Radius);
                        int size = half * 2 + 1;
                        if (theme) painter.drawRect(x - Radius, y - Radius, GridSize, GridSize);
                        else painter.drawRect(x - half, y - half, size, size);

                    }

                    if (color == BLACK || color == WHITE) {
                        int half = GridSize / 2;
                        int size = half * 2 + 1;

                        if (color == BLACK) {
                            painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
                            painter.setBrush(QColor(0, 0, 0));
                            painter.drawEllipse(QPointF(x + 0.5, y + 0.5), Radius, Radius);
                        }
                        else {
                            painter.setPen(QPen(QColor(36, 36, 36), 1.01, Qt::SolidLine));
                            painter.setBrush(QColor(240, 240, 240));
                            painter.drawEllipse(QPointF(x + 0.5, y + 0.5), Radius, Radius);
                        }
                    }

                    // Mark //
                    if ((View & VIEW_SCORE) && label != BOTH_AREA) {
                        if (label == BLACK_DEAD || label == WHITE_DEAD) {
                            QPolygonF Poly = Mark.translated(x + 0.55, y + 1);
                            painter.setPen(QPen(color == BLACK ? Qt::white : Qt::black, 1.1, Qt::SolidLine));
                            painter.drawLine(Poly[0], Poly[3]);
                            painter.drawLine(Poly[1], Poly[2]);
                        }
                    }
                }
            }

            // Label //
            int total = Child.Record.Node[Child.Record.Index].Prop.size();
            for (int i = 0; i < total; i++) {
                Board::GoProp Prop = Child.Record.Node[Child.Record.Index].Prop[i];
                int color = Child.GetColor(Prop.Col, Prop.Row);
                int x = BOARD_LEFT + Prop.Col * GridSize;
                int y = BOARD_TOP + Prop.Row * GridSize;

                if (Prop.Label == Board::TOKEN_PLAY || Prop.Label == Board::TOKEN_CIRCLE) {
                    QColor color2 = (color == BLACK ? QColor(240, 240, 240) : Qt::black);
                    int Half = double (GridSize) / 12;
                    painter.setPen(QPen(color2, 1.04, Qt::SolidLine));
                    painter.setBrush(color2);
                    painter.drawEllipse(QPointF(x + 0.5, y + 0.5), Half, Half);
                }
                else if (Prop.Label == Board::TOKEN_TRIANGLE) {
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(QBrush(color == BLACK ? Qt::white : Qt::black));
                    painter.drawPolygon(Triangle.translated(x + 0.65, y + 1.4));
                }
                else if (Prop.Label == Board::TOKEN_MARK) {
                    QPolygonF Poly = Mark.translated(x + 0.55, y + 1);
                    painter.setPen(QPen(color == BLACK ? Qt::white : Qt::black, 1.2, Qt::SolidLine));
                    painter.drawLine(Poly[0], Poly[3]);
                    painter.drawLine(Poly[1], Poly[2]);
                }
                else if (Prop.Label == Board::TOKEN_SQUARE) {
                    int Half = GridSize / 6;
                    int size = Half * 2;
                    painter.setBrush(Qt::NoBrush);
                    painter.setPen(QPen(color == BLACK ? QColor(240, 240, 240) : Qt::black, 1, Qt::SolidLine));
                    painter.setRenderHint(QPainter::Antialiasing, false);
                    painter.drawRect(x - Half , y - Half, size, size);
                    painter.setRenderHint(QPainter::Antialiasing);
                }
                else if (Prop.Label == Board::TOKEN_LABEL || Prop.Label == Board::TOKEN_NUMBER) {
                    if (Font >= 6) {
                        if (color == EMPTY) painter.setBackgroundMode(Qt::OpaqueMode);
                        painter.setPen(QPen(color == BLACK ? Qt::white : Qt::black, 1, Qt::SolidLine));
                        painter.setFont(QFont("Arial", Font));
                        painter.drawText(x - GridSize + 1, y - GridSize + 1, Grid, Grid, Qt::AlignCenter,
                            Prop.Label == Board::TOKEN_LABEL ? QChar(Prop.Value) : QString::number(Prop.Value));
                        if (color == EMPTY) painter.setBackgroundMode(Qt::TransparentMode);
                    }
                }
            }

            // Cursor //
            if (Cursor[0].x() >=0 && Cursor[0].y() >= 0) {
                QColor color = Child.Record2.Index < 0 ? QColor(0, 224, 0) : Qt::red;
                painter.setPen(QPen(color, 2, Qt::SolidLine));
                painter.setBrush(color);
                painter.drawEllipse(BOARD_LEFT + Cursor[0].x() * GridSize - 2, BOARD_TOP + Cursor[0].y() * GridSize - 2, 5, 5);
            }
        }
    }

}

Window::Window()
{
    Child = new Widget(this);
    SetTitle("Window");
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
    if (Child->Mode == 0 || Child->Mode == BOARD_FILE) {
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
    else if (Child->Mode == BOARD_PLAY) {
        if (Child->Play[BLACK]) {
            QDockWidget *dock = new QDockWidget(" ", this);
            dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
            Child->Play[BLACK]->TextEdit = new QTextEdit(dock);
            Child->Play[BLACK]->TextEdit->setFontPointSize(10);
            dock->setWidget(Child->Play[BLACK]->TextEdit);
            addDockWidget(Qt::RightDockWidgetArea, dock);
        }

        if (Child->Play[WHITE]) {
            QDockWidget *dock = new QDockWidget(" ", this);
            dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
            Child->Play[WHITE]->TextEdit = new QTextEdit(dock);
            Child->Play[WHITE]->TextEdit->setFontPointSize(10);
            dock->setWidget(Child->Play[WHITE]->TextEdit);
            addDockWidget(Qt::RightDockWidgetArea, dock);
        }
    }
}
