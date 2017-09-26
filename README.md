# Qt Go Board
Graphical Go Board in Qt and C++ with GTP support

![Alt text](A01.png?raw=true)

### Usage Examples

Board file.sgf (or drag file to the program)

Board file.sgf 100 (Open file and move to step 100)

Board -black "C:\GNU Go 3.8\gnugo" "--mode gtp --level 16" -size 19 -komi 7.5

Board -white "C:\Fuego 1.1.4\fuego.exe" -size 13 -komi 7.5 -handicap 2

Board -white "C:\Pachi 11\pachi.exe" "-f book.dat -t =100000 threads=1,pondering=0" -size 13 -komi 7.5

Board -white "C:\Pachi 11\pachi.exe" "-d 0 -f book.dat -t =100000 threads=1,pondering=0" -size 13 -komi 7.5

Board -white "C:\MoGo\mogo.exe" "--13 --time 24 --nbThreads 1" -size 13 -komi 7.5

Board -black "C:\GNU Go 3.6\gnugo.exe" "--mode gtp" -white "C:\GNU Go 3.8\gnugo" "--mode gtp --level 16" -size 13 -komi 7.5 -handicap 2

Board -black "C:\MoGo\mogo.exe" "--13 --time 24 --nbThreads 1" -white "C:\Pachi 11\pachi.exe" "-f book.dat -t =100000 threads=1,pondering=0" -size 13 -komi 7.5

Board -black "C:\Fuego 1.1.4\fuego.exe" -white "C:\Pachi 11\pachi.exe" "-f book.dat -t =100000 threads=1,pondering=0" -size 13 -komi 7.5

Board -black "C:\Fuego 1.1.4\fuego.exe" "--config fuegoconf.cfg" -white "C:\Pachi 11\pachi.exe" "-f book.dat -t =100000 threads=1,pondering=0" -size 13 -komi 7.5

### Key Controls

    Navigation : Left, Right, Up, Down, Page Up, Page Down, Home, End, Esc, Wheel Up, Wheel Down

    Other : F1 Coord, F2 Score, S Save 001.SGF, P Pass, T Pause

![Alt text](A02.png?raw=true)

![Alt text](A03.png?raw=true)

![Alt text](A04.png?raw=true)

![Alt text](A05.png?raw=true)
