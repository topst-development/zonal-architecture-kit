/*
 * Copyright Telechips Inc.
 *
 * TCC Version 1.0
 *
 * This source code contains confidential information of Telechips.
 *
 * Any unauthorized use without a written permission of Telechips including not
 * limited to re-distribution in source or binary form is strictly prohibited.
 *
 * This source code is provided "AS IS" and nothing contained in this source code
 * shall constitute any express or implied warranty of any kind, including without
 * limitation, any warranty of merchantability, fitness for a particular purpose
 * or non-infringement of any patent, copyright or other third party intellectual
 * property right.
 * No warranty is made, express or implied, regarding the information's accuracy,
 * completeness, or performance.
 *
 * In no event shall Telechips be liable for any claim, damages or other
 * liability arising from, out of or in connection with this source code or
 * the use in the source code.
 *
 * This source code is provided subject to the terms of a Mutual Non-Disclosure
 * Agreement between Telechips and Company.
 */

#include <iostream>
#include <sys/time.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

#include "opencv_api.h"

/**
 * @brief Draws dynamic information (FPS, Network Info, Memory Usage, NPU Info) on an image.
 *
 * This function draws various types of information (e.g., FPS, Network Info, Memory Usage, NPU Info)
 * on an image at a specified location with adjustable font size, color, and position based on the scaling
 * of the destination size. The text is drawn on the image using the specified parameters, including type of
 * information, font size, color, and position.
 *
 * @param desBuffer A pointer to the destination image buffer (in RGB format).
 * @param width The width of the destination image (in pixels).
 * @param height The height of the destination image (in pixels).
 * @param infoType The type of information to be displayed (e.g., "FPS", "Network", "Memory", or "NPU").
 * @param value The value to be displayed (e.g., FPS value, network time in ms, memory usage in %, NPU utilization in %).
 * @param deviceIdx The index of the device for Network and NPU information (e.g., network index, NPU index).
 * @param xPos The x-axis position (in percentage of image width) where the text will be drawn.
 * @param yPos The y-axis position (in percentage of image height) where the text will be drawn.
 * @param textColor The color of the text in BGR format (e.g., `cv::Scalar(255, 0, 0)` for blue).
 * @param fontSize A scaling factor for the font size, based on the image dimensions.
 *
 * @return None. This function does not return a value.
 *
 * @note The x and y positions are provided as percentages of the image width and height, and are scaled
 * accordingly based on the destination image dimensions.
 *
 * @note The function handles different types of information based on the @p infoType string. Valid values for
 * @p infoType are "FPS", "Network", "Memory", and "NPU".
 */
void cvDrawInfo(unsigned char *desBuffer, uint32_t outputWidth, uint32_t outputHeight,
				draw_info_t infoType, double value, int deviceIdx,
				int xPos, int yPos, double fontSize, const Color_t textColor)
{
	cv::Mat image(cv::Size(outputWidth, outputHeight), CV_8UC3, desBuffer);

	char buf[30];

	if (infoType == DRAW_INFO_FPS)
	{
		sprintf(buf, "FPS: %.1f", value);
	}
	else if (infoType == DRAW_INFO_NETWORK)
	{
		sprintf(buf, "NET%d: %dms", deviceIdx, static_cast<int>(value));
	}
	else if (infoType == DRAW_INFO_CPU)
	{
		sprintf(buf, "CPU: %d%%", static_cast<int>(value));
	}
	else if (infoType == DRAW_INFO_MEMORY)
	{
		sprintf(buf, "MEM: %d%%", static_cast<int>(value));
	}
	else if (infoType == DRAW_INFO_NPU)
	{
		sprintf(buf, "NPU%d: %d%%", deviceIdx, static_cast<int>(value));
	}
	else
	{
		return; // Return if infoType is not recognized
	}

	cv::putText(image, buf, cv::Point(xPos, yPos), cv::FONT_HERSHEY_SIMPLEX, fontSize, cv::Scalar(textColor.b, textColor.g, textColor.r));
}

