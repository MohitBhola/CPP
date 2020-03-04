#include <iostream>
#include <string>
#include <sstream>

using namespace std;

string decimalToBinaryStr(double num)
{
    ostringstream oss{};
    oss << ".";
    
    while (num)
    {
        if (oss.str().size() >= 32)
        {
            return "ERROR";
        }
        
        double newNum = num * 2;
        if (newNum >= 1)
        {
            oss << "1";
            num = newNum - 1;
        }
        else
        {
            oss << "0";
            num = newNum;
        }
    }
    
    return oss.str();
}

int main()
{
    double d = .125;
    
    cout << decimalToBinaryStr(d) << '\n';
    
    return 0;
}
