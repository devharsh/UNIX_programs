/* Use aes-256-cbc to cipher and decipher plaintext
 *
 */
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include <openssl/evp.h>
#include <sys/stat.h>
#include <sys/types.h>

#define ENC 1
#define DEC 0
#define NON -1
#define BUFF 1024

int f_enc = NON; /* init val: -1, encrpyt: 1, decrypt: 0 */
int f_pass = 0;
int f_salt = 0;

int inlen = -1;
char *passphrase = NULL;
char *salt = NULL;

unsigned char key[EVP_MAX_KEY_LENGTH]; /* 32 */
unsigned char iv[EVP_MAX_IV_LENGTH];   /* 16 */
unsigned char inbuff[BUFF];
unsigned char outbuff[BUFF];

static void usage();
static int generate_key();
static int get_passphrase();
static void salt_check(char *salt);
static int do_code(int infd, int outfd, int enc);

void *Malloc(size_t size);

static void usage() {
  (void)fprintf(stderr, " usage: aed <dehps>\n  \
            -d Decryption input stream.\n   \
            -e Encryption input stream.\n   \
            -h usage.\n   \
            -p passphrase.\n   \
            -s salt.\n");
  exit(EXIT_FAILURE);
}

static void salt_check(char *salt) {
  char *psalt = salt;
  char HexString[] = "0123456789abcdefABCDEF";
  if (strlen(psalt) != 8) {
    (void)fprintf(stderr, "Given salt should be 8 hexadecimal characters.\n");
    exit(EXIT_FAILURE);
  }
  while (psalt && *psalt != '\0') {
    if (!strchr(HexString, *psalt)) { /* or isxdigit() */
      (void)fprintf(stderr, "Not a hexadecimal characters: %c.\n", *psalt);
      exit(EXIT_FAILURE);
    }
    psalt++;
  }
  return;
}

/* return 0 if success */
static int get_passphrase() {
  FILE *ph_fd;

  char *u_pass = Malloc(sizeof(char) * BUFF);
  char *u_pass2 = Malloc(sizeof(char) * BUFF);
  inlen = read(STDIN_FILENO, inbuff, sizeof(inbuff));
  if ((ph_fd = fopen("/dev/tty", "r+")) == NULL)
    err(1, "open error");
  if (f_pass == 0) { /* request passphrase */
    fputs("Please input passphrase:", ph_fd);
    fgets(u_pass, 1024, ph_fd);

    if (f_enc == ENC) {
      fputs("Please input passphrase again:", ph_fd);
      fgets(u_pass2, BUFF, ph_fd);
      if (strcmp(u_pass, u_pass2) != 0)
        fputs("Passphrase not the same.\n", ph_fd), exit(1);
    }

    bcopy(u_pass, passphrase, strlen(u_pass));
    *(passphrase + strlen(u_pass) - 1) = '\0';
  }
  return 0;
}

/* return 0 if success */
static int do_code(int infd, int outfd, int enc) {
  int outlen;
  EVP_CIPHER_CTX ctx;
  EVP_CIPHER_CTX_init(&ctx);

  generate_key();
  EVP_CipherInit_ex(&ctx, EVP_aes_256_cbc(), NULL, NULL, NULL, enc);
  EVP_CIPHER_CTX_set_key_length(&ctx, EVP_MAX_KEY_LENGTH);
  EVP_CipherInit_ex(&ctx, EVP_aes_256_cbc(), NULL, key, iv, enc);
  for (;;) {
    if (!EVP_CipherUpdate(&ctx, outbuff, &outlen, inbuff, inlen)) {
      /* Error */
      EVP_CIPHER_CTX_cleanup(&ctx);
      printf("cleanup error\n");
      return 1;
    }
    write(outfd, outbuff, outlen);
    inlen = read(infd, inbuff, BUFF);
    if (inlen <= 0)
      break;
  }

  if (!EVP_CipherFinal_ex(&ctx, outbuff, &outlen)) {
    /* Error */
    EVP_CIPHER_CTX_cleanup(&ctx);
    fprintf(stderr, "Wrong passphrase\n");
    return 1;
  }
  write(outfd, outbuff, outlen);

  EVP_CIPHER_CTX_cleanup(&ctx);
  return 0;
}

/* generate key with aes 256 cbc, sha1 as digest */
static int generate_key() {
  const EVP_CIPHER *cipher;
  const EVP_MD *dgst = NULL; /* dgst set as NULL */
  const unsigned char *_salt = (unsigned char *)salt;
  memset(key, 0, EVP_MAX_KEY_LENGTH);
  memset(iv, 0, EVP_MAX_IV_LENGTH);
  OpenSSL_add_all_algorithms();

  if ((cipher = EVP_get_cipherbyname("aes-256-cbc")) == NULL) {
    (void)fprintf(stderr, "no such cipher\n");
    return 1;
  }
  if ((dgst = EVP_get_digestbyname("sha1")) == NULL) {
    (void)fprintf(stderr, "no such digest\n");
    return 1;
  }
  if (EVP_BytesToKey(cipher, dgst, _salt, (unsigned char *)passphrase,
                     strlen(passphrase), 1, key, iv) == 0) {
    (void)fprintf(stderr, "EVP_BytesToKey failed\n");
    return 1;
  }

  return 0;
}

int main(int argc, char *argv[]) {
  int opt = 0;

  /* Check Malloc return code */
  passphrase = Malloc(sizeof(char) * BUFF);
  memset(inbuff, 0, sizeof(inbuff));
  memset(outbuff, 0, sizeof(outbuff));
  const char *optString = "dehp:s:";
  while ((opt = getopt(argc, argv, optString)) != -1) {
    switch (opt) {
    case 'd':
      f_enc = DEC;
      break;
    case 'e':
      f_enc = ENC;
      break;
    case 'h':
      usage();
      break;
    case 'p':
      f_pass = 1;
      bcopy(optarg, passphrase, strlen(optarg));
      memset(optarg, '*', strlen(optarg));
      break;
    case 's':
      f_salt = 1;
      salt = optarg;
      (void)salt_check(salt);
      break;
    default:
      usage();
      break;
    }
  }
  argc -= optind;
  argv += optind;

  if (f_enc == NON) {
    fprintf(stderr, "Please select -e or -d.\n");
    usage();
  }

  if (get_passphrase() != 0) {
    fprintf(stderr, "get passphrase error.\n");
    exit(EXIT_FAILURE);
  }

  if (do_code(STDIN_FILENO, STDOUT_FILENO, f_enc)) {
    /* fprintf(stderr, "Cipher Error.\n"); */
    exit(EXIT_FAILURE);
  }

  return 0;
}

void *Malloc(size_t size) {
  void *ptr;
  if ((ptr = malloc(size)) == NULL) {
    fprintf(stderr, "malloc error");
    exit(1);
  }
  return (ptr);
}

