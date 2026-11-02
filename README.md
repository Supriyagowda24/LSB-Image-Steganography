# 🔐 Image Steganography using LSB Encoding and Decoding

### C Programming | Image Processing | Data Hiding

## 📌 Project Overview

Image Steganography using LSB (Least Significant Bit) is a C programming project that demonstrates how secret information can be embedded into a BMP image and retrieved through decoding.

The project uses the LSB technique to hide data within image bytes while maintaining the visual appearance of the original image.

## 🚀 Key Features

- **Encoding:** Embed secret data into a BMP image using LSB encoding.
- **Decoding:** Extract hidden information from an encoded image.
- **BMP File Handling:** Process image data using C file operations.
- **Bitwise Data Manipulation:** Work with binary data at the bit level.
- **Modular Programming:** Organize encoding and decoding functionality.

## 🛠️ Technologies & Concepts

- C Programming
- File Handling
- Pointers
- Bitwise Operations
- Structures
- Modular Programming
- BMP Image Processing

## ⚙️ Working Principle

The Least Significant Bit technique stores secret data by modifying selected least significant bits of image data. These small changes generally do not cause noticeable visual differences in the image.

### Encoding Process

1. Read the source BMP image.
2. Read the secret data.
3. Embed the data into the image using LSB encoding.
4. Generate the encoded image.

### Decoding Process

1. Read the encoded image.
2. Extract the hidden data.
3. Reconstruct the secret information.

## 🎯 Learning Outcomes

- Understood the working principle of LSB-based steganography.
- Practiced file handling and binary data manipulation in C.
- Applied bitwise operations and pointers.
- Improved modular programming and problem-solving skills.
