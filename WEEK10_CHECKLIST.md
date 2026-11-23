# 老師 Week 10 最低功能驗收清單

| 老師要求 | 對應程式 | 驗收方法 |
|---|---|---|
| 建立 QMainWindow 主視窗框架 | `src/mainwindow.h/.cpp` 中 `MainWindow : public QMainWindow` | 執行後出現主視窗 |
| File 與 Image / Edit 類別 Menu | `MainWindow::createMenusAndToolbar()` | 上方可找到兩類選單 |
| 建立 Open、Save、Exit 等 QAction | `MainWindow::createActions()` | 選單可點選開啟、儲存、離開 |
| 常用 QAction 同時出現在 ToolBar | `MainWindow::createMenusAndToolbar()` | 工具列可找到與選單相同的開啟、儲存 QAction |
| centralWidget 作為未來影像工作區 | `MainWindow::createWorkArea()` / `setCentralWidget(m_view)` | 開啟圖片後顯示在中央區域 |
| QAction 有合適 Icon / Shortcut | `resources/icons.qrc`、`MainWindow::createActions()` | 圖示顯示，Ctrl+O / Ctrl+S 等快捷鍵可使用 |
| 程式可 Build、Run，操作結構清楚 | `CMakeLists.txt` 或 `.pro` | 在 Qt Creator 選用 Desktop Kit，Build & Run 後操作選單 |

## V1 自訂功能檢查

- [ ] 開啟一張 PNG，能正常顯示圖片與尺寸。
- [ ] 開啟一張 JPEG，方向與內容正確。
- [ ] 開啟不支援或損壞檔案時，顯示訊息且目前圖片不消失。
- [ ] 滾輪縮放、滑鼠中鍵平移、Ctrl+0 適合視窗可操作。
- [ ] 裁剪可框選、套用、取消；按取消後圖片不變。
- [ ] 旋轉 90° 後，顯示正確的圖片尺寸。
- [ ] Ctrl+S 第一次會出現儲存視窗。
- [ ] 另存可選 PNG 或 JPEG；格式與副檔名不一致時會提示。
- [ ] 輸出檔已存在時會提醒是否覆寫。
- [ ] 儲存路徑無效或沒有寫入權限時會提示錯誤。
- [ ] 開啟／儲存／裁剪後，StatusBar 有對應提示。
- [ ] Ctrl+Q 可以正常離開程式。
