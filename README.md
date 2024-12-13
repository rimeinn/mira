# Mira / 韻鏡

Mira 是 Rime 方案單元測試工具。

Mira 之名來自 mirror，義爲「鏡」，而「鏡」則截取自《韻鏡》一書之題，義爲 “reflect-on-rhyme (Rime)”。

## 示例

首先編寫方案的測試文件。測試文件格式爲 YAML（對 Rime 用戶來說應該不陌生）。

```yaml
# moran.test.yaml
schema: moran            # 被測試的方案 ID
source_dir: ".."         # 方案所在目錄
deploy:
  default:               # 出場設置
    tests:
      - { send: "a", assert: cand[0].text == "啊" and cand[0].comment == "⚡" }
      - { send: "bk", assert: cand[0].text == "報" and cand[0].comment == "⚡" }
      - { send: "sxey;", assert: cand[0].text == "三心二意" }
      - { send: "y;", assert: cand[0].text == "又" }

  emoji:                 # 繪文字
    options:
      emoji: true
    tests:
      - { send: "o", assert: cand[1].text == "😯" }
      - { send: "ou", assert: cand[1].text == "€" }
      - { send: "mzgo", assert: cand[1].text == "🇺🇸" }
      - { send: "y;", assert: committed == "又" }

  quick_code_indicator:  # 需要 patch 的情況
    patch:
      moran/quick_code_indicator: "🆒"
    tests:
      - { send: "a", assert: cand[0].comment == "🆒" }
```

然後在該文件上運行 `mira` 程序即可：

```
mira moran.test.yaml
```

## 構建

依賴如下外部庫：

- librime >= 1.12.0
- yaml-cpp >= 0.8.0
- argparse >= 3.1.0
- lua >= 5.4.7
