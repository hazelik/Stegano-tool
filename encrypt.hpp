#ifndef ENCRYPT_HPP
#define ENCRYPT_HPP
#include <bits/stdc++.h>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;
class endec{
    int key;
    string fileIn;
    string fileOut;
public:
    string encryptedText;
    string decryptedText;

    endec(string inFile, string outFile): fileIn(inFile), fileOut(outFile), key(0)  {}
    void encr();
    void decr();
};
#endif // ENCRYPT_HPP