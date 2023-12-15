#include "cpoint.h"
#include "screen.h"
#include <ctype.h>
#include <ncurses.h>
#include <list>
#include <string>
#include <iostream>
#include <cstdlib>

using namespace std;

class CView
{
protected:
    CRect geom;

public:
    CView(CRect g) : geom(g){};
    virtual void paint() = 0;
    virtual bool handleEvent(int key) = 0;
    virtual void move(const CPoint &delta)
    {
        geom.topleft += delta;
    };
    virtual ~CView(){};
};

class CWindow : public CView
{
protected:
    char c;

public:
    CWindow(CRect r, char _c = '*') : CView(r), c(_c){};
    void paint()
    {
        for (int i = geom.topleft.y; i < geom.topleft.y + geom.size.y; i++)
        {
            gotoyx(i, geom.topleft.x);
            for (int j = 0; j < geom.size.x; j++)
                printw("%c", c);
        };
    };
    bool handleEvent(int key)
    {
        switch (key)
        {
        case KEY_UP:
            move(CPoint(0, -1));
            return true;
        case KEY_DOWN:
            move(CPoint(0, 1));
            return true;
        case KEY_RIGHT:
            move(CPoint(1, 0));
            return true;
        case KEY_LEFT:
            move(CPoint(-1, 0));
            return true;
        };
        return false;
    };
};

class CFramedWindow : public CWindow
{
public:
    CFramedWindow(CRect r, char _c = '\'') : CWindow(r, _c){};
    void paint()
    {
        for (int i = geom.topleft.y; i < geom.topleft.y + geom.size.y; i++)
        {
            gotoyx(i, geom.topleft.x);
            if ((i == geom.topleft.y) ||
                (i == geom.topleft.y + geom.size.y - 1))
            {
                printw("+");
                for (int j = 1; j < geom.size.x - 1; j++)
                    printw("-");
                printw("+");
            }
            else
            {
                printw("|");
                for (int j = 1; j < geom.size.x - 1; j++)
                    printw("%c", c);
                printw("|");
            }
        }
    };
};

class CInputLine : public CFramedWindow
{
    string text;

public:
    CInputLine(CRect r, char _c = ',') : CFramedWindow(r, _c){};
    void paint()
    {
        CFramedWindow::paint();
        gotoyx(geom.topleft.y + 1, geom.topleft.x + 1);

        for (unsigned j = 1, i = 0;
             (j + 1 < (unsigned)geom.size.x) && (i < text.length()); j++, i++)
            printw("%c", text[i]);
    };
    bool handleEvent(int c)
    {
        if (CFramedWindow::handleEvent(c))
            return true;
        if ((c == KEY_DC) || (c == KEY_BACKSPACE))
        {
            if (text.length() > 0)
            {
                text.erase(text.length() - 1);
                return true;
            };
        }
        if ((c > 255) || (c < 0))
            return false;
        if (!isalnum(c) && (c != ' '))
            return false;
        text.push_back(c);
        return true;
    }
};

class CSnake : public CFramedWindow
{
private:
    list<CPoint> snake;
    CPoint fruit;
    int direction;
    unsigned int score;
    unsigned int Timeout;

    bool gameStarted;
    bool gamePaused;
    bool helpScreen;
    bool endGame;

public:
    CSnake(CRect r, char _c = ' ') : CFramedWindow(r, _c)
    {
        snake.push_back(CPoint(4, 4));
        fruit.x = 6;
        fruit.y = 6;
        this->score = 0;
        this->gameStarted = false;
        this->Timeout = 350;
    }

    void setTimeout()
    {
        if (this->Timeout >= 90)
        {
            this->Timeout -= 10;
            timeout(Timeout);
        }
    }
    void paint() override
    {
        if (gameStarted == false)
        {

            showWelcomeScreen();
            snake.front().x = 4 + geom.topleft.x;
            snake.front().y = 4 + geom.topleft.y;

            fruit.x = 6 + geom.topleft.x;
            fruit.y = 6 + geom.topleft.y;
            return;
        }
        else if (gamePaused == true)
        {
            showPauseScreen();
            helpScreen = false;
            return;
        }
        else if (helpScreen == true)
        {
            showHelpScreen();
            gamePaused = false;
            return;
        }
        else if (endGame == true)
        {
            showEndGame();
            return;
        }
        CFramedWindow::paint();
        printSnakeFruit();
        printScore();
        autoMove();
    }

    void printSnakeFruit()
    {
        for (const auto &point : snake)
        {
            gotoyx(point.y, point.x);
            if (point.x == snake.front().x && point.y == snake.front().y)
            {
                printw("0");
            }
            else
                printw("O");
        }

        gotoyx(fruit.y, fruit.x);
        printw("*");
    }

    void printScore()
    {
        gotoyx(geom.topleft.y - 1, geom.topleft.x);
        printw("| Wynik: %d |", score);
    }

