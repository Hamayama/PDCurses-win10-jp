#ifdef PDC_WIN10_JP
/* for windows 10 jp */

#include "pdcwin.h"

/* interval structure */
struct interval
{
    int first;
    int last;
};

/* inner functions */
static int search_table(int ch, const struct interval *table, int size);
static int is_surrogate(int ch);
static int is_wide(int ch);
static int is_ambwidth(int ch);
static int is_emoji(int ch);
static int adjust_cur_x(int y, int x, int disp_width, int disp_height, chtype *scr_line_buf);
static int adjust_buf_and_len(int y, int x, WCHAR *buffer, int len, int disp_width, int disp_height);
static BOOL goto_yx(HANDLE hout, int y, int x);

/* api functions */
BOOL PDC_set_console_cursor_position(HANDLE hout, COORD cur_pos, COORD disp_size, chtype *scr_line_buf);
BOOL PDC_write_console_w(HANDLE hout, WCHAR *buffer, DWORD len, LPDWORD written_num_ptr,
                         COORD cur_pos, COORD disp_size, chtype *scr_line_buf);
BOOL PDC_write_console_w_with_attribute(HANDLE hout, WCHAR *buffer, DWORD len, LPDWORD written_num_ptr,
                                        WORD attr, COORD cur_pos, COORD disp_size, chtype *scr_line_buf);

/* search a character from interval table */
static int search_table(int ch, const struct interval *table, int size)
{
    int min = 0;
    int max = (size / sizeof(struct interval)) - 1;
    int mid;

    /* search a character from interval table */
    if (ch < table[0].first || ch > table[max].last) {
        return 0;
    }
    while (min <= max) {
        mid = (min + max) / 2;
        if (ch < table[mid].first) {
            max = mid - 1;
        } else if (ch > table[mid].last) {
            min = mid + 1;
        } else {
            return 1;
        }
    }
    return 0;
}

/* check if UTF-16 surrogate pair character */
static int is_surrogate(int ch)
{
    return (ch >= 0xd800 && ch <= 0xdfff);
}