/**
 * @brief Draws bounding boxes and class labels on an image with dynamic parameters.
 *
 * This function draws bounding boxes and class labels on an image at specified locations
 * with adjustable font size, color, and position based on the scaling of the destination size.
 *
 * @param desBuffer A pointer to the destination image buffer (in RGB format).
 * @param boxes An array of `box_t` structs containing the bounding boxes and class information.
 * @param boxCount The number of bounding boxes to be drawn.
 * @param width The width of the destination image (in pixels).
 * @param height The height of the destination image (in pixels).
 * @param boxColor The color of the bounding box (e.g., `cv::Scalar(255, 0, 0)` for blue).
 * @param textColor The color of the text in BGR format (e.g., `cv::Scalar(255, 0, 0)` for blue).
 * @param fontSize A scaling factor for the font size, based on the image dimensions.
 * @param textOffset The y-axis offset for the label text from the bounding box (relative to `yMin`).
 *
 * @return None. This function does not return a value.
 */
void cvDrawBoxes(unsigned char *desBuffer, Box_t *boxes, int boxCount,
				 uint32_t outputWidth, uint32_t outputHeight, uint32_t boxImageWidth, uint32_t boxImageHeight,
				 const Color_t boxColor, const Color_t textColor,
				 double fontSize, int textOffset)
{
	char buf[30];
	cv::Mat image(cv::Size(outputWidth, outputHeight), CV_8UC3, desBuffer);

	for (int boxIdx = 0; boxIdx < boxCount; boxIdx++)
	{
		/* Coordinates of the box */
		int xMin = boxes[boxIdx].xmin * ((float)outputWidth / boxImageWidth);
		int yMin = boxes[boxIdx].ymin * ((float)outputHeight / boxImageHeight);
		int xMax = boxes[boxIdx].xmax * ((float)outputWidth / boxImageWidth);
		int yMax = boxes[boxIdx].ymax * ((float)outputHeight / boxImageHeight);

		xMin = std::max(0, std::min(xMin, static_cast<int>(outputWidth)));
		yMin = std::max(0, std::min(yMin, static_cast<int>(outputHeight)));
		xMax = std::max(0, std::min(xMax, static_cast<int>(outputWidth)));
		yMax = std::max(0, std::min(yMax, static_cast<int>(outputHeight)));

		/* Prepare the label text with tracking ID, class ID, and score */
		sprintf(buf, "[ID:%d][%d][%.2lf]", boxes[boxIdx].track_id, boxes[boxIdx].cls, boxes[boxIdx].score);
		/* Draw the bounding box on the image */
		cv::rectangle(image, cv::Rect(cv::Point(xMin, yMin), cv::Point(xMax, yMax)), cv::Scalar(boxColor.b, boxColor.g, boxColor.r), 2, 8, 0);

		/* Draw the label text near the bounding box */
		cv::putText(image, buf, cv::Point(xMin, yMin - textOffset), cv::FONT_HERSHEY_SIMPLEX, fontSize, cv::Scalar(textColor.b, textColor.g, textColor.r));
		// cv::putText(image, buf, cv::Point(xMin, yMin - textOffset), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(textColor.b, textColor.g, textColor.r));
	}
}

/**
 * @brief Draws a class label on an image with dynamic parameters.
 *
 * This function draws a class label on the image at a specified location with
 * adjustable font size, color, and position based on the scaling of the destination size.
 *
 * @param desBuffer A pointer to the destination image buffer (in RGB format).
 * @param width The width of the destination image (in pixels).
 * @param height The height of the destination image (in pixels).
 * @param className The name of the class.
 * @param xPos The x-axis position where the class label will be drawn.
 * @param yPos The y-axis position where the class label will be drawn.
 * @param textColor The color of the text in BGR format (e.g., `cv::Scalar(255, 0, 0)` for blue).
 * @param fontSize A scaling factor for the font size, based on the image dimensions.
 *
 * @return None. This function does not return a value.
 */
void cvDrawCls(unsigned char *desBuffer, uint32_t width, uint32_t height,
			   const int className, int xPos, int yPos,
			   const Color_t textColor, double fontSize)
{
	cv::Mat image(cv::Size(width, height), CV_8UC3, desBuffer);

	char buf[30];
	sprintf(buf, "[CLS][%d]", className);

	cv::putText(image, buf, cv::Point(xPos, yPos), cv::FONT_HERSHEY_SIMPLEX, fontSize, cv::Scalar(textColor.b, textColor.g, textColor.r));
}

