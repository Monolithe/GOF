#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include <cstdlib>
#include <ctime>

#ifdef __linux__
#include <unistd.h>
#endif

#include <curses.h>

using namespace std;

namespace game {

bool running = true;

class renderer {
  public:
    renderer(int x, int y) {
        initscr();
        noecho();

        size.s_x = x + 2;
        size.s_y = y + 3;

        win = newwin(size.s_y, size.s_x, 0, 0);

        wmove(win, 0, 0);
    }

    ~renderer() {
        delwin(win);
        endwin();
    }

    void render(vector<vector<int>> univ, int gen) {
        waddch(win, '+');
        for (int i = 1; i < size.s_x - 1; i++) {
            waddch(win, '-');
        }
        waddch(win, '+');

        wmove(win, 1, 0);

        int y = 1, x = 0;

        for (auto i : univ) {
            waddch(win, '|');
            for (auto j : i) {
                if (j) {
                    waddch(win, '*');
                } else {
                    waddch(win, ' ');
                }
            }
            waddch(win, '|');
            y++;
            wmove(win, y, 0);
        }

        waddch(win, '+');
        for (int i = 1; i < size.s_x - 1; i++) {
            waddch(win, '-');
        }
        waddch(win, '+');

        wmove(win, y + 1, 0);

        wprintw(win, "Generation %d", gen);

        getyx(win, y, x);

        while (x < size.s_x) {
            waddch(win, ' ');
            x++;
        }

        nodelay(stdscr, TRUE);
        int ch = getch();

        if (ch == 'q') {
            running = false;
        } else if (ch == ERR) {
        }

        wrefresh(win);
    }

    void intro() {
        bool choice = false;

        nodelay(stdscr, TRUE);

        while (!choice) {
            int ch = getch();
            switch (ch) {
            case 's':
                choice = true;
                wmove(win, 0, 0);
                wrefresh(win);
                break;
            case 'q':
                running = false;
                choice = true;
                wrefresh(win);
                break;
            default:
                wmove(win, 0, 0);
                waddstr(win, "----Conway's Game of Life----\n");
                waddstr(win, "Use S to start game\n");
                waddstr(win, "Use Q to quit game\n");
                wrefresh(win);
                break;
            }
        }
    }

  private:
    WINDOW *win;

    struct _size {
        int s_x;
        int s_y;
    };

    struct _size size;
};

class universe {
  public:
    universe(int x, int y, float prob, int _freq, bool _ncurses) {
        living_prob = prob;
        freq = _freq;

        gen = 1;

        srand(time(NULL));

        univ.resize(y);

        for (int i = 0; i < y; i++) {
            univ[i].resize(x);
        }

        for (auto &i : univ) {
            for (auto &j : i) {
                float r = (float)rand() / (float)RAND_MAX;
                if (r <= living_prob) {
                    j = 1;
                }
            }
        }

        size.s_x = x;
        size.s_y = y;

        if (_ncurses) {
            ncurses = true;
            rend = make_shared<renderer>(x, y);
        }
    }

    void render() {
        if (!ncurses) {
            cout << "+";
            for (int i = 0; i < size.s_x; i++) {
                cout << "-";
            }
            cout << "+";
            cout << endl;

            for (auto i : univ) {
                cout << "|";
                for (auto j : i) {
                    if (j) {
                        cout << "*";
                    } else {
                        cout << " ";
                    }
                }
                cout << "|";
                cout << endl;
            }

            cout << "+";
            for (int i = 0; i < size.s_x; i++) {
                cout << "-";
            }
            cout << "+";
            cout << endl;
        } else {
            rend->render(univ, gen);
        }
    }

    void play() {
        if (ncurses) {
            rend->intro();
        }
        while (running) {
            if (!ncurses) {
#ifdef __linux__
                system("clear");
#elif _WIN32
                system("cls");
#endif
                render(gen);
            } else {
                render();
            }
            this_thread::sleep_for(chrono::milliseconds(freq));
            iteration();
            gen++;
        }
    }

  private:
    vector<vector<int>> univ;

    int gen;

    float living_prob;

    int freq;

    struct _size {
        int s_x;
        int s_y;
    };

    struct _size size;

    bool ncurses;

    shared_ptr<renderer> rend;

    void iteration() {
        vector<vector<int>> t_univ(univ);

        int x = 0, y = 0;

        for (auto i : univ) {
            for (auto j : i) {
                bool life;
                if (j) {
                    life = living(y, x);
                } else {
                    life = dead(y, x);
                }
                t_univ[y][x] = life ? 1 : 0;
                x++;
            }
            y++;
            x = 0;
        }

        univ = t_univ;
    }

    bool living(int y, int x) {
        int count = neighbours(y, x);
        return count == 2 || count == 3;
    }

    bool dead(int y, int x) {
        int count = neighbours(y, x);
        return count == 3;
    }

    int neighbours(int y, int x) {
        int count = 0;
        if (y > 0) {
            if (x > 0) {
                count += univ[y - 1][x - 1];
            }
            count += univ[y - 1][x];
            if (x < univ[y].size() - 1) {
                count += univ[y - 1][x + 1];
            }
        }
        if (y < univ.size() - 1) {
            if (x > 0) {
                count += univ[y + 1][x - 1];
            }
            count += univ[y + 1][x];
            if (x < univ[y].size() - 1) {
                count += univ[y + 1][x + 1];
            }
        }
        if (x > 0) {
            count += univ[y][x - 1];
        }
        if (x < univ[y].size() - 1) {
            count += univ[y][x + 1];
        }

        return count;
    }

    void render(int gen) {
        render();
        cout << "Generation : " << gen << endl;
    }
};
} // namespace game

int main(int argc, char *argv[]) {
    int opt;

    float r = 0.10f;
    int freq = 500;

#ifdef __linux__
    while ((opt = getopt(argc, argv, "p:f:")) != -1) {
        switch (opt) {
        case 'p':
            r = strtof(optarg, NULL);
            break;
        case 'f':
            freq = strtol(optarg, NULL, 10);
            break;
        }
    }
#endif

    game::universe univ(120, 30, r, freq, true);

    univ.play();
    return 0;
}
