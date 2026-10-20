#include "encrypt.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/aes.h>
#include <cryptopp/gcm.h>
#include <cryptopp/sha.h>
#include <cryptopp/filters.h>

using namespace std;

void deriveKeyAndIV(const string& password, CryptoPP::byte* key, size_t keyLen, CryptoPP::byte* iv, size_t ivLen) {
    CryptoPP::SHA256 sha;
    CryptoPP::byte digest[CryptoPP::SHA256::DIGESTSIZE];
    
    sha.CalculateDigest(digest, (const CryptoPP::byte*)password.data(), password.size());
    
    size_t bytesToKey = (keyLen < CryptoPP::SHA256::DIGESTSIZE) ? keyLen : CryptoPP::SHA256::DIGESTSIZE;
    std::memcpy(key, digest, bytesToKey);
    
    size_t bytesToIV = (ivLen < (CryptoPP::SHA256::DIGESTSIZE - bytesToKey)) ? ivLen : (CryptoPP::SHA256::DIGESTSIZE - bytesToKey);
    std::memcpy(iv, digest + bytesToKey, bytesToIV);

    std::memset(digest, 0, CryptoPP::SHA256::DIGESTSIZE);
}

void endec::encr(){
    cout << "Enter key (integer 1-255): ";
    cin >> key;
    if (key > 255 || key < 0) {
        key = 3;
    }
    
    ifstream fin(fileIn, ios::binary);
    if (!fin) {
        cerr << "[ERROR] Cannot open file: " << fileIn << endl;
        encryptedText = "";
        return;
    }
    
    ostringstream ss;
    ss << fin.rdbuf();
    string plainText = ss.str();
    fin.close();

    encryptedText = "";
    for(char c : plainText){
        char temp = c + (key % 256); 
        encryptedText += temp;
    }
}

void endec::decr(){
    cout << "Enter key: ";
    cin >> key;
    
    ifstream fin(fileIn, ios::binary);
    if (!fin) {
        fin.open(fileOut, ios::binary);
    }
    if (!fin) {
        cerr << "[ERROR] Cannot open encrypted file.\n";
        return;
    }
    
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

void endec::encryptAES(const string& pwd){
    this->password = pwd;

    ifstream fin(fileIn, ios::binary);
    if (!fin) {
        cerr << "[ERROR] Cannot open secret file: " << fileIn << endl;
        encryptedText = "";
        return;
    }
    ostringstream ss;
    ss << fin.rdbuf();
    string plainText = ss.str();
    fin.close();

    CryptoPP::byte key[CryptoPP::AES::DEFAULT_KEYLENGTH];
    CryptoPP::byte iv[12];
    deriveKeyAndIV(password, key, sizeof(key), iv, sizeof(iv));

    string cipherText;
    try {
        CryptoPP::GCM<CryptoPP::AES>::Encryption enc;
        enc.SetKeyWithIV(key, sizeof(key), iv, sizeof(iv));

        CryptoPP::StringSource(plainText, true,
            new CryptoPP::AuthenticatedEncryptionFilter(
                enc,
                new CryptoPP::StringSink(cipherText)
            )
        );
        encryptedText = cipherText;
    }
    catch (const CryptoPP::Exception& e) {
        cerr << "[CRYPTOPP ERROR] " << e.what() << endl;
        encryptedText = "";
        return;
    }

    ofstream fout(fileOut, ios::binary);
    if (fout) {
        fout.write(encryptedText.data(), encryptedText.size());
        fout.close();
    }

    std::memset(key, 0, sizeof(key));
    std::memset(iv, 0, sizeof(iv));
}

void endec::decryptAES(const string& pwd){
    this->password = pwd;

    ifstream fin(fileIn, ios::binary);
    if (!fin) {
        fin.open(fileOut, ios::binary);
    }
    if (!fin) {
        cerr << "[ERROR] Cannot open file for decryption: " << fileIn << endl;
        decryptedText = "";
        return;
    }
    ostringstream ss;
    ss << fin.rdbuf();
    string cipherText = ss.str();
    fin.close();

    CryptoPP::byte key[CryptoPP::AES::DEFAULT_KEYLENGTH];
    CryptoPP::byte iv[12];
    deriveKeyAndIV(password, key, sizeof(key), iv, sizeof(iv));

    string recoveredText;
    try {
        CryptoPP::GCM<CryptoPP::AES>::Decryption dec;
        dec.SetKeyWithIV(key, sizeof(key), iv, sizeof(iv));
        CryptoPP::StringSource(cipherText, true,
            new CryptoPP::AuthenticatedDecryptionFilter(
                dec,
                new CryptoPP::StringSink(recoveredText)
            )
        );
        decryptedText = recoveredText;
    }
    catch (const CryptoPP::Exception& e) {
        cerr << "[DECRYPTION ERROR] Invalid password or corrupted data.\n";
        decryptedText = "";
        return;
    }

    ofstream fout(fileOut, ios::binary);
    if (fout) {
        fout.write(decryptedText.data(), decryptedText.size());
        fout.close();
    }

    std::memset(key, 0, sizeof(key));
    std::memset(iv, 0, sizeof(iv));
}