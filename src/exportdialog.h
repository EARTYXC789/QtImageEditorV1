#pragma once

#include <QDialog>
#include <QByteArray>
#include <QString>
#include <QSize>

class QComboBox;
class QLineEdit;
class QSpinBox;

// 存檔／另存新檔視窗：檔名、路徑、格式與 JPEG 品質。
class ExportDialog final : public QDialog
{
    Q_OBJECT
public:
    ExportDialog(const QString &suggestedPath, const QSize &imageSize,
                 QWidget *parent = nullptr);

    QString outputPath() const { return m_finalPath; }
    QByteArray outputFormat() const;
    int jpegQuality() const;

protected:
    void accept() override;

private slots:
    void choosePath();
    void formatChanged();

private:
    QLineEdit *m_pathEdit = nullptr;
    QComboBox *m_formatCombo = nullptr;
    QSpinBox *m_qualitySpin = nullptr;
    QString m_finalPath;
};
