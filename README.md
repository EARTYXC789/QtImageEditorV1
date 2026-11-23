# Qt Image Editor V1 — Qt Widgets 桌面影像處理程式

本專案用 **C++17 + Qt Widgets** 實作「老師 Week 10 最低功能」與自訂 V1 操作需求。支援 **Qt 6** 或 **Qt 5.15**，可以使用 **Qt Creator** 開啟。

## 專案結構

```text
QtImageEditorV1/
├── CMakeLists.txt          # Qt Creator 建議開啟這個（CMake）
├── QtImageEditorV1.pro     # 學校若採 qmake，可改開啟這個
├── README.md
├── WEEK10_CHECKLIST.md
├── src/
│   ├── main.cpp            # QApplication 與主程式入口
│   ├── mainwindow.h/.cpp   # QMainWindow、選單、工具列、QAction、檔案 I/O
│   ├── imageview.h/.cpp    # QGraphicsView 畫布與滑鼠操作
│   └── exportdialog.h/.cpp # 自訂儲存／另存視窗
├── resources/
│   ├── icons.qrc           # Qt Resource System（qrc）
│   └── icons/              # 使用你上傳的 images.rar 中的 PNG 圖示
└── examples/
    └── sample.png          # 測試用圖片
```

## 在 Qt Creator 開啟（建議做法）

1. 安裝有 **Qt Widgets / Desktop** 套件的 Qt 5.15 或 Qt 6，以及對應 C++ 編譯器（Windows 可使用 MinGW 或 MSVC）。
2. 開啟 Qt Creator → `File` → `Open File or Project…` → 選擇 `CMakeLists.txt`。
3. 選擇可用的 Desktop Kit；如老師使用 qmake，可改選 `QtImageEditorV1.pro`。
4. 點 **Build**，再點 **Run**。
5. 點工具列「開啟影像」，選專案 `examples/sample.png` 測試。

命令列 CMake 也可以（前提是有安裝 Qt 開發套件）：

```bash
cmake -S . -B build
cmake --build build --config Release
```

> Windows 若無法找到 Qt：請在 Qt Creator 選擇有 Qt 的 Kit，或在 CMake 設定 Qt 安裝路徑，例如 `-DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/mingw_64"`。此處版本與路徑請依你的實際安裝調整。

## 操作說明

| 操作 | 方式 |
|---|---|
| 開啟影像 | `Ctrl + O` 或 File → 開啟 |
| 儲存影像 | `Ctrl + S`。開啟新圖片後第一次儲存，會先開啟儲存視窗，避免直接覆蓋原圖 |
| 另存新檔 | `Ctrl + Shift + S`。可選 PNG / JPEG，JPEG 可以選擇品質 |
| 離開 | `Ctrl + Q` 或 File → 離開 |
| 裁剪 | `C`、工具列「裁剪圖片」，左鍵在圖片上拖曳選取範圍，按右側「套用裁剪」；或「取消裁剪」 |
| 左右旋轉 | Image / Edit → 向左／右旋轉 90°；快捷鍵 `Ctrl + L` / `Ctrl + R` |
| 放大／縮小 | 滑鼠滾輪；或 `Ctrl + +` / `Ctrl + -` |
| 適合視窗 | `Ctrl + 0` |
| 平移畫布 | **按住滑鼠中鍵拖曳**（放大後較明顯） |

* **格式範圍**：開啟 PNG / JPEG；儲存 PNG / JPEG。
* **錯誤處理**：格式不支援、檔案讀取失敗、檔案輸出失敗，會顯示錯誤對話框；失敗時不會清除目前畫布。
* **狀態列**：回報開啟／儲存／裁剪／旋轉的結果，並顯示尺寸與縮放比例。
* **編輯確認**：裁剪範圍必須按「套用」才會改變影像；「取消」不改變畫布。裁剪中切換圖片時，會提醒本次裁剪會被取消。
* **來源圖片保護**：打開新圖片後的第一次 `Ctrl + S` 會先要求指定輸出檔案；不會自動覆蓋來源圖片。儲存時使用 QSaveFile 降低輸出失敗造成檔案損壞的風險。

## 目前明確不做（V1 範圍）

依照選擇 **2、5、6、7 不採用**：

- **不做**復原／重做（Undo / Redo）。裁剪與旋轉一旦套用，就不能在程式內回復上一步。
- **不做**「圖片已修改 `*`」標記及完整修改狀態管理。
- **不做**「關閉程式前未儲存」提醒；建議操作前先儲存。
- **不額外實作**「匯出後可恢復編輯狀態」或專案檔機制。儲存不會自動關閉視窗，但重新啟動程式不會載入上次編輯歷史。
- 不做多張圖片分頁、圖層、濾鏡、批次處理。

## 教師 Week 10 驗收對照

請見 `WEEK10_CHECKLIST.md`。本專案的 `Open`、`Save`、`Exit` 都由 QAction 建立，常用 QAction 會在 Menu 與 ToolBar 共用同一個物件。

## 重要備註

* `images.rar` 的 PNG 圖示已內建在 `resources/icons`，不需另外放在執行檔旁。
* 若 Qt Creator 沒有顯示中文，請確認原始碼以 UTF-8 儲存。
* **此環境沒有 Qt 開發標頭與編譯套件，因此程式碼與結構已進行靜態檢查，但尚未在此環境完成實際 GUI 編譯與操作測試。** 請在 Qt Creator 編譯驗收。