/* check if wide character */
static int is_wide(int ch)
{
    /* generated from
       https://unicode.org/Public/UNIDATA/EastAsianWidth.txt
       (EastAsianWidth "F" "W" are selected) */
    static const struct interval wide_table[] = {
        { 0x1100, 0x115f }, { 0x231a, 0x231b }, { 0x2329, 0x232a },
        { 0x23e9, 0x23ec }, { 0x23f0, 0x23f0 }, { 0x23f3, 0x23f3 },
        { 0x25fd, 0x25fe }, { 0x2614, 0x2615 }, { 0x2648, 0x2653 },
        { 0x267f, 0x267f }, { 0x2693, 0x2693 }, { 0x26a1, 0x26a1 },
        { 0x26aa, 0x26ab }, { 0x26bd, 0x26be }, { 0x26c4, 0x26c5 },
        { 0x26ce, 0x26ce }, { 0x26d4, 0x26d4 }, { 0x26ea, 0x26ea },
        { 0x26f2, 0x26f3 }, { 0x26f5, 0x26f5 }, { 0x26fa, 0x26fa },
        { 0x26fd, 0x26fd }, { 0x2705, 0x2705 }, { 0x270a, 0x270b },
        { 0x2728, 0x2728 }, { 0x274c, 0x274c }, { 0x274e, 0x274e },
        { 0x2753, 0x2755 }, { 0x2757, 0x2757 }, { 0x2795, 0x2797 },
        { 0x27b0, 0x27b0 }, { 0x27bf, 0x27bf }, { 0x2b1b, 0x2b1c },
        { 0x2b50, 0x2b50 }, { 0x2b55, 0x2b55 }, { 0x2e80, 0x2e99 },
        { 0x2e9b, 0x2ef3 }, { 0x2f00, 0x2fd5 }, { 0x2ff0, 0x2ffb },
        { 0x3000, 0x303e }, { 0x3041, 0x3096 }, { 0x3099, 0x30ff },
        { 0x3105, 0x312f }, { 0x3131, 0x318e }, { 0x3190, 0x31e3 },
        { 0x31f0, 0x321e }, { 0x3220, 0x3247 }, { 0x3250, 0x4dbf },
        { 0x4e00, 0xa48c }, { 0xa490, 0xa4c6 }, { 0xa960, 0xa97c },
        { 0xac00, 0xd7a3 }, { 0xf900, 0xfaff }, { 0xfe10, 0xfe19 },
        { 0xfe30, 0xfe52 }, { 0xfe54, 0xfe66 }, { 0xfe68, 0xfe6b },
        { 0xff01, 0xff60 }, { 0xffe0, 0xffe6 }, { 0x16fe0, 0x16fe4 },
        { 0x16ff0, 0x16ff1 }, { 0x17000, 0x187f7 }, { 0x18800, 0x18cd5 },
        { 0x18d00, 0x18d08 }, { 0x1b000, 0x1b11e }, { 0x1b150, 0x1b152 },
        { 0x1b164, 0x1b167 }, { 0x1b170, 0x1b2fb }, { 0x1f004, 0x1f004 },
        { 0x1f0cf, 0x1f0cf }, { 0x1f18e, 0x1f18e }, { 0x1f191, 0x1f19a },
        { 0x1f200, 0x1f202 }, { 0x1f210, 0x1f23b }, { 0x1f240, 0x1f248 },
        { 0x1f250, 0x1f251 }, { 0x1f260, 0x1f265 }, { 0x1f300, 0x1f320 },
        { 0x1f32d, 0x1f335 }, { 0x1f337, 0x1f37c }, { 0x1f37e, 0x1f393 },
        { 0x1f3a0, 0x1f3ca }, { 0x1f3cf, 0x1f3d3 }, { 0x1f3e0, 0x1f3f0 },
        { 0x1f3f4, 0x1f3f4 }, { 0x1f3f8, 0x1f43e }, { 0x1f440, 0x1f440 },
        { 0x1f442, 0x1f4fc }, { 0x1f4ff, 0x1f53d }, { 0x1f54b, 0x1f54e },
        { 0x1f550, 0x1f567 }, { 0x1f57a, 0x1f57a }, { 0x1f595, 0x1f596 },
        { 0x1f5a4, 0x1f5a4 }, { 0x1f5fb, 0x1f64f }, { 0x1f680, 0x1f6c5 },
        { 0x1f6cc, 0x1f6cc }, { 0x1f6d0, 0x1f6d2 }, { 0x1f6d5, 0x1f6d7 },
        { 0x1f6eb, 0x1f6ec }, { 0x1f6f4, 0x1f6fc }, { 0x1f7e0, 0x1f7eb },
        { 0x1f90c, 0x1f93a }, { 0x1f93c, 0x1f945 }, { 0x1f947, 0x1f978 },
        { 0x1f97a, 0x1f9cb }, { 0x1f9cd, 0x1f9ff }, { 0x1fa70, 0x1fa74 },
        { 0x1fa78, 0x1fa7a }, { 0x1fa80, 0x1fa86 }, { 0x1fa90, 0x1faa8 },
        { 0x1fab0, 0x1fab6 }, { 0x1fac0, 0x1fac2 }, { 0x1fad0, 0x1fad6 },
        { 0x20000, 0x2fffd }, { 0x30000, 0x3fffd }
    };
    return search_table(ch, wide_table, sizeof(wide_table));
}

