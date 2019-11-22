#include <openssl/blowfish.h>
#include <openssl/evp.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define IP_SIZE 1024
#define OP_SIZE 1040

unsigned char key[32];
unsigned char key1[16];
unsigned char key2[16];
unsigned char iv[16];

int
generate_key ()
{
	int i, fd;
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
	return 0;
}

int
aed_decrypt ()
{
	unsigned char outbuf[IP_SIZE];
	int olen, tlen, n;
	unsigned char inbuff[OP_SIZE];
	EVP_CIPHER_CTX ctx;
	EVP_CIPHER_CTX_init (&ctx);
	EVP_DecryptInit (&ctx, EVP_bf_cbc (), key, iv);

	for (;;)
	  {
		  bzero (&inbuff, OP_SIZE);
		  if ((n = read (stdout, inbuff, OP_SIZE)) == -1)
		    {
			    perror ("read error");
			    break;
		    }
		  else if (n == 0)
			  break;

		  bzero (&outbuf, IP_SIZE);

		  if (EVP_DecryptUpdate (&ctx, outbuf, &olen, inbuff, n) != 1)
		    {
			    printf ("error in decrypt update\n");
			    return 0;
		    }

		  if (EVP_DecryptFinal (&ctx, outbuf + olen, &tlen) != 1)
		    {
			    printf ("error in decrypt final\n");
			    return 0;
		    }
		  olen += tlen;
		  if ((n = write (stdin, outbuf, olen)) == -1)
			  perror ("write error");
	  }

	EVP_CIPHER_CTX_cleanup (&ctx);
	return 1;
}

int
aed_encrypt ()
{
	unsigned char outbuf[OP_SIZE];
	int olen, tlen, n;
	unsigned char inbuff[IP_SIZE];
	EVP_CIPHER_CTX ctx;
	EVP_CIPHER_CTX_init (&ctx);
	EVP_EncryptInit (&ctx, EVP_bf_cbc (), key, iv);
	bzero (&inbuff, IP_SIZE);
	
	while(fgets(inbuff, IP_SIZE, stdin) != NULL) {
		  if (EVP_EncryptUpdate (&ctx, outbuf, &olen, inbuff, strlen(inbuff)) != 1) {
			    printf ("error in encrypt update\n");
			    return 0;
		  }

		  if (EVP_EncryptFinal (&ctx, outbuf + olen, &tlen) != 1)
		    {
			    printf ("error in encrypt final\n");
			    return 0;
		    }
		  olen += tlen;
		  if ((n = write (stdout, outbuf, olen)) == -1)
			  perror ("write error");
	}

	EVP_CIPHER_CTX_cleanup (&ctx);
	return 1;
}

int
main (void)
{
	bzero (&key, 32);
	bzero (&iv, 16);

	generate_key();

			aed_encrypt ();

			aed_decrypt ();

			printf("To encrypt the contents of the file 'file' and storing the encrypted output in 'file.enc':\n");
			printf("aed -e < file > file.enc\n");
			printf("To decrypt the contents of that file again:\n");
			printf("aed -d <file.enc\n");
			printf("Since aed operates on stdin and stdout, the above two commands could also be chained:\n");
			printf("cat file | aed -e | aed -d\n"); 
			return 0;

}
