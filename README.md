# Stegano-tool
Steganography tool that hide and extract messages from images(BMP) combined with encryption/decryption. Tool using Least Significant Bit(LSB) manipulations.
<br>

## Features
* **LSB Stenography -** Embeds binary data into the color channels of BMP images with no perceptible visual changes.
* **Symmetric encryption -** Built-in modular ASCII-shift encryption to secure message with a custom key integer before embedding.
* **Dual interference -** Supports both **Command-line interface(CLI)** and **Interactive Menu** for ease of use.
* **No heavy dependencies -** Written entirely in C++using binary file streams and vectors.


## Project Architecture

├── Stegano.cpp      *Main entry point, CLI parser, hider/extractor logic*

├── encrypt.cpp      *Class implementation for encryption/decryption*

├── encrypt.hpp      *Class declaration*

└── .gitignore       *Excludes binaries, test files and artefacts*

<br>

# Usage
## Prerequirements:
You need a C++ compiler that supports C++11 or >(e.g g++, clang++, etc)

## Compilation
Clone the repository and compile the source files together:
> g++ stegano.cpp encrypt.cpp -o stegano

## Usage Guide
You can run the tool either via interactive menu(by launching ./stegano without arguments) or via CLI flags:
### 1.Show help menu
> ./stegano --help
### 2. Hide a message with encryption
> ./stegano -e -m secret.txt -i input.bmp -o output.bmp
### 3. Hide a message withour encryption
> ./stegano -ne -m secret.txt -i input.bmp -o output.bmp
### 4. Extract and Decrypt a message
> ./stegano -d -i input.bmp
### 5. Extract a message without encryption
> ./stegano -nd -i input.bmp

## How it works
​1. Pixel Skip Header: Standard uncompressed BMP files feature a 54-byte file and info header. The tool safely skips these first 54 bytes to preserve image integrity.<br><br>
​2. Bit-Level Manipulation: Each character of the secret message (plus the [STOP] marker) is broken down into 8 bits. The algorithm modifies only the Least Significant Bit (LSB) of each color byte using bitwise masks (& 0xFE and | bit), meaning color values change by at most 1 unit (e.g., from 205 to 204), which is entirely invisible to the human eye.<br><br>
​3. Streamlined Extraction: The decoder reads pixel bytes sequentially, reconstructs bytes bit-by-bit, and halts instantly upon encountering the [STOP] signature.<br>