/* check if ambiguous width character */
static int is_ambwidth(int ch)
{
    /* generated from
       https://unicode.org/Public/UNIDATA/EastAsianWidth.txt
       (EastAsianWidth "A" is selected) */
    static const struct interval ambwidth_table[] = {
        { 0x00a1, 0x00a1 }, { 0x00a4, 0x00a4 }, { 0x00a7, 0x00a8 },
        { 0x00aa, 0x00aa }, { 0x00ad, 0x00ae }, { 0x00b0, 0x00b4 },
        { 0x00b6, 0x00ba }, { 0x00bc, 0x00bf }, { 0x00c6, 0x00c6 },
        { 0x00d0, 0x00d0 }, { 0x00d7, 0x00d8 }, { 0x00de, 0x00e1 },
        { 0x00e6, 0x00e6 }, { 0x00e8, 0x00ea }, { 0x00ec, 0x00ed },
        { 0x00f0, 0x00f0 }, { 0x00f2, 0x00f3 }, { 0x00f7, 0x00fa },
        { 0x00fc, 0x00fc }, { 0x00fe, 0x00fe }, { 0x0101, 0x0101 },
        { 0x0111, 0x0111 }, { 0x0113, 0x0113 }, { 0x011b, 0x011b },
        { 0x0126, 0x0127 }, { 0x012b, 0x012b }, { 0x0131, 0x0133 },
        { 0x0138, 0x0138 }, { 0x013f, 0x0142 }, { 0x0144, 0x0144 },
        { 0x0148, 0x014b }, { 0x014d, 0x014d }, { 0x0152, 0x0153 },
        { 0x0166, 0x0167 }, { 0x016b, 0x016b }, { 0x01ce, 0x01ce },
        { 0x01d0, 0x01d0 }, { 0x01d2, 0x01d2 }, { 0x01d4, 0x01d4 },
        { 0x01d6, 0x01d6 }, { 0x01d8, 0x01d8 }, { 0x01da, 0x01da },
        { 0x01dc, 0x01dc }, { 0x0251, 0x0251 }, { 0x0261, 0x0261 },
        { 0x02c4, 0x02c4 }, { 0x02c7, 0x02c7 }, { 0x02c9, 0x02cb },
        { 0x02cd, 0x02cd }, { 0x02d0, 0x02d0 }, { 0x02d8, 0x02db },
        { 0x02dd, 0x02dd }, { 0x02df, 0x02df }, { 0x0300, 0x036f },
        { 0x0391, 0x03a1 }, { 0x03a3, 0x03a9 }, { 0x03b1, 0x03c1 },
        { 0x03c3, 0x03c9 }, { 0x0401, 0x0401 }, { 0x0410, 0x044f },
        { 0x0451, 0x0451 }, { 0x2010, 0x2010 }, { 0x2013, 0x2016 },
        { 0x2018, 0x2019 }, { 0x201c, 0x201d }, { 0x2020, 0x2022 },
        { 0x2024, 0x2027 }, { 0x2030, 0x2030 }, { 0x2032, 0x2033 },
        { 0x2035, 0x2035 }, { 0x203b, 0x203b }, { 0x203e, 0x203e },
        { 0x2074, 0x2074 }, { 0x207f, 0x207f }, { 0x2081, 0x2084 },
        { 0x20ac, 0x20ac }, { 0x2103, 0x2103 }, { 0x2105, 0x2105 },
        { 0x2109, 0x2109 }, { 0x2113, 0x2113 }, { 0x2116, 0x2116 },
        { 0x2121, 0x2122 }, { 0x2126, 0x2126 }, { 0x212b, 0x212b },
        { 0x2153, 0x2154 }, { 0x215b, 0x215e }, { 0x2160, 0x216b },
        { 0x2170, 0x2179 }, { 0x2189, 0x2189 }, { 0x2190, 0x2199 },
        { 0x21b8, 0x21b9 }, { 0x21d2, 0x21d2 }, { 0x21d4, 0x21d4 },
        { 0x21e7, 0x21e7 }, { 0x2200, 0x2200 }, { 0x2202, 0x2203 },
        { 0x2207, 0x2208 }, { 0x220b, 0x220b }, { 0x220f, 0x220f },
        { 0x2211, 0x2211 }, { 0x2215, 0x2215 }, { 0x221a, 0x221a },
        { 0x221d, 0x2220 }, { 0x2223, 0x2223 }, { 0x2225, 0x2225 },
        { 0x2227, 0x222c }, { 0x222e, 0x222e }, { 0x2234, 0x2237 },
        { 0x223c, 0x223d }, { 0x2248, 0x2248 }, { 0x224c, 0x224c },
        { 0x2252, 0x2252 }, { 0x2260, 0x2261 }, { 0x2264, 0x2267 },
        { 0x226a, 0x226b }, { 0x226e, 0x226f }, { 0x2282, 0x2283 },
        { 0x2286, 0x2287 }, { 0x2295, 0x2295 }, { 0x2299, 0x2299 },
        { 0x22a5, 0x22a5 }, { 0x22bf, 0x22bf }, { 0x2312, 0x2312 },
        { 0x2460, 0x24e9 }, { 0x24eb, 0x254b }, { 0x2550, 0x2573 },
        { 0x2580, 0x258f }, { 0x2592, 0x2595 }, { 0x25a0, 0x25a1 },
        { 0x25a3, 0x25a9 }, { 0x25b2, 0x25b3 }, { 0x25b6, 0x25b7 },
        { 0x25bc, 0x25bd }, { 0x25c0, 0x25c1 }, { 0x25c6, 0x25c8 },
        { 0x25cb, 0x25cb }, { 0x25ce, 0x25d1 }, { 0x25e2, 0x25e5 },
        { 0x25ef, 0x25ef }, { 0x2605, 0x2606 }, { 0x2609, 0x2609 },
        { 0x260e, 0x260f }, { 0x261c, 0x261c }, { 0x261e, 0x261e },
        { 0x2640, 0x2640 }, { 0x2642, 0x2642 }, { 0x2660, 0x2661 },
        { 0x2663, 0x2665 }, { 0x2667, 0x266a }, { 0x266c, 0x266d },
        { 0x266f, 0x266f }, { 0x269e, 0x269f }, { 0x26bf, 0x26bf },
        { 0x26c6, 0x26cd }, { 0x26cf, 0x26d3 }, { 0x26d5, 0x26e1 },
        { 0x26e3, 0x26e3 }, { 0x26e8, 0x26e9 }, { 0x26eb, 0x26f1 },
        { 0x26f4, 0x26f4 }, { 0x26f6, 0x26f9 }, { 0x26fb, 0x26fc },
        { 0x26fe, 0x26ff }, { 0x273d, 0x273d }, { 0x2776, 0x277f },
        { 0x2b56, 0x2b59 }, { 0x3248, 0x324f }, { 0xe000, 0xf8ff },
        { 0xfe00, 0xfe0f }, { 0xfffd, 0xfffd }, { 0x1f100, 0x1f10a },
        { 0x1f110, 0x1f12d }, { 0x1f130, 0x1f169 }, { 0x1f170, 0x1f18d },
        { 0x1f18f, 0x1f190 }, { 0x1f19b, 0x1f1ac }, { 0xe0100, 0xe01ef },
        { 0xf0000, 0xffffd }, { 0x100000, 0x10fffd }
    };
    return search_table(ch, ambwidth_table, sizeof(ambwidth_table));
}