    void updateSnake(int key)
    {
        if (snake.size() == 1)
        {
            if (key == KEY_UP)
            {
                snake.front().y -= 1;
            }
            else if (key == KEY_DOWN)
            {
                snake.front().y += 1;
            }
            else if (key == KEY_LEFT)
            {
                snake.front().x -= 1;
            }
            else if (key == KEY_RIGHT)
            {
                snake.front().x += 1;
            }
        }
        else
        {
            for (list<CPoint>::iterator it = snake.begin(); it != snake.end(); it++)
            {

                if (key == KEY_UP)
                {
                    it->y -= 1;
                }
                else if (key == KEY_DOWN)
                {
                    it->y += 1;
                }
                else if (key == KEY_LEFT)
                {
                    it->x -= 1;
                }
                else if (key == KEY_RIGHT)
                {
                    it->x += 1;
                }
            }
        }
    }
    void updateFruit(int key)
    {

        if (key == KEY_UP)
        {
            fruit.y -= 1;
        }
        else if (key == KEY_DOWN)
        {
            fruit.y += 1;
        }
        else if (key == KEY_LEFT)
        {
            fruit.x -= 1;
        }
        else if (key == KEY_RIGHT)
        {
            fruit.x += 1;
        }
    }
    bool handleEvent(int key) override
    {
        if (CFramedWindow::handleEvent(key))
        {
            updateFruit(key);
            updateSnake(key);
            return true;
        }

        if (key == 'p')
        {
            gamePaused = true;
            return true;
        }

        if (key == 'h')
        {
            helpScreen = true;
            return true;
        }

        if (key == 'r')
        {
            restart();
            endGame = false;
            return true;
        }

        if (key == 'w' || key == 's' || key == 'a' || key == 'd')
        {
            changeDirection(key);

            if (gameStarted == false)
            {
                startGame();
            }
            else if (gamePaused == true)
            {
                gamePaused = false;
            }
            else if (helpScreen == true)
            {
                helpScreen = false;
            }

            return true;
        }
        return false;
    }

    void changeDirection(int key)
    {
        if (key == 'w' && direction != 1)
        {
            direction = 0;
        }
        else if (key == 's' && direction != 0)
        {
            direction = 1;
        }
        else if (key == 'a' && direction != 3)
        {
            direction = 2;
        }

        else if (key == 'd' && direction != 2)
        {
            direction = 3;
        }
    }
    void restart()
    {
        snake.clear();
        snake.push_back(CPoint(4 + geom.topleft.x, 4 + geom.topleft.y));
        fruit.x = 6 + geom.topleft.x;
        fruit.y = 6 + geom.topleft.y;

        gameStarted = false;
        gamePaused = false;
        helpScreen = false;
        endGame = false;
        score = 0;
    }

    void update()
    {
        int newHeadX = snake.front().x;
        int newHeadY = snake.front().y;

        switch (this->direction)
        {
        case 0:
            newHeadY--;
            break;
        case 1:
            newHeadY++;
            break;
        case 2:
            newHeadX--;
            break;
        case 3:
            newHeadX++;
            break;
        }

        teleport(newHeadX, newHeadY);
        CPoint newHead = {newHeadX, newHeadY};

        snake.push_front(newHead);

        if (eatItself() == true)
        {
            endGame = true;
            return;
        }

        if (eat() == true)
        {
            score += 10;
            snake.push_back(CPoint(fruit.x, fruit.y));
            if (this->score % 10 == 0 && this->score != 0)
            {
                setTimeout();
            }
            spawnFruit();
        }

        if (snake.size() > 1)
        {
            snake.pop_back();
        }
    }
    void spawnFruit()
    {
        int minX = geom.topleft.x + 1;
        int maxX = geom.size.x - 1;

        int minY = geom.topleft.y + 1;
        int maxY = geom.size.y - 1;

        do
        {
            fruit.x = minX + rand() % ((maxX - minX) + 1);
            fruit.y = minY + rand() % ((maxY - minY) + 1);
        } while (foodInSnake());
    }
    void teleport(int &newHeadX, int &newHeadY)
    {
        if (newHeadX == geom.topleft.x + geom.size.x - 1)
        {
            newHeadX = geom.topleft.x + 1;
        }
        else if (newHeadY == geom.size.y + geom.topleft.y - 1)
        {
            newHeadY = geom.topleft.y + 1;
        }
        else if (newHeadX == geom.topleft.x)
        {
            newHeadX = geom.topleft.x + geom.size.x - 3;
        }
        else if (newHeadY == geom.topleft.y)
        {
            newHeadY = geom.topleft.y + geom.size.y - 2;
        }
    }

