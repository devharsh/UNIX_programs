#include "aed.h"

int
main(void) {
	char buf[SIZE];

	unsigned char key[32];
	unsigned char iv[16];
	unsigned char *plaintext = NULL;
	
	generate_key(key, iv);

	while(fgets(buf, SIZE, stdin) != NULL) {
		if(!plaintext) {
			plaintext = (unsigned char*)(&buf);
		} else {
			strncat((char*)plaintext, buf, strlen(buf));		
		}
	}

	if(plaintext != NULL) {
    		unsigned char ciphertext[SIZE];
    		unsigned char decryptedtext[SIZE];

    		int decryptedtext_len, ciphertext_len;

    		ciphertext_len = aed_encrypt (plaintext, strlen ((char *)plaintext), key, iv,
                              ciphertext);

    		BIO_dump_fp (stdout, (const char *)ciphertext, ciphertext_len);

    		decryptedtext_len = aed_decrypt(ciphertext, ciphertext_len, key, iv,
                                decryptedtext);

    		decryptedtext[decryptedtext_len] = '\0';

    		printf("%s\n", decryptedtext);
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

    	if(!(ctx = EVP_CIPHER_CTX_new()))
        	handleErrors();

    	if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        	handleErrors();

    	if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
        	handleErrors();
    	ciphertext_len = len;

    	if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
        	handleErrors();
    	ciphertext_len += len;

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

    	if(!(ctx = EVP_CIPHER_CTX_new()))
        	handleErrors();

    	if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        	handleErrors();

    	if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
        	handleErrors();
    	plaintext_len = len;

    	if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
        	handleErrors();
    	plaintext_len += len;

    	EVP_CIPHER_CTX_free(ctx);

    	return plaintext_len;
}
