#ifdef PDC_WIN10_JP
/* for windows 10 jp */
#ifdef PDC_VT_MOUSE_INPUT
/* use vt escape sequence of mouse input */

#include "pdcwin.h"

/* #define USE_DBG_PRINT */

#ifdef USE_DBG_PRINT
#define DBG_PRINT(...)                                          \
    do {                                                        \
        FILE *log_file = fopen("log_pdckbd_sub_0001.txt", "a"); \
        fprintf(log_file, __VA_ARGS__);                         \
        fclose(log_file);                                       \
    } while (0)
#else
#define DBG_PRINT(...) do { } while (0)
#endif

#define MAX_INPUT_REC_LEN    20
#define MAX_VT_INPUT_SEQ_LEN 6

/* vt escape sequence input structure */
struct vt_input
{
    char  seq[MAX_VT_INPUT_SEQ_LEN + 1];
    int   seq_len;
    WORD  vk;
    WORD  vs;
    WCHAR uchar;
    DWORD ctrl;
    int   modifier_index;
};

/* vt escape sequence input table
   ( https://docs.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences#input-sequences )
*/
static const struct vt_input vt_input_table[] = {
    { "[A",     2, 0x26, 0x48, 0,    0x0100, 0 }, /* Up Arrow */
    { "[B",     2, 0x28, 0x50, 0,    0x0100, 0 }, /* Down Arrow */
    { "[C",     2, 0x27, 0x4d, 0,    0x0100, 0 }, /* Right Arrow */
    { "[D",     2, 0x25, 0x4b, 0,    0x0100, 0 }, /* Left Arrow */
    { "[H",     2, 0x24, 0x47, 0,    0x0100, 0 }, /* Home */
    { "[F",     2, 0x23, 0x4f, 0,    0x0100, 0 }, /* End */

    { "[1;@A",  5, 0x26, 0x48, 0,    0x0100, 3 }, /* ModifierKeys + Up Arrow */
    { "[1;@B",  5, 0x28, 0x50, 0,    0x0100, 3 }, /* ModifierKeys + Down Arrow */
    { "[1;@C",  5, 0x27, 0x4d, 0,    0x0100, 3 }, /* ModifierKeys + Right Arrow */
    { "[1;@D",  5, 0x25, 0x4b, 0,    0x0100, 3 }, /* ModifierKeys + Left Arrow */
    { "[1;@H",  5, 0x24, 0x47, 0,    0x0100, 3 }, /* ModifierKeys + Home */
    { "[1;@F",  5, 0x23, 0x4f, 0,    0x0100, 3 }, /* ModifierKeys + End */

    { "[2~",    3, 0x2d, 0x52, 0,    0x0100, 0 }, /* Insert */
    { "[3~",    3, 0x2e, 0x53, 0,    0x0100, 0 }, /* Delete */
    { "[5~",    3, 0x21, 0x49, 0,    0x0100, 0 }, /* Page Up */
    { "[6~",    3, 0x22, 0x51, 0,    0x0100, 0 }, /* Page Down */

    { "[2;@~",  5, 0x2d, 0x52, 0,    0x0100, 3 }, /* ModifierKeys + Insert */
    { "[3;@~",  5, 0x2e, 0x53, 0,    0x0100, 3 }, /* ModifierKeys + Delete */
    { "[5;@~",  5, 0x21, 0x49, 0,    0x0100, 3 }, /* ModifierKeys + Page Up */
    { "[6;@~",  5, 0x22, 0x51, 0,    0x0100, 3 }, /* ModifierKeys + Page Down */

    { "OP",     2, 0x70, 0x3b, 0,    0x0000, 0 }, /* F1 */
    { "OQ",     2, 0x71, 0x3c, 0,    0x0000, 0 }, /* F2 */
    { "OR",     2, 0x72, 0x3d, 0,    0x0000, 0 }, /* F3 */
    { "OS",     2, 0x73, 0x3e, 0,    0x0000, 0 }, /* F4 */
    { "[15~",   4, 0x74, 0x3f, 0,    0x0000, 0 }, /* F5 */
    { "[17~",   4, 0x75, 0x40, 0,    0x0000, 0 }, /* F6 */
    { "[18~",   4, 0x76, 0x41, 0,    0x0000, 0 }, /* F7 */
    { "[19~",   4, 0x77, 0x42, 0,    0x0000, 0 }, /* F8 */
    { "[20~",   4, 0x78, 0x43, 0,    0x0000, 0 }, /* F9 */
    { "[21~",   4, 0x79, 0x44, 0,    0x0000, 0 }, /* F10 */
    { "[23~",   4, 0x7a, 0x57, 0,    0x0000, 0 }, /* F11 */
    { "[24~",   4, 0x7b, 0x58, 0,    0x0000, 0 }, /* F12 */

    { "[1;@P",  5, 0x70, 0x3b, 0,    0x0000, 3 }, /* ModifierKeys + F1 */
    { "[1;@Q",  5, 0x71, 0x3c, 0,    0x0000, 3 }, /* ModifierKeys + F2 */
    { "[1;@R",  5, 0x72, 0x3d, 0,    0x0000, 3 }, /* ModifierKeys + F3 */
    { "[1;@S",  5, 0x73, 0x3e, 0,    0x0000, 3 }, /* ModifierKeys + F4 */
    { "[15;@~", 6, 0x74, 0x3f, 0,    0x0000, 4 }, /* ModifierKeys + F5 */
    { "[17;@~", 6, 0x75, 0x40, 0,    0x0000, 4 }, /* ModifierKeys + F6 */
    { "[18;@~", 6, 0x76, 0x41, 0,    0x0000, 4 }, /* ModifierKeys + F7 */
    { "[19;@~", 6, 0x77, 0x42, 0,    0x0000, 4 }, /* ModifierKeys + F8 */
    { "[20;@~", 6, 0x78, 0x43, 0,    0x0000, 4 }, /* ModifierKeys + F9 */
    { "[21;@~", 6, 0x79, 0x44, 0,    0x0000, 4 }, /* ModifierKeys + F10 */
    { "[23;@~", 6, 0x7a, 0x57, 0,    0x0000, 4 }, /* ModifierKeys + F11 */
    { "[24;@~", 6, 0x7b, 0x58, 0,    0x0000, 4 }, /* ModifierKeys + F12 */

    { "[Z",     2, 0x9,  0xf,  0x9,  0x0000, 0 }  /* Shift + Tab */
};