    bool foodInSnake()
    {
        for (list<CPoint>::iterator it = snake.begin(); it != snake.end(); ++it)
        {
            if (it->x == fruit.x && it->y == fruit.y)
                return true;
        }
        return false;
    }
    void autoMove()
    {
        update();
    }

    bool eatItself()
    {
        const CPoint &head = snake.front();
        for (list<CPoint>::iterator it = next(snake.begin()); it != snake.end(); ++it)
        {
            if (it->x == head.x && it->y == head.y)
            {
                return true;
            }
        }
        return false;
    }

    bool eat()
    {
        if (fruit.x == snake.front().x && fruit.y == snake.front().y)
        {
            return true;
        }

        return false;
    }

    void startGame()
    {
        gameStarted = true;
    }

    void showWelcomeScreen()
    {
        CFramedWindow::paint();
        gotoyx(geom.topleft.y + 1, geom.topleft.x + 1);
        printw("Gra w weza");
        gotoyx(geom.topleft.y + 3, geom.topleft.x + 1);
        printw("instrukcja");
        gotoyx(geom.topleft.y + 4, geom.topleft.x + 1);
        printw("Uzyj 'w', 's', 'a', 'd' aby kontrolowac weza.");
        gotoyx(geom.topleft.y + 5, geom.topleft.x + 1);
        printw("Uzyj 'p' aby spauzowac weza.");
        gotoyx(geom.topleft.y + 6, geom.topleft.x + 1);
        printw("Uzyj 'h' aby wyswietlic pomoc.");
        gotoyx(geom.topleft.y + 7, geom.topleft.x + 1);
        printw("Wcisnij klawisz ruchu aby rozpoczac.");

        refresh();
    }

    void showPauseScreen()
    {

        CFramedWindow::paint();
        gotoyx(geom.topleft.y + 1, geom.topleft.x + 1);
        printw("Gra wstrzymana");
        gotoyx(geom.topleft.y + 3, geom.topleft.x + 1);
        printw("Wcisnij klawisz ruchu aby wznowic");

        refresh();
    }

    void showHelpScreen()
    {

        CFramedWindow::paint();
        gotoyx(geom.topleft.y + 1, geom.topleft.x + 1);
        printw("Pomoc");
        gotoyx(geom.topleft.y + 3, geom.topleft.x + 1);
        printw("Uzyj 'w', 's', 'a', 'd' aby kontrolowac weza.");
        gotoyx(geom.topleft.y + 4, geom.topleft.x + 1);
        printw("Wcisnij klawisz ruchu aby kontynowac");

        refresh();
    }

    void showEndGame()
    {
        CFramedWindow::paint();
        gotoyx(geom.topleft.y + 1, geom.topleft.x + 1);
        printw("Przegrales aby zrestartowac wcisnij 'r'");
        gotoyx(geom.topleft.y + 3, geom.topleft.x + 1);
        printw("Twoj wynik %d", score);
        refresh();
    }
};

class CGroup : public CView
{
protected:
    list<CView *> children;

public:
    CGroup(CRect g) : CView(g){};
    void paint()
    {
        for (list<CView *>::iterator i = children.begin(); i != children.end();
             i++)
            (*i)->paint();
    };
    bool handleEvent(int key)
    {
        if (!children.empty() && children.back()->handleEvent(key))
            return true;
        if (key == '\t')
        {
            if (!children.empty())
            {
                children.push_front(children.back());
                children.pop_back();
            };
            return true;
        }
        return false;
    };
    void insert(CView *v)
    {
        children.push_back(v);
    };
    ~CGroup()
    {
        for (list<CView *>::iterator i = children.begin(); i != children.end();
             i++)
            delete (*i);
    };
};

class CDesktop : public CGroup
{
public:
    CDesktop() : CGroup(CRect())
    {
        int y, x;
        init_screen();
        getscreensize(y, x);
        geom.size.x = x;
        geom.size.y = y;
    };
    ~CDesktop()
    {
        done_screen();
    };

    void paint()
    {
        for (int i = geom.topleft.y; i < geom.topleft.y + geom.size.y; i++)
        {
            gotoyx(i, geom.topleft.x);
            for (int j = 0; j < geom.size.x; j++)
                printw(".");
        };
        CGroup::paint();
    }

    int getEvent()
    {
        return ngetch();
    };

    void run()
    {
        srand(time(NULL));
        int c;
        paint();
        refresh();

        while (1)
        {
            c = getEvent();
            if (c == 27)
                break;

            handleEvent(c);

            refresh();
            paint();
        };
    };
};

int main()
{
    CDesktop d;
    d.insert(new CInputLine(CRect(CPoint(5, 7), CPoint(15, 15)), ' '));
    d.insert(new CWindow(CRect(CPoint(2, 3), CPoint(20, 10)), '?'));

    d.insert(new CSnake(CRect(CPoint(5, 5), CPoint(50, 20))));

    d.run();
    return 0;
}
