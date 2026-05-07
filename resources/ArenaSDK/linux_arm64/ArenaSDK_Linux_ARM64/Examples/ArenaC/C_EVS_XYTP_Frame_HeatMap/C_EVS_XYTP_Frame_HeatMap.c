/***************************************************************************************
 ***                                                                                 ***
 ***  Copyright (c) 2025, Lucid Vision Labs, Inc.                                    ***
 ***                                                                                 ***
 ***  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     ***
 ***  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       ***
 ***  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    ***
 ***  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         ***
 ***  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  ***
 ***  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  ***
 ***  SOFTWARE.                                                                      ***
 ***                                                                                 ***
 ***************************************************************************************/
#include "stdafx.h"
#include "ArenaCApi.h"
#include "SaveCApi.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <float.h>

#if (!defined _WIN32 && !defined _WIN64)
#define scanf_s scanf
#endif

#define TAB1 "  "
#define TAB2 "    "

// EVS: XYTP Heat Map
//    This example demonstrates saving a BGR heatmap of a XYTP frame. It captures
//    events as XYTP frame from an EVS camera, interprets the XYTP data to retrieve
//    the time value for each event pixel, then converts this data into a BGR buffer.
//    The buffer is used to create a jpg heatmap image.

// =-=-=-=-=-=-=-=-=-
// =-=- SETTINGS =-=-
// =-=-=-=-=-=-=-=-=-

// file name
#define JPG_FILE_NAME "Images/C_EVS_XYTP_Frame_HeatMap.jpg"

// pixel format
#define PIXEL_FORMAT PFNC_BGR8 // BGR8

// image timeout
#define IMAGE_TIMEOUT 2000

// device timeout
#define DEVICE_TIMEOUT 100

// maximum buffer length
#define MAX_BUF 1024

// =-=-=-=-=-=-=-=-=-
// =-=- EXAMPLE -=-=-
// =-=-=-=-=-=-=-=-=-

// gets node value
// (1) gets node
// (2) checks access mode
// (3) gets value
AC_ERROR GetNodeValue(acNodeMap hNodeMap, const char* nodeName, char* pValue, size_t* pLen)
{
	AC_ERROR err = AC_ERR_SUCCESS;

	// get node
	acNode hNode = NULL;
	AC_ACCESS_MODE accessMode = 0;

	err = acNodeMapGetNodeAndAccessMode(hNodeMap, nodeName, &hNode, &accessMode);
	if (err != AC_ERR_SUCCESS)
		return err;

	// check access mode
	if (accessMode != AC_ACCESS_MODE_RO && accessMode != AC_ACCESS_MODE_RW)
		return AC_ERR_ERROR;

	// get value
	err = acValueToString(hNode, pValue, pLen);
	return err;
}

// sets node value
// (1) gets node
// (2) checks access mode
// (3) gets value
AC_ERROR SetNodeValue(acNodeMap hNodeMap, const char* nodeName, const char* pValue)
{
	AC_ERROR err = AC_ERR_SUCCESS;

	// get node
	acNode hNode = NULL;
	AC_ACCESS_MODE accessMode = 0;

	err = acNodeMapGetNodeAndAccessMode(hNodeMap, nodeName, &hNode, &accessMode);
	if (err != AC_ERR_SUCCESS)
		return err;

	// check access mode
	if (accessMode != AC_ACCESS_MODE_WO && accessMode != AC_ACCESS_MODE_RW)
		return AC_ERR_ERROR;

	// get value
	err = acValueFromString(hNode, pValue);
	return err;
}