/* check if emoji character */
static int is_emoji(int ch)
{
    /* generated from
       https://unicode.org/Public/UNIDATA/emoji/emoji-data.txt
       (excluding less than U+1000) */
    static const struct interval emoji_table[] = {
        { 0x203c, 0x203c }, { 0x2049, 0x2049 }, { 0x2122, 0x2122 },
        { 0x2139, 0x2139 }, { 0x2194, 0x2199 }, { 0x21a9, 0x21aa },
        { 0x231a, 0x231b }, { 0x2328, 0x2328 }, { 0x23cf, 0x23cf },
        { 0x23e9, 0x23f3 }, { 0x23f8, 0x23fa }, { 0x24c2, 0x24c2 },
        { 0x25aa, 0x25ab }, { 0x25b6, 0x25b6 }, { 0x25c0, 0x25c0 },
        { 0x25fb, 0x25fe }, { 0x2600, 0x2604 }, { 0x260e, 0x260e },
        { 0x2611, 0x2611 }, { 0x2614, 0x2615 }, { 0x2618, 0x2618 },
        { 0x261d, 0x261d }, { 0x2620, 0x2620 }, { 0x2622, 0x2623 },
        { 0x2626, 0x2626 }, { 0x262a, 0x262a }, { 0x262e, 0x262f },
        { 0x2638, 0x263a }, { 0x2640, 0x2640 }, { 0x2642, 0x2642 },
        { 0x2648, 0x2653 }, { 0x265f, 0x2660 }, { 0x2663, 0x2663 },
        { 0x2665, 0x2666 }, { 0x2668, 0x2668 }, { 0x267b, 0x267b },
        { 0x267e, 0x267f }, { 0x2692, 0x2697 }, { 0x2699, 0x2699 },
        { 0x269b, 0x269c }, { 0x26a0, 0x26a1 }, { 0x26a7, 0x26a7 },
        { 0x26aa, 0x26ab }, { 0x26b0, 0x26b1 }, { 0x26bd, 0x26be },
        { 0x26c4, 0x26c5 }, { 0x26c8, 0x26c8 }, { 0x26ce, 0x26cf },
        { 0x26d1, 0x26d1 }, { 0x26d3, 0x26d4 }, { 0x26e9, 0x26ea },
        { 0x26f0, 0x26f5 }, { 0x26f7, 0x26fa }, { 0x26fd, 0x26fd },
        { 0x2702, 0x2702 }, { 0x2705, 0x2705 }, { 0x2708, 0x270d },
        { 0x270f, 0x270f }, { 0x2712, 0x2712 }, { 0x2714, 0x2714 },
        { 0x2716, 0x2716 }, { 0x271d, 0x271d }, { 0x2721, 0x2721 },
        { 0x2728, 0x2728 }, { 0x2733, 0x2734 }, { 0x2744, 0x2744 },
        { 0x2747, 0x2747 }, { 0x274c, 0x274c }, { 0x274e, 0x274e },
        { 0x2753, 0x2755 }, { 0x2757, 0x2757 }, { 0x2763, 0x2764 },
        { 0x2795, 0x2797 }, { 0x27a1, 0x27a1 }, { 0x27b0, 0x27b0 },
        { 0x27bf, 0x27bf }, { 0x2934, 0x2935 }, { 0x2b05, 0x2b07 },
        { 0x2b1b, 0x2b1c }, { 0x2b50, 0x2b50 }, { 0x2b55, 0x2b55 },
        { 0x3030, 0x3030 }, { 0x303d, 0x303d }, { 0x3297, 0x3297 },
        { 0x3299, 0x3299 }, { 0x1f004, 0x1f004 }, { 0x1f0cf, 0x1f0cf },
        { 0x1f170, 0x1f171 }, { 0x1f17e, 0x1f17f }, { 0x1f18e, 0x1f18e },
        { 0x1f191, 0x1f19a }, { 0x1f1e6, 0x1f1ff }, { 0x1f201, 0x1f202 },
        { 0x1f21a, 0x1f21a }, { 0x1f22f, 0x1f22f }, { 0x1f232, 0x1f23a },
        { 0x1f250, 0x1f251 }, { 0x1f300, 0x1f321 }, { 0x1f324, 0x1f393 },
        { 0x1f396, 0x1f397 }, { 0x1f399, 0x1f39b }, { 0x1f39e, 0x1f3f0 },
        { 0x1f3f3, 0x1f3f5 }, { 0x1f3f7, 0x1f4fd }, { 0x1f4ff, 0x1f53d },
        { 0x1f549, 0x1f54e }, { 0x1f550, 0x1f567 }, { 0x1f56f, 0x1f570 },
        { 0x1f573, 0x1f57a }, { 0x1f587, 0x1f587 }, { 0x1f58a, 0x1f58d },
        { 0x1f590, 0x1f590 }, { 0x1f595, 0x1f596 }, { 0x1f5a4, 0x1f5a5 },
        { 0x1f5a8, 0x1f5a8 }, { 0x1f5b1, 0x1f5b2 }, { 0x1f5bc, 0x1f5bc },
        { 0x1f5c2, 0x1f5c4 }, { 0x1f5d1, 0x1f5d3 }, { 0x1f5dc, 0x1f5de },
        { 0x1f5e1, 0x1f5e1 }, { 0x1f5e3, 0x1f5e3 }, { 0x1f5e8, 0x1f5e8 },
        { 0x1f5ef, 0x1f5ef }, { 0x1f5f3, 0x1f5f3 }, { 0x1f5fa, 0x1f64f },
        { 0x1f680, 0x1f6c5 }, { 0x1f6cb, 0x1f6d2 }, { 0x1f6d5, 0x1f6d7 },
        { 0x1f6e0, 0x1f6e5 }, { 0x1f6e9, 0x1f6e9 }, { 0x1f6eb, 0x1f6ec },
        { 0x1f6f0, 0x1f6f0 }, { 0x1f6f3, 0x1f6fc }, { 0x1f7e0, 0x1f7eb },
        { 0x1f90c, 0x1f93a }, { 0x1f93c, 0x1f945 }, { 0x1f947, 0x1f978 },
        { 0x1f97a, 0x1f9cb }, { 0x1f9cd, 0x1f9ff }, { 0x1fa70, 0x1fa74 },
        { 0x1fa78, 0x1fa7a }, { 0x1fa80, 0x1fa86 }, { 0x1fa90, 0x1faa8 },
        { 0x1fab0, 0x1fab6 }, { 0x1fac0, 0x1fac2 }, { 0x1fad0, 0x1fad6 }
    };
    return search_table(ch, emoji_table, sizeof(emoji_table));
}

