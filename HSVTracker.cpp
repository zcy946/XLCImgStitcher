void HSVTracker(cv::Mat &img)
{
    // 读取图像
    cv::Mat img_hsv, img_mask;
    if (img.empty())
    {
        spdlog::error("Failed to load image");
        return;
    }

    // 转换为hsv
    cv::cvtColor(img, img_hsv, cv::COLOR_RGB2HSV);

    int hmin = 0, smin = 0, vmin = 0;
    int hmax = 179, smax = 255, vmax = 255;

    cv::namedWindow("trackbars");
    cv::createTrackbar("Hue Min", "trackbars", &hmin, 179);
    cv::createTrackbar("Hue Max", "trackbars", &hmax, 179);
    cv::createTrackbar("Sat Min", "trackbars", &smin, 255);
    cv::createTrackbar("Sat Max", "trackbars", &smax, 255);
    cv::createTrackbar("Val Min", "trackbars", &vmin, 255);
    cv::createTrackbar("Val Max", "trackbars", &vmax, 255);

    while (true)
    {
        cv::Scalar lower(hmin, smin, vmin);
        cv::Scalar upper(hmax, smax, vmax);
        cv::inRange(img_hsv, lower, upper, img_mask);

        // 展示
        cv::imshow("image", img);
        cv::imshow("img_mask", img_mask);

        cv::waitKey(1);
    }
}