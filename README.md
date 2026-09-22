# bare-metal-printf

組込みシステム（ベアメタル/SoC）向けの超軽量 `printf` ライブラリ。

## 特徴
- C99準拠
- コードサイズとスタック消費量を極限まで削減
- `%08x` や `%02d` などのゼロ埋め幅指定に対応
- レジスタやアドレスのデバッグに便利な2進数（`%b`）やポインタ（`%p`）出力を標準サポート
- `stdio.h` や `stdlib.h` などの標準Cライブラリに一切依存しない完全スタンドアロン仕様
- 動的メモリ割り当て（`malloc` 等）を使用しないため、ヒープメモリが不要

## サポートする識別子
メモリ制約の厳しい環境向けに最適化された最低限の仕様としています。浮動小数点数（`%f` など）はサポートしていません。

- `%c` : 文字 (char)
- `%d`, `%i` : 符号付き10進数 (int)
- `%u` : 符号なし10進数 (unsigned int)
- `%x` : 符号なし16進数・小文字 (unsigned int) [例: `%08x`]
- `%X` : 符号なし16進数・大文字 (unsigned int) [例: `%04X`]
- `%b` : 符号なし2進数 (unsigned int)
- `%s` : 文字列 (const char *)
- `%p` : ポインタアドレス ("0x" + 8桁16進数表示)
- `%%` : '%' 文字自体の出力

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
3. `bmt_printf("ADDR: %p, VAL: 0x%08X\n", addr, val);` のように呼び出します。

## ライセンス
このプロジェクトはMIT-0ライセンスのもとで公開されています。詳細は[LICENSE](LICENSE)ファイルをご覧ください。

<p align="right"><small><a name="note" class="muted-link">※一部AIによる生成・調整コードが含まれることがあります。</a></small></p>
