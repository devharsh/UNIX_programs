#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <openssl/evp.h>
#include <openssl/rand.h>

void usage();
int genKeyIV(int en, const char *passphrase, unsigned char *salt, unsigned char *key, unsigned char *iv);
int aed_encrypt(const char *passphrase);
int aed_decrypt(const char *passphrase);

int main(int argc, char *argv[])
{
    int flag_d = -1;
    int flag_e = -1;
    const char *passin = NULL;
    FILE *f;
    char * line = NULL;
    size_t len = 0;
    ssize_t n;
    int opt;
    while ((opt = getopt(argc, argv, "dehp:")) != -1) {
        switch(opt) {
            case 'd': flag_d = 1; flag_e = 0; break;
            case 'e': flag_e = 1; flag_d = 0; break;
            case 'h': usage(); exit(EXIT_SUCCESS); break;
            case 'p': passin = optarg; break;
            case '?':
            default:
              usage();
              exit(EXIT_FAILURE);
        }
    }
    argc -= optind;
    argv += optind;
    if (argc != 0)
        fprintf(stderr, "There are more arguments than needed, they will be ignored.\n");
    if (passin == NULL)
        passin = getenv("AED_PASS");

    if ((f = fopen(passin, "rb")) == NULL) {
        fprintf(stderr, "Cannot open passin file!\n");
        exit(EXIT_FAILURE);
    }
    if ((n = getline(&line, &len, f)) == -1) {
        fprintf(stderr, "Cannot read passin file!\n");
        exit(EXIT_FAILURE);
    }
    line[strlen(line) - 1] = '\0';
    
    if (flag_d == 1) {
        aed_decrypt(line);
    } else if (flag_e == 1) {
        aed_encrypt(line);
    }

    EVP_cleanup();
    if (line)
        free(line);
    fclose(f);
    return 0;
}

void
usage()
{
    printf("aed [ −deh ] −p passin\n");
}

int
genKeyIV(int en, const char *passphrase, unsigned char *salt , unsigned char *key, unsigned char *iv)
{
    int key_len = -1;
    const EVP_CIPHER *cipher = NULL;
    const EVP_MD *md = NULL;

    if (en) {
        if (RAND_bytes(salt, 8) != 1) {
            fprintf(stderr, "Fail to RAND_bytes!\n");
            return -1;
        }
        if (write(STDOUT_FILENO, "Salted__", 8) != 8) {
            perror("Fail to write salt");
            return -1;
        }
        if (write(STDOUT_FILENO, salt, 8) != 8) {
            perror("Fail to write salt");
            return -1;
        }
    } else {
        if (read(STDIN_FILENO, (char *)salt, 8) != 8) {
            perror("Fail to read salt");
            return -1;
        }
        if (strncmp((char *)salt, "Salted__", 8) != 0) {
            fprintf(stderr, "Fail to read salt\n");
            return -1;
        }
        if (read(STDIN_FILENO, salt, 8) != 8) {
            perror("Fail to read salt");
            return -1;
        }
    }

    cipher = EVP_aes_256_cbc();
    if (cipher == NULL) {
        fprintf(stderr, "Fail to get cipher!\n");
        return -1;
    }
    md = EVP_sha1();
    if (md == NULL) {
        fprintf(stderr, "Fail to get digest!\n");
        return -1;
    }
    key_len = EVP_BytesToKey(cipher, md, salt, (const unsigned char *)passphrase, strlen(passphrase), 1024, key, iv);
    return key_len;
}

int aed_encrypt(const char *passphrase)
{
    unsigned char key[EVP_MAX_KEY_LENGTH];
    unsigned char iv[EVP_MAX_IV_LENGTH];
    unsigned char salt[8] = {0};
    unsigned char inbuf[256];
    unsigned char outbuf[256 + EVP_MAX_BLOCK_LENGTH];
    int nread, nwrite;
    int outlen;
    EVP_CIPHER_CTX ctx;
    EVP_CIPHER_CTX_init(&ctx);

    genKeyIV(1, passphrase, salt, key, iv);
    EVP_EncryptInit_ex(&ctx, EVP_aes_256_cbc(), NULL, key, iv);

    for (;;) {
        if ((nread = read(STDIN_FILENO, inbuf, 255)) < 0) {
            perror("Fail to read");
            return -1;
        } else if (nread == 0)
            break;
        if (!EVP_EncryptUpdate(&ctx, outbuf, &outlen, inbuf, nread)) {
            fprintf(stderr, "Fail to EncryptUpdate!\n");
            EVP_CIPHER_CTX_cleanup(&ctx);
            return -1;
        }
        if ((nwrite = write(STDOUT_FILENO, outbuf, outlen)) < 0) {
            perror("Fail to write");
            return -1;
        }
    }
    if (!EVP_EncryptFinal_ex(&ctx, outbuf, &outlen)) {
        EVP_CIPHER_CTX_cleanup(&ctx);
        return -1;
    }
    if (write(STDOUT_FILENO, outbuf, outlen) < 0) {
        perror("Fail to write");
        return -1;
    }

    EVP_CIPHER_CTX_cleanup(&ctx);
    return 0;
}

int aed_decrypt(const char *passphrase)
{
    unsigned char key[EVP_MAX_KEY_LENGTH];
    unsigned char iv[EVP_MAX_IV_LENGTH];
    unsigned char salt[8] = {0};
    unsigned char inbuf[256];
    unsigned char outbuf[256 + EVP_MAX_BLOCK_LENGTH];
    int nread, nwrite;
    int outlen;
    EVP_CIPHER_CTX ctx;
    EVP_CIPHER_CTX_init(&ctx);

    genKeyIV(0, passphrase, salt, key, iv);
    EVP_DecryptInit_ex(&ctx, EVP_aes_256_cbc(), NULL, key, iv);

    for (;;) {
        if ((nread = read(STDIN_FILENO, inbuf, 255)) < 0) {
            perror("Fail to read");
            return -1;
        } else if (nread == 0)
            break;
        if (!EVP_DecryptUpdate(&ctx, outbuf, &outlen, inbuf, nread)) {
            fprintf(stderr, "Fail to EncryptUpdate!\n");
            EVP_CIPHER_CTX_cleanup(&ctx);
            return -1;
        }
        if ((nwrite = write(STDOUT_FILENO, outbuf, outlen)) < 0) {
            perror("Fail to write");
            return -1;
        }
    }
    if (!EVP_DecryptFinal_ex(&ctx, outbuf, &outlen)) {
        EVP_CIPHER_CTX_cleanup(&ctx);
        return -1;
    }
    if (write(STDOUT_FILENO, outbuf, outlen) < 0) {
        perror("Fail to write");
        return -1;
    }

    EVP_CIPHER_CTX_cleanup(&ctx);
    return 0;
}

