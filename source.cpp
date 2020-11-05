#include <iostream>
#include <windows.h>
#include <thread>
#include <vector>
using namespace std;

wstring teromino[7];
int fieldwidth = 12;
int fieldheight = 22;
unsigned char *field = nullptr;

int nScreenWidth = 80;			// Console Screen Size X (columns)
int nScreenHeight = 40;			// Console Screen Size Y (rows)

int rotate(int px, int py, int r) {
    switch (r % 4)
    {
    case 0: return py * 4 + px;         // 0 degree
    case 1: return 12 + py - px * 4;    // 90 degree
    case 2: return 15 - py * 4 - px;    // 180 degree
    case 3: return 3 - py + px * 4;     // 270 degree
    }
    return -1;
}

bool DoesPieceFit(int nTeromino, int nCurrentRotation, int nPosX, int nPosY) {
    for (int px = 0; px < 4; ++px) {
        for (int py = 0; py < 4; ++py) {
            // piece index after rotation
            int pi = rotate(px, py, nCurrentRotation);
            // field index
            int fi = (nPosY + py) * fieldwidth + (nPosX + px);

            if (nPosX + px >= 0 && nPosX + px < fieldwidth && nPosY + py >= 0 && nPosY + py < fieldheight) {
                if (teromino[nTeromino][pi] == L'X' && field[fi] != 0)
                    return false;
            }
        }
    }
    return true;
}

int main() {
    
    //create teromino
    teromino[0].append(L"..X.");
    teromino[0].append(L"..X.");
    teromino[0].append(L"..X.");
    teromino[0].append(L"..X.");

    teromino[1].append(L".X..");
    teromino[1].append(L".XX.");
    teromino[1].append(L"..X.");
    teromino[1].append(L"....");

    teromino[2].append(L"..X.");
    teromino[2].append(L".XX.");
    teromino[2].append(L".X..");
    teromino[2].append(L"....");

    teromino[3].append(L"....");
    teromino[3].append(L".XX.");
    teromino[3].append(L".XX.");
    teromino[3].append(L"....");

    teromino[4].append(L"..X.");
    teromino[4].append(L".XX.");
    teromino[4].append(L"..X.");
    teromino[4].append(L"....");

    teromino[5].append(L"....");
    teromino[5].append(L".XX.");
    teromino[5].append(L"..X.");
    teromino[5].append(L"..X.");

    teromino[6].append(L"....");
    teromino[6].append(L".XX.");
    teromino[6].append(L".X..");
    teromino[6].append(L".X..");

    field = new unsigned char[fieldwidth * fieldheight];
    for (int i = 0; i < fieldheight; ++i) {
        for (int j = 0; j < fieldwidth; ++j) {
            field[i * fieldwidth + j] = (j == 0 || j == fieldwidth - 1 || i == fieldheight - 1) ? 9 : 0;
        }
    }

    // Create Screen Buffer
	wchar_t *screen = new wchar_t[nScreenWidth*nScreenHeight];
    for (int i = 0; i < nScreenWidth * nScreenHeight; ++i)
        screen[i] = L' ';
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleScreenBufferSize(hConsole, {nScreenWidth, nScreenHeight});
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

    // game loop
    bool bGameOver = false;

    int nCurrentPiece = rand() % 7;
    int nCurrentRotation = 0;
    int nCurrentX = fieldwidth / 2;
    int nCurrentY = 0;

    bool bKey[4];   // 0: left, 1 : up, 2: right, 3: down
    bool bRotationhold = false; // make rotation latchable

    int nSpeed = 20;
    int nSpeedCounter = 0;
    int bForceDown = false;
    int nPieceCount = 0;
    int nScore = 0;
    
    vector<int> vLine; 

    while (!bGameOver) {

        // timing
        this_thread::sleep_for(50ms);
        ++nSpeedCounter;
        bForceDown = (nSpeedCounter == nSpeed);

        // input
        for (int i = 0; i < 4; ++i)     //L   U   R   D
            bKey[i] = (GetAsyncKeyState("\x25\x26\x27\x28"[i]) & 0x8000) != 0; 

        // game logic
        // press left/right/down key
        nCurrentX -= (bKey[0] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;
        nCurrentX += (bKey[2] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
        nCurrentY += (bKey[3] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;
        // press up key to rotate
        if (bKey[1]) {
            nCurrentRotation += (!bRotationhold && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY)) ? 1 : 0;
            bRotationhold = true;
        }
        else
            bRotationhold = false;
        
        if (bForceDown) {
            nSpeedCounter = 0;
            if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) 
                ++nCurrentY;    // fall down
            else {
                // lock the piece in the field
                for (int px = 0; px < 4; ++px) {
                    for (int py = 0; py < 4; ++py) {
                        if (teromino[nCurrentPiece][rotate(px, py, nCurrentRotation)] == L'X')
                            field[(nCurrentY + py) * fieldwidth + nCurrentX + px] = nCurrentPiece + 1;
                    }
                }

                ++nPieceCount;
                if (nPieceCount % 10) {
                    if (nSpeedCounter >= 10)
                        --nSpeed;
                }

                // check we got any line
                for (int py = 0; py < 4; ++py) {
                    if (nCurrentY + py < fieldheight - 1) {
                        bool bLine = true;
                        for (int px = 1; px < fieldwidth - 1; ++px)
                            bLine &= (field[(nCurrentY + py) * fieldwidth + px] != 0);
                        
                        if (bLine) {
                            vLine.push_back(nCurrentY + py);
                            for (int px = 1; px < fieldwidth - 1; ++px)
                                field[(nCurrentY + py) * fieldwidth + px] = 8;
                        }
                    }
                }

                // count the score
                nScore += 25;
                if (!vLine.empty())
                    nScore += (1 << vLine.size()) * 100;

                // choose next piece
                nCurrentX = fieldwidth / 2;
                nCurrentY = 0;
                nCurrentRotation = 0;
                nCurrentPiece = rand() % 7;

                // game over or not?
                bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
            }
        }

        // render output

        // draw field
        for (int i = 0; i < fieldheight; ++i) {
            for (int j = 0; j < fieldwidth; ++j) 
                screen[(i + 2) * nScreenWidth + j + 2] = L" ABCDEFG=#"[field[i * fieldwidth + j]];
        }

        // draw current(falling) piece
        for (int px = 0; px < 4; ++px) {
            for (int py = 0; py < 4; ++py) {
                if (teromino[nCurrentPiece][rotate(px, py, nCurrentRotation)] == L'X')
                    screen[(nCurrentY + py + 2) * nScreenWidth + nCurrentX + px + 2] = nCurrentPiece + 65;
            }
        }

        // display score
        swprintf_s(&screen[2 * nScreenWidth + fieldwidth + 6], 16, L"Score: %8d", nScore);

        if (!vLine.empty()) {
            WriteConsoleOutputCharacterW(hConsole, screen, nScreenWidth * nScreenHeight, {0, 0}, &dwBytesWritten);
            this_thread::sleep_for(400ms);

            for (int &i : vLine) {
                for (int px = 1; px < fieldwidth - 1; ++px) {
                    for (int py = i; py > 0; --py)
                        field[py * fieldwidth + px] = field[(py - 1) * fieldwidth + px];
                    field[px] = 0;
                }
            }
            vLine.clear();
        }

        // display frame
        WriteConsoleOutputCharacterW(hConsole, screen, nScreenWidth * nScreenHeight, {0, 0}, &dwBytesWritten);
    }

    CloseHandle(hConsole);
    cout << "GAME OVER!!\n";
    cout << "Score: " << nScore << endl;
    system("pause");

    return 0;
}