// demonstrates saving evs heatmap image
// (1) sets acquisition mode
// (2) sets buffer handling mode
// (3) sets Event Format to EVS and camera event rate to 10 Mev/s
// (4) sets EVS output format to XYTPFrame
// (5) starts the stream
// (6) acquires one image
// (7) collects time pixel channel for time range
// (8) sets pixel color buffers based on time bound
// (9) writes jpg heatmap image using output buffer
// (10) stops the stream
// (11) requeues the buffer & cleans up
AC_ERROR AcquireImageAndCreateHeatMapColoring(acDevice hDevice)
{
	AC_ERROR err = AC_ERR_SUCCESS;

	// get node map
	acNodeMap hNodeMap = NULL;

	err = acDeviceGetNodeMap(hDevice, &hNodeMap);
	if (err != AC_ERR_SUCCESS)
		return err;

	// get node values that will be changed in order to return their values at
	// the end of the example
	char pAcquisitionModeInitial[MAX_BUF];
	size_t len = MAX_BUF;

	err = GetNodeValue(hNodeMap, "AcquisitionMode", pAcquisitionModeInitial, &len);
	if (err != AC_ERR_SUCCESS)
		return err;

	// set acquisition mode
	printf("%sSet acquisition mode to 'Continuous'\n", TAB1);

	err = SetNodeValue(
		hNodeMap,
		"AcquisitionMode",
		"Continuous");

	if (err != AC_ERR_SUCCESS)
		return err;

	// set buffer handling mode
	printf("%sSet buffer handling mode to 'NewestOnly'\n", TAB1);

	// get stream node map
	acNodeMap hTLStreamNodeMap = NULL;

	err = acDeviceGetTLStreamNodeMap(hDevice, &hTLStreamNodeMap);
	if (err != AC_ERR_SUCCESS)
		return err;

	err = SetNodeValue(
		hTLStreamNodeMap,
		"StreamBufferHandlingMode",
		"NewestOnly");

	// The EventFormat node determines whether the camera can use the EVS datastream engine.
	// When set to EVS, Arena switches to the EVS engine. If EVS is not supported,
	// the acquisition mode is restored to its original setting, and the process is exited.

	acNode hEventFormatNode = NULL;
	AC_ACCESS_MODE accessModeEventFormat = 0;

	err = acNodeMapGetNodeAndAccessMode(hNodeMap, "EventFormat", &hEventFormatNode, &accessModeEventFormat);
	if (err != AC_ERR_SUCCESS)
		return err;

	if (accessModeEventFormat != AC_ACCESS_MODE_NI)
	{
		// get node value
		char pEventFormatInitial[MAX_BUF];
		size_t len = MAX_BUF;

		err = GetNodeValue(hNodeMap, "EventFormat", pEventFormatInitial, &len);
		if (err != AC_ERR_SUCCESS)
			return err;

		printf("%sSet Event Format to EVT3.0\n", TAB1);

		err = acNodeMapSetEnumerationValue(hNodeMap, "EventFormat", "EVT3_0");
		if (err != AC_ERR_SUCCESS)
			return err;

		// set camera event rate to 10 Mev/s
		printf("%sSet Camera Event Rate to 10 Mev/s\n", TAB1);
		bool8_t ercEnableInitial = false;
		err = acNodeMapGetBooleanValue(hNodeMap, "ErcEnable", &ercEnableInitial);
		if (err != AC_ERR_SUCCESS)
			return err;

		err = acNodeMapSetBooleanValue(hNodeMap, "ErcEnable", true);
		if (err != AC_ERR_SUCCESS)
			return err;

		double cameraEventRateInitial = 0.0;

		err = acNodeMapGetFloatValue(hNodeMap, "ErcRateLimit", &cameraEventRateInitial);
		if (err != AC_ERR_SUCCESS)
			return err;

		err = acNodeMapSetFloatValue(hNodeMap, "ErcRateLimit", 10.0);
		if (err != AC_ERR_SUCCESS)
			return err;

		// Configure EVS output format to XYTPFrame
		// Setting the XYTPFrame format will instruct Arena to generate a 3D Arena::IImage object
		// with a pixel format of ABCY16.
		printf("%sSet EVS output format to XYTPFrame\n", TAB1);

		err = acNodeMapSetStringValue(hTLStreamNodeMap, "StreamEvsOutputFormat", "XYTPFrame");
		if (err != AC_ERR_SUCCESS)
			return err;

		// start stream
		printf("%sStart stream\n", TAB1);

		err = acDeviceStartStream(hDevice);
		if (err != AC_ERR_SUCCESS)
			return err;

		// acquire image
		printf("%sGet one image\n", TAB2);
		acBuffer hBuffer = NULL;
		err = acDeviceGetBuffer(hDevice, IMAGE_TIMEOUT, &hBuffer);
		if (err != AC_ERR_SUCCESS)
			return err;

		// prepare input buffer info
		size_t width = 0;
		size_t height = 0;
		size_t srcBpp = 0;
		size_t srcSizeFilled = 0;
		uint8_t* pInput = NULL;

		err = acImageGetWidth(hBuffer, &width) |
			  acImageGetHeight(hBuffer, &height) |
			  acImageGetBitsPerPixel(hBuffer, &srcBpp) |
			  acImageGetData(hBuffer, &pInput) |
			  acBufferGetSizeFilled(hBuffer, &srcSizeFilled);
		if (err != AC_ERR_SUCCESS)
			return err;

		size_t size = width * height;
		size_t srcPixelSize = srcBpp / 8;
		const float* pIn = (const float*)pInput;
		size_t validEventSize = srcSizeFilled / srcPixelSize;

		// prepare memory output buffer
		size_t dstBpp = 0;
		err = acGetBitsPerPixel(PIXEL_FORMAT, &dstBpp);
		if (err != AC_ERR_SUCCESS)
			return err;

		size_t dstDataSize = size * (dstBpp / 8);
		uint8_t* pOutput = (uint8_t*)malloc(dstDataSize);
		memset(pOutput, 0, dstDataSize);

		// find min and max time values
		float minT = FLT_MAX;
		float maxT = -FLT_MAX;
		for (size_t i = 0; i < validEventSize; i++)
		{
			float t = *(pIn + 2);
			if (t < minT)
				minT = t;
			if (t > maxT)
				maxT = t;
			pIn += 4;
		}

		// reset input pointer
		pIn = (const float*)pInput;

		// determine color boundaries
		double range = maxT - minT;
		double yellowColorBorder = minT + range / 4;
		double greenColorBorder = minT + 2 * range / 4;
		double cyanColorBorder = minT + 3 * range / 4;
		double blueColorBorder = maxT;

		int RGBmax = 255;
		int RGBmin = 0;

		// pixel color assignment based on time
		for (size_t i = 0; i < validEventSize; i++)
		{
			float x = *pIn;
			float y = *(pIn + 1);
			float t = *(pIn + 2);
			pIn += 4;

			double coordinateColorRed, coordinateColorGreen, coordinateColorBlue;

			// map colors based on t within the defined boundaries
			if (t <= yellowColorBorder)
			{
				double yellowColorPercentage = (t - minT) / (yellowColorBorder - minT);
				coordinateColorRed = RGBmax;
				coordinateColorGreen = RGBmax * yellowColorPercentage;
				coordinateColorBlue = RGBmin;
			}
			else if (t <= greenColorBorder)
			{
				double greenColorPercentage = (t - yellowColorBorder) / (greenColorBorder - yellowColorBorder);
				coordinateColorRed = RGBmax - RGBmax * greenColorPercentage;
				coordinateColorGreen = RGBmax;
				coordinateColorBlue = RGBmin;
			}
			else if (t <= cyanColorBorder)
			{
				double cyanColorPercentage = (t - greenColorBorder) / (cyanColorBorder - greenColorBorder);
				coordinateColorRed = RGBmin;
				coordinateColorGreen = RGBmax;
				coordinateColorBlue = RGBmax * cyanColorPercentage;
			}
			else
			{
				double blueColorPercentage = (t - cyanColorBorder) / (blueColorBorder - cyanColorBorder);
				coordinateColorRed = RGBmin;
				coordinateColorGreen = RGBmax - RGBmax * blueColorPercentage;
				coordinateColorBlue = RGBmax;
			}

			// calculate offset using x and y positions
			int xPos = (int)x;
			int yPos = (int)y;
			size_t offset = (yPos * width + xPos) * (dstBpp / 8);

			// set BGR values for JPG output
			pOutput[offset] = (uint8_t)coordinateColorBlue;
			pOutput[offset + 1] = (uint8_t)coordinateColorGreen;
			pOutput[offset + 2] = (uint8_t)coordinateColorRed;
		}

		printf("%sCreate BGR heatmap from XYTP Frame\n", TAB2);

		saveWriter hJpegWriter = NULL;
		err = saveWriterCreate(width, height, dstBpp, &hJpegWriter);
		if (err != AC_ERR_SUCCESS)
			return err;

		// save image as jpg
		printf("%sSave heatmap image as jpg to ", TAB2);

		// create image from buffer and save
		acBuffer hJpegCreate = NULL;
		uint8_t* hJpegImageData = NULL;
		const char* pJpegFileNamePattern = JPG_FILE_NAME;

		err = acImageFactoryCreate(pOutput, dstDataSize, width, height, PIXEL_FORMAT, &hJpegCreate);
		if (err != AC_ERR_SUCCESS)
			return err;

		err = acImageGetData(hJpegCreate, &hJpegImageData);
		if (err != AC_ERR_SUCCESS)
			return err;

		err = saveWriterSetFileNamePattern(hJpegWriter, pJpegFileNamePattern);
		if (err != AC_ERR_SUCCESS)
			return err;

		err = saveWriterSave(hJpegWriter, hJpegImageData);
		if (err != AC_ERR_SUCCESS)
			return err;

		char jpegFileName[MAX_BUF];
		size_t jpegFileLen = MAX_BUF;

		err = saveWriterGetLastFileName(hJpegWriter, jpegFileName, &jpegFileLen);
		if (err != AC_ERR_SUCCESS)
			return err;
		printf("%s\n", jpegFileName);

		// stop stream
		printf("%sStop stream\n", TAB1);

		err = acDeviceStopStream(hDevice);
		if (err != AC_ERR_SUCCESS)
			return err;

		// return node to its initial values
		err = acNodeMapSetBooleanValue(hNodeMap, "ErcEnable", ercEnableInitial);
		if (err != AC_ERR_SUCCESS)
			return err;

		err = acNodeMapSetFloatValue(hNodeMap, "ErcRateLimit", cameraEventRateInitial);
		if (err != AC_ERR_SUCCESS)
			return err;

		err = SetNodeValue(hNodeMap, "EventFormat", pEventFormatInitial);
		if (err != AC_ERR_SUCCESS)
			return err;

		// clean up
		free(pOutput);
		acDeviceRequeueBuffer(hDevice, hBuffer);
		acDeviceStopStream(hDevice);
		saveWriterDestroy(hJpegWriter);
	}
	else
	{

		printf("%s\nConnected camera does not support any EventFormats\n", TAB1);
	}

	// return node to its initial values
	err = SetNodeValue(
		hNodeMap,
		"AcquisitionMode",
		pAcquisitionModeInitial);

	return err;
}