/* adjust cursor-x position */
static int adjust_cur_x(int y, int x, int disp_width, int disp_height, chtype *scr_line_buf)
{
    int i;
    int new_x;
    int ch;

    /* check arguments */
    if (x < 0 || x >= disp_width || y < 0 || y >= disp_height) {
        return x;
    }
    if (scr_line_buf == NULL) {
        return x;
    }

    /* adjust cursor-x position */
    new_x = 0;
    for (i = 0; i < x; i++) {
        ch = scr_line_buf[i] & A_CHARTEXT;
        /* zero-width-space character (U+200B) */
        if (ch == 0x200b) {
            continue;
        }
        new_x++;
        /* surrogate pair character */
        if (is_surrogate(ch)) {
            /* nop */
        /* wide character */
        /* ambiguous width character */
        /* emoji character */
        } else if (is_wide(ch) ||
                   (is_ambwidth(ch) && pdc_ambiguous_width > 1) ||
                   (is_emoji(ch) && pdc_emoji_width > 1)) {
            new_x++;
        }
    }
    return new_x;
}

/* adjust buffer and length */
/* (the part of string that exceeds display width is dropped) */
static int adjust_buf_and_len(int y, int x, WCHAR *buffer, int len, int disp_width, int disp_height)
{
    int i, j;
    int new_x;
    int new_len;
    int ch;

    /* check arguments */
    if (x < 0 || x >= disp_width || y < 0 || y >= disp_height) {
        return 0;
    }

    /* don't write bottom right corner of windows console */
#ifdef PDC_RIGHT_MARGIN
    /* consider right margin */
    if (y == disp_height - 1 && PDC_RIGHT_MARGIN == 0) {
        disp_width--;
    }
#else
    if (y == disp_height - 1) {
        disp_width--;
    }
#endif

    /* adjust cursor-x position */
    /* new_x = adjust_cur_x(y, x, disp_width, disp_height, scr_line_buf); */
    new_x = x;

    /* check start position */
    if (new_x >= disp_width) {
        return 0;
    }

    /* adjust buffer and length */
    new_len = 0;
    for (i = 0; i < len; i++) {
        ch = buffer[i];
        /* zero-width-space character (U+200B) */
        if (ch == 0x200b) {
            len--;
            for (j = i; j < len; j++) {
                buffer[j] = buffer[j + 1];
            }
            continue;
        }
        new_len++;
        new_x++;
        /* surrogate pair character */
        if (is_surrogate(ch)) {
            /* nop */
        /* wide character */
        /* ambiguous width character */
        /* emoji character */
        } else if (is_wide(ch) ||
                   (is_ambwidth(ch) && pdc_ambiguous_width > 1) ||
                   (is_emoji(ch) && pdc_emoji_width > 1)) {
            new_x++;
            /* check half size overflow of wide character */
            if (new_x > disp_width) {
                buffer[i] = 0x0020;
                break;
            }
        }
        if (new_x >= disp_width) {
            break;
        }
    }
    return new_len;
}