/* inner functions */
static int is_vt_input(const INPUT_RECORD *input_rec_ptr);
static void set_key_event(PINPUT_RECORD input_rec_ptr, WORD vk, WORD vs, WCHAR uchar, DWORD ctrl);
static int drop_left_alt_key_state(int ctrl_state);
static BOOL consume_vt_input(HANDLE hin, int input_seq_len);
static BOOL read_console_input_w_sub(HANDLE hin, PINPUT_RECORD input_rec_ptr, DWORD input_rec_len, LPDWORD read_event_num_ptr, int peek_flag);

/* api functions */
BOOL PDC_peek_console_input_w(HANDLE hin, PINPUT_RECORD input_rec_ptr, DWORD input_rec_len, LPDWORD read_event_num_ptr);
BOOL PDC_read_console_input_w(HANDLE hin, PINPUT_RECORD input_rec_ptr, DWORD input_rec_len, LPDWORD read_event_num_ptr);

/* check vt escape sequence */
static int is_vt_input(const INPUT_RECORD *input_rec_ptr)
{
    return (input_rec_ptr->EventType == KEY_EVENT &&
            input_rec_ptr->Event.KeyEvent.bKeyDown &&
            input_rec_ptr->Event.KeyEvent.wVirtualKeyCode  == 0 &&
            input_rec_ptr->Event.KeyEvent.wVirtualScanCode == 0);
}

