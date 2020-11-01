/* PDCurses */

#if defined(PDC_WIDE) && !defined(UNICODE)
# define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef MOUSE_MOVED
#include <curspriv.h>

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_DEPRECATE)
# define _CRT_SECURE_NO_DEPRECATE 1   /* kill nonsense warnings */
#endif

typedef struct {short r, g, b; bool mapped;} PDCCOLOR;

extern PDCCOLOR pdc_color[PDC_MAXCOL];

extern HANDLE pdc_con_out, pdc_con_in;
extern DWORD pdc_quick_edit;
extern DWORD pdc_last_blink;
extern short pdc_curstoreal[16], pdc_curstoansi[16];
extern short pdc_oldf, pdc_oldb, pdc_oldu;
extern bool pdc_conemu, pdc_ansi;

extern void PDC_blink_text(void);

#ifdef PDC_WIN10_JP
/* for windows 10 jp */
extern DWORD pdc_con_in_mode;       /* console input mode */
extern DWORD pdc_con_out_mode;      /* console output mode */
extern DWORD pdc_con_in_mode_orig;  /* preserved console input mode */
extern DWORD pdc_con_out_mode_orig; /* preserved console output mode */
extern bool pdc_mintty;             /* mintty (winpty is needed) detection */
extern bool pdc_winterm;            /* Windows Terminal (windows 10) detection */
extern int pdc_ambiguous_width;     /* width of ambiguous width characters (=1 or 2) */
extern int pdc_emoji_width;         /* width of emoji characters (=1 or 2) */
extern void PDC_set_console_cursor_position(int y, int x);
extern void PDC_write_console_w(int y, int x, WCHAR *buffer, int len);
extern void PDC_write_console_output_w(int y, int x, CHAR_INFO *ci_buffer, int len);
#ifdef PDC_VT_MOUSE_INPUT
/* use vt escape sequence of mouse input */
extern BOOL PDC_peek_console_input_w(HANDLE hin, PINPUT_RECORD input_rec_ptr, DWORD input_rec_len, LPDWORD read_event_num_ptr);
extern BOOL PDC_read_console_input_w(HANDLE hin, PINPUT_RECORD input_rec_ptr, DWORD input_rec_len, LPDWORD read_event_num_ptr);
#endif
#endif