/**
 * @brief Loads an image file and converts it to RGBA format, storing the result in a buffer.
 *
 * This function reads an image file from the specified path, converts the image data from BGR to RGB,
 * and then converts it to RGBA format, storing the result in the provided `desBuffer`. The function can also
 * return the image's width and height through pointer arguments.
 *
 * @param desBuffer A buffer to store the converted RGBA image data.
 *                  This buffer should be allocated with enough space to hold the image data.
 * @param filePath The path of the image file to be loaded.
 * @param width A pointer to a variable where the width of the image will be stored.
 *              If `nullptr`, the width is not returned.
 * @param height A pointer to a variable where the height of the image will be stored.
 *               If `nullptr`, the height is not returned.
 *
 * @return The total number of bytes of the image. Returns `-1` in case of failure.
 *
 * @note This function uses `cv::imread` to load the image and `cv::cvtColor` to convert the color space.
 *       The returned byte count is `width * height * 4` (for RGBA format with 4 channels).
 */
int cvLoadImage(unsigned char *desBuffer, const char *filePath, uint32_t *width, uint32_t *height)
{
	cv::Mat image = cv::imread(filePath, cv::IMREAD_COLOR);
	if (image.empty())
	{
		std::cerr << "Invalid image file: " << filePath << std::endl;
		return -1;
	}
	cv::Mat imageRGBA;
	cv::cvtColor(image, imageRGBA, cv::COLOR_RGB2RGBA);

	if (width != nullptr && height != nullptr)
	{
		*width = static_cast<uint32_t>(imageRGBA.cols);
		*height = static_cast<uint32_t>(imageRGBA.rows);
	}

	memcpy(desBuffer, imageRGBA.data, imageRGBA.total() * imageRGBA.elemSize());
	return static_cast<int>(imageRGBA.total() * imageRGBA.elemSize());
}

/**
 * @brief Saves an image from the buffer to a file in the specified format.
 *
 * This function takes an image stored in a buffer, converts it into a `cv::Mat` object, and saves it to the
 * specified file path in a standard image format (e.g., PNG, JPG). The function avoids potential memory access
 * issues by creating a deep copy of the image before saving.
 *
 * @param srcBuffer A pointer to the buffer containing the image data. This buffer must contain the image data
 *                  in RGB format with dimensions specified by `width` and `height`.
 * @param filePath The path to the file where the image will be saved. The function will attempt to write the image
 *                 to this file location.
 * @param width The width of the image (in pixels).
 * @param height The height of the image (in pixels).
 *
 * @return None. This function does not return a value.
 *
 * @note This function uses `cv::imwrite` to save the image and performs a deep copy of the `cv::Mat` object
 *       to avoid potential issues with memory access (bus errors).
 *       The image is assumed to be in RGB format (with 3 channels).
 */
void cvSaveImage(unsigned char *srcBuffer, const char *filePath, const uint32_t width, const uint32_t height)
{
	cv::Mat image(cv::Size(width, height), CV_8UC3, srcBuffer);
	if (!cv::imwrite(filePath, image))
	{
		std::cerr << "Failed to save image to " << filePath << std::endl;
	}
}

/**
 * @brief Resizes an image from the source buffer to the destination buffer in the specified format.
 *
 * This function reads an image from the source buffer, resizes it to the specified dimensions,
 * and stores the resized image in the destination buffer. The resizing is performed using OpenCV's
 * `cv::resize` function. The image format can be RGB (3 channels), RGBA (4 channels), or Grayscale (1 channel).
 *
 * @param srcBuffer A pointer to the source image data. This buffer must contain the image in the specified format
 *                  with dimensions specified by `srcW` and `srcH`. The source image can be in RGB, RGBA, or Grayscale format.
 * @param srcW The width of the source image (in pixels).
 * @param srcH The height of the source image (in pixels).
 * @param desBuffer A pointer to the destination buffer where the resized image data will be stored.
 *                  This buffer must be allocated with enough space to hold the resized image data.
 * @param desW The width of the destination image (in pixels).
 * @param desH The height of the destination image (in pixels).
 * @param channels The number of channels in the source image. Should be 1 (Grayscale), 3 (RGB), or 4 (RGBA).
 *
 * @return None. This function does not return a value.
 *
 * @note The destination buffer must have enough space allocated to store the resized image.
 */
