# Mira / 韻鏡

Mira 是 Rime 方案單元測試工具。

Mira 之名來自 mirror，義爲「鏡」，而「鏡」則截取自《韻鏡》一書之題，義爲 “reflect-on-rhyme (Rime)”。

## 示例

首先編寫方案的測試文件，如 [luna_pinyin.test.yaml](https://github.com/rimeinn/mira/blob/master/examples/luna_pinyin.test.yaml)。測試文件格式爲 YAML（對 Rime 用戶來說應該不陌生）。

然後在該文件上運行 `mira` 程序即可：

```
mira luna_pinyin.test.yaml
```

如果涉及多次部署，建議使用 `-C` / `--cache-dir` 參數緩存產物以加速測試流程。

## 格式

YAML 文件格式見 [mira.schema.json](./spec/mira.schema.json)。

Lua 中可以使用的變量有：

- `cand`：數組
  - `cand[i].text` 第 i 個候選的候選詞，字符串類型
  - `cand[i].comment` 第 i 個候選的註釋，字符串類型
- `commit`：字符串或 `nil`，表示上屏的文字
- `preedit`：字符串或 `nil`，表示當前預編輯文字

## 注意事項

- `cand` 數組下標從 1 開始，且最多只取 100 個候選。
- 在一組 deploy 測試中，每個測試可以產生副作用。例如，如果第一個測試中讀寫了 userdb，那麼第二個測試中會受到 userdb 的影響。因此 `tests` 順序很重要。

## 構建

依賴如下外部庫：

- librime >= 1.10
- yaml-cpp >= 0.8
- argparse >= 3.0
- lua >= 5.4

本項目構建系統使用 Meson + Ninja，參考用法如下：

```bash
meson setup build --buildtype=release --prefer-static
cd build
ninja
```

其中 `--prefer-static` 是可選的：
- 如果提供了該參數，則會從 Mira 倉庫關聯的 librime submodule 中構建靜態庫。
- 如果不提供該參數，則會使用系統上已安裝的 librime。
