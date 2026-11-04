# Image Steganography using LSB Encoding and Decoding

## Project Overview

Designed and implemented an Image Steganography application in C that securely embeds secret text data within BMP image files using the Least Significant Bit (LSB) technique. The application supports both encoding and decoding operations, enabling hidden data to be embedded into an image and later extracted without significantly altering its visual appearance.

This project demonstrates practical implementation of file handling, bitwise operations, pointers, and binary data processing in C.

## Features

- Encode secret text files into BMP images.
- Decode and extract hidden information from encoded images.
- Validate encoded data using a magic string.
- Preserve secret file size and extension during encoding and decoding.

## Technologies Used

- C Programming
- Linux
- GCC Compiler

## Compilation and Execution

### Compile

```bash
gcc *.c
```

### Encoding

```bash
./a.out -e beautiful.bmp secret.txt steged_img.bmp
```

### Decoding

```bash
./a.out -d steged_img.bmp
```

## Concepts Implemented

- Least Significant Bit (LSB) Encoding and Decoding
- File Handling
- Pointers and Structures
- Bitwise Operations
- Binary Data Manipulation
