# Qt Go Board
Graphical Go Board in Qt and C++ with GTP support

![Alt text](A01.png?raw=true)

Usage Examples

Board file.sgf (or drag file to the program)

Board file.sgf 100 (Open file and move to step 100)

Board -black "C:\GNU Go 3.8\gnugo" "--mode gtp --level 16" -size 19 -komi 7.5

Board -white "C:\Fuego 1.1.4\fuego.exe" -size 13 -komi 7.5

Board -white "C:\Pachi 11\pachi.exe" "-f book.dat -t =100000 threads=1,pondering=0" -size 13 -komi 7.5

Board -white "C:\Pachi 11\pachi.exe" "-d 0 -f book.dat -t =100000 threads=1,pondering=0" -size 13 -komi 7.5

Board -white "C:\MoGo\mogo.exe" "--13 --time 24 --nbThreads 1" -size 13 -komi 7.5

Board -black "C:\GNU Go 3.6\gnugo.exe" "--mode gtp" -white "C:\GNU Go 3.8\gnugo" "--mode gtp --level 16" -size 13 -komi 7.5

Board -black "C:\MoGo\mogo.exe" "--13 --time 24 --nbThreads 1" -white "C:\Pachi 11\pachi.exe" "-f book.dat -t =100000 threads=1,pondering=0" -size 13 -komi 7.5

Board -black "C:\Fuego 1.1.4\fuego.exe" -white "C:\Pachi 11\pachi.exe" "-f book.dat -t =100000 threads=1,pondering=0" -size 13 -komi 7.5

Board -black "C:\Fuego 1.1.4\fuego.exe" "--config fuegoconf.cfg" -white "C:\Pachi 11\pachi.exe" "-f book.dat -t =100000 threads=1,pondering=0" -size 13 -komi 7.5

Key Controls

Start - Home

Forward - Right or Down or Wheel Down

Forward 10 - Page Down

Undo - Left or Up or Wheel Up

Undo 10 - Page Up

End - End

Grid Label - Space

Test Play - Left Click

Test Restore - Esc

Pass - P

Pause - T

Score - A

Save - S (001.SGF)

![Alt text](A02.png?raw=true)

![Alt text](A03.png?raw=true)

![Alt text](A04.png?raw=true)

![Alt text](A05.png?raw=true)
