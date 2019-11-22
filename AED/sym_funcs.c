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
unsigned char iv[16];

int
generate_key ()
{
	int i, fd;
	if ((fd = open ("/dev/random", O_RDONLY)) == -1)
		perror ("open error");

	if ((read (fd, key, 32)) == -1)
		perror ("read key error");

	if ((read (fd, iv, 16)) == -1)
		perror ("read iv error");
	
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
aed_decrypt (int infd, int outfd)
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
		  if ((n = read (infd, inbuff, OP_SIZE)) == -1)
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
		  if ((n = write (outfd, outbuf, olen)) == -1)
			  perror ("write error");
	  }

	EVP_CIPHER_CTX_cleanup (&ctx);
	return 1;
}

int
aed_encrypt (int infd, int outfd)
{
	unsigned char outbuf[OP_SIZE];
	int olen, tlen, n;
	unsigned char inbuff[IP_SIZE];
	EVP_CIPHER_CTX ctx;
	EVP_CIPHER_CTX_init (&ctx);
	EVP_EncryptInit (&ctx, EVP_bf_cbc (), key, iv);

	for (;;)
	  {
		  bzero (&inbuff, IP_SIZE);

		  if ((n = read (infd, inbuff, IP_SIZE)) == -1)
		    {
			    perror ("read error");
			    break;
		    }
		  else if (n == 0)
			  break;

		  if (EVP_EncryptUpdate (&ctx, outbuf, &olen, inbuff, n) != 1)
		    {
			    printf ("error in encrypt update\n");
			    return 0;
		    }

		  if (EVP_EncryptFinal (&ctx, outbuf + olen, &tlen) != 1)
		    {
			    printf ("error in encrypt final\n");
			    return 0;
		    }
		  olen += tlen;
		  if ((n = write (outfd, outbuf, olen)) == -1)
			  perror ("write error");
	  }
	EVP_CIPHER_CTX_cleanup (&ctx);
	return 1;
}

int
main ()
{
	int flags1 = 0, flags2 = 0, outfd, infd, decfd;
	mode_t mode;
	char choice;
	int done = 0;

	bzero (&key, 32);
	bzero (&iv, 16);
	bzero (&mode, sizeof (mode));

	flags1 = flags1 | O_RDONLY;
	flags2 = flags2 | O_RDONLY;
	flags2 = flags2 | O_WRONLY;
	flags2 = flags2 | O_CREAT;

	mode = mode | S_IRUSR;
	mode = mode | S_IWUSR;

	generate_key();

	while (!done)
	  {
		  printf ("E - Encrypt a file\n");
		  printf ("D - Decrypt a file\n");
		  printf ("Q - Quit\n");

		  choice = getchar ();

		  switch (choice)
		    {
		    case 'e':
		    case 'E':

			    if ((infd = open ("aed.c", flags1, mode)) == -1)
				    perror ("open input file error");

			    if ((outfd = open ("aed-enc.c", flags2, mode)) == -1)
				    perror ("open output file error");

			    aed_encrypt (infd, outfd);

			    close (infd);
			    close (outfd);
			    break;

		    case 'd':
		    case 'D':

			    if ((outfd = open ("aed-enc.c", flags1, mode)) == -1)
				    perror ("open output file error");

			    if ((decfd = open ("aed-dec.c", flags2, mode)) == -1)
				    perror ("open output file error");

			    aed_decrypt (outfd, decfd);

			    close (outfd);
			    fsync (decfd);
			    close (decfd);
			    break;

		    case 'Q':
		    case 'q':
			    done = 1;
			    break;

		    default:
			    printf ("ERROR: Unrecognized command.  Try again.\n");
			    break;
		    }
	  }
	return 0;
}

