#include "aed.h"

int
main(void) {
	char buf[SIZE];

	unsigned char *key = NULL;
	unsigned char *iv = NULL;
	unsigned char *plaintext = NULL;
	
	generate_key(key, iv);

	while(fgets(buf, SIZE, stdin) != NULL) {
		printf("%s", buf);
		if(!plaintext) {
			plaintext = (unsigned char*)(&buf);
		} else {
			strncat((char*)plaintext, buf, strlen(buf));		
		}
	}

	if(plaintext != NULL) {
		/*
     		 * Buffer for ciphertext. Ensure the buffer is long enough for the
     		 * ciphertext which may be longer than the plaintext, depending on the
     		 * algorithm and mode.
     		 */
    		unsigned char ciphertext[128];

    		/* Buffer for the decrypted text */
    		unsigned char decryptedtext[128];

    		int decryptedtext_len, ciphertext_len;

    		/* Encrypt the plaintext */
    		ciphertext_len = aed_encrypt (plaintext, strlen ((char *)plaintext), key, iv,
                              ciphertext);

    		/* Do something useful with the ciphertext here */
    		printf("Ciphertext is:\n");
    		BIO_dump_fp (stdout, (const char *)ciphertext, ciphertext_len);

    		/* Decrypt the ciphertext */
    		decryptedtext_len = aed_decrypt(ciphertext, ciphertext_len, key, iv,
                                decryptedtext);

    		/* Add a NULL terminator. We are expecting printable text */
    		decryptedtext[decryptedtext_len] = '\0';

    		/* Show the decrypted text */
    		printf("Decrypted text is:\n");
    		printf("%s\n", decryptedtext);
	}
	
	return 0;
}

void 
handleErrors(void)
{
	ERR_print_errors_fp(stderr);
    	abort();
}

void
generate_key (unsigned char* key, unsigned char* iv) {
	int i, fd;
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
	
	printf("256 bit key:\n");
	for (i = 0; i < 32; i++)
		printf ("%d \t", key[i]);
	printf ("\n ------ \n");

	printf("Initialization vector\n");
	for (i = 0; i < 16; i++)
		printf ("%d \t", iv[i]);

	printf ("\n ------ \n");
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
