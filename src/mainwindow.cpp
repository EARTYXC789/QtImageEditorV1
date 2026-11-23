#include "mainwindow.h"
#include "imageview.h"
#include "exportdialog.h"

#include <QAction>
#include <QApplication>
#include <QDockWidget>
#include <QDir>
#include <QIcon>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QGraphicsView>
#include <QImageReader>
#include <QImageWriter>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QSaveFile>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QTransform>
#include <QFrame>
#include <QKeySequence>
#include <QSizePolicy>

namespace {
QToolButton *makeToolButton(QAction *action, QWidget *parent)
{
    auto *button = new QToolButton(parent);
    button->setDefaultAction(action);
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    button->setIconSize(QSize(22, 22));
    button->setMinimumHeight(37);
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    return button;
}
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("Qt Image Editor V1 — 未開啟影像"));
    setWindowIcon(QIcon(QStringLiteral(":/icons/editor.png")));
    resize(1120, 750);
    setMinimumSize(820, 540);

    setStyleSheet(QString::fromUtf8(R"(
        QMainWindow { background: #f3f6fb; }
        QMenuBar { background: #ffffff; padding: 4px; }
        QMenuBar::item { padding: 6px 10px; }
        QMenuBar::item:selected { background: #e8f3fd; border-radius: 4px; }
        QToolBar { background: #ffffff; border-bottom: 1px solid #dce4ed; spacing: 5px; padding: 6px; }
        QToolBar QToolButton { padding: 5px; border-radius: 5px; }
        QToolBar QToolButton:hover { background: #e9f2ff; }
        QToolBar QToolButton:checked { background: #cfe5fd; }
        QDockWidget { font-weight: bold; color: #24405d; }
        QDockWidget > QWidget { background: white; }
        QToolButton { padding: 7px; text-align: left; border: 1px solid transparent; border-radius: 6px; }
        QToolButton:hover { background: #e9f2ff; }
        QToolButton:checked { background: #d8ebfc; border-color: #8dc4f2; }
        QPushButton { background: #eff4fa; border: 1px solid #cad6e3; padding: 8px 12px; border-radius: 6px; }
        QPushButton:hover { background: #e0ecfa; }
        QPushButton:disabled { color: #a0a9b5; background: #f5f7fa; }
        QPushButton#applyCrop { background: #166ac6; color: white; border-color: #166ac6; }
        QPushButton#applyCrop:hover { background: #1156a1; }
        QPushButton#applyCrop:disabled { color: #9ca3af; background: #e9edf3; border-color: #d6dde6; }
        QLabel#sectionTitle { color: #27425c; font-weight: bold; }
        QLabel#helper { color: #6b7c90; }
        QStatusBar { background: #ffffff; border-top: 1px solid #dde6ef; }
    )"));

    createActions();
    createMenusAndToolbar();
    createWorkArea();
    updateActionStates();
    statusBar()->showMessage(QStringLiteral("準備完成，請先開啟圖片。"));
}

void MainWindow::createActions()
{
    m_openAction = new QAction(QIcon(QStringLiteral(":/icons/open.png")),
                               QStringLiteral("開啟影像(&O)…"), this);
    m_openAction->setShortcut(QKeySequence::Open);
    m_openAction->setStatusTip(QStringLiteral("開啟 PNG 或 JPEG 圖片"));
    connect(m_openAction, &QAction::triggered, this, &MainWindow::openImage);

    m_saveAction = new QAction(QIcon(QStringLiteral(":/icons/save.png")),
                               QStringLiteral("儲存(&S)"), this);
    m_saveAction->setShortcut(QKeySequence::Save);
    m_saveAction->setStatusTip(QStringLiteral("首次儲存時開啟輸出視窗，避免直接覆蓋來源檔"));
    connect(m_saveAction, &QAction::triggered, this, &MainWindow::saveImage);

    m_saveAsAction = new QAction(QIcon(QStringLiteral(":/icons/asave.png")),
                                 QStringLiteral("另存新檔…"), this);
    m_saveAsAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+S")));
    connect(m_saveAsAction, &QAction::triggered, this, &MainWindow::saveImageAs);

    m_exitAction = new QAction(QStringLiteral("離開(&X)"), this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    connect(m_exitAction, &QAction::triggered, this, &QWidget::close);

    m_cropAction = new QAction(QIcon(QStringLiteral(":/icons/cut.png")),
                               QStringLiteral("裁剪圖片"), this);
    m_cropAction->setShortcut(QKeySequence(QStringLiteral("C")));
    m_cropAction->setCheckable(true);
    connect(m_cropAction, &QAction::triggered, this, &MainWindow::startCrop);

    m_rotateLeftAction = new QAction(QStringLiteral("向左旋轉 90°"), this);
    m_rotateLeftAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+L")));
    connect(m_rotateLeftAction, &QAction::triggered, this, &MainWindow::rotateLeft);

    m_rotateRightAction = new QAction(QStringLiteral("向右旋轉 90°"), this);
    m_rotateRightAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+R")));
    connect(m_rotateRightAction, &QAction::triggered, this, &MainWindow::rotateRight);

    m_zoomInAction = new QAction(QStringLiteral("放大"), this);
    m_zoomInAction->setShortcut(QKeySequence(QStringLiteral("Ctrl++")));
    m_zoomInAction->setStatusTip(QStringLiteral("放大影像（也可滾動滑鼠滾輪）"));

    m_zoomOutAction = new QAction(QStringLiteral("縮小"), this);
    m_zoomOutAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+-")));

    m_fitAction = new QAction(QStringLiteral("適合視窗"), this);
    m_fitAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+0")));
}

void MainWindow::createMenusAndToolbar()
{
    QMenu *fileMenu = menuBar()->addMenu(QStringLiteral("File（檔案）(&F)"));
    fileMenu->addAction(m_openAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_saveAction);
    fileMenu->addAction(m_saveAsAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exitAction);

    QMenu *editMenu = menuBar()->addMenu(QStringLiteral("Image / Edit（影像編輯）(&I)"));
    editMenu->addAction(m_cropAction);
    editMenu->addSeparator();
    editMenu->addAction(m_rotateLeftAction);
    editMenu->addAction(m_rotateRightAction);

    QMenu *viewMenu = menuBar()->addMenu(QStringLiteral("View（檢視）(&V)"));
    viewMenu->addAction(m_zoomInAction);
    viewMenu->addAction(m_zoomOutAction);
    viewMenu->addAction(m_fitAction);

    QToolBar *toolBar = addToolBar(QStringLiteral("常用工具"));
    toolBar->setObjectName(QStringLiteral("MainToolBar"));
    toolBar->setMovable(false);
    toolBar->setIconSize(QSize(26, 26));
    toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->addAction(m_openAction);
    toolBar->addAction(m_saveAction);
    toolBar->addAction(m_saveAsAction);
    toolBar->addSeparator();
    toolBar->addAction(m_cropAction);
    toolBar->addSeparator();
    toolBar->addAction(m_zoomInAction);
    toolBar->addAction(m_zoomOutAction);
    toolBar->addAction(m_fitAction);
}

void MainWindow::createWorkArea()
{
    m_view = new ImageView(this);
    setCentralWidget(m_view);  // 老師 Week 10：centralWidget 是未來影像工作區

    // QAction 已經建立，集中連接視圖操作，讓同一 QAction 同時服務 Menu / ToolBar。
    connect(m_zoomInAction, &QAction::triggered, m_view, &ImageView::zoomIn);
    connect(m_zoomOutAction, &QAction::triggered, m_view, &ImageView::zoomOut);
    connect(m_fitAction, &QAction::triggered, m_view, &ImageView::fitImageToView);
    connect(m_view, &ImageView::cropSelectionChanged,
            this, &MainWindow::onCropSelectionChanged);
    connect(m_view, &ImageView::zoomChanged, this, &MainWindow::updateZoomLabel);

    // 左側：基礎編輯工具面板
    auto *toolsDock = new QDockWidget(QStringLiteral("功能列表"), this);
    toolsDock->setObjectName(QStringLiteral("ToolsDock"));
    toolsDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    toolsDock->setMinimumWidth(170);
    auto *toolsPanel = new QWidget(toolsDock);
    auto *toolsLayout = new QVBoxLayout(toolsPanel);
    toolsLayout->setContentsMargins(12, 14, 12, 14);
    toolsLayout->setSpacing(8);
    auto *toolsTitle = new QLabel(QStringLiteral("常用功能"), toolsPanel);
    toolsTitle->setObjectName(QStringLiteral("sectionTitle"));
    toolsLayout->addWidget(toolsTitle);
    toolsLayout->addWidget(makeToolButton(m_openAction, toolsPanel));
    toolsLayout->addWidget(makeToolButton(m_cropAction, toolsPanel));
    toolsLayout->addWidget(makeToolButton(m_rotateLeftAction, toolsPanel));
    toolsLayout->addWidget(makeToolButton(m_rotateRightAction, toolsPanel));
    toolsLayout->addSpacing(12);
    auto *viewTitle = new QLabel(QStringLiteral("畫布檢視"), toolsPanel);
    viewTitle->setObjectName(QStringLiteral("sectionTitle"));
    toolsLayout->addWidget(viewTitle);
    toolsLayout->addWidget(makeToolButton(m_zoomInAction, toolsPanel));
    toolsLayout->addWidget(makeToolButton(m_zoomOutAction, toolsPanel));
    toolsLayout->addWidget(makeToolButton(m_fitAction, toolsPanel));
    toolsLayout->addStretch();
    auto *panHint = new QLabel(QStringLiteral("滑鼠滾輪：縮放\n按住中鍵拖曳：平移"), toolsPanel);
    panHint->setObjectName(QStringLiteral("helper"));
    panHint->setWordWrap(true);
    toolsLayout->addWidget(panHint);
    toolsDock->setWidget(toolsPanel);
    addDockWidget(Qt::LeftDockWidgetArea, toolsDock);

    // 右側：裁剪參數（僅在裁剪模式可按套用／取消）
    auto *cropDock = new QDockWidget(QStringLiteral("工具參數"), this);
    cropDock->setObjectName(QStringLiteral("CropDock"));
    cropDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    cropDock->setMinimumWidth(195);
    auto *cropPanel = new QWidget(cropDock);
    auto *cropLayout = new QVBoxLayout(cropPanel);
    cropLayout->setContentsMargins(14, 14, 14, 14);
    cropLayout->setSpacing(12);
    auto *cropTitle = new QLabel(QStringLiteral("裁剪設定"), cropPanel);
    cropTitle->setObjectName(QStringLiteral("sectionTitle"));
    cropLayout->addWidget(cropTitle);
    m_cropHint = new QLabel(QStringLiteral("先開啟影像，再選擇「裁剪圖片」。"), cropPanel);
    m_cropHint->setObjectName(QStringLiteral("helper"));
    m_cropHint->setWordWrap(true);
    cropLayout->addWidget(m_cropHint);
    m_cropSize = new QLabel(QStringLiteral("選取範圍：—"), cropPanel);
    cropLayout->addWidget(m_cropSize);
    m_applyCropButton = new QPushButton(QStringLiteral("套用裁剪"), cropPanel);
    m_applyCropButton->setObjectName(QStringLiteral("applyCrop"));
    m_cancelCropButton = new QPushButton(QStringLiteral("取消裁剪"), cropPanel);
    cropLayout->addWidget(m_applyCropButton);
    cropLayout->addWidget(m_cancelCropButton);
    cropLayout->addStretch();
    auto *cropNote = new QLabel(QStringLiteral("裁剪尚未套用前，不會修改圖片內容。"), cropPanel);
    cropNote->setObjectName(QStringLiteral("helper"));
    cropNote->setWordWrap(true);
    cropLayout->addWidget(cropNote);
    cropDock->setWidget(cropPanel);
    addDockWidget(Qt::RightDockWidgetArea, cropDock);
    connect(m_applyCropButton, &QPushButton::clicked, this, &MainWindow::applyCrop);
    connect(m_cancelCropButton, &QPushButton::clicked, this, &MainWindow::cancelCrop);

    // 下方：QStatusBar 顯示尺寸、縮放比例和即時提示。
    m_dimensionsLabel = new QLabel(QStringLiteral("影像：—"), this);
    m_zoomLabel = new QLabel(QStringLiteral("縮放：—"), this);
    m_dimensionsLabel->setMinimumWidth(150);
    m_zoomLabel->setMinimumWidth(90);
    statusBar()->addPermanentWidget(m_dimensionsLabel);
    statusBar()->addPermanentWidget(m_zoomLabel);
}

void MainWindow::updateActionStates()
{
    const bool loaded = m_view && m_view->hasImage();
    const bool cropping = loaded && m_view->cropMode();
    m_saveAction->setEnabled(loaded && !cropping);
    m_saveAsAction->setEnabled(loaded && !cropping);
    m_cropAction->setEnabled(loaded && !cropping);
    m_cropAction->setChecked(cropping);
    m_rotateLeftAction->setEnabled(loaded && !cropping);
    m_rotateRightAction->setEnabled(loaded && !cropping);
    m_zoomInAction->setEnabled(loaded);
    m_zoomOutAction->setEnabled(loaded);
    m_fitAction->setEnabled(loaded);
    m_applyCropButton->setEnabled(cropping && !m_view->selectedCropRect().isEmpty());
    m_cancelCropButton->setEnabled(cropping);
}

void MainWindow::notifyError(const QString &title, const QString &detail)
{
    statusBar()->showMessage(title, 7000);
    QMessageBox::warning(this, title, detail);
}

void MainWindow::openImage()
{
    const QString path = QFileDialog::getOpenFileName(
        this, QStringLiteral("開啟影像"),
        m_sourcePath.isEmpty() ? QString() : QFileInfo(m_sourcePath).absolutePath(),
        QStringLiteral("影像 (*.png *.jpg *.jpeg);;所有檔案 (*)"));
    if (path.isEmpty())
        return;

    if (m_view->cropMode()) {
        const auto reply = QMessageBox::question(
            this, QStringLiteral("裁剪尚未完成"),
            QStringLiteral("目前的裁剪選取尚未套用。繼續開啟圖片將取消本次裁剪，是否繼續？"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (reply != QMessageBox::Yes)
            return;
    }

    statusBar()->showMessage(QStringLiteral("正在開啟圖片：%1").arg(QFileInfo(path).fileName()));
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QImageReader reader(path);
    reader.setAutoTransform(true);
    const QByteArray detectedFormat = reader.format().toLower();
    if (detectedFormat != QByteArray("png") && detectedFormat != QByteArray("jpeg") &&
        detectedFormat != QByteArray("jpg")) {
        QApplication::restoreOverrideCursor();
        notifyError(QStringLiteral("不支援的影像格式"),
                    QStringLiteral("第一版只支援開啟 PNG 和 JPEG 圖片。\n檔案：%1").arg(path));
        return;
    }
    const QImage image = reader.read();
    const QString error = reader.errorString();
    QApplication::restoreOverrideCursor();

    if (image.isNull()) {
        notifyError(QStringLiteral("影像開啟失敗"),
                    QStringLiteral("無法讀取：\n%1\n\n原因：%2").arg(path, error));
        return;
    }

    m_view->setImage(image);
    m_sourcePath = path;
    m_lastSavedPath.clear(); // 新圖片首次 Save 一律開啟另存視窗，避免誤覆蓋原圖。
    m_lastSavedFormat = QByteArray("PNG");
    m_lastJpegQuality = 90;
    m_dimensionsLabel->setText(
        QStringLiteral("影像：%1 × %2 px").arg(image.width()).arg(image.height()));
    setWindowTitle(QStringLiteral("Qt Image Editor V1 — %1").arg(QFileInfo(path).fileName()));
    m_cropHint->setText(QStringLiteral("請選擇「裁剪圖片」開始框選。"));
    updateActionStates();
    statusBar()->showMessage(QStringLiteral("已成功開啟：%1").arg(QFileInfo(path).fileName()), 5000);
}

QString MainWindow::suggestedExportPath() const
{
    if (!m_lastSavedPath.isEmpty())
        return m_lastSavedPath;
    if (m_sourcePath.isEmpty())
        return QStringLiteral("edited_image.png");

    QFileInfo info(m_sourcePath);
    return info.dir().filePath(info.completeBaseName() + QStringLiteral("_edited.png"));
}

bool MainWindow::ensureCropFinished()
{
    if (!m_view->cropMode())
        return true;
    QMessageBox::information(this, QStringLiteral("請完成裁剪"),
                             QStringLiteral("請先按「套用裁剪」或「取消裁剪」，再執行其他編輯或儲存。"));
    return false;
}

void MainWindow::saveImage()
{
    if (!m_view->hasImage() || !ensureCropFinished())
        return;
    if (m_lastSavedPath.isEmpty()) {
        saveImageAs();
        return;
    }
    writeImageToFile(m_lastSavedPath, m_lastSavedFormat, m_lastJpegQuality);
}

void MainWindow::saveImageAs()
{
    if (!m_view->hasImage() || !ensureCropFinished())
        return;
    ExportDialog dialog(suggestedExportPath(), m_view->image().size(), this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    if (writeImageToFile(dialog.outputPath(), dialog.outputFormat(), dialog.jpegQuality())) {
        m_lastSavedPath = dialog.outputPath();
        m_lastSavedFormat = dialog.outputFormat();
        m_lastJpegQuality = dialog.jpegQuality();
    }
}

bool MainWindow::writeImageToFile(const QString &path, const QByteArray &format, int jpegQuality)
{
    if (m_view->image().isNull())
        return false;

    statusBar()->showMessage(QStringLiteral("正在儲存圖片…"));
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QSaveFile output(path); // 原子式寫入，避免失敗時留下不完整輸出檔。
    if (!output.open(QIODevice::WriteOnly)) {
        const QString reason = output.errorString();
        QApplication::restoreOverrideCursor();
        notifyError(QStringLiteral("影像儲存失敗"),
                    QStringLiteral("無法寫入檔案：\n%1\n\n原因：%2").arg(path, reason));
        return false;
    }

    QImage toWrite = m_view->image();
    if (format == QByteArray("JPEG") && toWrite.hasAlphaChannel()) {
        // JPEG 不支援透明度；以白底合成，不會修改編輯中的原始 QImage。
        QImage opaque(toWrite.size(), QImage::Format_RGB32);
        opaque.fill(Qt::white);
        QPainter painter(&opaque);
        painter.drawImage(0, 0, toWrite);
        painter.end();
        toWrite = opaque;
    }

    bool succeeded = false;
    QString reason;
    {
        QImageWriter writer(&output, format);
        if (format == QByteArray("JPEG"))
            writer.setQuality(jpegQuality);
        succeeded = writer.write(toWrite);
        if (!succeeded)
            reason = writer.errorString();
    }
    if (!succeeded) {
        output.cancelWriting();
        QApplication::restoreOverrideCursor();
        notifyError(QStringLiteral("影像儲存失敗"),
                    QStringLiteral("無法輸出：\n%1\n\n原因：%2").arg(path, reason));
        return false;
    }
    if (!output.commit()) {
        reason = output.errorString();
        QApplication::restoreOverrideCursor();
        notifyError(QStringLiteral("影像儲存失敗"),
                    QStringLiteral("寫入檔案時發生錯誤：\n%1\n\n原因：%2").arg(path, reason));
        return false;
    }

    QApplication::restoreOverrideCursor();
    statusBar()->showMessage(QStringLiteral("儲存成功：%1").arg(path), 6500);
    return true;
}

void MainWindow::startCrop()
{
    if (!m_view->hasImage())
        return;
    m_view->setCropMode(true);
    m_cropHint->setText(QStringLiteral("請在中央畫布按住滑鼠左鍵拖曳，框選保留範圍。"));
    m_cropSize->setText(QStringLiteral("選取範圍：—"));
    updateActionStates();
    statusBar()->showMessage(QStringLiteral("裁剪模式：拖曳選取範圍，再選擇套用或取消。"), 7000);
}

void MainWindow::onCropSelectionChanged(const QRect &rect)
{
    if (!m_cropSize || !m_applyCropButton || !m_view)
        return;
    if (rect.isEmpty())
        m_cropSize->setText(QStringLiteral("選取範圍：—"));
    else
        m_cropSize->setText(QStringLiteral("範圍：%1 × %2 px").arg(rect.width()).arg(rect.height()));
    m_applyCropButton->setEnabled(m_view->cropMode() && !rect.isEmpty());
}

void MainWindow::applyCrop()
{
    if (!m_view->cropMode())
        return;
    const QRect cropRect = m_view->selectedCropRect();
    if (cropRect.width() < 2 || cropRect.height() < 2) {
        QMessageBox::information(this, QStringLiteral("尚未選取範圍"),
                                 QStringLiteral("請在影像上拖曳一個有效的裁剪矩形。"));
        return;
    }
    const QImage cropped = m_view->image().copy(cropRect);
    m_view->setImage(cropped);
    m_dimensionsLabel->setText(
        QStringLiteral("影像：%1 × %2 px").arg(cropped.width()).arg(cropped.height()));
    m_cropHint->setText(QStringLiteral("裁剪已套用。可以再次裁剪或儲存。"));
    updateActionStates();
    statusBar()->showMessage(QStringLiteral("已套用裁剪：%1 × %2 px").arg(cropped.width()).arg(cropped.height()), 5000);
}

void MainWindow::cancelCrop()
{
    if (!m_view->cropMode())
        return;
    m_view->setCropMode(false);
    m_cropHint->setText(QStringLiteral("已取消裁剪，影像內容沒有變更。"));
    updateActionStates();
    statusBar()->showMessage(QStringLiteral("已取消裁剪。"), 4500);
}

void MainWindow::rotateLeft()
{
    if (!m_view->hasImage() || !ensureCropFinished())
        return;
    QTransform transform;
    transform.rotate(-90);
    const QImage rotated = m_view->image().transformed(transform);
    m_view->setImage(rotated);
    m_dimensionsLabel->setText(
        QStringLiteral("影像：%1 × %2 px").arg(rotated.width()).arg(rotated.height()));
    updateActionStates();
    statusBar()->showMessage(QStringLiteral("已向左旋轉 90°。"), 4500);
}

void MainWindow::rotateRight()
{
    if (!m_view->hasImage() || !ensureCropFinished())
        return;
    QTransform transform;
    transform.rotate(90);
    const QImage rotated = m_view->image().transformed(transform);
    m_view->setImage(rotated);
    m_dimensionsLabel->setText(
        QStringLiteral("影像：%1 × %2 px").arg(rotated.width()).arg(rotated.height()));
    updateActionStates();
    statusBar()->showMessage(QStringLiteral("已向右旋轉 90°。"), 4500);
}

void MainWindow::updateZoomLabel(int percent)
{
    if (m_zoomLabel)
        m_zoomLabel->setText(QStringLiteral("縮放：%1%").arg(percent));
}
