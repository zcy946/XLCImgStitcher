#include <spdlog/spdlog.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>
#include <filesystem>
#include <QApplication>
#include "MainWindow.h"
#include <cxxopts.hpp> // 命令行参数解析库

namespace fs = std::filesystem;


// 查找文本框的函数
bool FindLineEdit(cv::Mat &img, cv::Rect &rect_target)
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

// 创建图片网格的函数
cv::Mat CreateImageGrid(const std::vector<cv::Mat> &images, int rows, int cols, int margin)
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
	cv::Mat grid(rows * max_height + (rows - 1) * margin,
				 cols * max_width + (cols - 1) * margin,
				 CV_8UC3,
				 cv::Scalar(255, 255, 255));

	// 将图片粘贴到画布上
	for (size_t i = 0; i < images.size(); ++i)
	{
		int row = i / cols;
		int col = i % cols;

		int x = col == 0 ? col * max_width : col * max_width + col * margin;
		int y = row == 0 ? row * max_height : row * max_height + row * margin;

		int w = images[i].cols;
		int h = images[i].rows;

		images[i].copyTo(grid(cv::Rect(x + (max_width - w) / 2, y + (max_height - h) / 2, w, h)));
	}

	return grid;
}

// 收集目录或文件列表中的所有图片路径
std::vector<std::string> CollectImagePaths(const std::vector<std::string> &inputs)
{
	std::vector<std::string> imagePaths;

	for (const auto &input : inputs)
	{
		if (fs::is_directory(input))
		{
			// 处理目录
			for (const auto &entry : fs::directory_iterator(input))
			{
				if (entry.is_regular_file())
				{
					std::string ext = entry.path().extension().string();
					std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
					if (ext == ".jpg" || ext == ".jpeg" || ext == ".png")
					{
						imagePaths.push_back(entry.path().string());
					}
				}
			}
		}
		else if (fs::is_regular_file(input))
		{
			// 处理单个文件
			std::string ext = fs::path(input).extension().string();
			std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
			if (ext == ".jpg" || ext == ".jpeg" || ext == ".png")
			{
				imagePaths.push_back(input);
			}
		}
	}

	return imagePaths;
}

// 绘制序号
void DrawSequence(cv::Mat &img, const int index)
{

	int fontFace = cv::FONT_HERSHEY_DUPLEX;
	double fontScale = 3;
	int thickness = 6;

	// 绘制阴影
	cv::Scalar color_shadow(255, 255, 255);
	cv::Point textOrg_shadow(OFFSET_X_SEQUENCE + OFFSET_X_SHADOW, OFFSET_Y_SEQUENCE + OFFSET_Y_SHADOW);
	cv::putText(img, std::to_string(index + 1), textOrg_shadow, fontFace, fontScale, color_shadow, thickness, cv::LINE_AA);

	// 绘制序号
	cv::Scalar color(0, 0, 255);
	cv::Point textOrg(OFFSET_X_SEQUENCE, OFFSET_Y_SEQUENCE);
	cv::putText(img, std::to_string(index + 1), textOrg, fontFace, fontScale, color, thickness, cv::LINE_AA);
}

// 绘制日期时间
void DrawDateTime(cv::Mat &img, const std::string &filePath)
{

	// 获取文件修改时间
	auto ftime = fs::last_write_time(filePath);
	auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
		ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
	std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);

	// 格式化时间
	char buffer[80];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&cftime));
	std::string dateTime(buffer);

	int fontFace = cv::FONT_HERSHEY_DUPLEX;
	double fontScale = 3;
	int thickness = 6;

	// 绘制阴影
	cv::Scalar color_shadow(255, 255, 255);
	cv::Point textOrg_shadow(OFFSET_X_DATETIME + OFFSET_X_SHADOW, OFFSET_Y_DATETIME + OFFSET_Y_SHADOW);
	cv::putText(img, dateTime, textOrg_shadow, fontFace, fontScale, color_shadow, thickness, cv::LINE_AA);

	// 绘制日期时间
	cv::Scalar color(243, 150, 33);
	cv::Point textOrg(OFFSET_X_DATETIME, OFFSET_Y_DATETIME);
	cv::putText(img, dateTime, textOrg, fontFace, fontScale, color, thickness, cv::LINE_AA);
}

