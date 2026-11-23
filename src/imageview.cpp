#include "imageview.h"

#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QScrollBar>
#include <QPainter>
#include <QPen>
#include <QFont>
#include <QBrush>
#include <QtGlobal>

#include <algorithm>
#include <cmath>

ImageView::ImageView(QWidget *parent) : QGraphicsView(parent)
{
    setScene(&m_scene);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setBackgroundBrush(QColor(QStringLiteral("#e8edf4")));
    setFrameShape(QFrame::NoFrame);
    setAlignment(Qt::AlignCenter);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setMouseTracking(true);
    displayPlaceholder();
}

void ImageView::displayPlaceholder()
{
    m_scene.setSceneRect(QRectF(0, 0, 850, 520));
    m_placeholder = m_scene.addText(QStringLiteral("請選擇「File → 開啟」載入 PNG / JPEG 影像"));
    QFont font = m_placeholder->font();
    font.setPointSize(14);
    m_placeholder->setFont(font);
    m_placeholder->setDefaultTextColor(QColor(QStringLiteral("#60748a")));
    const QRectF bounds = m_placeholder->boundingRect();
    m_placeholder->setPos((850 - bounds.width()) / 2.0,
                          (520 - bounds.height()) / 2.0);
}

void ImageView::setImage(const QImage &image)
{
    m_image = image;
    m_cropMode = false;
    m_drawingCrop = false;
    m_middlePanning = false;
    m_fitMode = true;
    m_scene.clear();  // 釋放舊的 pixmap、裁剪框與提示文字
    m_pixmapItem = nullptr;
    m_cropBox = nullptr;
    m_placeholder = nullptr;
    resetTransform();

    if (m_image.isNull()) {
        displayPlaceholder();
        unsetCursor();
        emit zoomChanged(100);
        emit cropSelectionChanged(QRect());
        return;
    }

    m_pixmapItem = m_scene.addPixmap(QPixmap::fromImage(m_image));
    m_pixmapItem->setTransformationMode(Qt::SmoothTransformation);
    m_scene.setSceneRect(QRectF(QPointF(0, 0), QSizeF(m_image.size())));

    QPen outline(QColor(QStringLiteral("#00b7ff")), 1.5, Qt::DashLine);
    outline.setCosmetic(true);  // 畫面放大時，框線維持可見的線寬
    m_cropBox = m_scene.addRect(QRectF(), outline,
                                QBrush(QColor(0, 183, 255, 32)));
    m_cropBox->setZValue(10);
    m_cropBox->hide();

    unsetCursor();
    fitImageToView();
    emit cropSelectionChanged(QRect());
}

void ImageView::setCropMode(bool enabled)
{
    if (!hasImage())
        enabled = false;
    m_cropMode = enabled;
    m_drawingCrop = false;
    resetCropSelection();
    setCursor(enabled ? Qt::CrossCursor : Qt::ArrowCursor);
}

void ImageView::resetCropSelection()
{
    if (m_cropBox) {
        m_cropBox->setRect(QRectF());
        m_cropBox->hide();
    }
    emit cropSelectionChanged(QRect());
}

QPointF ImageView::clampedScenePosition(const QPoint &viewportPoint) const
{
    const QPointF point = mapToScene(viewportPoint);
    return QPointF(std::clamp(point.x(), 0.0, static_cast<qreal>(m_image.width())),
                   std::clamp(point.y(), 0.0, static_cast<qreal>(m_image.height())));
}

QRect ImageView::selectedCropRect() const
{
    if (!m_cropBox || !m_cropBox->isVisible() || m_image.isNull())
        return QRect();

    const QRectF rect = m_cropBox->rect().normalized();
    const int x1 = std::clamp(static_cast<int>(std::floor(rect.left())), 0, m_image.width());
    const int y1 = std::clamp(static_cast<int>(std::floor(rect.top())), 0, m_image.height());
    const int x2 = std::clamp(static_cast<int>(std::ceil(rect.right())), 0, m_image.width());
    const int y2 = std::clamp(static_cast<int>(std::ceil(rect.bottom())), 0, m_image.height());
    return QRect(x1, y1, x2 - x1, y2 - y1);
}

void ImageView::mousePressEvent(QMouseEvent *event)
{
    if (hasImage() && event->button() == Qt::MiddleButton) {
        m_middlePanning = true;
        m_lastPanPos = event->pos();
        m_fitMode = false;
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    if (hasImage() && m_cropMode && event->button() == Qt::LeftButton) {
        m_drawingCrop = true;
        m_cropStart = clampedScenePosition(event->pos());
        m_cropBox->setRect(QRectF(m_cropStart, m_cropStart));
        m_cropBox->show();
        emit cropSelectionChanged(QRect());
        event->accept();
        return;
    }

    QGraphicsView::mousePressEvent(event);
}

void ImageView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_middlePanning) {
        const QPoint offset = event->pos() - m_lastPanPos;
        m_lastPanPos = event->pos();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - offset.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - offset.y());
        event->accept();
        return;
    }

    if (m_drawingCrop && m_cropBox) {
        const QPointF current = clampedScenePosition(event->pos());
        m_cropBox->setRect(QRectF(m_cropStart, current).normalized());
        emit cropSelectionChanged(selectedCropRect());
        event->accept();
        return;
    }

    QGraphicsView::mouseMoveEvent(event);
}

void ImageView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton && m_middlePanning) {
        m_middlePanning = false;
        setCursor(m_cropMode ? Qt::CrossCursor : Qt::ArrowCursor);
        event->accept();
        return;
    }

    if (event->button() == Qt::LeftButton && m_drawingCrop) {
        m_drawingCrop = false;
        const QRect rect = selectedCropRect();
        if (rect.width() < 2 || rect.height() < 2)
            resetCropSelection();
        else
            emit cropSelectionChanged(rect);
        event->accept();
        return;
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void ImageView::applyZoom(qreal factor)
{
    if (!hasImage())
        return;
    const qreal current = transform().m11();
    const qreal target = current * factor;
    if (target < 0.05 || target > 12.0)
        return;

    m_fitMode = false;
    scale(factor, factor);
    emitZoomPercent();
}

void ImageView::zoomIn()
{
    applyZoom(1.25);
}

void ImageView::zoomOut()
{
    applyZoom(0.8);
}

void ImageView::fitImageToView()
{
    if (!hasImage() || viewport()->width() < 1 || viewport()->height() < 1)
        return;
    m_fitMode = true;
    resetTransform();
    fitInView(m_pixmapItem, Qt::KeepAspectRatio);
    emitZoomPercent();
}

void ImageView::wheelEvent(QWheelEvent *event)
{
    if (!hasImage() || event->angleDelta().y() == 0) {
        QGraphicsView::wheelEvent(event);
        return;
    }
    applyZoom(event->angleDelta().y() > 0 ? 1.15 : (1.0 / 1.15));
    event->accept();
}

void ImageView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    if (m_fitMode && hasImage())
        fitImageToView();
}

void ImageView::emitZoomPercent()
{
    emit zoomChanged(static_cast<int>(std::round(transform().m11() * 100.0)));
}