/* set cursor position */
static BOOL goto_yx(HANDLE hout, int y, int x)
{
    COORD coord;
    CONSOLE_SCREEN_BUFFER_INFO scrn_info;

    /* workaround for cursor position problem of windows 10
       ( https://github.com/microsoft/terminal/issues/724 ) */
    GetConsoleScreenBufferInfo(hout, &scrn_info);
    coord.X = 0;
    coord.Y = scrn_info.dwCursorPosition.Y;
    SetConsoleCursorPosition(hout, coord);
    coord.X = x;
    coord.Y = y;
    return SetConsoleCursorPosition(hout, coord);
}

/* set cursor position
    limitations:
     - not compatible with Win32 API (SetConsoleCursorPosition).
*/
BOOL PDC_set_console_cursor_position(HANDLE hout, COORD cur_pos, COORD disp_size, chtype *scr_line_buf)
{
    int x = cur_pos.X;
    int y = cur_pos.Y;
    int disp_width  = disp_size.X;
    int disp_height = disp_size.Y;
    return goto_yx(hout, y, adjust_cur_x(y, x, disp_width, disp_height, scr_line_buf));
}

/* write buffer to console
    limitations:
     - not compatible with Win32 API (WriteConsoleW).
*/
BOOL PDC_write_console_w(HANDLE hout, WCHAR *buffer, DWORD len, LPDWORD written_num_ptr,
                         COORD cur_pos, COORD disp_size, chtype *scr_line_buf)
{
    int x = cur_pos.X;
    int y = cur_pos.Y;
    int disp_width  = disp_size.X;
    int disp_height = disp_size.Y;
    int i;
    int len1;
    int x1, x2;
    WCHAR space[4] = {0x0020, 0x0020, 0, 0};
    WCHAR *buffer1;
    int ch;
    DWORD written;

    /* adjust cursor-x position */
    x1 = adjust_cur_x(y, x, disp_width, disp_height, scr_line_buf);

    /* adjust buffer and length */
    len = adjust_buf_and_len(y, x1, buffer, len, disp_width, disp_height);

    /* check buffer length */
    if (len <= 0) {
        *written_num_ptr = 0;
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    /* write buffer to console */
    len1 = 0;
    x2 = x1;
    buffer1 = buffer;
    for (i = 0; i < (int)len; i++) {
        ch = buffer[i];
        len1++;
        x2++;
        /* surrogate pair character */
        if (is_surrogate(ch)) {
            /* nop */
        /* wide character */
        } else if (is_wide(ch)) {
            x2++;
        /* ambiguous width character */
        /* emoji character */
        } else if ((is_ambwidth(ch) && pdc_ambiguous_width > 1) ||
                   (is_emoji(ch) && pdc_emoji_width > 1)) {
            x2++;
            /* clear 2 cells for half size font */
            goto_yx(hout, y, x2 - 2);
            WriteConsoleW(hout, space, 2, &written, NULL);
            goto_yx(hout, y, x1);
            WriteConsoleW(hout, buffer1, len1, &written, NULL);
            buffer1 += len1;
            len1 = 0;
            x1 = x2;
        }
    }
    if (len1 > 0) {
        goto_yx(hout, y, x1);
        WriteConsoleW(hout, buffer1, len1, &written, NULL);
    }

    /* set return value */
    *written_num_ptr = len;
    return TRUE;
}

/* write buffer to console
    limitations:
     - not compatible with Win32 API (WriteConsoleOutputW).
*/
BOOL PDC_write_console_w_with_attribute(HANDLE hout, WCHAR *buffer, DWORD len, LPDWORD written_num_ptr,
                                        WORD attr, COORD cur_pos, COORD disp_size, chtype *scr_line_buf)
{
    int x = cur_pos.X;
    int y = cur_pos.Y;
    int disp_width  = disp_size.X;
    int disp_height = disp_size.Y;
    int i;
    int len1;
    int x1, x2;
    WCHAR space[4] = {0x0020, 0x0020, 0, 0};
    WCHAR *buffer1;
    int ch;
    COORD coord;
    DWORD written;

    /* adjust cursor-x position */
    x1 = adjust_cur_x(y, x, disp_width, disp_height, scr_line_buf);

    /* adjust buffer and length */
    len = adjust_buf_and_len(y, x1, buffer, len, disp_width, disp_height);

    /* check buffer length */
    if (len <= 0) {
        *written_num_ptr = 0;
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    /* write attribute to console */
    len1 = adjust_cur_x(y, x + len, disp_width, disp_height, scr_line_buf) - x1;
    coord.X = x1;
    coord.Y = y;
    FillConsoleOutputAttribute(hout, attr, len1, coord, &written);

    /* write buffer to console */
    len1 = 0;
    x2 = x1;
    buffer1 = buffer;
    for (i = 0; i < (int)len; i++) {
        ch = buffer[i];
        len1++;
        x2++;
        /* surrogate pair character */
        if (is_surrogate(ch)) {
            /* nop */
        /* wide character */
        } else if (is_wide(ch)) {
            x2++;
        /* ambiguous width character */
        /* emoji character */
        } else if ((is_ambwidth(ch) && pdc_ambiguous_width > 1) ||
                   (is_emoji(ch) && pdc_emoji_width > 1)) {
            x2++;
            /* clear 2 cells for half size font */
            coord.X = x2 - 2;
            coord.Y = y;
            WriteConsoleOutputCharacterW(hout, space, 2, coord, &written);
            coord.X = x1;
            coord.Y = y;
            WriteConsoleOutputCharacterW(hout, buffer1, len1, coord, &written);
            buffer1 += len1;
            len1 = 0;
            x1 = x2;
        }
    }
    if (len1 > 0) {
        coord.X = x1;
        coord.Y = y;
        WriteConsoleOutputCharacterW(hout, buffer1, len1, coord, &written);
    }

    /* set return value */
    *written_num_ptr = len;
    return TRUE;
}
#endif