// =-=-=-=-=-=-=-=-=-
// =- PREPARATION -=-
// =- & CLEAN UP =-=-
// =-=-=-=-=-=-=-=-=-

AC_ERROR SelectDevice(acSystem hSystem, size_t* pNumDevices, size_t* pSelection)
{
	AC_ERROR err = AC_ERR_SUCCESS;

	if (*pNumDevices == 1)
	{
		printf(TAB1 "Only one device detected, automatically selecting this device.\n");
		*pSelection = 0;
		return AC_ERR_SUCCESS;
	}

	printf(TAB1 "Select device:\n");
	for (size_t i = 0; i < *pNumDevices; i++)
	{
		// get device model
		char pDeviceModel[MAX_BUF];
		size_t pDeviceModelLen = MAX_BUF;
		err = acSystemGetDeviceModel(hSystem, i, pDeviceModel, &pDeviceModelLen);
		if (err != AC_ERR_SUCCESS)
			return err;

		// get device serial
		char pDeviceSerial[MAX_BUF];
		size_t pDeviceSerialLen = MAX_BUF;
		err = acSystemGetDeviceSerial(hSystem, i, pDeviceSerial, &pDeviceSerialLen);
		if (err != AC_ERR_SUCCESS)
			return err;

		// get device IP address
		char pIpAddressStr[MAX_BUF];
		size_t pIpAddressStrBufLen = MAX_BUF;
		err = acSystemGetDeviceIpAddressStr(hSystem, i, pIpAddressStr, &pIpAddressStrBufLen);
		if (err != AC_ERR_SUCCESS)
			return err;

		printf(TAB2 "%zu. %s%s%s%s%s\n", i + 1, pDeviceModel, TAB1, pDeviceSerial, TAB1, pIpAddressStr);
	}

	do
	{
		printf(TAB1 "Make selection (1-%zu): ", *pNumDevices);

		if (scanf_s("%zu", pSelection) != 1)
		{
			while (getchar() != '\n')
				;
			printf(TAB1 "Invalid input. Please enter a number.\n");
			continue;
		}

		if (*pSelection <= 0 || *pSelection > *pNumDevices)
		{
			printf(TAB1 "Invalid device selected. Please select a device in the range (1-%zu).\n", *pNumDevices);
		}
	} while (*pSelection <= 0 || *pSelection > *pNumDevices);

	*pSelection -= 1;
	return AC_ERR_SUCCESS;
}

