# nchess
A curses based, UCI compatible, chess GUI.

![nchess game between bitbit and stockfish](files/nchess.png?raw=true)

## Building
### Linux
Run
```
$ make
```
in the root directory.

### Windows
The windows build has currently only been tested with gcc from MinGW-w64, but
might work with other compilers. It also depends on PDCurses. Run
```
$ make -f Makefile.win
```
in the root directory.
