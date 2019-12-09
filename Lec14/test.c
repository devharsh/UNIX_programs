#include <stdio.h>
#include <string.h>

void htmler(char* dest, char* str_html);

int main(void) {
  char* str_html;
  char* dest = "devharsh";
	htmler(dest, str_html);
  printf("%s\n", str_html);
  return 0;
}

void htmler(char* dest, char* str_html) {
  strcpy(str_html, "<html><head><title>");
	strcat(str_html, "</title><body>");
  strcat(str_html, dest);
	strcat(str_html, "</body></html>");
}
