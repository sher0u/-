#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_SIGNATURE_LENGTH 8
#define MAX_LENGTH 100  // Maximum length of each line in the file
#define SIGNATURE_SIZE 4


//comparing signature 
bool compareSignatures(const char *signature1, const char *signature2) {
    // Get the lengths of the signatures
    size_t len1 = strlen(signature1);
    size_t len2 = strlen(signature2);

    // If the lengths are different, signatures cannot match
    if (len1 != len2) {
        return false;
    }

    // Compare the signatures character by character
    for (size_t i = 0; i < len1; ++i) {
        if (signature1[i] != signature2[i]) {
            // Signatures do not match
            return false;
        }
    }

    // Signatures match
    return true;
}



// reading the signature from the exe
unsigned int read_file_at_offset(const char* file_path, long offset) {
    FILE* file = fopen(file_path, "rb");
    if (file == NULL) {
        printf("Unable to open file '%s'.\n", file_path);
        return 0;  // Return 0 if file cannot be opened
    }

    // Seek to the specified offset
    if (fseek(file, offset, SEEK_SET) != 0) {
        printf("Error seeking to offset.\n");
        fclose(file);
        return 0;  // Return 0 if seeking fails
    }

    // Read 4 bytes (32 bits) from the file
    unsigned char data[SIGNATURE_SIZE];
    size_t bytes_read = fread(data, 1, SIGNATURE_SIZE, file);
    if (bytes_read != SIGNATURE_SIZE) {
        printf("Error reading signature from file.\n");
        fclose(file);
        return 0;  // Return 0 if reading fails
    }

    // Convert the signature bytes to an unsigned integer value
    unsigned int signature = 0;
    for (int i = 0; i < SIGNATURE_SIZE; ++i) {
        signature = (signature << 8) | data[i];
    }

    // Clean up
    fclose(file);

    return signature;
}

//function to read the offset and the signature adn the name from the text file 
int read_signature_and_offset(const char *file_name, char *signature, char *offset) {
    FILE *file = fopen(file_name, "r");
    if (file == NULL) {
        printf("Error opening file.\n");
        return 1;
    }

    // Read the first line (signature)
    if (fgets(signature, MAX_LENGTH, file) == NULL) {
        printf("Error reading signature.\n");
        fclose(file);
        return 1;
    }
    // Remove newline character if present
    if (signature[strlen(signature) - 1] == '\n')
        signature[strlen(signature) - 1] = '\0';

    // Read the second line (offset)
    if (fgets(offset, MAX_LENGTH, file) == NULL) {
        printf("Error reading offset.\n");
        fclose(file);
        return 1;
    }
    // Remove newline character if present
    if (offset[strlen(offset) - 1] == '\n')
        offset[strlen(offset) - 1] = '\0';

    

    fclose(file);
    return 0;
}

// Function to search for a signature in a file
void searchSignature(const char *filename, const char *signature, int signatureLength) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    // Get file size for better efficiency
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Read the entire file into a buffer
    unsigned char *buffer = (unsigned char *)malloc(fileSize);
    if (buffer == NULL) {
        fclose(file);
        perror("Memory allocation error");
        return;
    }

    fread(buffer, 1, fileSize, file);
    fclose(file);

    // Check for MZ header in the first two bytes of the file
    if (buffer[0] == 0x4D && buffer[1] == 0x5A) {
        printf("The file has an MZ header, indicating a PE format executable.\n");
    } else {
        printf("The file does not have an MZ header, possibly not a Windows executable.\n");
    }

    // Allocate memory for hexadecimal representation (twice the file size + 1 for null terminator)
    char *hexBuffer = (char *)malloc(2 * fileSize + 1);
    if (hexBuffer == NULL) {
        free(buffer);
        perror("Memory allocation error");
        return;
    }

    // Convert binary buffer to hexadecimal representation
    for (int i = 0; i < fileSize; i++) {
        sprintf(hexBuffer + 2 * i, "%02X", buffer[i]);
    }
    hexBuffer[2 * fileSize] = '\0';

    // Convert input signature to uppercase
    char *signatureCopy = strdup(signature);
    if (signatureCopy == NULL) {
        perror("Memory allocation error");
        free(buffer);
        free(hexBuffer);
        return;
    }

    for (int i = 0; i < signatureLength; i++) {
        signatureCopy[i] = toupper(signatureCopy[i]);
    }

    // Search for the signature in the hexadecimal buffer
    char *pos = strstr(hexBuffer, signatureCopy);
    if (pos != NULL) {
        printf("Signature found in file: %s\n", filename);
    } else {
        printf("Signature not found in file: %s\n", filename);
    }

    // Free allocated memory
    free(buffer);
    free(hexBuffer);
    free(signatureCopy);
}

int main() {
    FILE *file;
    char signature[MAX_SIGNATURE_LENGTH + 1]; // +1 for null terminator
    size_t bytes_read;
    char filepath[100]; // Assuming the maximum length of the file path is 100 characters
    char filepathToScan[100];
    char offset[MAX_LENGTH];
    char offsetFake[MAX_LENGTH];

    // Prompt the user to input the file path
    printf("Please enter the path of the file containing the hexadecimal signature: ");
    fgets(filepath, sizeof(filepath), stdin);
    filepath[strcspn(filepath, "\n")] = 0; // Remove trailing newline

    printf("Please enter the path of the Program: ");
    fgets(filepathToScan, sizeof(filepathToScan), stdin);
    filepathToScan[strcspn(filepathToScan, "\n")] = 0; // Remove trailing newline

    // Open the file in binary mode
    file = fopen(filepath, "rb");
    if (file == NULL) {
        printf("Error: Unable to open file at path '%s'\n", filepath);
        return 1;
    }

    // Read the signature and offset  from the file
    read_signature_and_offset(filepath,signature,offset);


    // Print the signature as it is
    printf("\nSignature read from file: %s\n", signature);
    // Print the signature as it is
    printf("Signature read from file: %s\n", offset);
    //printf the offset from the exe file

    // Assign offset from the variable
    long offsetValue = strtol(offset, NULL, 16);


    // Read data from the file at the specified offset
    unsigned int Signature;
    Signature =read_file_at_offset(filepathToScan, offsetValue);
    printf("Signature read from file: %X\n", Signature);

    searchSignature(filepathToScan, signature, strlen(signature));

    // Convert the unsigned int signature to a string representation
    char hexSignature[MAX_SIGNATURE_LENGTH + 1]; // +1 for null terminator
    sprintf(hexSignature, "%X", Signature);

// Now you can call the compareSignatures function with the string representations
    if(compareSignatures(hexSignature, signature)) {
        printf("Signatures match!\n");
    } else {
        printf("Signatures do not match!\n");
    }

    return 0;
}
