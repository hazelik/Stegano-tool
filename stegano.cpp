#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <sstream>
#include <limits>
#include "encrypt.hpp"

using namespace std;

void clearInputBuffer() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

string readTextFile(const string& filename) {
    ifstream f(filename, ios::binary);
    if (!f) return "";
    ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

void writeTextFile(const string& filename, const string& content) {
    ofstream f(filename, ios::binary);
    if (f) {
        f.write(content.data(), content.size());
    }
}

void hider(const string& message, const string& inputFile, const string& outputFile){
    ifstream file(inputFile, ios::binary);
    if(!file){
        cerr << "[ERROR] Cannot open input image: " << inputFile << endl;
        return;
    }
    string fullMessage = message + "[STOP]"; 
    vector<uint8_t> buffer((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();

    vector<uint8_t> secretBinary;
    for(uint8_t byteVal : fullMessage){
        for(int i = 7; i >= 0; --i){
            secretBinary.push_back((byteVal >> i) & 1);
        }
    }

    size_t availableBits = (buffer.size() - 54);  
    if(secretBinary.size() > availableBits){
        cerr << "[ERROR] Message size exceeds image capacity." << endl;
        return;
    }

    size_t index54 = 54;
    for(uint8_t bit : secretBinary){
        buffer[index54] = (buffer[index54] & 0xFE) | bit;
        index54++;
    }

    ofstream outFile(outputFile, ios::binary);
    if(!outFile){
        cerr << "[ERROR] Cannot write to output image: " << outputFile << endl;
        return;
    }
    outFile.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    outFile.close();
}

string extracter(const string& inputFile){       
    ifstream file(inputFile, ios::binary);
    if(!file){
        cerr << "[ERROR] Cannot open image for extraction: " << inputFile << endl;
        return "";
    }
    vector<uint8_t> buffer((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();

    string decodedMessage = "";
    uint8_t currentByte = 0;
    int bitCount = 0;
    string stopMarker = "[STOP]";
    
    for(size_t i = 54; i < buffer.size(); ++i){
        uint8_t bit = buffer[i] & 1; 
        currentByte |= (bit << (7 - bitCount)); 
        bitCount++;
        if(bitCount == 8){
            decodedMessage += static_cast<char>(currentByte);
            bitCount = 0;
            currentByte = 0;
            if(decodedMessage.size() >= stopMarker.size()){ 
                string tail = decodedMessage.substr(decodedMessage.size() - stopMarker.size());
                if(tail == stopMarker){
                    decodedMessage.erase(decodedMessage.size() - stopMarker.size()); 
                    break;
                }
            }
        }
    }
    return decodedMessage;
} 

void getHelp(){
    cout << "Usage: stegano [options]\n"
         << "Options:\n"
         << "  -h, --help          Show help message\n"
         << "  -e, --encrypt       Hide message with simple encryption\n"
         << "  -a, --aes           Hide message with AES encryption\n"
         << "  -d, --decrypt       Extract and decrypt simple message\n"
         << "  -da, --decrypt-aes  Extract message with AES decryption\n"
         << "  -m, --message FILE  Secret message text file\n"
         << "  -i, --input FILE    Input image file (BMP)\n"
         << "  -o, --output FILE   Output image file (BMP)\n";
}

int main(int argc, char* argv[]){
    if(argc > 1){
        string option = argv[1];

        if(option == "-h" || option == "--help"){
            getHelp();
            return 0;
        }

        string secretFile = "", inputFile = "", outputFile = "";
    for(int i = 2; i < argc; i++){
            string arg = argv[i];
            if((arg == "-m" || arg == "--message") && i + 1 < argc) {
                secretFile = argv[++i];
            }
            else if((arg == "-i" || arg == "--input") && i + 1 < argc) {
                inputFile = argv[++i];
            }
            else if((arg == "-o" || arg == "--output") && i + 1 < argc) {
                outputFile = argv[++i];
            }
        }

        if(option == "-e" || option == "--encrypt"){
            string rawText = readTextFile(secretFile);
            if(rawText.empty()) {
                cerr << "[ERROR] Secret file is empty or missing!\n";
                return 1;
            }

            endec encoder(secretFile, secretFile); 
            encoder.encr(); 
            string textToHide = encoder.encryptedText; 

            hider(textToHide, inputFile, outputFile);
            cout << "Message hidden and encrypted successfully." << endl;
        }
        else if(option == "-a" || option == "--aes"){
            string rawText = readTextFile(secretFile);
            if(rawText.empty()) {
                cerr << "[ERROR] Secret file is empty or missing!\n";
                return 1;
            }
            
            string pwd;
            cout << "Enter AES password: ";
            cin >> pwd;

            endec encoder(secretFile, "temp_cipher.txt"); 
            encoder.encryptAES(pwd); 
            string textToHide = encoder.encryptedText; 

            if(!textToHide.empty()) {
                hider(textToHide, inputFile, outputFile);
                cout << "Message hidden successfully with AES." << endl;
            }
        }
        else if(option == "-d" || option == "--decrypt"){
            string extractedCipher = extracter(inputFile);
            writeTextFile("temp_cipher.txt", extractedCipher);

            endec decoder("temp_cipher.txt", "decrypted_out.txt"); 
            decoder.decr(); 
        }
        else if(option == "-da" || option == "--decrypt-aes"){
            string pwd;
            cout << "Enter AES password: ";
            cin >> pwd;

            string extractedCipher = extracter(inputFile);
            writeTextFile("temp_cipher.txt", extractedCipher);

            endec decoder("temp_cipher.txt", "decrypted_out.txt"); 
            decoder.decryptAES(pwd); 
            cout << "Decrypted message: " << decoder.decryptedText << endl;
        }
        else {
            cerr << "Unknown option: " << option << endl;
            getHelp();
            return 1;
        }
        return 0;
    }

    string secretFile, inputFile, outputFile;
    int choice;
    
    cout << "Welcome to Steganography Tool\n";
    cout << "-------------------------------------\n";
    while(true){
        cout << "Choose an option:\n"
             << " 1. Hide a message with simple encryption\n"
             << " 2. Hide a message with AES encryption\n"
             << " 3. Extract and decrypt simple message\n"
             << " 4. Extract and decrypt AES message\n"
             << " 5. Exit\nEnter: ";
        
        if (!(cin >> choice)) {
            cout << "Invalid input. Exiting.\n";
            break;
        }
        clearInputBuffer();

        if(choice == 1){
            cout << "Enter the secret message text file: ";
            getline(cin, secretFile);
            string rawText = readTextFile(secretFile);
            if(rawText.empty()) {
                cout << "[ERROR] Secret file is empty or missing!\n\n";
                continue;
            }
            cout << "Enter the input BMP file name: ";
            getline(cin, inputFile);
            cout << "Enter the output BMP file name: ";
            getline(cin, outputFile);

            endec encoder(secretFile, secretFile);
    encoder.encr(); 
            string textToHide = encoder.encryptedText; 
  
            hider(textToHide, inputFile, outputFile);
            cout << "Message hidden successfully.\n-------------------------------------\n";
        }
        else if(choice == 2){
            cout << "Enter the secret message text file: ";
            getline(cin, secretFile);
            string rawText = readTextFile(secretFile);
            if(rawText.empty()) {
                cout << "[ERROR] Secret file is empty or missing!\n\n";
                continue;
            }
            cout << "Enter the input BMP file name: ";
            getline(cin, inputFile);
            cout << "Enter the output BMP file name: ";
            getline(cin, outputFile);

            string pwd;
            cout << "Enter AES password: ";
            getline(cin, pwd);
            
            endec encoder(secretFile, "temp_cipher.txt"); 
            encoder.encryptAES(pwd); 
            string textToHide = encoder.encryptedText; 

            if(!textToHide.empty()) {
                hider(textToHide, inputFile, outputFile);
                cout << "Message hidden successfully with AES.\n-------------------------------------\n";
            }
        }
        else if(choice == 3){
            cout << "Enter the BMP file name to extract: ";
            getline(cin, inputFile);
            
            string extractedCipher = extracter(inputFile);
            writeTextFile("temp_cipher.txt", extractedCipher);

            endec decoder("temp_cipher.txt", "decrypted_out.txt"); 
            decoder.decr(); 
            cout << "-------------------------------------\n";
        }
        else if(choice == 4){
            cout << "Enter the BMP file name to extract: ";
            getline(cin, inputFile);

            string pwd;
            cout << "Enter AES password: ";
            getline(cin, pwd);

            string extractedCipher = extracter(inputFile);
            writeTextFile("temp_cipher.txt", extractedCipher);

            endec decoder("temp_cipher.txt", "decrypted_out.txt"); 
            decoder.decryptAES(pwd); 
            cout << "Decrypted message:\n" << decoder.decryptedText << "\n-------------------------------------\n";
        }
        else if(choice == 5){
            cout << "Exiting. Goodbye!\n";
            break;
        }
        else{
            cout << "Invalid choice. Please try again.\n-------------------------------------\n";
        }
    }
    return 0;
}