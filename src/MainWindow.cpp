#include "MainWindow.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <spdlog/spdlog.h>
#include <QFileDialog>
#include <QMessageBox>
#include <QSpacerItem>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    QFormLayout *fLayout = new QFormLayout();
    fLayout->setContentsMargins(30, 30, 30, 30);
    fLayout->setSpacing(10);

    m_lineedit_margin = new QLineEdit(this);
    m_lineedit_margin->setText(QString::number(10));
    m_lineedit_margin->setPlaceholderText("单位: 像素");
    m_lineedit_margin->setToolTip("图像之间的边距，单位: 像素");
    fLayout->addRow("边距:", m_lineedit_margin);

    m_lineedit_row = new QLineEdit(this);
    fLayout->addRow("行数:", m_lineedit_row);

    m_lineedit_columns = new QLineEdit(this);
    fLayout->addRow("列数:", m_lineedit_columns);

    m_lineedit_filename = new QLineEdit(this);
    m_lineedit_filename->setText("stitched_image");
    m_lineedit_filename->setPlaceholderText("拼接后的文件名");
    m_lineedit_filename->setToolTip("输出文件的名称");
    fLayout->addRow("文件名:", m_lineedit_filename);

    m_combobox_format = new QComboBox(this);
    m_combobox_format->addItem("png");
    m_combobox_format->addItem("jpg");
    m_combobox_format->setCurrentIndex(0);
    m_combobox_format->setToolTip("输出文件的后缀");
    fLayout->addRow("文件格式:", m_combobox_format);

    m_checkbox_sequence = new QCheckBox(this);
    m_checkbox_sequence->setText("添加序列号");
    m_checkbox_sequence->setChecked(true);
    fLayout->addWidget(m_checkbox_sequence);

    m_checkbox_datetime = new QCheckBox(this);
    m_checkbox_datetime->setText("添加日期时间");
    m_checkbox_datetime->setChecked(true);
    fLayout->addWidget(m_checkbox_datetime);

    m_checkbox_mosaic = new QCheckBox(this);
    m_checkbox_mosaic->setText("手机号打码");
    m_checkbox_mosaic->setChecked(true);
    fLayout->addWidget(m_checkbox_mosaic);

    QHBoxLayout *hLayout_pushbutton = new QHBoxLayout();
    hLayout_pushbutton->setContentsMargins(0, 0, 0, 0);
    hLayout_pushbutton->setSpacing(10);
    m_pushbutton_select = new QPushButton(this);
    m_pushbutton_select->setText("选择图片");
    connect(m_pushbutton_select, &QPushButton::clicked, this, &MainWindow::SelectImages);
    hLayout_pushbutton->addWidget(m_pushbutton_select);
    m_pushbutton_start = new QPushButton(this);
    m_pushbutton_start->setText("开始拼接");
    connect(m_pushbutton_start, &QPushButton::clicked, this, &MainWindow::Start);
    m_pushbutton_start->setDisabled(true);
    hLayout_pushbutton->addWidget(m_pushbutton_start);
    fLayout->addRow(hLayout_pushbutton);

    QHBoxLayout *hLayout_progress = new QHBoxLayout();
    hLayout_progress->setContentsMargins(0, 0, 0, 0);
    hLayout_progress->setSpacing(10);
    hLayout_progress->setContentsMargins(0, 0, 0, 0);
    m_label_state = new QLabel(this);
    m_label_state->setText("准备中...");
    hLayout_progress->addWidget(m_label_state);
    m_progressbar = new QProgressBar(this);
    hLayout_progress->addWidget(m_progressbar);
    fLayout->addRow(hLayout_progress);

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    vLayout->addLayout(fLayout);

    connect(this, &MainWindow::sig_finish, this, &MainWindow::slot_finish);
    connect(this, &MainWindow::sig_update_status, this, &MainWindow::slot_update_status);
    connect(this, &MainWindow::sig_update_progress, this, &MainWindow::slot_update_progress);
    connect(this, &MainWindow::sig_set_progress_range, this, &MainWindow::slot_set_progress_range);
    connect(this, &MainWindow::sig_show_message, this, &MainWindow::slot_show_message);
}

QVector<cv::Mat> MainWindow::LoadImagesFromQStringList(const QStringList &qstringlist)
{
    QVector<cv::Mat> images;
    for (const auto &path : qstringlist)
    {
        cv::Mat img = cv::imread(path.toStdString());
        if (!img.empty())
        {
            images.push_back(img);
        }
    }
    return images;
}

