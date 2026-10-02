#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <sstream>
#include "encrypt.hpp"

using namespace std;

// read text from a text file
string readTextFile(const string& filename) {
    ifstream f(filename);
    if (!f) return "";
    ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

// write text to a text file
void writeTextFile(const string& filename, const string& content) {
    ofstream f(filename);
    f << content;
}

// LSB Hider for BMP
void hider(const string& message, const string& inputFile, const string& outputFile){
    ifstream file(inputFile, ios::binary);
    if(!file){
        cerr << "Error opening input file: " << inputFile << endl;
        return;
    }
    string fullMessage = message + "[STOP]"; // stop marker 
    vector<uint8_t> buffer((istreambuf_iterator<char>(file)), istreambuf_iterator<char>()); // file into buffer
    file.close();

    vector<uint8_t> secretBinary;
    for(char c : fullMessage){
        for(int i = 7; i >= 0; --i){
            secretBinary.push_back((c >> i) & 1);
        }
    }

    size_t availableBits = (buffer.size() - 54);  
    if(secretBinary.size() > availableBits){
        cerr << "Message is too large for the image." << endl;
        return;
    }

    size_t index54 = 54;
    for(uint8_t bit : secretBinary){
        buffer[index54] = (buffer[index54] & 0xFE) | bit;
        index54++;
    }

    ofstream outFile(outputFile, ios::binary);
    if(!outFile){
        cerr << "Error opening output file: " << outputFile << endl;
        return;
    }
    outFile.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    outFile.close();
}

string extracter(const string& inputFile){       
    ifstream file(inputFile, ios::binary);
    if(!file){
        cerr << "Error opening input file: " << inputFile << endl;
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
            decodedMessage += currentByte;
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
         << "  -e, --encrypt       Hide message with encryption\n"
         << "  -ne, --no-encrypt   Hide message without encryption\n"
         << "  -d, --decrypt       Extract and decrypt message\n"
         << "  -nd, --no-decrypt   Extract message without decryption\n"
         << "  -m, --message FILE  Secret message text file\n"
         << "  -i, --input FILE    Input image file (BMP)\n"
         << "  -o, --output FILE   Output image file (BMP)\n"
         << "  -x, --exit          Exit the program\n"
         << "Examples:\n"
         << "  ./stegano -e -m secret.txt -i input.bmp -o output.bmp\n"
         << "  ./stegano -d -i output.bmp\n";
}

int main(int argc, char* argv[]){
    // 1. If command line arguments are provided
    if(argc > 1){
        string option = argv[1];

        if(option == "-h" || option == "--help"){
            getHelp();
            return 0;
        }

        string secretFile = "", inputFile = "", outputFile = "";
// first, parse the command line arguments
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

        // now, when we have all the necessary files, we can proceed with the chosen operation
        if(option == "-e" || option == "--encrypt"){
            cout << "Running encryption and hiding mode...\n";
            string rawText = readTextFile(secretFile);
            if(rawText.empty()) {
                cerr << "Error: Secret file is empty or missing!\n";
                return 1;
            }

            endec encoder(secretFile, secretFile); 
            encoder.encr(); 
            string textToHide = encoder.encryptedText; 

            hider(textToHide, inputFile, outputFile);
            cout << "Message hidden and encrypted successfully." << endl;
        }
        else if(option == "-ne" || option == "--no-encrypt"){
            cout << "Running hiding mode without encryption...\n";
            string rawText = readTextFile(secretFile);
            if(rawText.empty()) {
                cerr << "Error: Secret file is empty or missing!\n";
                return 1;
            }
            hider(rawText, inputFile, outputFile);
            cout << "Message hidden successfully." << endl;
        }
        else if(option == "-d" || option == "--decrypt"){
            cout << "Running extraction and decryption mode...\n";
            // try change to inputFile(it worked)
            string extractedCipher = extracter(inputFile);
            writeTextFile("temp_cipher.txt", extractedCipher);

            endec decoder("temp_cipher.txt", "decrypted_out.txt"); 
            decoder.decr(); 
        }
        else if(option == "-nd" || option == "--no-decrypt"){
            cout << "Running extraction mode without decryption...\n";
            string decodedMessage = extracter(outputFile);
            cout << "Decoded message:\n" << decodedMessage << endl;
        }
        else {
            cerr << "Unknown option: " << option << endl;
            getHelp();
            return 1;
        }
        return 0;
    }

    // 2. if no command line arguments, run interactive mode
    string secretFile, inputFile, outputFile;
    int choice;
    
    cout << "Welcome to Stenography\n";
    cout << "-------------------------------------\n";
    while(true){
        cout << "Choose an option:\n"
             << " 1. Hide a message with encryption\n"
             << " 2. Hide a message without encryption\n"
             << " 3. Extract and decrypt a message\n"
             << " 4. Extract a message without encryption\n"
             << " 5. Exit\nEnter: ";
        
        if (!(cin >> choice)) {
            cout << "Invalid input. Exiting.\n";
            break;
        }
        cin.ignore(); 

        if(choice == 1){
            cout << "Enter the secret message text file (e.g., secret.txt): ";
            getline(cin, secretFile);
            string rawText = readTextFile(secretFile);
            if(rawText.empty()) {
                cout << "Secret file is empty or missing!\n\n";
                continue;
            }
            cout << "Enter the input BMP file name: ";
            getline(cin, inputFile);
            cout << "Enter the output BMP file name: ";
            getline(cin, outputFile);

            /*string rawText = readTextFile(secretFile);
            if(rawText.empty()) {
                cout << "Secret file is empty or missing!\n\n";
                continue; ---- checks added after the enter txt
            }*/

            endec encoder(secretFile, secretFile); 
            encoder.encr(); 
            string textToHide = encoder.encryptedText; 
  
            hider(textToHide, inputFile, outputFile);
            cout << "Message hidden and encrypted successfully.\n-------------------------------------\n";
        }
        else if(choice == 2){
            cout << "Enter the secret message text file: ";
            getline(cin, secretFile);
            string rawText = readTextFile(secretFile);
            if(rawText.empty()) {
                cout << "Secret file is empty or missing!\n\n";
                continue;
            }
            cout << "Enter the input BMP file name: ";
            getline(cin, inputFile);
            cout << "Enter the output BMP file name: ";
            getline(cin, outputFile);

            
            hider(rawText, inputFile, outputFile);
            cout << "Message hidden successfully.\n-------------------------------------\n";
        }
        else if(choice == 3){
            cout << "Enter the BMP file name to extract: ";
            getline(cin, outputFile);
            
            string extractedCipher = extracter(outputFile);
            writeTextFile("temp_cipher.txt", extractedCipher);

            endec decoder("temp_cipher.txt", "decrypted_out.txt"); 
            decoder.decr(); 
            cout << "-------------------------------------\n";
        }
        else if(choice == 4){
            cout << "Enter the BMP file name to extract: ";
            getline(cin, outputFile);
            string decodedMessage = extracter(outputFile);
            cout << "Decoded message:\n" << decodedMessage << "\n-------------------------------------\n";
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