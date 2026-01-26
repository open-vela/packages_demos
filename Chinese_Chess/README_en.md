# Chinese_Chess
Chinese_Chess is a Chinese chess game developed based on the OpenVela system.

## Core Features
1. Click a piece to select it, then click a position on the board to move the piece.  
2. Rook, Knight, Cannon, Elephant, Advisor, General/King, and Soldier pieces move according to the rules of Chinese chess.  
3. Pieces can capture opponent pieces.  
4. The game ends when the opponent’s General/King is captured.

## Documentation
- [Update Log](Update.md)  
- [Gameplay Demo](Chinese_Chess_Show/Chinese_Chess_game.gif)

## Asset Description
The chessboard and chess pieces displayed in the program are all drawn by the author using basic graphics.

## Project Structure
```
Chinese_Chess/
├── Chinese_chess.c
├── Chinese_chess.h
├── Chinese_chess_main.c
├── Kconfig
├── Makefile
├── Make.defs
├── README.md
├── README-en.md
├── Update.md
├── Update_en.md
│
├── res/
│    ├── fonts/
│    └── icons/  
└── Chinese_Chess_Show/
      └── Chinese_Chess_game.gif

```  