void cvResize(unsigned char *srcBuffer, uint32_t srcW, uint32_t srcH,
			  unsigned char *desBuffer, uint32_t desW, uint32_t desH, int channels)
{
	cv::Mat originImage(cv::Size(srcW, srcH), CV_8UC(channels), srcBuffer);
	cv::Mat resizedImage(desH, desW, CV_8UC(channels));
	cv::resize(originImage, resizedImage, cv::Size(desW, desH));
	std::memcpy(desBuffer, resizedImage.data, desW * desH * channels);
}

void cvDrawLines(unsigned char *desBuffer, uint32_t width, uint32_t height,
				 Point_t *P1, Point_t *P2, int N, const Color_t color, int thickness)
{
	cv::Mat image(cv::Size(width, height), CV_8UC3, desBuffer);

	for (int i = 0; i < N; i++)
	{
		cv::Point start(P1[i].x, P1[i].y);
		cv::Point end(P2[i].x, P2[i].y);

		cv::line(image, start, end, cv::Scalar(color.b, color.g, color.r), thickness);
	}
}

/**
 * @brief Draws a series of points (coordinates) on an image.
 *
 * This function draws circles at the given set of points on an image. The points are drawn at
 * the specified positions, and the size and color of the circles are adjustable.
 *
 * @param desBuffer A pointer to the destination image buffer (in RGB format).
 * @param width The width of the destination image (in pixels).
 * @param height The height of the destination image (in pixels).
 * @param points An array of custom `point` structures representing the points to be drawn on the image.
 * @param numPoints The number of points in the `points` array.
 * @param pointRadius The radius of the circles drawn at each point.
 * @param pointColor The color of the points (circles) in BGR format (e.g., `cv::Scalar(0, 0, 255)` for red).
 *
 * @return None. This function does not return a value.
 *
 * @note The points are drawn at the specified pixel positions, and their positions are scaled
 * based on the image size.
 */
void cvDrawPoints(unsigned char *desBuffer, uint32_t width, uint32_t height,
				  Point_t *points, int numPoints, int pointRadius, const Color_t pointColor)
{
	cv::Mat image(cv::Size(width, height), CV_8UC3, desBuffer);

	for (int i = 0; i < numPoints; i++)
	{
		cv::circle(image, cv::Point(points[i].x, points[i].y), pointRadius, cv::Scalar(pointColor.b, pointColor.g, pointColor.r), -1);
	}
}

/**
 * @brief Draws custom text on an image at a specified location.
 *
 * This function allows the user to draw a custom string on an image at a specified location with
 * adjustable font size, color, and position based on the scaling of the destination size.
 * The text is drawn using the specified parameters, including font size, color, and position.
 *
 * @param desBuffer A pointer to the destination image buffer (in RGB format).
 * @param width The width of the destination image (in pixels).
 * @param height The height of the destination image (in pixels).
 * @param customText The custom string to be displayed on the image.
 * @param xPos The x-axis position (in percentage of image width) where the text will be drawn.
 * @param yPos The y-axis position (in percentage of image height) where the text will be drawn.
 * @param textColor The color of the text in BGR format (e.g., `cv::Scalar(255, 0, 0)` for blue).
 * @param fontSize A scaling factor for the font size, based on the image dimensions.
 *
 * @return None. This function does not return a value.
 *
 * @note The x and y positions are provided as percentages of the image width and height, and are scaled
 * accordingly based on the destination image dimensions.
 */
void cvDrawCustomText(unsigned char *desBuffer, uint32_t width, uint32_t height,
					  const char *customText, int xPos, int yPos,
					  const Color_t textColor, double fontSize)
{
	cv::Mat image(cv::Size(width, height), CV_8UC3, desBuffer);

	cv::putText(image, customText, cv::Point(xPos, yPos), cv::FONT_HERSHEY_SIMPLEX, fontSize, cv::Scalar(textColor.b, textColor.g, textColor.r));
}
