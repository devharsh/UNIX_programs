#include "aed.h"

int
main(void) {
	char buf[SIZE];

	/*unsigned char key[32];
	unsigned char iv[16];

	generate_key(key, iv);*/

	/* A 256 bit key */
    	unsigned char *key = (unsigned char *)"01234567890123456789012345678901";

    	/* A 128 bit IV */
    	unsigned char *iv = (unsigned char *)"0123456789012345";

	while (fgets(buf, SIZE, stdin) != NULL) {
		/*unsigned char plaintext[SIZE];*/
    		unsigned char ciphertext[SIZE];
    		unsigned char decryptedtext[SIZE];

    		int ciphertext_len, decryptedtext_len;

		if(ciphertext_len > decryptedtext_len) {}	
		
		/*memcpy(plaintext, buf, strlen(buf));
		ciphertext_len = aed_encrypt(plaintext, strlen ((char *)plaintext), key, iv,
                              ciphertext);
		printf("%s", ciphertext);*/

		memcpy(ciphertext, buf, strlen(buf));
		decryptedtext_len = aed_decrypt(ciphertext, strlen ((char *)ciphertext), key, iv,
                                decryptedtext);
		decryptedtext[decryptedtext_len] = '\0';
		printf("%s", decryptedtext);
	}

	return 0;
}

void
printHelp(void) {
	printf("To encrypt the contents of the file 'file' and storing the encrypted output in 'file.enc':\n");
	printf("aed -e < file > file.enc\n");
	printf("To decrypt the contents of that file again:\n");
	printf("aed -d <file.enc\n");
	printf("Since aed operates on stdin and stdout, the above two commands could also be chained:\n");
	printf("cat file | aed -e | aed -d\n"); 
}

void 
handleErrors(void) {
	ERR_print_errors_fp(stderr);
    	abort();
}

void
generate_key (unsigned char* key, unsigned char* iv) {
	int fd;
	unsigned char key1[16];
	unsigned char key2[16];

	if ((fd = open ("/dev/random", O_RDONLY)) == -1)
		perror ("open error");

	if ((read (fd, key1, 16)) == -1)
		perror ("read key1 error");

	if ((read (fd, key2, 16)) == -1)
		perror ("read key2 error");

	if ((read (fd, iv, 16)) == -1)
		perror ("read iv error");

	memcpy(key, key1, 16);
	memcpy(key+16, key2, 16);
	
	close (fd);
}

int 
aed_encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
        unsigned char *iv, unsigned char *ciphertext)
{
    EVP_CIPHER_CTX *ctx;

    int len;

    int ciphertext_len;

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();

    /*
     * Initialise the encryption operation. IMPORTANT - ensure you use a key
     * and IV size appropriate for your cipher
     * In this example we are using 256 bit AES (i.e. a 256 bit key). The
     * IV size for *most* modes is the same as the block size. For AES this
     * is 128 bits
     */
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        handleErrors();

    /*
     * Provide the message to be encrypted, and obtain the encrypted output.
     * EVP_EncryptUpdate can be called multiple times if necessary
     */
    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
        handleErrors();
    ciphertext_len = len;

    /*
     * Finalise the encryption. Further ciphertext bytes may be written at
     * this stage.
     */
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
        handleErrors();
    ciphertext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}

int 
aed_decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
        unsigned char *iv, unsigned char *plaintext)
{
    EVP_CIPHER_CTX *ctx;

    int len;

    int plaintext_len;

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();

    /*
     * Initialise the decryption operation. IMPORTANT - ensure you use a key
     * and IV size appropriate for your cipher
     * In this example we are using 256 bit AES (i.e. a 256 bit key). The
     * IV size for *most* modes is the same as the block size. For AES this
     * is 128 bits
     */
    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        handleErrors();

    /*
     * Provide the message to be decrypted, and obtain the plaintext output.
     * EVP_DecryptUpdate can be called multiple times if necessary.
     */
    if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
        handleErrors();
    plaintext_len = len;

    /*
     * Finalise the decryption. Further plaintext bytes may be written at
     * this stage.
     */
    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
        handleErrors();
    plaintext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}