// error buffer length
#define ERR_BUF 512

#define CHECK_RETURN                                  \
	if (err != AC_ERR_SUCCESS)                        \
	{                                                 \
		char pMessageBuf[ERR_BUF];                    \
		size_t pBufLen = ERR_BUF;                     \
		acGetLastErrorMessage(pMessageBuf, &pBufLen); \
		printf("\nError: %s", pMessageBuf);           \
		printf("\n\nPress enter to complete\n");      \
		getchar();                                    \
		return -1;                                    \
	}

int main()
{
	printf("C_EVS_XYTP_Frame_HeatMap\n");
	AC_ERROR err = AC_ERR_SUCCESS;

	// prepare example
	acSystem hSystem = NULL;
	err = acOpenSystem(&hSystem);
	CHECK_RETURN;
	err = acSystemUpdateDevices(hSystem, DEVICE_TIMEOUT);
	CHECK_RETURN;
	size_t numDevices = 0;
	err = acSystemGetNumDevices(hSystem, &numDevices);
	CHECK_RETURN;
	if (numDevices == 0)
	{
		printf("\nNo camera connected\nPress enter to complete\n");
		getchar();
		return 0;
	}
	acDevice hDevice = NULL;
	size_t selection = 0;
	err = SelectDevice(hSystem, &numDevices, &selection);
	CHECK_RETURN;
	err = acSystemCreateDevice(hSystem, selection, &hDevice);
	CHECK_RETURN;

	printf("Commence example\n\n");

	// run example
	err = AcquireImageAndCreateHeatMapColoring(hDevice);

	CHECK_RETURN;
	printf("\nExample complete\n");

	// clean up example
	err = acSystemDestroyDevice(hSystem, hDevice);
	CHECK_RETURN;
	err = acCloseSystem(hSystem);
	CHECK_RETURN;

	printf("Press enter to complete\n");
	while (getchar() != '\n') {};
	getchar();
	return 0;
}
