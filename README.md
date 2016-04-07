# Qt Go Board
Graphical Go Board in Qt and C++ with GTP support

![Alt text](A01.png?raw=true)

Usage Examples

board file.sgf (or drag file to the program)

board -black "C:\GNU Go 3.8\gnugo" "--mode gtp --level 16" -size 19 -komi 7.5

board -white "C:\Fuego 1.1.4\fuego.exe" -size 13 -komi 7.5

board -white "C:\Pachi 11\pachi.exe" "-f book.dat -t =100000 threads=1,pondering=0" -size 13 -komi 7.5

board -white "C:\Pachi 11\pachi.exe" "-d 0 -f book.dat -t =100000 threads=1,pondering=0" -size 13 -komi 7.5

board -white "C:\MoGo\mogo.exe" "--13 --time 24 --nbThreads 1" -size 13 -komi 7.5

board -black "C:\GNU Go 3.6\gnugo.exe" "--mode gtp" -white "C:\GNU Go 3.8\gnugo" "--mode gtp --level 16" -size 13 -komi 7.5

board -black "C:\MoGo\mogo.exe" "--13 --time 24 --nbThreads 1" -white "C:\Pachi 11\pachi.exe" "-f book.dat -t =100000 threads=1,pondering=0" -size 13 -komi 7.5

board -black "C:\Fuego 1.1.4\fuego.exe" -white "C:\Pachi 11\pachi.exe" "-f book.dat -t =100000 threads=1,pondering=0" -size 13 -komi 7.5

board -black "C:\Fuego 1.1.4\fuego.exe" "--config fuegoconf.cfg" -white "C:\Pachi 11\pachi.exe" "-f book.dat -t =100000 threads=1,pondering=0" -size 13 -komi 7.5

Key Controls

Next Move - Right or Down or Wheel Down

Back Move - Left or Up or Wheel Up

Next 10 Moves - Page Down

Back 10 Moves - Page Up

Start - Home

End - End

Board Label - Space

Undo - Left or Up or Wheel Up

Test Play - Left Click

Test Restore - Esc

Pass - P

Save - S (001.SGF)

![Alt text](A02.png?raw=true)

![Alt text](A03.png?raw=true)
