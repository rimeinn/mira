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

## 構建

依賴如下外部庫：

- librime >= 1.10
- yaml-cpp >= 0.8
- argparse >= 3.0
- lua >= 5.4
