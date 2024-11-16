#include <iostream>
#include <sstream>
#include <algorithm>
#include <string>
#include <queue>
#include <deque>
#include <stack>
#include <vector>
#include <getopt.h>
#include <list>
#include <set>
#include <tuple>
#include <map>

using namespace std;

enum class Mode {
    kNone = 0,
    kStack,
    kQueue,
    kMap,
    kList,
};

struct Position {
    size_t level;
    size_t row;
    size_t col;
    char direction;

    Position() {};
    Position(size_t level, size_t row, size_t col, char direction) 
        : level(level), row(row), col(col), direction(direction) {}

    bool operator!=(const Position& other) const {
        return level != other.level || row != other.row || col != other.col || direction != other.direction;
    }
};

struct Options {
    Mode routingmode = Mode::kNone;
    Mode outputmode = Mode::kMap;
};

void printHelp(char *argv[]) {
    cout << "Usage: " << argv[0] << " --stack|-s | --queue|-q [--output M|L] | -h\n";
    cout << "Options:\n";
    cout << "  --stack, -s      Use the stack-based routing scheme.\n";
    cout << "  --queue, -q      Use the queue-based routing scheme.\n";
    cout << "  --output M|L     Specify output format: M for map, L for list.\n";
    cout << "  -h, --help       For help....\n";
    exit(0);
}

void getMode(int argc, char *argv[], Options &options) {
    struct option long_options[] = {
        {"stack", no_argument, nullptr, 's'},
        {"queue", no_argument, nullptr, 'q'},
        {"output", required_argument, nullptr, 'o'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, 0, nullptr, 0}
    };

    int choice;
    int routingCount = 0;
    while ((choice = getopt_long(argc, argv, "sqo:h", long_options, nullptr)) != -1) {
        switch (choice) {
            case 's':
                options.routingmode = Mode::kStack;
                routingCount++;
                break;
            case 'q':
                options.routingmode = Mode::kQueue;
                routingCount++;
                break;
            case 'o':
                if (string(optarg) == "M") {
                    options.outputmode = Mode::kMap;
                } else if (string(optarg) == "L") {
                    options.outputmode = Mode::kList;
                } else {
                    cerr << "Unknown command line option: " << optarg << endl;
                    exit(1);
                }
                break;
            case 'h':
                printHelp(argv);
                break;
            default:
                cerr << "Unknown command line option" << endl;
                exit(1);
        }
    }

    if (routingCount == 0) {
       cerr << "Stack or queue must be specified" << endl;
        exit(1);
    }

    if (routingCount > 1) {
        cerr << "Stack or queue can only be specified once" << endl;
        exit(1);
    }
    if(options.outputmode == Mode::kNone){
        options.outputmode = Mode::kMap;
    }
}

int readInput(vector<vector<string>>& levels) {
    char type;
    size_t numberoflevels;
    size_t levelsize;

    cin >> type >> numberoflevels >> levelsize;
    cin.ignore();

    levels.resize(numberoflevels, vector<string>(levelsize, string(levelsize, '.')));

    if (type == 'M') {
        string line;
        for (size_t i = 0; i < numberoflevels; ++i) {
            for (size_t j = 0; j < levelsize; ++j) {
                getline(cin, line);
                while (line.empty() || line[0] == '/') {
                    getline(cin, line);
                }
                for (char ch : line) {
                    if (ch != '.' && ch != '#' && ch != 'E' && ch != 'H' && ch != 'S') {
                        cerr << "Unknown map character: " << ch << endl;
                        return 1;
                    }
                }
                levels[i][j] = line;
            }
        }
    } else if (type == 'L') {
        string line;
        while (getline(cin, line)) {
            if (line.empty()) break;
            if (line[0] == '/') continue;

            size_t level, row, col;
            char ch;
            sscanf(line.c_str(), "(%zu,%zu,%zu,%c)", &level, &row, &col, &ch);

            if (level >= numberoflevels) {
                cerr << "Invalid level number: " << level << endl;
                return 1;
            }
            if (row >= levelsize || col >= levelsize) {
                if (row >= levelsize) cerr << "Invalid row number: " << row << endl;
                if (col >= levelsize) cerr << "Invalid column number: " << col << endl;
                return 1;
            }
            if (line[0] != '(' || line[line.size() - 1] != ')') {
                cerr << "Unknown map character" << endl;
                return 1;
            }
    
            levels[level][row][col] = ch;
        }
    } else {
        cerr << "Unknown map character: " << type << endl;
        return 1;
    }
    return 0;
}

bool isValid(const vector<vector<string>>& levels, size_t level, size_t row, size_t col) {
    if (level >= levels.size() || row >= levels[level].size() || col >= levels[level][row].size())
        return false;
    char c = levels[level][row][col];
    return c == '.' || c == 'E' || c == 'H';
}

vector<Position> getNextPositions(const vector<vector<string>>& levels, const Position& current, set<tuple<size_t, size_t, size_t>>& visited) {
    vector<Position> positions;
    vector<pair<int, int>> moves = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}}; 

    for (auto& move : moves) {
        size_t newRow = current.row + move.first;
        size_t newCol = current.col + move.second;
        if (isValid(levels, current.level, newRow, newCol)) {
            char direction = ' ';
            if (move.first == -1) direction = 'n';
            else if (move.first == 1) direction = 's';
            else if (move.second == -1) direction = 'w';
            else if (move.second == 1) direction = 'e';
            positions.emplace_back(current.level, newRow, newCol, direction);
        }
    }
    
    if (levels[current.level][current.row][current.col] == 'E') {  
        for (size_t newLevel = 0; newLevel < levels.size(); ++newLevel) {
            if (newLevel != current.level && levels[newLevel][current.row][current.col] == 'E') {
                auto elevatorPosKey = make_tuple(newLevel, current.row, current.col);
                if (visited.find(elevatorPosKey) == visited.end()) {
                    
                    char direction = (newLevel < current.level) ? 'n' : 's';  
                    positions.emplace_back(newLevel, current.row, current.col, direction);  
                }
            }
        }
    }

    return positions;
}


