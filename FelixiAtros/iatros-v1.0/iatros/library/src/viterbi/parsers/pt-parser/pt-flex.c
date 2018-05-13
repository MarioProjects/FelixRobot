#include "pt-flex.h"

/*
 * global variable
 */
int pt_debug = 0;

/*
 * lokal variable
 */
static FILE *flex_file;
static int flex_eof = 0;
static int flex_n_row = 0;
static int flex_n_buffer = 0;
static int flex_l_buffer = 0;
static int flex_n_token_start = 0;
static int flex_n_token_leptth = 0;
static int flex_n_token_next_start = 0;
static int flex_l_max_buffer = 1000;
static char *flex_buffer;



/****** Utilities ******/
char *pt_copy_string(const char *txt, int remove_quotes) {
  char *ptr;
  int len = (remove_quotes)? strlen(txt)-2:strlen(txt);
  ptr = (char *)malloc(len+1 * sizeof(char));
  strncpy(ptr,txt + ((remove_quotes)?1:0), len);
  ptr[len] = '\0';
  return ptr;
}
/****** end utilities ******/


/*--------------------------------------------------------------------
 * dump_char
 *
 * printable version of a char
 *------------------------------------------------------------------*/
static
char pt_dump_char(char c) {
  if (isprint(c)) return c;
  return '@';
}

/*--------------------------------------------------------------------
 * dump_stript
 *
 * printable version of a string upto 100 character
 *------------------------------------------------------------------*/
static
char *pt_dump_string(char *s) {
  static char buf[101];
  int i;
  int n = strlen(s);

  if (n > 100) n = 100;

  for (i=0; i<n; i++) buf[i] = pt_dump_char(s[i]);
  buf[i] = 0;
  return buf;
}

/*--------------------------------------------------------------------
 * dump_row
 *
 * dumps the contents of the current row
 *------------------------------------------------------------------*/
extern
void pt_dump_row(void) {
  if (flex_n_row == 0) {
    int i;
    for (i=1; i<71; i++) {
      if (i % 10 == 0)     fprintf(stdout, ":");
      else if (i % 5 == 0) fprintf(stdout, "+");
      else                 fprintf(stdout, ".");
    }
  }
  else {
    fprintf(stdout, "%.*s", flex_l_buffer, flex_buffer);
  }
}

/*--------------------------------------------------------------------
 * print_error
 *
 * marks the current read token
 *------------------------------------------------------------------*/
extern
void pt_print_error(char *errorstring, ...) {
  static char errmsg[10000];
  va_list args;

  int start=flex_n_token_start;
  int end=start + flex_n_token_leptth - 1;
  int i;

  pt_dump_row();
  if (flex_eof) {
    for (i=0; i<flex_l_buffer; i++) fprintf(stdout, ".");
    fprintf(stdout, "^-EOF\n");
  }
  else {
    for (i=1; i<start; i++)             fprintf(stdout, ".");
    for (i=start; i<=end; i++)          fprintf(stdout, "^");
    for (i=end+1; i<flex_l_buffer; i++) fprintf(stdout, ".");
    fprintf(stdout, " at %d.%d-%d\n", flex_n_row, start, end);
  }

  /*================================================================*/
  /* print it usipt variable arguments -----------------------------*/
  va_start(args, errorstring);
  vsprintf(errmsg, errorstring, args);
  va_end(args);

  fprintf(stdout, "Error: %s\n", errmsg);
}

/*--------------------------------------------------------------------
 * get_next_line
 *
 * reads a line into the buffer
 *------------------------------------------------------------------*/
static
int pt_get_next_line(void) {
  char *p;

  flex_n_buffer = 0;
  flex_n_token_start = -1;
  flex_n_token_next_start = 1;
  flex_eof = false;

  /* read a line ---------------------------------------------------*/
  p = fgets(flex_buffer, flex_l_max_buffer, flex_file);
  if (p == NULL) {
    if (ferror(flex_file)) return -1;
    flex_eof = true;
    return 1;
  }

  flex_n_row += 1;
  flex_l_buffer = strlen(flex_buffer);
  if (pt_debug) pt_dump_row();

  return 0;
}

/*--------------------------------------------------------------------
 * get_next_char
 *
 * reads a character from input for flex
 *------------------------------------------------------------------*/
extern
int pt_get_next_char(char *b, int UNUSED(max_buffer)) {
  int frc;

  /*================================================================*/
  /*----------------------------------------------------------------*/
  if (flex_eof)
    return 0;

  /*================================================================*/
  /* read next line if at the end of the current -------------------*/
  while (flex_n_buffer >= flex_l_buffer) {
    frc = pt_get_next_line();
    if (frc != 0)
      return 0;
    }

  /*================================================================*/
  /* ok, return character ------------------------------------------*/
  b[0] = flex_buffer[flex_n_buffer];
  flex_n_buffer += 1;

  if (pt_debug)
    printf("pt_get_next_char() => '%c'0x%02x at %d\n",
                        pt_dump_char(b[0]), b[0], flex_n_buffer);
  return b[0]==0?0:1;
}

/*--------------------------------------------------------------------
 * begin_token
 *
 * marks the beginnipt of a new token
 *------------------------------------------------------------------*/
extern
void pt_begin_token(char *t) {
  /*================================================================*/
  /* remember last read token --------------------------------------*/
  flex_n_token_start = flex_n_token_next_start;
  flex_n_token_leptth = strlen(t);
  flex_n_token_next_start = flex_n_buffer; // + 1;

  /*================================================================*/
  /* location for bison --------------------------------------------*/
  pt_lloc.first_line = flex_n_row;
  pt_lloc.first_column = flex_n_token_start;
  pt_lloc.last_line = flex_n_row;
  pt_lloc.last_column = flex_n_token_start + flex_n_token_leptth - 1;

  if (pt_debug) {
    printf("Token '%s' at %d.%d next at %d\n", pt_dump_string(t),
                        pt_lloc.first_column,
                        pt_lloc.last_column, flex_n_token_next_start);
  }
}



/*------------------------------------------------------------------------------
 * pt_error
 *----------------------------------------------------------------------------*/
extern
void pt_error(grammar_t *UNUSED(pt), char *s) {
  pt_print_error(s);
}

/*--------------------------------------------------------------------
 * pt_load
 *------------------------------------------------------------------*/
extern
int grammar_load_phrase_table(grammar_t *pt, FILE *file) {
  flex_file=file;

  /*================================================================*/
  /*----------------------------------------------------------------*/
  flex_buffer = (char *)malloc(flex_l_max_buffer);
  if (  flex_buffer == NULL  ) {
    printf("cannot allocate %d bytes of memory\n", flex_l_max_buffer);
    return 12;
  }

  /*================================================================*/
  /* parse it ------------------------------------------------------*/
  if (pt_get_next_line() == 0) pt_parse(pt);

  /*================================================================*/
  /* ending... -----------------------------------------------------*/
  free(flex_buffer);
  // http://sourceforge.net/mailarchive/message.php?msg_id=46164E88.1030007%40nemesys.hu
  pt_lex_destroy();

  return 0;
}
