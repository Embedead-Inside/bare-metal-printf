/**
 * @file bmt_printf.c
 * @brief 超軽量 printf (bmt_printf) および snprintf (bmt_snprintf) の実装ファイル
 * 
 * @copyright Copyright (c) 2026 Embedead-Inside
 * @license SPDX-License-Identifier: MIT-0
 */

#include "bmt_printf.h"
#include <stdarg.h>
#include <stdint.h>

/**
 * @brief 1文字出力関数（プラットフォーム依存の下位レイヤー関数）
 * @details ターゲットSoCのUARTレジスタ等へ1文字を物理送信します。
 *          使用するSoCのハードウェア仕様に合わせて内部を実装してください。
 * 
 * @param[in] c 送信する1文字 (ASCIIコード)
 */
void uart_putchar(char c)
{
    /* 
     * [SoC移植時の注意点]
     * ここにターゲットSoCのUART送信レジスタへの書き込み処理を記述します。
     * 例:
     *     while (!(UART0_STATUS & UART_TX_READY));
     *     UART0_TX_DATA = c;
     */
}

/**
 * @brief 出力先状態管理コンテキスト構造体
 */
typedef struct {
    char *buf;      /**< 出力先バッファ (NULLの場合はUART直接出力) */
    size_t size;    /**< バッファ最大サイズ */
    size_t count;   /**< 出力した（または出力しようとした）文字数 */
} out_ctx_t;

/**
 * @brief 1文字出力抽象化関数 (インライン展開対象)
 * 
 * @param[in,out] ctx  出力先および状態を保持するコンテキスト構造体へのポインタ
 * @param[in]     c    送信する1文字 (ASCIIコード)
 */
static inline void put_char_ctx(out_ctx_t *ctx, char c)
{
    if (ctx->buf) {
        // 現在のカウントが (最大サイズ - 1) より小さい場合のみ、安全にバッファへ書き込む
        if (ctx->size > 0 && ctx->count < (ctx->size - 1)) {
            ctx->buf[ctx->count] = c;
        }
    } else {
        // printf モード: 直接UARTへ出力 (スタックバッファ不要)
        uart_putchar(c);
    }
    ctx->count++; // カウントはバッファの有無に関わらず常に進める（C99規格準拠）
}


/**
 * @brief 数値変換およびパディング制御コア関数
 * @details 数値を指定された基数で文字列へ変換し、符号の処理、幅指定のゼロ埋め、
 *          および逆順出力までを一括して行います。
 * 
 * @param[in,out] ctx       出力先および状態を保持するコンテキスト構造体へのポインタ
 * @param[in]     n         変換対象の数値
 * @param[in]     base      基数 (10: 10進数, 16: 16進数, 2: 2進数)
 * @param[in]     width     表示領域の最小幅 (ゼロ埋め適用幅)
 * @param[in]     is_signed 符号付きフラグ (1: 符号付き, 0: 符号なし)
 * @param[in]     upper     大文字化フラグ (1: A-F を大文字で出力, 0: 小文字)
 */
static void print_num(out_ctx_t *ctx, unsigned long n, int base, int width, int is_signed, int upper)
{
    char buf[32];
    int i = 0;
    int neg = (is_signed && (long)n < 0);

    // 負数の場合は絶対値に変換 (INT_MIN対策として unsigned long キャスト)
    if (neg) {
        n = (unsigned long)(-(long)n);
    }

    // 数値 -> 文字変換 (下位桁から順にバッファへ格納)
    do {
        char c = "0123456789abcdef"[n % base];
        buf[i++] = (upper && c >= 'a') ? (char)(c - 32) : c;
    } while (n /= base);

    // 負数符号の先行出力
    if (neg) {
        put_char_ctx(ctx, '-');
    }

    // 0埋めパディングの出力
    while (width > i + neg) {
        put_char_ctx(ctx, '0');
        width--;
    }

    // 逆順（上位桁から）に文字を出力
    while (i > 0) {
        put_char_ctx(ctx, buf[--i]);
    }
}

/**
 * @brief 内部コアフォーマット解析・出力エンジン
 * @details フォーマット文字列を走査・解析し、指定子に応じた変換処理を行って
 *          1文字出力抽象化関数 (put_char_ctx) 経由で文字を出力します。
 *          printf（UART直接出力）と snprintf（バッファ出力）の両方の
 *          コアロジックとして機能します。
 * 
 * @param[in,out] ctx  出力先および状態を保持するコンテキスト構造体へのポインタ
 * @param[in]     fmt  フォーマット文字列
 * @param[in]     args 可変長引数リスト (va_list)
 * @return int         出力した（または出力しようとした）合計文字数（終端 '\0' は除く）
 */
