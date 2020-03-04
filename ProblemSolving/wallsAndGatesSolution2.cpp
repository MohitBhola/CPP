#include <vector>
#include <iostream>

using namespace std;

void wallsAndGatesCore(vector<vector<int>>& grid, vector<vector<int>>& results, int distance, int row, int col, int rows, int cols, vector<vector<int>>& visited) 
    {
        if (row < 0 || row >= rows || col < 0 || col >= cols || visited[row][col])
        {
            return;
        }

        if (grid[row][col] == -1)
        {
            return;
        }
    
        // are we inside a room right now?
        if (grid[row][col] == 2147483647)
        {
            if (distance < results[row][col])
            {
                results[row][col] = distance;
            }
            else
            {
                return;
            }
        }
        
        visited[row][col] = true;
            
        wallsAndGatesCore(grid, results, distance + 1, row, col - 1, rows, cols, visited);
        wallsAndGatesCore(grid, results, distance + 1, row, col + 1, rows, cols, visited);
        wallsAndGatesCore(grid, results, distance + 1, row - 1, col, rows, cols, visited);
        wallsAndGatesCore(grid, results, distance + 1, row + 1, col, rows, cols, visited);
    
        visited[row][col] = false;
    }

    void wallsAndGates(vector<vector<int>>& rooms) 
    {
        if (rooms.empty())
            return;

        int rows = static_cast<int>(rooms.size());
        int cols = static_cast<int>(rooms[0].size());

        vector<vector<int>> visited(rows, vector<int>(cols, 0));
        vector<vector<int>> results = rooms;

        for (int i = 0; i < rows; ++i)
        {
            for (int j = 0; j < cols; ++j)
            {
                if (rooms[i][j] == 0)
                {
                    wallsAndGatesCore(rooms, results, 0, i, j, rows, cols, visited);
                }
            }
        }

        rooms = results;
    }

int main()
{
    vector<vector<int>> ivec{{0,0}, {0,0}};
    
    wallsAndGates(ivec);
    
    int rows = static_cast<int>(ivec.size());
    int cols = static_cast<int>(ivec[0].size());
    
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            cout << ivec[i][j] << '\t';
        }
        
        cout << '\n';
    }
    
    return 0;
}

/*
0 0
0 0
*/
