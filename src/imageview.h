#pragma once

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QImage>
#include <QPoint>
#include <QRect>

class QGraphicsPixmapItem;
class QGraphicsRectItem;
class QGraphicsTextItem;
class QMouseEvent;
class QWheelEvent;
class QResizeEvent;

// 中央影像畫布：只負責顯示、縮放、平移、裁剪範圍選取。
// 實際裁剪與儲存由 MainWindow 負責，讓責任清楚分離。
class ImageView final : public QGraphicsView
{
    Q_OBJECT

public:
    explicit ImageView(QWidget *parent = nullptr);

    bool hasImage() const { return !m_image.isNull(); }
    const QImage &image() const { return m_image; }
    void setImage(const QImage &image);

    void setCropMode(bool enabled);
    bool cropMode() const { return m_cropMode; }
    QRect selectedCropRect() const;

public slots:
    void zoomIn();
    void zoomOut();
    void fitImageToView();

signals:
    void cropSelectionChanged(const QRect &pixelRect);
    void zoomChanged(int percent);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QPointF clampedScenePosition(const QPoint &viewportPoint) const;
    void applyZoom(qreal factor);
    void resetCropSelection();
    void displayPlaceholder();
    void emitZoomPercent();

    QGraphicsScene m_scene;
    QGraphicsPixmapItem *m_pixmapItem = nullptr;
    QGraphicsRectItem *m_cropBox = nullptr;
    QGraphicsTextItem *m_placeholder = nullptr;
    QImage m_image;

    bool m_cropMode = false;
    bool m_drawingCrop = false;
    bool m_middlePanning = false;
    bool m_fitMode = true;
    QPointF m_cropStart;
    QPoint m_lastPanPos;
};
