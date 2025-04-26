#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QFileInfo>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <QVector>
#include <thread>

constexpr int OFFSET_X_SEQUENCE = 300;
constexpr int OFFSET_Y_SEQUENCE = 230;
constexpr int OFFSET_X_DATETIME = 450;
constexpr int OFFSET_Y_DATETIME = 230;
constexpr int OFFSET_X_SHADOW = 4;
constexpr int OFFSET_Y_SHADOW = 4;
constexpr int OFFSET_X_MOSAIC = 48;
constexpr int OFFSET_Y_MOSAIC = 10;
constexpr int WIDTH_MOSAIC = 62;
constexpr int HEIGHT_MOSAIC = 40;

class MainWindow : public QWidget
{
    Q_OBJECT

Q_SIGNALS:
    void sig_finish();
    void sig_update_status(const QString &text);
    void sig_update_progress(int value);
    void sig_set_progress_range(int min, int max);
    void sig_show_message(bool success, const QString &message);

private Q_SLOTS:
    void slot_finish();
    void slot_update_status(const QString &text);
    void slot_update_progress(int value);
    void slot_set_progress_range(int min, int max);
    void slot_show_message(bool success, const QString &message);

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

private:
    QLineEdit *m_lineedit_margin;
    QLineEdit *m_lineedit_row;
    QLineEdit *m_lineedit_columns;
    QLineEdit *m_lineedit_filename;
    QComboBox *m_combobox_format;
    QCheckBox *m_checkbox_sequence;
    QCheckBox *m_checkbox_datetime;
    QCheckBox *m_checkbox_mosaic;
    QCheckBox *m_checkbox_compress;
    QPushButton *m_pushbutton_select;
    QPushButton *m_pushbutton_start;
    QLabel *m_label_state;
    QProgressBar *m_progressbar;
    QStringList m_image_paths;
    QFileInfo m_fileinfo;
    std::thread m_worker;
    int m_step;

private:
    QVector<cv::Mat> LoadImagesFromQStringList(const QStringList &qstringlist);
    bool FindLineEdit(cv::Mat &img, cv::Rect &rect_target);
    cv::Mat CreateImageGrid(const QVector<cv::Mat> &images);
    void SelectImages();
    void Start();
    void DrawSequence(cv::Mat &img, const int index);
    void DrawDateTime(cv::Mat &img, const int index);
    void DrawMosaic(cv::Mat &img, const cv::Rect &rect_target);
    void ImageProcessing();
    void CompressAsPNG(const cv::Mat& image, const std::string& outputPath, int compressionLevel = 3);
};

#endif