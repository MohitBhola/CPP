#include <iostream>
#include <vector>
using namespace std; 

constexpr int MAX = 100; 

// Returns maximum amount of gold that can be collected 
// when journey started from first column and moves 
// allowed are right, right-up and right-down 
int getMaxGold(vector<vector<int>>& goldVec) 
{ 
    int rows = static_cast<int>(goldVec.size());
    int cols = static_cast<int>(goldVec[0].size());
    
	vector<vector<int>> dp(rows, vector<int>(cols, 0));
	
	for (int row = 0; row < rows; ++row) 
	{
	    dp[row][0] = goldVec[row][0];
	}

	for (int col = 1; col < cols; ++col) 
	{
	    for (int row = 0; row < rows; ++row)
	    {
	        // Gold collected on going to the cell on the right(->) 
    		int right = dp[row][col-1]; 
    
    		// Gold collected on going to the cell to right up (/) 
    		int right_up = (row == 0) ? 0 : dp[row-1][col-1]; 
    
    		// Gold collected on going to the cell to right down (\) 
    		int right_down = (row == rows-1) ? 0 : dp[row+1][col-1]; 
    
    		// Max gold collected from taking either of the 
    		// above 3 paths 
    		dp[row][col] = goldVec[row][col] + 
    						max(right, max(right_up, right_down));
	        
	    }
	} 

	// The max amount of gold collected will be the max 
	// value in last column of all rows 
	int res{dp[0][cols-1]};
	
	for (int i=1; i<rows; i++) 
	{
		res = max(res, dp[i][cols-1]); 
	}
	
	return res; 
} 

// Driver Code 
int main() 
{ 
    vector<vector<int>> goldVec { {1, 3, 1, 5}, 
		                          {2, 2, 4, 1}, 
		                          {5, 0, 2, 3}, 
		                          {0, 6, 1, 2} 
	                            }; 
	                            
	cout << getMaxGold(goldVec); 
	
	return 0; 
} 

/*
16
*/
