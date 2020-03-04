#include <vector>
#include <iostream>
#include <deque>

using namespace std;

void wallsAndGatesCore(vector<vector<int>>& grid, vector<vector<int>>& results, deque<pair<int, int>>& deq, int rows, int cols, vector<vector<int>>& visited) 
{
    while (!deq.empty())
    {
        pair<int, int> aPair = deq.back(); deq.pop_back();
        
        int row = aPair.first;
        int col = aPair.second;
        
        {
            int r = row - 1;
            int c = col;
            
            if (r >= 0 && r < rows && c >= 0 && c < cols && grid[r][c] == 2147483647 && !visited[r][c])
            {
                results[r][c] = results[row][col] + 1;
                deq.push_front({r, c});
                visited[r][c] = 1;
            }
        }
        
        {
            int r = row + 1;
            int c = col;
            
            if (r >= 0 && r < rows && c >= 0 && c < cols && grid[r][c] == 2147483647 && !visited[r][c])
            {
                results[r][c] = results[row][col] + 1;
                deq.push_front({r, c});
                visited[r][c] = 1;
            }
        }
        
        {
            int r = row;
            int c = col + 1;
            
            if (r >= 0 && r < rows && c >= 0 && c < cols && grid[r][c] == 2147483647 && !visited[r][c])
            {
                results[r][c] = results[row][col] + 1;
                deq.push_front({r, c});
                visited[r][c] = 1;
            }
        }
        
        {
            int r = row;
            int c = col - 1;
            
            if (r >= 0 && r < rows && c >= 0 && c < cols && grid[r][c] == 2147483647 && !visited[r][c])
            {
                results[r][c] = results[row][col] + 1;
                deq.push_front({r, c});
                visited[r][c] = 1;
            }
        } 
    }
}

void wallsAndGates(vector<vector<int>>& rooms) 
{
    if (rooms.empty())
        return;

    int rows = static_cast<int>(rooms.size());
    int cols = static_cast<int>(rooms[0].size());

    vector<vector<int>> results = rooms;
    deque<pair<int, int>> deq{};
    vector<vector<int>> visited(rows, vector<int>(cols, 0));

    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            if (rooms[i][j] == 0)
            {
                deq.push_back({i,j});
            }
        }
    }
    
    wallsAndGatesCore(rooms, results, deq, rows, cols, visited);

    rooms = results;
}

int main()
{
    vector<vector<int>> ivec{{2147483647, -1, 0, 2147483647}, {2147483647, 2147483647, 2147483647, -1}, {2147483647, -1, 2147483647, -1}, {0, -1, 2147483647, 2147483647}};
    //vector<vector<int>> ivec{{0,0}, {0,0}};

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
