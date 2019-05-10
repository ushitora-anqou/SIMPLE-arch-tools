# 使い方

## 原則

- 雰囲気で使う。
- とりあえず `examples/*.s` を見てみる。
- 分からなければソースコードを見る。
- 面倒なら anqou に聞く。

## ビルド

`src/` の中に `cd` で入ってから `make` するとビルドされ、
`macro`, `assembler`, `emulator`, `debugger` ができます。

## 基本的な使い方

`cat ../examples/fiblt55.s | ./macro | ./assembler -mif`

パイプのところを1段ずつ外して出力を確かめるといいさ。

デバッガを直接使うのは手間なので次のようにしてください： `./aqdb.sh ../examples/fiblt55.s`

なおデバッガは`./macro`を通した結果で表示されますが、仕様です。改良募集。

## `macro` で使える命令

随時増えます。一般論としてドキュメントが整備されるのは実装が行われて
少し経ってからなので、最新の仕様が知りたいときはソースコードを読みましょう。
あるいは自分で実装しましょう。

具体的な使い方は `examples/*.s` とか `src/test.sh` とかを調べると良いかも知れません。

凡例：

- `macroに食わせる入力`
    - `macroが吐く出力 or assemblerに食わせる入力`
    - 注意など

一般的に`Rn/Rm`はレジスタをさし`imm`は即値をさし、`label`はラベルをさします。
`imm`には10進数のほか`0x`を先頭につけることで16進数が使えます。

- `ADD Rn, Rm`
    - `ADD Rn, Rm`
    - `SUB`, `CMP`, `AND`, `OR`, `XOR`, `CMP`, `MOV`について同様。
- `ADD Rn, imm`
    - `ADDI Rn, imm`
    - `CMPI`について同様。
- `MOV Rn, imm`
    - `LI Rn, imm`
- `MOV Rn, [Rm + imm]`
    - `LD Rn, imm(Rm)`
    - `imm`は省略可能。`+`の代わりに`-`も使用可能。
- `MOV [Rn + imm], Rm`
    - `ST Rm, imm(Rn)`
    - `imm`は省略可能。`+`の代わりに`-`も使用可能。
- `Rn = Rm`
    - `MOV Rn, Rm`
- `[Rn + imm] = Rm`
    - `ST Rm, imm(Rn)`
- `Rn = [Rm + imm]`
    - `LD Rn, imm(Rm)`
- `SLL Rn, imm`
    - `SLL Rn, imm`
    - `SLR`, `SRL`, `SRA`について同様。
- `IN Rn`
    - `IN Rn`
    - `OUT`について同様。
- `HLT`
    - `HLT`
- `JMP label`
    - `B labelへの相対アドレス`
- `goto label`
    - `B labelへの相対アドレス`
- `JE label`
    - `BE labelへの相対アドレス`
- `JL label`
    - `BL labelへの相対アドレス`
- `JLE label`
    - `BLE labnelへの相対アドレス`
- `JNE label`
    - `BNE labelへの相対アドレス`
- `if Rn == Rm then goto label`
    - `CMP Rn, Rm（改行）BE labelへの相対アドレス`
    - `Rn == Rm` という条件が成立するときに`label`へ飛ぶようなアセンブリを出力する。なお`!=`の他`<`, `<=`についても同様。
- `define name body`
    - なし。
    - マクロを定義できる。すなわち、この行よりあとの全ての`name`は`body`に置換される。`examples/fibonmem_macro.s`などを参照のこと。

## バグかな？と思ったら

Issueをぶっ立てるか anqou に相談するかソースコード読んで自分で解決するかしてください。
自分で解決した場合はPull Requestを送ってもらえると私を含めSIMPLE-arch-toolsを使う
全ての人が助かります。