// 核心图片处理函数
bool ProcessImages(const std::vector<std::string> &imagePaths,
				   int rows, int cols,
				   int margin,
				   const std::string &outputPath,
				   bool addSequence,
				   bool addDateTime,
				   bool addMosaic)
{
	try
	{
		// 1. 加载图片
		std::vector<cv::Mat> images;
		for (const auto &path : imagePaths)
		{
			cv::Mat img = cv::imread(path);
			if (!img.empty())
			{
				images.push_back(img);
			}
			spdlog::info("Loaded image: {}", path);
		}

		if (images.empty())
		{
			spdlog::error("No valid images loaded");
			return false;
		}

		// 2. 计算自动的行列数
		if (rows <= 0 || cols <= 0)
		{
			int total = images.size();
			rows = static_cast<int>(std::ceil(std::sqrt(total)));
			cols = static_cast<int>(std::ceil(static_cast<double>(total) / rows));
			spdlog::info("Auto calculated rows: {}, cols: {}", rows, cols);
		}

		// 3. 处理每张图片
		for (size_t i = 0; i < images.size(); ++i)
		{
			cv::Mat &img = images[i];

			// 添加序号
			if (addSequence)
			{
				DrawSequence(img, static_cast<int>(i));
			}

			// 添加日期时间
			if (addDateTime)
			{
				DrawDateTime(img, imagePaths[i]);
			}

			// 添加马赛克
			if (addMosaic)
			{
				cv::Rect rect_target;
				if (FindLineEdit(img, rect_target))
				{
					int x = rect_target.x + OFFSET_X_MOSAIC;
					int y = rect_target.y + OFFSET_Y_MOSAIC;
					int w = WIDTH_MOSAIC;
					int h = HEIGHT_MOSAIC;

					cv::Mat img_mosaic = img(cv::Rect(x, y, w, h)).clone();
					cv::GaussianBlur(img_mosaic, img_mosaic, cv::Size(25, 25), 0);
					img_mosaic.copyTo(img(cv::Rect(x, y, w, h)));
				}
			}
		}

		// 4. 创建图片网格
		cv::Mat grid = CreateImageGrid(images, rows, cols, margin);

		// 5. 保存结果
		spdlog::info("Saving result to: {}", outputPath);
		if (!cv::imwrite(outputPath, grid))
		{
			spdlog::error("Failed to save image to {}", outputPath);
			return false;
		}

		spdlog::info("Successfully processed and saved image to {}", outputPath);
		return true;
	}
	catch (const std::exception &e)
	{
		spdlog::error("Error processing images: {}", e.what());
		return false;
	}
}

int main(int argc, char **argv)
{
	spdlog::set_level(spdlog::level::debug);

	// 如果没有参数，直接启动GUI
	if (argc == 1)
	{
		QApplication app(argc, argv);
		MainWindow w;
		w.show();
		return app.exec();
	}

	try
	{
		// 1. 解析命令行参数
		cxxopts::Options options(argv[0], "Image stitching and processing tool");
		options.add_options()("i,input", "Input files or directories", cxxopts::value<std::vector<std::string>>())("r,rows", "Number of rows (0 for auto)", cxxopts::value<int>()->default_value("0"))("c,cols", "Number of columns (0 for auto)", cxxopts::value<int>()->default_value("0"))("m,margin", "Margin between images", cxxopts::value<int>()->default_value("10"))("o,output", "Output file path", cxxopts::value<std::string>()->default_value("stitched_image.png"))("s,sequence", "Add sequence numbers")("d,datetime", "Add datetime stamps")("M,mosaic", "Add mosaic effect")("h,help", "Print help");

		auto result = options.parse(argc, argv);

		// 2. 处理帮助选项
		if (result.count("help"))
		{
			spdlog::info(options.help());
			return 0;
		}

		// 3. 检查输入参数
		if (!result.count("input"))
		{
			spdlog::error("No input files or directories specified");
			spdlog::info(options.help());
			return 1;
		}

		// 4. 收集所有图片路径
		auto inputs = result["input"].as<std::vector<std::string>>();
		auto imagePaths = CollectImagePaths(inputs);

		if (imagePaths.empty())
		{
			spdlog::error("No valid image files found");
			return 1;
		}

		// 5. 处理图片
		bool success = ProcessImages(imagePaths,
									 result["rows"].as<int>(),
									 result["cols"].as<int>(),
									 result["margin"].as<int>(),
									 result["output"].as<std::string>(),
									 result.count("sequence"),
									 result.count("datetime"),
									 result.count("mosaic"));

		return success ? 0 : 1;
	}
	catch (const std::exception &e)
	{
		spdlog::error("Error: {}", e.what());
		return 1;
	}
}