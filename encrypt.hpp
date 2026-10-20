#ifndef ENCRYPT_HPP
#define ENCRYPT_HPP
#include <bits/stdc++.h>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;
class endec{
    string fileIn;
    string fileOut;
    string password;
    int key;

public:
    string encryptedText;
    string decryptedText;

    endec(string in, string out) : fileIn(in), fileOut(out), key(3) {}

    void encr();
    void decr();

    void encryptAES(const string& pwd);
    void decryptAES(const string& pwd);
};
#endif // ENCRYPT_HPP