bool MainWindow::FindLineEdit(cv::Mat &img, cv::Rect &rect_target)
{
    // 颜色过滤
    cv::Mat img_hsv, img_mask;
    cv::cvtColor(img, img_hsv, cv::COLOR_RGB2HSV);
    cv::inRange(img_hsv, cv::Scalar(0, 0, 255), cv::Scalar(0, 0, 255), img_mask);

    // 二值化处理
    cv::Mat img_binary;
    cv::threshold(img_mask, img_binary, 200, 255, cv::THRESH_BINARY);

    // 边缘检测
    cv::Mat edges;
    cv::Canny(img_binary, edges, 50, 150);

    // 查找轮廓
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(edges, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    for (int i = 0; i < contours.size(); ++i)
    {
        cv::Rect current_rect = cv::boundingRect(contours[i]);
        // 比例大小筛选
        if (current_rect.width / current_rect.height > 30)
        {
            continue;
        }

        // 查找宽度最大的
        if (current_rect.width > rect_target.width)
        {
            rect_target = current_rect;
        }
    }

    if (rect_target.empty())
    {
        return false;
    }
    return true;
}

cv::Mat MainWindow::CreateImageGrid(const QVector<cv::Mat> &images)
{
    int rows = m_lineedit_columns->text().toInt();
    int cols = m_lineedit_row->text().toInt();
    // int cols = static_cast<int>(std::ceil(static_cast<double>(m_image_paths.size()) / rows));
    if (images.empty() || rows * cols < images.size())
    {
        QMessageBox::warning(this, "错误", "行数/列数错误", QMessageBox::Ok);
    }

    // 获取最大图片的宽度和高度
    int max_width = 0, max_height = 0;
    for (const auto &img : images)
    {
        max_width = std::max(max_width, img.cols);
        max_height = std::max(max_height, img.rows);
    }

    // 创建空白画布
    cv::Mat grid(rows * max_height, cols * max_width, CV_8UC3, cv::Scalar(255, 255, 255));

    // 将图片粘贴到画布上
    for (size_t i = 0; i < images.size(); ++i)
    {
        int row = i / cols;
        int col = i % cols;

        int x = col * max_width;
        int y = row * max_height;

        int w = images[i].cols;
        int h = images[i].rows;

        images[i].copyTo(grid(cv::Rect(x + (max_width - w) / 2, y + (max_height - h) / 2, w, h)));
    }

    return grid;
}

void MainWindow::SelectImages()
{
    m_image_paths.clear();
    m_image_paths = QFileDialog::getOpenFileNames(nullptr, "选择一张或多张图片", QDir::homePath(), "图像文件 (*.png *.jpg);;所有文件 (*)");
    if (!m_image_paths.isEmpty())
    {
        spdlog::debug("Number of files selected: {}", m_image_paths.size());
        for (const QString &path : m_image_paths)
        {
            spdlog::debug("----{}", path.toStdString());
        }
        m_fileinfo.setFile(m_image_paths.first());
        m_lineedit_filename->setText(m_fileinfo.absoluteDir().dirName());
        m_lineedit_columns->setText(QString::number(static_cast<int>(std::ceil(std::sqrt(m_image_paths.size())))));
        m_lineedit_row->setText(QString::number(static_cast<int>(std::ceil(static_cast<double>(m_image_paths.size()) / m_lineedit_columns->text().toInt()))));
        m_label_state->setText("已选" + QString::number(m_image_paths.size()) + "张图片");
        m_pushbutton_start->setDisabled(false);
    }
}

void MainWindow::Start()
{
    m_lineedit_margin->setDisabled(true);
    m_lineedit_row->setDisabled(true);
    m_lineedit_columns->setDisabled(true);
    m_lineedit_filename->setDisabled(true);
    m_combobox_format->setDisabled(true);
    m_checkbox_sequence->setDisabled(true);
    m_checkbox_datetime->setDisabled(true);
    m_checkbox_mosaic->setDisabled(true);
    m_pushbutton_select->setDisabled(true);
    m_pushbutton_start->setDisabled(true);

    m_worker = std::thread(&MainWindow::ImageProcessing, this);
}

void MainWindow::DrawSequence(cv::Mat &img, const int index)
{
    // 设置字体类型、大小、颜色和位置
    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    double fontScale = 3; // 字体缩放因子
    int thickness = 4;    // 线宽
    // 绘制阴影
    cv::Scalar color_shadow(255, 255, 255);                                                             // BGR颜色
    cv::Point textOrg_shadow(OFFSET_X_SEQUENCE + OFFSET_X_SHADOW, OFFSET_Y_SEQUENCE + OFFSET_Y_SHADOW); // 文本起始位置
    cv::putText(img, std::to_string(index + 1), textOrg_shadow, fontFace, fontScale, color_shadow, thickness, cv::LINE_AA);
    // 绘制序号
    cv::Scalar color(0, 0, 255);
    cv::Point textOrg(OFFSET_X_SEQUENCE, OFFSET_Y_SEQUENCE);
    cv::putText(img, std::to_string(index + 1), textOrg, fontFace, fontScale, color, thickness, cv::LINE_AA);
}

void MainWindow::DrawDateTime(cv::Mat &img, const int index)
{
    QFileInfo currentFile(m_image_paths[index]);
    std::string dateTime = currentFile.lastModified().toString("yyyy-MM-dd hh:mm:ss").toStdString();
    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    double fontScale = 3;
    int thickness = 4;
    cv::Scalar color_shadow(255, 255, 255);
    cv::Point textOrg_shadow(OFFSET_X_DATETIME + OFFSET_X_SHADOW, OFFSET_Y_DATETIME + OFFSET_Y_SHADOW);
    cv::putText(img, dateTime, textOrg_shadow, fontFace, fontScale, color_shadow, thickness, cv::LINE_AA);
    cv::Scalar color(243, 150, 33);
    cv::Point textOrg(OFFSET_X_DATETIME, OFFSET_Y_DATETIME);
    cv::putText(img, dateTime, textOrg, fontFace, fontScale, color, thickness, cv::LINE_AA);
}

void MainWindow::DrawMosaic(cv::Mat &img, const cv::Rect &rect_target)
{
    // cv::rectangle(img, rect_target.tl(), rect_target.br(), cv::Scalar(255, 0, 255), 5);
    // 截取打码区域
    int x = rect_target.x + OFFSET_X_MOSAIC;
    int y = rect_target.y + OFFSET_Y_MOSAIC;
    int w = WIDTH_MOSAIC;
    int h = HEIGHT_MOSAIC;
    cv::Mat img_mosaic = img(cv::Rect(x, y, w, h)).clone();
    // 打码
    cv::GaussianBlur(img_mosaic, img_mosaic, cv::Size(25, 25), 0);
    // 粘贴到原图
    img_mosaic.copyTo(img(cv::Rect(x, y, w, h)));
}

void MainWindow::ImageProcessing()
{
    // 加载图片
    Q_EMIT sig_update_status("正在读取...");
    Q_EMIT sig_set_progress_range(0, 1 + m_image_paths.size() + 1); // 加载 + 绘制&拼接 + 保存
    int step = 0;
    QVector<cv::Mat> images = LoadImagesFromQStringList(m_image_paths);
    Q_EMIT sig_update_progress(++step);

    // 处理图片
    Q_EMIT sig_update_status("正在绘制&拼接...");
    if (m_checkbox_sequence->isChecked() || m_checkbox_datetime->isChecked() || m_checkbox_mosaic->isChecked())
    {
        // 查找lineedit
        for (int i = 0; i < images.size(); ++i)
        {
            cv::Mat &img = images[i];
            /*绘制序号*/
            if (m_checkbox_sequence->isChecked())
            {
                DrawSequence(img, i);
            }
            /*绘制时间*/
            if (m_checkbox_datetime->isChecked())
            {
                DrawDateTime(img, i);
            }
            /*绘制马赛克*/
            if (m_checkbox_mosaic->isChecked())
            {
                cv::Rect rect_target;
                if (!FindLineEdit(img, rect_target))
                {
                    spdlog::error("Failed to find lineedit");
                }
                else
                {
                    // cv::rectangle(img, rect_target.tl(), rect_target.br(), cv::Scalar(255, 0, 255), 5);
                    DrawMosaic(img, rect_target);
                }
            }
            Q_EMIT sig_update_progress(++step);
        }
    }

    // 保存拼接后的图片
    cv::Mat img_result = CreateImageGrid(images);
    Q_EMIT sig_update_status("正在保存...");
    std::string output_file = QString(m_fileinfo.absoluteDir().path() + "/" + m_lineedit_filename->text() + "." + m_combobox_format->currentText()).toStdString();
    if (!cv::imwrite(output_file, img_result))
    {
        Q_EMIT sig_update_progress(++step);
        spdlog::error("Failed to save image");
        Q_EMIT sig_show_message(false, "保存失败");
        Q_EMIT sig_update_status("保存失败");
    }
    else
    {
        Q_EMIT sig_update_progress(++step);
        spdlog::info("Success");
        Q_EMIT sig_show_message(true, "文件已保存至: " + QString::fromStdString(output_file));
        Q_EMIT sig_update_status("保存完成");
    }
    Q_EMIT sig_finish();
}

void MainWindow::slot_finish()
{
    if (m_worker.joinable())
    {
        m_worker.join();
    }

    m_lineedit_margin->setDisabled(false);
    m_lineedit_row->setDisabled(false);
    m_lineedit_columns->setDisabled(false);
    m_lineedit_filename->setDisabled(false);
    m_combobox_format->setDisabled(false);
    m_checkbox_sequence->setDisabled(false);
    m_checkbox_datetime->setDisabled(false);
    m_checkbox_mosaic->setDisabled(false);
    m_pushbutton_select->setDisabled(false);
    m_pushbutton_start->setDisabled(false);
}

void MainWindow::slot_update_status(const QString &text)
{
    m_label_state->setText(text);
}

void MainWindow::slot_update_progress(int value)
{
    m_progressbar->setValue(value);
}

void MainWindow::slot_set_progress_range(int min, int max)
{
    m_progressbar->setRange(min, max);
}

void MainWindow::slot_show_message(bool success, const QString &message)
{
    if (success)
    {
        QMessageBox::information(this, "成功", message, QMessageBox::Ok);
    }
    else
    {
        QMessageBox::warning(this, "错误", message, QMessageBox::Ok);
    }
}