void printSolutionMap(const vector<Position>& path, const vector<vector<string>>& levels) {
    if (path.empty()) {
        cout << "Start in level " << path.front().level << ", row " << path.front().row << ", column " << path.front().col << endl;
        return;
    }

    cout << "Start in level " << path.front().level << ", row " << path.front().row << ", column " << path.front().col << endl;
    vector<vector<string>> updatedLevels = levels;

    for (size_t i = 0; i < path.size(); ++i) {
        const Position& pos = path[i];
        if (updatedLevels[pos.level][pos.row][pos.col] != 'H') {
            updatedLevels[pos.level][pos.row][pos.col] = pos.direction;
        }
    }

    for (size_t i = 0; i < updatedLevels.size(); ++i) {
        cout << "//level " << i << endl;
        for (const auto& row : updatedLevels[i]) {
            cout << row << endl;
        }
    }
}

void printSolutionList(const vector<Position>& path) {
    if (path.empty()) {
        cout << "//path taken" << endl;
        return;
    }

    cout << "//path taken" << endl;
    for (size_t i = 0; i < path.size() - 1; ++i) {
        const auto& pos = path[i];
        if (pos.direction != ' ') {
            cout << "(" << pos.level << "," << pos.row << "," << pos.col << "," << pos.direction << ")" << endl;
        }
    }
}

void pathfinding(const vector<vector<string>>& levels, const Position& start, Mode mode, Mode outputMode) {
    deque<Position> dq;
    dq.push_back(start);

    map<tuple<size_t, size_t, size_t>, Position> parentMap;
    parentMap[{start.level, start.row, start.col}] = start;

    set<tuple<size_t, size_t, size_t>> visited;
    visited.insert({start.level, start.row, start.col});

    while (!dq.empty()) {
        Position current;

        if (mode == Mode::kStack) {
            current = dq.back();
            dq.pop_back();
        } else if (mode == Mode::kQueue) {
            current = dq.front();
            dq.pop_front();
        }

        if (levels[current.level][current.row][current.col] == 'H') {  
            
            vector<Position> path;
            tuple<size_t, size_t, size_t> currentPos = {current.level, current.row, current.col};
            
          
            while (currentPos != make_tuple(start.level, start.row, start.col)) {
                path.push_back(current);
                current = parentMap[currentPos];
                currentPos = {current.level, current.row, current.col};
            }
            path.push_back(start); 
            reverse(path.begin(), path.end());

           
            for (size_t i = 0; i < path.size() - 1; ++i) {
                Position& current = path[i];
                const Position& next = path[i + 1];
                
                
                if (current.level == next.level) {
                    if (current.row == next.row) {
                        current.direction = (current.col < next.col) ? 'e' : 'w';  
                    } else {
                        current.direction = (current.row < next.row) ? 's' : 'n';  
                    }
                } else {
                    
                   current.direction = static_cast<char>('0' + static_cast<int>(next.level));

                }
            }

           
            if (!path.empty() && path.size() > 1) {
                path[0].direction = path[1].direction;
            }

           
            if (outputMode == Mode::kMap) {
                printSolutionMap(path, levels);
            } else if (outputMode == Mode::kList) {
                printSolutionList(path);
            }
            return;
        }

    
        vector<Position> nextPositions = getNextPositions(levels, current, visited);
        for (const Position& next : nextPositions) {
            auto nextPosKey = make_tuple(next.level, next.row, next.col);
            if (visited.find(nextPosKey) == visited.end()) {
                visited.insert(nextPosKey); 
                parentMap[nextPosKey] = current; 
                if (mode == Mode::kStack) {
                    dq.push_back(next);
                } else if (mode == Mode::kQueue) {
                    dq.push_back(next);
                }
            }
        }
    }

    
    if (outputMode == Mode::kMap) {
        cout << "Start in level " << start.level << ", row " << start.row << ", column " << start.col << endl;
        for (size_t i = 0; i < levels.size(); ++i) {
            cout << "//level " << i << endl;
            for (const auto& row : levels[i]) {
                cout << row << endl;
            }
        }
    } else {  
        cout << "//path taken" << endl; 
    }
}



int main(int argc, char *argv[]) {
    ios_base::sync_with_stdio(false);

    Options options;
    getMode(argc, argv, options);

    vector<vector<string>> levels;
    if (readInput(levels) != 0) {
        return 1;
    }

    Position start;
    bool startFound = false;
    for (size_t i = 0; i < levels.size(); ++i) {
        for (size_t j = 0; j < levels[i].size(); ++j) {
            for (size_t k = 0; k < levels[i][j].size(); ++k) {
                if (levels[i][j][k] == 'S') {
                    start = Position(i, j, k, ' ');
                    startFound = true;
                    break;
                }
            }
            if (startFound) break;
        }
        if (startFound) break;
    }

    pathfinding(levels, start, options.routingmode, options.outputmode);

    return 0;
}
