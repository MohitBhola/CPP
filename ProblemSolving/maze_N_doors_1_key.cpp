/*
Given an N * N binary maze where a 0 denotes that the position can be visited and a 1 denotes that the position cannot be visited without a key, the task is to find whether it is possible to visit the bottom-right cell from the top-left cell with only one key along the way. If possible then print “Yes” else print “No”.

Example:

Input: maze[][] = {
{0, 0, 1},
{1, 0, 1},
{1, 1, 0}}
Output: Yes

*/

#include <vector>
#include <iostream>
#include <cstring>

using namespace std;

#define N 3

bool fooCore(int maze[N][N], int row, int col, int rows, int cols, bool* mazeVisited, bool& isKeyAvailable)
{
    if (row == rows - 1 && col == cols - 1)
    {
        return true;
    }
    
    bool isVisited = mazeVisited[row * cols + col];
    if (row <= rows - 1 && col <= cols - 1 && !isVisited && (maze[row][col] == 0 || isKeyAvailable))
    {
        if (maze[row][col] == 1)
        {
            isKeyAvailable = false;
        }
         
        mazeVisited[row * cols + col] = true;    
        if (fooCore(maze, row + 1, col, rows, cols, mazeVisited, isKeyAvailable) 
                        || fooCore(maze, row, col + 1, rows, cols, mazeVisited, isKeyAvailable))
        {
            return true;
        }
        else
        {
            // backtrack
            if (maze[row][col] == 1)
            {
                isKeyAvailable = true;
            }
            
            mazeVisited[row * cols + col] = false;    
        }
    }
    
    return false;
}

bool foo(int maze[N][N])
{
    bool isKeyAvailable = true;
    bool* aMazeVisited = new bool[N*N];
    memset(aMazeVisited, false, N*N);
    
    return fooCore(maze, 0, 0, N, N, aMazeVisited, isKeyAvailable);
}

int main()
{
    int aMaze[N][N] = {{0,0,1}, {1,0,1}, {1,1,0}};
    cout << foo(aMaze);
    
    return 0;   
}

/*
1
*/