/* set key event */
static void set_key_event(PINPUT_RECORD input_rec_ptr, WORD vk, WORD vs, WCHAR uchar, DWORD ctrl)
{
    input_rec_ptr->EventType = KEY_EVENT;
    input_rec_ptr->Event.KeyEvent.bKeyDown = 1;
    input_rec_ptr->Event.KeyEvent.wRepeatCount = 1;
    input_rec_ptr->Event.KeyEvent.wVirtualKeyCode   = vk;
    input_rec_ptr->Event.KeyEvent.wVirtualScanCode  = vs;
    input_rec_ptr->Event.KeyEvent.uChar.UnicodeChar = uchar;
    input_rec_ptr->Event.KeyEvent.dwControlKeyState = ctrl;
}

/* drop left alt key state
   (to avoid unwanted character code conversion on PDCurses) */
static int drop_left_alt_key_state(int ctrl_state)
{
    return (ctrl_state & ~LEFT_ALT_PRESSED);
}

/* consume vt escape sequence */
static BOOL consume_vt_input(HANDLE hin, int input_seq_len) {
    INPUT_RECORD input_rec[MAX_INPUT_REC_LEN];
    DWORD read_event_num;

    /* check arguments */
    if (input_seq_len > MAX_INPUT_REC_LEN) {
        DBG_PRINT("internal error. (consume)\n");
        SetLastError(ERROR_INTERNAL_ERROR);
        return FALSE;
    }

    /* consume vt escape sequence */
    if (!ReadConsoleInputW(hin, input_rec, input_seq_len, &read_event_num)) {
        DBG_PRINT("ReadConsoleInputW failed. (consume)\n");
        return FALSE;
    }
    if ((int)read_event_num < input_seq_len) {
        DBG_PRINT("ReadConsoleInputW returned before read. (consume)\n");
        SetLastError(ERROR_INTERNAL_ERROR);
        return FALSE;
    }
    return TRUE;
}

