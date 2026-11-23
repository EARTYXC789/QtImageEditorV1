#pragma once

#include <QMainWindow>
#include <QByteArray>
#include <QImage>
#include <QString>

class QAction;
class QLabel;
class QPushButton;
class QToolBar;
class ImageView;

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void openImage();
    void saveImage();
    void saveImageAs();
    void startCrop();
    void applyCrop();
    void cancelCrop();
    void rotateLeft();
    void rotateRight();
    void onCropSelectionChanged(const QRect &rect);
    void updateZoomLabel(int percent);

private:
    void createActions();
    void createMenusAndToolbar();
    void createWorkArea();
    void updateActionStates();
    bool writeImageToFile(const QString &path, const QByteArray &format, int jpegQuality);
    bool ensureCropFinished();
    QString suggestedExportPath() const;
    void notifyError(const QString &title, const QString &detail);

    ImageView *m_view = nullptr;
    QAction *m_openAction = nullptr;
    QAction *m_saveAction = nullptr;
    QAction *m_saveAsAction = nullptr;
    QAction *m_exitAction = nullptr;
    QAction *m_cropAction = nullptr;
    QAction *m_rotateLeftAction = nullptr;
    QAction *m_rotateRightAction = nullptr;
    QAction *m_zoomInAction = nullptr;
    QAction *m_zoomOutAction = nullptr;
    QAction *m_fitAction = nullptr;

    QLabel *m_cropHint = nullptr;
    QLabel *m_cropSize = nullptr;
    QLabel *m_dimensionsLabel = nullptr;
    QLabel *m_zoomLabel = nullptr;
    QPushButton *m_applyCropButton = nullptr;
    QPushButton *m_cancelCropButton = nullptr;

    QString m_sourcePath;
    QString m_lastSavedPath;
    QByteArray m_lastSavedFormat = QByteArray("PNG");
    int m_lastJpegQuality = 90;
};
