# SetMouse

根据前台窗口的窗口名自动打开/关闭鼠标加速。

## 原理

每过一段时间检查前台窗口的窗口名，如果符合要求，则关闭鼠标加速，否则打开鼠标加速。

目前的检查方式为全文匹配，即，如果窗口名与 `main_window::matched_window_text` 完全相等，就关闭鼠标加速，否则打开鼠标加速。

## License

```
Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
See the LICENCE file in the repository root for full licence text.
```