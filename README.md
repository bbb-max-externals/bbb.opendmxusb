# bbb.opendmxusb

ENTTEC **OPEN DMX USB**用のMax external。macOS 11以降、Apple Silicon / IntelのUniversal Binary。FTDIの仮想シリアルポート（VCP）を使い、250000 baud / 8N2、BREAK、MAB、start code 0 + DMXデータを専用スレッドから送る。

`bbb.artnet.controller` のDMX操作APIを引き継ぎ、ユニバース、IP、Art-Net、OSC設定を除いた。USB PROのパケット形式とRDMには非対応。シリアルポート名から機器の種類を判別する機能はない。

## インストールと開始

1. このパッケージを `~/Documents/Max 9/Packages/bbb.opendmxusb` に置く（開発時はシンボリックリンクでも可）。
2. externalを更新した場合は、作業を保存してMaxを再起動する。
3. `[bbb.opendmxusb]` を作成してヘルプを開く。
4. `[attrui @attr port]` を接続し、ENTTECに対応する `/dev/cu.usbserial-...` を選ぶ。選択すると非同期に接続する。
5. `channel 1 255` または `255 128 0` などを送る。

ポートは起動時に列挙する。USB抜き差し後は `ports` または `refresh` で選択肢を更新する。`port none` は切断、`close` は選択を保持したまま切断、`open` は選択中ポートに再接続する。自動で先頭ポートを開くことはない。保存した `@port` はパッチ再読込時に再接続する。

FTDI VCPポートがOSから見えている必要がある。D2XX専用構成のままでは使用できない。`ports` はBluetoothを含むOSのcalloutシリアルポート全体を列挙するため、OPEN DMX USBに対応するものを選ぶ。

## メッセージ

| メッセージ | 動作 |
| --- | --- |
| `list V...` / 数値リスト | 先頭チャンネルから更新し、modeに応じて送信要求 |
| `channel CH VALUE` | 1ch更新し、modeに応じて送信要求 |
| `set V...` | 先頭から格納。送信要求を出さない |
| `setchannel CH VALUE` | 1ch格納。送信要求を出さない |
| `set_offset CH V...` | 指定chから格納。送信要求を出さない |
| `bang` | 現在の値の送信を要求（全mode共通） |
| `dump` | 格納値を左outletにリスト出力。blackout前の値 |
| `ports` / `refresh` | 再列挙し、attruiの候補を更新。右outletに `ports PATH...` |
| `port PATH` / `open PATH` | ポート選択・非同期接続 |
| `open` | 選択ポートへ再接続 |
| `close` | 切断。ポート選択とDMXバッファを保持 |
| `status` | 右outletに `status CONNECTED SELECTED_PORT` |

値は0〜255にクリップし、小数は切り捨てる。非数値、NaN、無効な開始chはエラー。バッファは常に512chで、リスト末尾の超過は切り捨て、指定しなかった値は保持する。`num_channels` を減らしても後半の格納値は消えない。

## 属性

| 属性 | 既定値 | 意味 |
| --- | --- | --- |
| `port` | `none` | シリアルポート。attruiで選択可能 |
| `num_channels` | `512` | 送るチャンネル数、1〜512 |
| `origin` | `1` | channel / setchannel / set_offset の番号起点（0または1） |
| `blackout` | `0` | 次に作る送信フレームをゼロ化。格納値を保持 |
| `mode` | `4` | 下表参照 |
| `framerate` | `40.` | 送信頻度の上限、0.01〜44 fps |
| `verbose` | `0` | 接続状態をMax Consoleにも表示 |

| mode | 名称 | 動作 |
| --- | --- | --- |
| 0 | automatic | list / channelで送信要求 |
| 1 | bang | bang時のみ送信要求 |
| 2 | update | list / channelで送信要求（参照APIと同じくautomaticと同動作） |
| 3 | change | list / channelで前回要求と送信内容が異なる場合だけ要求 |
| 4 | forced | 更新がなくても連続送信。set系も次の周期に反映 |

OPEN DMXにはハードウェア側の自動再送がないため、Art-Net参照実装の既定値0から **forced=4に変更**している。イベント送信のmodeではDMXが途切れ、照明器具のタイムアウト動作になることがある。

全modeで送信は非同期。送信能力を超えた要求は最新1フレームにまとめるため、入力メッセージ数とフレーム数は一致しない。set系は既に要求済みのフレームを書き換えない。ポート切替・切断で未送信要求を破棄する。接続エラー後は明示的な `open` で再試行する。

`framerate` は上限で、USBドライバ、OSスケジューラ、ch数によって実効fpsは下がる。BREAK 200μs / MAB 20μsはホスト側の最小待機値。UARTの取りこぼしを避けるため出力キュー排出後にも1フレーム分+1msの余裕を取る。正確な電気的タイミングは実機計測が必要。

左outletはシリアル送信処理完了後のbangと `dump` のlist。これは灯体による受信確認ではない。右outletは `ports`、`opened PATH`、`closed`、`error DESCRIPTION`、`status`。Maxが長時間ブロックされた場合、通知のcatch-upは256bangまでに制限し、残数を `coalesced N` で通知する。

## ビルド

CMake、Xcode Command Line Tools、既存のCycling '74 `max-sdk-base` が必要。SDKやシリアルライブラリを自動ダウンロードしない。

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DMAX_SDK_BASE=/path/to/max-sdk-base
cmake --build build -j 4
```

このワークスペースでは隣の `bbb.utils/deps/min-api/max-sdk-base` を既定で使う。成果物は `externals/bbb.opendmxusb.mxo`。ローカル実行用にad-hoc署名する。

## 検証とログ

```sh
cmake -S . -B build -DOPEN_DMX_BUILD_TESTS=ON
cmake --build build -j 4
ctest --test-dir build --output-on-failure
```

テストは実機に送信せず、frame境界、blackoutでの格納値保持、送信mode、refresh変更、切断、再接続、終了処理を確認する。

Max上のAPIテストは `tests/max-smoke.maxpat` を開く。実在しないテストポート以外には自動接続しない。事前に `artifacts/` を作成すると結果を `artifacts/max-smoke.json` に保存する。結果の根拠と未検証事項は [docs/verification.md](docs/verification.md) を参照。

接続とエラーはmacOS Unified Logにも記録する（DMX値そのものは記録しない）。

```sh
/usr/bin/log show --last 10m --style compact \
  --predicate 'subsystem == "jp.2bit.bbb.opendmxusb"'
```

## 参照

- [ENTTEC OPEN DMX USB](https://www.enttec.com/product/dmx-usb-interfaces/open-dmx-usb/): 512chのUSB DMXインターフェース、FTDIドライバと開発資料。
- [ENTTEC datasheet](https://cdn.enttec.com/pdf/assets/70303/70303_OPEN_DMX_USB_DATASHEET.pdf): VCP / USB Serial Deviceとしての列挙。
- [Apple SerialPortSample](https://developer.apple.com/library/archive/samplecode/SerialPortSample/): termios / IOSSIOSPEEDによるシリアル設定。
- [Max SDK attributes](https://sdk.cdn.cycling74.com/max-sdk-8.2.0/group__attr.html): 属性登録とアクセス。
- [attrui](https://docs.cycling74.com/reference/attrui/): オブジェクト属性の編集。
- API参照元: `/Users/2bit/prog/maxproj/bbb.artnet/source/projects/bbb.artnet.controller/bbb.artnet.controller.cpp`。
