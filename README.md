# bare-metal-printf

[![License: MIT-0](https://img.shields.io/badge/License-MIT--0-blue.svg)](https://spdx.org/licenses/MIT-0.html)
[![Language: C99](https://img.shields.io/badge/Language-C99-green.svg)](#)

## Bare-Metal printf / snprintf
組込みシステム（ベアメタル/SoC）向けの超軽量 `printf` & `snprintf` ライブラリ。

## 特徴
- C99準拠・完全スタンドアロン: stdio.h や stdlib.h などの標準Cライブラリに依存しません。
- スタックバッファゼロ設計: 内部の共通出力コンテキスト（out_ctx_t）により1文字出力を抽象化。bmt_printf 呼び出し時はスタック上にフォーマットバッファを一切確保せずUART直接出力します。
- snprintf による安全なバッファ書き込み: バッファオーバーフローを考慮したバッファサイズ制限・自動終端 \0 付与に対応。
- ゼロ埋め表示に対応: %08x や %02d などのゼロ埋め幅指定に対応。
- 組み込みデバッグ向けの拡張指定子: レジスタやアドレスの確認に便利な2進数（%b）やポインタ（%p）出力を標準サポート。
- 動的メモリ割り当て不使用: malloc 等を使用しないため、ヒープメモリが不要です。

## サポートする識別子
メモリ制約の厳しい環境向けに最適化された最低限の仕様としています。
浮動小数点数（`%f` など）のほか、左寄せ（`-` フラグ）や任意のフィールド幅指定（`%8d` などのスペース埋め）はサポートしていません。
（※ `%08x` や `%04X` のような、リストの例にある特定の固定桁数のゼロ埋め右寄せのみ動作します）
- `%c` : 文字 (char)
- `%d`, `%i` : 符号付き10進数 (int)
- `%u` : 符号なし10進数 (unsigned int)
- `%x` : 符号なし16進数・小文字 (unsigned int) [例: `%08x`]
- `%X` : 符号なし16進数・大文字 (unsigned int) [例: `%04X`]
- `%b` : 符号なし2進数 (unsigned int)
- `%s` : 文字列 (const char *)
- `%p` : ポインタアドレス ("0x" + 環境に応じた桁数の16進数表示。32bit環境なら8桁、64bit環境なら16桁)
- `%%` : '%' 文字自体の出力

## 提供API
   ```c
   /* UARTへの直接フォーマット出力 */
   void bmt_printf(const char *fmt, ...);
   
   /* バッファへの安全なフォーマット文字列作成 */
   int bmt_snprintf(char *buf, int size, const char *fmt, ...);
   ```

## 使い方
1. `bmt_printf.c` と `bmt_printf.h` をプロジェクトに追加します。
2. `bmt_printf.c` 内の `uart_putchar()` を、使用するSoCのUART送信処理に合わせて実装します。
   ```c
   /**
    * @brief 1文字送信（プラットフォーム依存関数）
    * @details ターゲットSoCのUARTレジスタ等へ1文字を出力します。
    *          環境に合わせて内部実装を書き換えてください。
    * 
    * @param[in] c 出力する文字
    */
   void uart_putchar(char c) {
       /* 
        * [SoC移植時の注意点]
        * 例:
        *     while (!(UART0_STATUS & UART_TX_READY));
        *     UART0_TX_DATA = c;
        */
   }
   ```
3. 各種フォーマット出力を呼び出します。
   ```c
   // 1. UARTへ直接出力
   bmt_printf("ADDR: %p, VAL: 0x%08X\n", addr, val);
   
   // 2. バッファへ文字列を作成 (バッファオーバーフロー安全)
   char buf[64];
   int len = bmt_snprintf(buf, sizeof(buf), "STATUS: %d", status);
   ```

## ライセンス
このプロジェクトはMIT-0ライセンスのもとで公開されています。詳細は[LICENSE](LICENSE)ファイルをご覧ください。
- 商用利用・個人利用を問わず、完全自由に使用できます。
- 著作権表示やライセンス文言の保持・記載義務すらありません。
- ソースコードの改変、流用、再配布、自社製品への組み込み等、制限なくご活用いただけます。

<p align="right"><small><a name="note" class="muted-link">※一部AIによる生成・調整コードが含まれることがあります。</a></small></p>
