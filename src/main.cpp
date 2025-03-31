#include <spdlog/spdlog.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>
#include <filesystem>
#include <QApplication>
#include "MainWindow.h"

const std::string input_images_folder = "res/test";
const std::string output_name = "output.png";
const std::string output_path = "E:/STUDY/CPP/OpenCV_STUDY/demo/bin/res";

bool LineEditFinder(cv::Mat &img, cv::Rect &rect_target)
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

std::vector<cv::Mat> LoadImagesFromFolder(const std::string &folderPath)
{
	std::vector<cv::Mat> images;
	std::vector<cv::String> filePaths;

	// 使用glob获取文件夹中的所有图片路径
	cv::glob(folderPath, filePaths);

	for (const auto &path : filePaths)
	{
		cv::Mat img = cv::imread(std::string(path));
		if (!img.empty())
		{
			images.push_back(img);
		}
	}

	return images;
}

cv::Mat CreateImageGrid(const std::vector<cv::Mat> &images, int rows, int cols)
{
	if (images.empty() || rows * cols < images.size())
	{
		throw std::invalid_argument("Invalid number of rows or columns");
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

int main(int argc, char **argv)
{
	spdlog::set_level(spdlog::level::debug);

	QApplication app(argc, argv);

	MainWindow w;
	w.show();

	// // 加载图片
	// std::vector<cv::Mat> images = LoadImagesFromFolder(input_images_folder);
	// spdlog::info("Total image size: {}", images.size());

	// // 查找lineedit
	// for (cv::Mat &img : images)
	// {
	// 	cv::Rect rect_target;
	// 	if (!LineEditFinder(img, rect_target))
	// 	{
	// 		spdlog::error("Failed to find lineedit");
	// 	}
	// 	cv::rectangle(img, rect_target.tl(), rect_target.br(), cv::Scalar(255, 0, 255), 5);
	// }

	// cv::Mat img_result = CreateImageGrid(images, 2, 3);
	// // 保存矩阵为图像文件
	// if (!cv::imwrite((std::filesystem::path(output_path) / output_name).string(), img_result))
	// {
	// 	spdlog::error("Failed to save image");
	// 	return -1;
	// }
	// spdlog::info("Success");

	return app.exec();;
};