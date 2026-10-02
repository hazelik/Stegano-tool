#include "encrypt.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

//simple encryption
void endec::encr(){
    cout << "Enter key (integer 1-255): ";
    cin >> key;
    if (key > 255 || key < 0) {
        cout << "Invalid key! Setting default key = 3\n";
        key = 3;
    }
    
    // read the secret file
    ifstream fin(fileIn, ios::binary);
    if (!fin) {
        cerr << "Error opening secret file for encryption: " << fileIn << endl;
        encryptedText = "";
        return;
    }
    
    ostringstream ss;
    ss << fin.rdbuf();
    string plainText = ss.str();
    fin.close();

    // Encrypt the text
    encryptedText = "";
    for(char c : plainText){
        char temp = c + (key % 256); 
        encryptedText += temp;
    }
}

void endec::decr(){
    cout << "Enter key: ";
    cin >> key;
    
    encryptedText = "";
    ifstream fin(fileIn, ios::binary); // fileIn here is the file containing the encrypted text
    if (!fin) {
        // If the file doesn't exist, can try to read from the output file instead
        fin.open(fileOut, ios::binary);
    }
    
    // Read the encrypted text from the file
    ostringstream ss;
    ss << fin.rdbuf();
    string cipherText = ss.str();
    fin.close();

    decryptedText = "";
    for(char c : cipherText){
        char temp = c - (key % 256); 
        decryptedText += temp;
    }

    cout << "Decrypted message:\n" << decryptedText << endl;
}
