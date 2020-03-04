#include <vector>
#include <iostream>

using namespace std;

void wallsAndGatesCore(vector<vector<int>>& rooms, int& distance, int row, int col, int rows, int cols, vector<vector<int>>& visited) 
{
    if (row < 0 || row > rows || col < 0 || col > cols || visited[row][col])
    {
        distance = 2147483647;
        return;
    }
    
    if (rooms[row][col] == -1)
    {
        distance = 2147483647;
        return;
    }
        
    // reached a gate?    
    if (rooms[row][col] == 0)
    {
        return;
    }
    
    visited[row][col] = true;
    
    int lDistance{0};
    wallsAndGatesCore(rooms, lDistance, row, col - 1, rows, cols, visited);
    
    int rDistance{0};
    wallsAndGatesCore(rooms, rDistance, row, col + 1, rows, cols, visited);
    
    int uDistance{0};
    wallsAndGatesCore(rooms, uDistance, row - 1, col, rows, cols, visited);
    
    int dDistance{0};
    wallsAndGatesCore(rooms, dDistance, row + 1, col, rows, cols, visited);
    
    int minDistance = std::min(lDistance, std::min(rDistance, std::min(uDistance, dDistance)));
    
    if (minDistance < 2147483647)
    {
        distance = 1 + minDistance;
    }
    else
    {
        distance = 2147483647;
    }
    
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
            if (rooms[i][j] == 2147483647)
            {
                wallsAndGatesCore(rooms, results[i][j], i, j, rows, cols, visited);
            }
        }
    }
    
    rooms = results;
}

int main()
{
    vector<vector<int>> ivec{{2147483647, -1, 0, 2147483647}, {2147483647, 2147483647, 2147483647, -1}, {2147483647, -1, 2147483647, -1}, {0, -1, 2147483647, 2147483647}};
    
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
3	-1	0	1	
2	2	1	-1	
1	-1	2	-1	
0	-1	3	4
*/
