/**
 * @file bmt_printf.h
 * @brief 組込みシステム向け超軽量 printf / snprintf ライブラリ (標準ヘッダー非依存)
 * @details コードサイズおよびメモリフットプリントを極限まで小さくした
 *          ベアメタル/SoC開発向けのフォーマット出力ライブラリです。
 * 
 * @copyright Copyright (c) 2026
 * @license SPDX-License-Identifier: MIT-0
 */

#ifndef BMT_PRINTF_H
#define BMT_PRINTF_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 1文字出力関数（プラットフォーム依存の下位レイヤー関数）
 * @details ターゲットSoCのUARTレジスタ等へ1文字を物理送信します。
 *          使用するSoCのハードウェア仕様に合わせて内部を実装してください。
 * 
 * @param[in] c 送信する1文字 (ASCIIコード)
 */
void uart_putchar(char c);

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
void bmt_printf(const char *fmt, ...);

/**
 * @brief バッファ安全型フォーマット文字列作成関数
 * 
 * @param[out] buf  出力先バッファ
 * @param[in]  size バッファの最大サイズ (終端 '\0' を含む)
 * @param[in]  fmt  フォーマット文字列
 * @param[in]  ...  可変長引数
 * @return int     出力した(しようとした)文字数 (終端 '\0' は含まない)
 */
int bmt_snprintf(char *buf, int size, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* BMT_PRINTF_H */
