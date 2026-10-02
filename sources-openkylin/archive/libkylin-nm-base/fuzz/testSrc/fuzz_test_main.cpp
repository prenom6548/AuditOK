#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

extern "C" int LLVMFuzzerTestOneInput_KyNMBase(const uint8_t *data, size_t size);
extern "C" int LLVMFuzzerTestOneInput_Enterprise(const uint8_t *data, size_t size);
extern "C" int LLVMFuzzerTestOneInput_KyKeyring(const uint8_t *data, size_t size);
extern "C" int LLVMFuzzerTestOneInput_KyNetworkManager(const uint8_t *data, size_t size);

int main(int argc, char *argv[]) {
    #if 1
    const char *testType = nullptr;
    const char *inputFile = nullptr;
    

    testType = argv[1];
    inputFile = argv[2];
    

    FILE *f = fopen(inputFile, "rb");

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t *buffer = (uint8_t*)malloc(fsize);


    size_t read_size = fread(buffer, 1, fsize, f);
    fclose(f);

    LLVMFuzzerTestOneInput_KyNetworkManager(buffer, read_size);
    LLVMFuzzerTestOneInput_Enterprise(buffer, read_size);


    free(buffer);
    #endif
    return 0;
}
