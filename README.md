# APM - Advanced Password Manager

## Description
Note: This is a bachelor thesis project.

**APM** is a secure and portable password manager for Windows, built using the Win32 API.  
It uses custom implementations of cryptographic algorithms (SHA-256, HMAC-SHA256, PBKDF2) to protect sensitive data.  
It provides password generation with calculated entropy, including both random and Diceware methods.

## Key Features

- Hashing and HMAC authentication using SHA-256.  
- PBKDF2 key derivation for enhanced security.  
- Random password generation with a customizable character set.  
- Diceware password generation for easy memorability.  
- Lightweight and efficient graphical interface without heavy dependencies.  
- Local encrypted database with salt.  
- Unit tests for critical components.

## Project Structure
```bash
.
├── build # Build output files
├── src # Source code
│   ├── crypto # Cryptographic algorithms (SHA256, HMAC, PBKDF2)
│   ├── generator # Password generators and entropy calculations
│   ├── ... # Other modules (GUI, DB, utils, etc.)
├── tests # Unit tests
├── tools # Resources and helper scripts
└── Makefile # Build script
```


## Dependencies and Installation

- GCC (preferably via MSYS2 on Windows)  
- sqlcipher  
- OpenSSL (for crypto libraries)  
- Windows libraries: bcrypt, comctl32, ws2_32

### Installing on MSYS2

```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-sqlcipher mingw-w64-x86_64-openssl
```

## Build

Compile the project by running:

    make

The output executable will be located at:
`build/apm.exe`

## Testing

Unit tests are available for cryptographic components:

- SHA-256
  Run:
      make run-sha256-test

- HMAC-SHA256
  Run:
      make run-hmac-test

- PBKDF2
  Run:
      make run-pbkdf2-test

## Usage

Run the password manager executable with:

    ./build/apm.exe
