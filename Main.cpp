#include "Window.h"

int main(int argc, char *argv[])
{
    QApplication App(argc, argv);
    QStringList args = QCoreApplication::arguments();
    Window Main;

    if (argc == 2 || argc == 3) {
        if (!args[1].startsWith("-")) {
            Main.Child->Mode = MODE_FILE;
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
            Main.Child->Mode = MODE_PLAY;
            Main.CreateDock();

            if (!str[0].isEmpty()) {
                Main.Child->Side &= 2; // human not black
                Main.Child->Other = Main.Player1;
                Main.Player1->Setup(str[0], arg[0]);
            }
            if (!str[1].isEmpty()) {
                Main.Child->Side &= 1; // human not white
                Main.Child->Other = Main.Player2;
                Main.Player2->Setup(str[1], arg[1]);
            }

            Main.Player1->Task.append("genmove black\r\n");
            Main.Player1->Send();
            Main.Player2->Send();
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