static int format_process(out_ctx_t *ctx, const char *fmt, va_list args)
{
    while (*fmt) {
        // '%' 以外の通常の文字は直接送信
        if (*fmt != '%') {
            put_char_ctx(ctx, *fmt++);
            continue;
        }

        fmt++; // '%' をスキップ

        // '%' 単体で文字列が終わっていた場合は終了
        if (*fmt == '\0') {
            put_char_ctx(ctx, '%');
            break;
        }

        int w = 0;
        // '0' から始まる幅指定（例: %08x の '0'）の解析
        if (*fmt == '0') {
            fmt++; // '0' をスキップ
            while (*fmt >= '0' && *fmt <= '9') {
                w = w * 10 + (*fmt - '0');
                fmt++;
            }
        }

        // 幅指定解析後に文字列が終わっていた場合は終了
        if (*fmt == '\0') {
            break;
        }

        // フォーマット指定子の解析と処理
        switch (*fmt) {
            case 'd':
            case 'i': // 符号付き10進数
                print_num(ctx, (unsigned long)va_arg(args, int), 10, w, 1, 0);
                break;
            case 'u':
                print_num(ctx, (unsigned long)va_arg(args, unsigned int), 10, w, 0, 0);
                break;
            case 'x':
                print_num(ctx, (unsigned long)va_arg(args, unsigned int), 16, w, 0, 0);
                break;
            case 'X':
                print_num(ctx, (unsigned long)va_arg(args, unsigned int), 16, w, 0, 1);
                break;
            case 'b':
                print_num(ctx, (unsigned long)va_arg(args, unsigned int), 2, w, 0, 0);
                break;
            case 'p':
                put_char_ctx(ctx, '0');
                put_char_ctx(ctx, 'x');
                // 16進数 0 埋め出力、C99標準の uintptr_t を使用しキャスト処理
                print_num(ctx, (unsigned long)(uintptr_t)va_arg(args, void *), 16, (int)(sizeof(void *) * 2), 0, 0);
                break;
            case 's': {
                const char *s = va_arg(args, const char *);
                if (!s) s = "(null)";
                while (*s) {
                    put_char_ctx(ctx, *s++);
                }
                break;
            }
            case 'c':
                put_char_ctx(ctx, (char)va_arg(args, int));
                break;
            case '%': // "%%" で '%' 自体を出力
                put_char_ctx(ctx, '%');
                break;
            default: // 未対応の指定子
                put_char_ctx(ctx, *fmt);
                break;
        }

        fmt++; // 次の文字へ進める
    }

    // snprintf の場合のみ終端 '\0' を付与
    if (ctx->buf && ctx->size > 0) {
        if (ctx->count < ctx->size) {
            ctx->buf[ctx->count] = '\0';
        } else {
            ctx->buf[ctx->size - 1] = '\0';
        }
    }

    return (int)ctx->count;
}

/**
 * @brief 簡易型フォーマット出力関数 (bmt_printf)
 * @details メモリ制約の厳しい環境向けに最適化された軽量な printf 実装です。
 *          通常の出力に加えて、ゼロ埋め幅指定 (例: `%08x`, `%02d`) に対応しています。
 * 
 * ### 対応フォーマット指定子:
 * - `%c` : 文字 (char)
 * - `%d`, `%i` : 符号付き10進数 (int)
 * - `%u` : 符号なし10進数 (unsigned int)
 * - `%x` : 符号なし16進数・小文字 (unsigned int) [例: `%08x`]
 * - `%X` : 符号なし16進数・大文字 (unsigned int) [例: `%04X`]
 * - `%b` : 符号なし2進数 (unsigned int)
 * - `%s` : 文字列 (const char *)
 * - `%p` : ポインタアドレス ("0x" + 8桁16進数表示)
 * - `%%` : '%' 文字自体の出力
 * 
 * @param[in] fmt フォーマット文字列
 * @param[in] ... 可変長引数
 */
void bmt_printf(const char *fmt, ...)
{
    out_ctx_t ctx = { .buf = NULL, .size = 0, .count = 0 };
    va_list args;
    va_start(args, fmt);
    (void)format_process(&ctx, fmt, args);
    va_end(args);
}

/**
 * @brief バッファ安全型フォーマット文字列作成関数
 * 
 * @param[out] buf  出力先バッファ
 * @param[in]  size バッファの最大サイズ (終端 '\0' を含む)
 * @param[in]  fmt  フォーマット文字列
 * @param[in]  ...  可変長引数
 * @return int      出力した(しようとした)文字数 (終端 '\0' は含まない)
 */
int bmt_snprintf(char *buf, int size, const char *fmt, ...)
{
    out_ctx_t ctx = { .buf = buf, .size = size, .count = 0 };
    va_list args;
    va_start(args, fmt);
    int ret = format_process(&ctx, fmt, args);
    va_end(args);
    return ret;
}
