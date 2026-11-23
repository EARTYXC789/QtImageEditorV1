#include "exportdialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

ExportDialog::ExportDialog(const QString &suggestedPath, const QSize &imageSize,
                           QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("儲存／另存新檔"));
    setMinimumWidth(510);
    setModal(true);

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(14);
    auto *description = new QLabel(QStringLiteral("輸出目前編輯的影像，選擇路徑與格式。"), this);
    description->setWordWrap(true);
    layout->addWidget(description);

    auto *form = new QFormLayout;
    form->setSpacing(12);
    auto *pathRow = new QWidget(this);
    auto *pathLayout = new QHBoxLayout(pathRow);
    pathLayout->setContentsMargins(0, 0, 0, 0);
    pathLayout->setSpacing(7);
    m_pathEdit = new QLineEdit(suggestedPath, this);
    m_pathEdit->setPlaceholderText(QStringLiteral("選擇要儲存的圖片路徑"));
    auto *browseButton = new QPushButton(QStringLiteral("瀏覽…"), this);
    pathLayout->addWidget(m_pathEdit, 1);
    pathLayout->addWidget(browseButton);
    form->addRow(QStringLiteral("檔案位置："), pathRow);

    m_formatCombo = new QComboBox(this);
    m_formatCombo->addItem(QStringLiteral("PNG（無損）"), QByteArray("PNG"));
    m_formatCombo->addItem(QStringLiteral("JPEG（照片）"), QByteArray("JPEG"));
    const QString suffix = QFileInfo(suggestedPath).suffix().toLower();
    if (suffix == QStringLiteral("jpg") || suffix == QStringLiteral("jpeg"))
        m_formatCombo->setCurrentIndex(1);
    form->addRow(QStringLiteral("輸出格式："), m_formatCombo);

    m_qualitySpin = new QSpinBox(this);
    m_qualitySpin->setRange(1, 100);
    m_qualitySpin->setValue(90);
    m_qualitySpin->setSuffix(QStringLiteral(" %"));
    m_qualitySpin->setToolTip(QStringLiteral("只在 JPEG 格式時使用"));
    form->addRow(QStringLiteral("JPEG 品質："), m_qualitySpin);

    auto *dimensions = new QLabel(
        QStringLiteral("%1 × %2 像素").arg(imageSize.width()).arg(imageSize.height()), this);
    form->addRow(QStringLiteral("影像尺寸："), dimensions);
    layout->addLayout(form);

    auto *note = new QLabel(QStringLiteral("提醒：已存在的檔案會在確認後覆寫。"), this);
    note->setStyleSheet(QStringLiteral("color: #64748b;"));
    layout->addWidget(note);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Save)->setText(QStringLiteral("儲存"));
    buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
    layout->addWidget(buttons);

    connect(browseButton, &QPushButton::clicked, this, &ExportDialog::choosePath);
    connect(m_formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ExportDialog::formatChanged);
    connect(buttons, &QDialogButtonBox::accepted, this, &ExportDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &ExportDialog::reject);
    m_qualitySpin->setEnabled(m_formatCombo->currentIndex() == 1);
}

QByteArray ExportDialog::outputFormat() const
{
    return m_formatCombo->currentData().toByteArray();
}

int ExportDialog::jpegQuality() const
{
    return m_qualitySpin->value();
}

void ExportDialog::formatChanged()
{
    const bool jpeg = m_formatCombo->currentIndex() == 1;
    m_qualitySpin->setEnabled(jpeg);
    QString path = m_pathEdit->text().trimmed();
    if (path.isEmpty())
        return;

    QFileInfo info(path);
    const QString suffix = info.suffix().toLower();
    if (suffix == QStringLiteral("png") || suffix == QStringLiteral("jpg") ||
        suffix == QStringLiteral("jpeg")) {
        path.chop(suffix.length());
        path += jpeg ? QStringLiteral("jpg") : QStringLiteral("png");
        m_pathEdit->setText(path);
    }
}

void ExportDialog::choosePath()
{
    const QString filters = QStringLiteral("PNG 圖片 (*.png);;JPEG 圖片 (*.jpg *.jpeg)");
    QString selectedFilter = (outputFormat() == QByteArray("PNG"))
        ? QStringLiteral("PNG 圖片 (*.png)")
        : QStringLiteral("JPEG 圖片 (*.jpg *.jpeg)");
    const QString path = QFileDialog::getSaveFileName(
        this, QStringLiteral("選擇儲存位置"), m_pathEdit->text(), filters, &selectedFilter);
    if (path.isEmpty())
        return;

    m_pathEdit->setText(path);
    if (selectedFilter.startsWith(QStringLiteral("JPEG")))
        m_formatCombo->setCurrentIndex(1);
    else
        m_formatCombo->setCurrentIndex(0);
    // 格式選項更新後，如果使用者輸入的副檔名不同，在 accept() 會再檢查。
}

void ExportDialog::accept()
{
    QString path = m_pathEdit->text().trimmed();
    if (path.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("缺少檔案名稱"),
                             QStringLiteral("請選擇輸出路徑與檔案名稱。"));
        return;
    }

    const bool jpeg = outputFormat() == QByteArray("JPEG");
    QString suffix = QFileInfo(path).suffix().toLower();
    if (suffix.isEmpty()) {
        path += jpeg ? QStringLiteral(".jpg") : QStringLiteral(".png");
        suffix = jpeg ? QStringLiteral("jpg") : QStringLiteral("png");
    }

    const bool extensionMatches = jpeg
        ? (suffix == QStringLiteral("jpg") || suffix == QStringLiteral("jpeg"))
        : (suffix == QStringLiteral("png"));
    if (!extensionMatches) {
        QMessageBox::warning(this, QStringLiteral("檔案格式不一致"),
                             QStringLiteral("檔案副檔名必須與所選的 PNG 或 JPEG 格式相符。"));
        return;
    }

    const QFileInfo info(path);
    if (info.exists()) {
        if (!info.isFile()) {
            QMessageBox::warning(this, QStringLiteral("儲存失敗"),
                                 QStringLiteral("指定路徑不是一般檔案。"));
            return;
        }
        if (QMessageBox::question(
                this, QStringLiteral("確認覆寫"),
                QStringLiteral("檔案已存在：\n%1\n\n確定要覆寫嗎？").arg(path),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
            return;
    }

    m_finalPath = info.absoluteFilePath();
    QDialog::accept();
}