/* peek/read console input
    limitations:
     - only one input record is returned.
     - separated receive of vt escape sequence is not supported.
     - double click event is not supported.
     - horizontal mouse wheel is not supported.
     - virtual key code and virtual scan code casually become zero.
     - Alt + X key combination input is split into Esc and X key events.
     - mouse coordinates origin is top left of screen (not top left of buffer).
*/
BOOL PDC_peek_console_input_w(HANDLE hin, PINPUT_RECORD input_rec_ptr, DWORD input_rec_len, LPDWORD read_event_num_ptr)
{
    return read_console_input_w_sub(hin, input_rec_ptr, input_rec_len, read_event_num_ptr, 1);
}
BOOL PDC_read_console_input_w(HANDLE hin, PINPUT_RECORD input_rec_ptr, DWORD input_rec_len, LPDWORD read_event_num_ptr)
{
    return read_console_input_w_sub(hin, input_rec_ptr, input_rec_len, read_event_num_ptr, 0);
}
static BOOL read_console_input_w_sub(HANDLE hin, PINPUT_RECORD input_rec_ptr, DWORD input_rec_len, LPDWORD read_event_num_ptr, int peek_flag)
{
    static int old_mouse_x = -1;
    static int old_mouse_y = -1;
    static int mouse_button_state = 0;
    static int ctrl_state = 0;
    INPUT_RECORD input_rec2[MAX_INPUT_REC_LEN];
    DWORD read_event_num2;
    char input_seq[MAX_INPUT_REC_LEN + 1];
    int input_seq_len;
    int input_seq_len2;
    int vt_input_table_len;
    int mouse_input_state;
    int mouse_button_no;
    int mouse_x;
    int mouse_y;
    int mouse_button_press;
    int ret_val;
    int i;

    /* check arguments */
    if (input_rec_len == 0) {
        *read_event_num_ptr = 0;
        /* DBG_PRINT("specified input record length is zero.\n"); */
        return TRUE;
    }

    /* check peek flag */
    if (peek_flag) {
        /* peek all console input */
        if (!PeekConsoleInputW(hin, input_rec2, MAX_INPUT_REC_LEN, &read_event_num2)) {
            *read_event_num_ptr = 0;
            DBG_PRINT("PeekConsoleInputW failed. (input_rec2)\n");
            return FALSE;
        }
        if (read_event_num2 == 0) {
            *read_event_num_ptr = 0;
            /* DBG_PRINT("PeekConsoleInputW returned before read. (input_rec2)\n"); */
            return TRUE;
        }
    } else {
        /* read one console input */
        if (!ReadConsoleInputW(hin, input_rec2, 1, &read_event_num2)) {
            *read_event_num_ptr = 0;
            DBG_PRINT("ReadConsoleInputW failed. (input_rec2)\n");
            return FALSE;
        }
        if (read_event_num2 == 0) {
            *read_event_num_ptr = 0;
            DBG_PRINT("ReadConsoleInputW returned before read. (input_rec2)\n");
            SetLastError(ERROR_INTERNAL_ERROR);
            return FALSE;
        }
        /* peek all console input */
        if (!PeekConsoleInputW(hin, &input_rec2[1], MAX_INPUT_REC_LEN - 1, &read_event_num2)) {
            *read_event_num_ptr = 0;
            DBG_PRINT("PeekConsoleInputW failed. (input_rec2) (after read)\n");
            return FALSE;
        }
        read_event_num2++;
    }

    /* set return data (only one input record is returned) */
    *read_event_num_ptr = 1;
    memcpy(input_rec_ptr, &input_rec2[0], sizeof(INPUT_RECORD));

    /* get/set modifier key state */
    if (input_rec2[0].EventType == KEY_EVENT) {
        if (!is_vt_input(&input_rec2[0])) {
            /* get modifier key state */
            ctrl_state = input_rec2[0].Event.KeyEvent.dwControlKeyState;
        } else {
            /* vt escape sequence doesn't have modifier key state.
               so, we set the current state here. */
            ctrl_state = drop_left_alt_key_state(ctrl_state);
            input_rec_ptr->Event.KeyEvent.dwControlKeyState = ctrl_state;
        }
    }

    /* convert some keys */
    if (is_vt_input(&input_rec2[0]) &&
        input_rec2[0].Event.KeyEvent.uChar.UnicodeChar == 0x7f) { /* Backspace */
        set_key_event(input_rec_ptr, 0x8, 0xe, 0x8, ctrl_state);
        return TRUE;
    }
    if (is_vt_input(&input_rec2[0]) &&
        input_rec2[0].Event.KeyEvent.uChar.UnicodeChar == 0x8) {  /* Ctrl + Backspace */
        set_key_event(input_rec_ptr, 0x8, 0xe, 0x7f, ctrl_state);
        return TRUE;
    }
    if (is_vt_input(&input_rec2[0]) &&
        input_rec2[0].Event.KeyEvent.uChar.UnicodeChar == 0x1a) { /* Pause */
        set_key_event(input_rec_ptr, 0x13, 0x45, 0, ctrl_state);
        return TRUE;
    }
    if (input_rec2[0].EventType == KEY_EVENT &&
        input_rec2[0].Event.KeyEvent.bKeyDown &&
        input_rec2[0].Event.KeyEvent.wVirtualKeyCode == 0x32 &&
        input_rec2[0].Event.KeyEvent.wVirtualScanCode == 0 &&
        input_rec2[0].Event.KeyEvent.uChar.UnicodeChar == 0) {    /* Ctrl + Space */
        set_key_event(input_rec_ptr, 0x20, 0x39, 0x20, ctrl_state);
        return TRUE;
    }
    if (is_vt_input(&input_rec2[0]) &&
        input_rec2[0].Event.KeyEvent.uChar.UnicodeChar == 0x9) {  /* Ctrl + Tab */
        set_key_event(input_rec_ptr, 0x9, 0xf, 0x0, ctrl_state);
        return TRUE;
    }

    /* process vt escape sequence */
    if (is_vt_input(&input_rec2[0]) &&
        input_rec2[0].Event.KeyEvent.uChar.UnicodeChar == 0x1b) {

        /* read vt escape sequence */
        input_seq_len = 0;
        for (i = 1; i < (int)read_event_num2; i++) {
            if (is_vt_input(&input_rec2[i]) &&
                input_rec2[i].Event.KeyEvent.uChar.UnicodeChar <= 0x7f) {
                input_seq[input_seq_len] = input_rec2[i].Event.KeyEvent.uChar.UnicodeChar;
                input_seq_len++;
                continue;
            }
            break;
        }
        input_seq[input_seq_len] = 0;

        /* process vt escape sequence of mouse input (sgr-1006) '[<' */
        if (input_seq_len >= 2 &&
            input_seq[0] == 0x5b &&
            input_seq[1] == 0x3c) {
            input_seq_len2 = 2;

            /* read parameters 'Db ; Dx ; Dy M/m' */
            mouse_input_state = 0;
            mouse_button_no = 0;
            mouse_x = 0;
            mouse_y = 0;
            mouse_button_press = 0;
            ret_val = FALSE;
            for (i = 2; i < input_seq_len; i++) {
                switch (mouse_input_state) {
                    case 0:
                        /* read mouse button number 'Db' */
                        if (input_seq[i] >= 0x30 && input_seq[i] <= 0x39) {
                            input_seq_len2++;
                            mouse_button_no *= 10;
                            mouse_button_no += input_seq[i] - 0x30;
                            if (mouse_button_no < 10000) {
                                continue;
                            }
                        }
                        /* read delimiter ';' */
                        if (input_seq[i] == 0x3b) {
                            input_seq_len2++;
                            mouse_input_state++;
                            continue;
                        }
                        break;
                    case 1:
                        /* read mouse position x 'Dx' */
                        if (input_seq[i] >= 0x30 && input_seq[i] <= 0x39) {
                            input_seq_len2++;
                            mouse_x *= 10;
                            mouse_x += input_seq[i] - 0x30;
                            if (mouse_x < 10000) {
                                continue;
                            }
                        }
                        /* read delimiter ';' */
                        if (input_seq[i] == 0x3b) {
                            input_seq_len2++;
                            mouse_input_state++;
                            continue;
                        }
                        break;
                    case 2:
                        /* read mouse position y 'Dy' */
                        if (input_seq[i] >= 0x30 && input_seq[i] <= 0x39) {
                            input_seq_len2++;
                            mouse_y *= 10;
                            mouse_y += input_seq[i] - 0x30;
                            if (mouse_y < 10000) {
                                continue;
                            }
                        }
                        /* read button on/off 'M/m' */
                        if ((input_seq[i] == 0x4d || input_seq[i] == 0x6d)) {
                            input_seq_len2++;
                            mouse_button_press = (input_seq[i] == 0x4d) ? 1 : 0;

                            /* make mouse event */
                            input_rec_ptr->EventType = MOUSE_EVENT;
                            input_rec_ptr->Event.MouseEvent.dwMousePosition.X = mouse_x - 1;
                            input_rec_ptr->Event.MouseEvent.dwMousePosition.Y = mouse_y - 1;
                            input_rec_ptr->Event.MouseEvent.dwButtonState = 0;
                            input_rec_ptr->Event.MouseEvent.dwControlKeyState = ctrl_state;
                            input_rec_ptr->Event.MouseEvent.dwEventFlags = 0;

                            /* set mouse button state */
                            mouse_button_state &= ~0xfff80000; /* Wheel direction off */
                            mouse_button_no    &= ~0x8;        /* Ctrl key state off */
                            switch (mouse_button_no) {
                                case 0:  /* left button */
                                    if (mouse_button_press) {
                                        mouse_button_state |=  FROM_LEFT_1ST_BUTTON_PRESSED;
                                    } else {
                                        mouse_button_state &= ~FROM_LEFT_1ST_BUTTON_PRESSED;
                                    }
                                    break;
                                case 1:  /* middle button */
                                    if (mouse_button_press) {
                                        mouse_button_state |=  FROM_LEFT_2ND_BUTTON_PRESSED;
                                    } else {
                                        mouse_button_state &= ~FROM_LEFT_2ND_BUTTON_PRESSED;
                                    }
                                    break;
                                case 2:  /* right button */
                                    if (mouse_button_press) {
                                        mouse_button_state |=  RIGHTMOST_BUTTON_PRESSED;
                                    } else {
                                        mouse_button_state &= ~RIGHTMOST_BUTTON_PRESSED;
                                    }
                                    break;
                                case 64: /* vertical wheel up */
                                    input_rec_ptr->Event.MouseEvent.dwEventFlags |= MOUSE_WHEELED;
                                    mouse_button_state |= 0x00780000;
                                    break;
                                case 65: /* vertical wheel down */
                                    input_rec_ptr->Event.MouseEvent.dwEventFlags |= MOUSE_WHEELED;
                                    mouse_button_state |= 0xff880000;
                                    break;
                            }
                            input_rec_ptr->Event.MouseEvent.dwButtonState = mouse_button_state;

                            /* set mouse event flags */
                            if (old_mouse_x != mouse_x || old_mouse_y != mouse_y) {
                                /* PDCurses uses MOUSE_MOVED with a different definition from Win32 API */
                                /* input_rec_ptr->Event.MouseEvent.dwEventFlags |= MOUSE_MOVED; */
                                input_rec_ptr->Event.MouseEvent.dwEventFlags |= 0x1;
                            }
                            if (!peek_flag) {
                                old_mouse_x = mouse_x;
                                old_mouse_y = mouse_y;
                            }

                            /* succeeded */
                            ret_val = TRUE;
                        }
                        break;
                }
                break;
            }
            /* check failure */
            if (ret_val == FALSE) {
                DBG_PRINT("mouse input failed.\n");
            }
            /* consume vt escape sequence */
            if (!peek_flag && !consume_vt_input(hin, input_seq_len2)) {
                return FALSE;
            }
            return ret_val;
        }

        /* process other vt escape sequence */
        if (input_seq_len > 0) {
            /* search vt escape sequence input table */
            vt_input_table_len = sizeof(vt_input_table) / sizeof(struct vt_input);
            for (i = 0; i < vt_input_table_len; i++) {
                if (input_seq_len >= vt_input_table[i].seq_len &&
                    ((vt_input_table[i].modifier_index <= 0 &&
                      !strncmp(vt_input_table[i].seq, input_seq, vt_input_table[i].seq_len)) ||
                     (!strncmp(vt_input_table[i].seq, input_seq, vt_input_table[i].modifier_index) &&
                      !strncmp(vt_input_table[i].seq + vt_input_table[i].modifier_index + 1,
                               input_seq + vt_input_table[i].modifier_index + 1,
                               vt_input_table[i].seq_len - vt_input_table[i].modifier_index - 1)))) {

                    /* set key event */
                    set_key_event(input_rec_ptr,
                                  vt_input_table[i].vk,
                                  vt_input_table[i].vs,
                                  vt_input_table[i].uchar,
                                  ctrl_state | vt_input_table[i].ctrl);

                    /* consume vt escape sequence */
                    if (!peek_flag && !consume_vt_input(hin, vt_input_table[i].seq_len)) {
                        return FALSE;
                    }
                    return TRUE;
                }
            }
        }
    }
    return TRUE;
}
#endif
#endif
