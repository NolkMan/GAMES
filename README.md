# GAMES

This whole repository is a collection of small games made in c using only ncurses. All packed in a single file.

### Compiling

Games can be compiled using cmake in standard fasion
```bash
cd GAME/
mkdir build; cd build
cmake ..
make
```

Or just by using
```bash
gcc GAME.c -o GAME -std=c99 -lcurses
```

#####Dependencies

Games only dependency is ncurses which can be downloaded some packet manager on linux system.
All games are tested on Linux and may not work on Windows. Those games are not really maintained either.
