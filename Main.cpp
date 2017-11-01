#include "Window.h"

int main(int argc, char *argv[])
{
    QApplication App(argc, argv);
    QStringList args = QCoreApplication::arguments();
    Window Main;

    if (argc == 2 || argc == 3) {
        if (!args[1].startsWith("-")) {
            Main.Child->Mode = BOARD_FILE;
            Main.CreateDock();
            Main.Child->Read(args[1], (argc == 2 ? 0 : args[2].toInt()));
        }
    }

    if (argc > 2 && Main.Child->Mode == 0) {
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
                    Main.Child->Child.Reset(size, size);
                    i++;
                }
            }
            else if (args[i] == "-komi") {
                if (i + 1 < argc) {
                    Main.Child->Child.BOARD_KOMI = args[i + 1];
                    Main.Child->Child.Komi = Main.Child->Child.BOARD_KOMI.toDouble();
                    i++;
                }
            }
            else if (args[i] == "-handicap") {
                if (i + 1 < argc) {
                    Main.Child->Child.BOARD_HANDICAP = args[i + 1];
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
            if (!str[0].isEmpty()) {
                Main.Child->Side |= BLACK; // computer black
                Main.Child->Play[BLACK] = new Player(Main.Child, BLACK, Main.Child->Child.Size, Main.Child->Child.Komi);
                Main.Child->Play[BLACK]->Setup(str[0], arg[0]);
            }
            if (!str[1].isEmpty()) {
                Main.Child->Side |= WHITE; // computer white
                Main.Child->Play[WHITE] = new Player(Main.Child, WHITE, Main.Child->Child.Size, Main.Child->Child.Komi);
                Main.Child->Play[WHITE]->Setup(str[1], arg[1]);
            }

            Main.Child->Mode = BOARD_PLAY;
            Main.CreateDock();

            if (!Main.Child->Child.BOARD_HANDICAP.isEmpty()) {
                if (Main.Child->Child.SetHandicap(Main.Child->Child.BOARD_HANDICAP.toInt())) {
                    Main.Child->Child.State.Turn = WHITE;
                    for (int i = 1; i <= Main.Child->Child.Handicap[0]; i += 2) {
                        if (Main.Child->Play[BLACK])
                            Main.Child->Play[BLACK]->Play(Main.Child->Child.Handicap[i], Main.Child->Child.Handicap[i + 1], Main.Child->Child.Size, BLACK);
                        if (Main.Child->Play[WHITE])
                            Main.Child->Play[WHITE]->Play(Main.Child->Child.Handicap[i], Main.Child->Child.Handicap[i + 1], Main.Child->Child.Size, BLACK);
                    }
                }
                else Main.Child->Child.BOARD_HANDICAP.clear();
            }

            if (Main.Child->Play[BLACK]) {
                if (Main.Child->Child.BOARD_HANDICAP.isEmpty())
                    Main.Child->Play[BLACK]->Append("genmove b");
                Main.Child->Play[BLACK]->Send();
            }
            if (Main.Child->Play[WHITE]) {
                if (!Main.Child->Child.BOARD_HANDICAP.isEmpty())
                    Main.Child->Play[WHITE]->Append("genmove w");
                Main.Child->Play[WHITE]->Send();
            }
        }
    }

    if (Main.Child->Mode == 0) {
        Main.CreateDock();
    }

    Main.show();
    Main.Child->setFocus();
    Main.showMaximized();

    return App.exec();
}
