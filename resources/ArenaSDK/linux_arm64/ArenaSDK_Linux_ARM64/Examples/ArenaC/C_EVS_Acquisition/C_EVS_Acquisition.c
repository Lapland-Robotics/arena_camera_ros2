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
#include <inttypes.h> // defines macros for printf functions
#include <stdbool.h>  // defines boolean type and values
#include <stdio.h>
#include <stdint.h>


#if (!defined _WIN32 && !defined _WIN64)
#define scanf_s scanf
#endif

#define TAB1 "  "
#define TAB2 "    "
#define TAB3 "     "

// Acquisition: EVS
//    This example demonstrates how to acquire images using the EVS
//    (Electronic Viewfinder System) stream protocol, which is designed
//    to provide efficient and high-quality image transfer from the camera
//    to the host system.

// =-=-=-=-=-=-=-=-=-
// =-=- SETTINGS =-=-
// =-=-=-=-=-=-=-=-=-

// image timeout
#define IMAGE_TIMEOUT 2000

// number of images to grab
#define NUM_IMAGES 25

// maximum buffer length
#define MAX_BUF 1024

// timeout for detecting camera devices (in milliseconds).
#define SYSTEM_TIMEOUT 100

// =-=-=-=-=-=-=-=-=-
// =-=- HELPER =-=-=-
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

// =-=-=-=-=-=-=-=-=-
// =-=- EXAMPLE -=-=-
// =-=-=-=-=-=-=-=-=-

void GetEvsEventRate(double rate, char* buffer, size_t buffer_size)
{
	if (rate < 1000)
	{
		snprintf(buffer, buffer_size, "%.0f ev/s", rate);
	}
	else if (rate < 1000 * 1000)
	{
		snprintf(buffer, buffer_size, "%.1f Kev/s", rate / 1000);
	}
	else if (rate < 1000 * 1000 * 1000)
	{
		snprintf(buffer, buffer_size, "%.1f Mev/s", rate / (1000 * 1000));
	}
	else
	{
		snprintf(buffer, buffer_size, "%.1f Gev/s", rate / (1000 * 1000 * 1000));
	}
}

void GetEvsGvspFrameRate(double rate, char* buffer, size_t buffer_size)
{
	snprintf(buffer, buffer_size, "%.0f Bid/s", rate);
}

void GetEvsLinkThroughput(double rate, char* buffer, size_t buffer_size)
{
	if (rate < 1000)
	{
		snprintf(buffer, buffer_size, "%.0f Bps", rate);
	}
	else if (rate < 1000 * 1000)
	{
		snprintf(buffer, buffer_size, "%.1f KBps", rate / 1000);
	}
	else if (rate < 1000 * 1000 * 1000)
	{
		snprintf(buffer, buffer_size, "%.1f MBps", rate / (1000 * 1000));
	}
	else
	{
		snprintf(buffer, buffer_size, "%.1f GBps", rate / (1000 * 1000 * 1000));
	}
}

// demonstrates acquisition
// (1) sets acquisition mode
// (2) sets buffer handling mode
// (3) sets Event Format to EVS and camera event rate to 10 Mev/s
// (4) sets EVS output format to CDFrame
// (5) sets EVS accumulation time to auto
// (6) starts the stream
// (7) gets a number of images
// (8) prints information from images
// (9) requeues buffers
// (10) stops the stream
AC_ERROR AcquireImages(acDevice hDevice)
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

		// set evs output format to CDFrame
		printf("%sSet EVS output format to CDFrame\n", TAB1);

		err = acNodeMapSetStringValue(hTLStreamNodeMap, "StreamEvsOutputFormat", "CDFrame");
		if (err != AC_ERR_SUCCESS)
			return err;

		// Set evs accumulation time to auto
		printf("%sSet EVS Accumulation Time to auto\n", TAB1);

		double frameGenFPS = 0.0;
		
		err = acNodeMapGetFloatValue(hTLStreamNodeMap, "StreamFrameGeneratorFPS", &frameGenFPS);
		if (err != AC_ERR_SUCCESS)
			return err;

		err = acNodeMapSetIntegerValue(hTLStreamNodeMap, "StreamFrameGeneratorAccumTime", (int64_t)(1000000 / frameGenFPS));
		if (err != AC_ERR_SUCCESS)
			return err;

		// start stream
		printf("%sStart stream\n", TAB1);

		err = acDeviceStartStream(hDevice);
		if (err != AC_ERR_SUCCESS)
			return err;

		// get images
		printf("%sGetting %d images\n", TAB1, NUM_IMAGES);

		int i = 0;
		for (i = 0; i < NUM_IMAGES; i++)
		{
			// get image
			printf("%sGet image %d\n", TAB2, i);
			acBuffer hBuffer = NULL;

			err = acDeviceGetBuffer(hDevice, IMAGE_TIMEOUT, &hBuffer);
			if (err != AC_ERR_SUCCESS)
				return err;

			bool8_t isIncomplete = false;
			err = acBufferIsIncomplete(hBuffer, &isIncomplete);
			if (err != AC_ERR_SUCCESS)
				return err;
			if (isIncomplete)
			{
				printf("%sImage is incomplete.\n", TAB3);
				return err;
			}

			// get EVS event rate
			double eventRate = 0.0;

			acNode hStreamEvsEventRateNode = NULL;
			err = acNodeMapGetNode(hTLStreamNodeMap, "StreamEvsEventRate", &hStreamEvsEventRateNode);
			if (err != AC_ERR_SUCCESS)
				return err;

			err = acFloatGetValue(hStreamEvsEventRateNode, &eventRate);
			if (err != AC_ERR_SUCCESS)
				return err;

			char buffer[64];
			GetEvsEventRate(eventRate, buffer, sizeof(buffer));

			printf("%sEvent Rate: %s\n", TAB3, buffer);

			// get EVS GVSP frame rate
			double gvspFrameRate = 0.0;

			acNode hStreamEvsGvspFrameRateNode = NULL;
			err = acNodeMapGetNode(hTLStreamNodeMap, "StreamEvsGvspFrameRate", &hStreamEvsGvspFrameRateNode);
			if (err != AC_ERR_SUCCESS)
				return err;

			err = acFloatGetValue(hStreamEvsGvspFrameRateNode, &gvspFrameRate);
			if (err != AC_ERR_SUCCESS)
				return err;

			char buffer2[64];
			GetEvsGvspFrameRate(gvspFrameRate, buffer2, sizeof(buffer2));

			printf("%sGVSP Frame Rate: %s\n", TAB3, buffer2);

			// get EVS link throughput
			double linkThroughput = 0.0;

			acNode hStreamEvsLinkThroughputNode = NULL;
			err = acNodeMapGetNode(hTLStreamNodeMap, "StreamEvsLinkThroughput", &hStreamEvsLinkThroughputNode);
			if (err != AC_ERR_SUCCESS)
				return err;

			err = acFloatGetValue(hStreamEvsLinkThroughputNode, &linkThroughput);
			if (err != AC_ERR_SUCCESS)
				return err;

			char buffer3[64];
			GetEvsLinkThroughput(linkThroughput, buffer3, sizeof(buffer3));

			printf("%sLink Throughput: %s\n", TAB3, buffer3);
			
			// requeue image buffer
			err = acDeviceRequeueBuffer(hDevice, hBuffer);
			if (err != AC_ERR_SUCCESS)
				return err;
		}

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
	}
	else {

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

	printf("Select device:\n");
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
	printf("C_EVS_Acquisition\n");
	AC_ERROR err = AC_ERR_SUCCESS;

	// prepare example
	acSystem hSystem = NULL;
	err = acOpenSystem(&hSystem);
	CHECK_RETURN;
	err = acSystemUpdateDevices(hSystem, SYSTEM_TIMEOUT);
	CHECK_RETURN;
	size_t numDevices = 0;
	err = acSystemGetNumDevices(hSystem, &numDevices);
	CHECK_RETURN;
	if (numDevices == 0)
	{
		printf("\nNo camera connected\nPress enter to complete\n");
		getchar();
		return -1;
	}
	acDevice hDevice = NULL;
	size_t selection = 0;
	err = SelectDevice(hSystem, &numDevices, &selection);
	CHECK_RETURN;
	err = acSystemCreateDevice(hSystem, selection, &hDevice);
	CHECK_RETURN;

	// run example
	printf("Commence example\n\n");
	err = AcquireImages(hDevice);
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
	return -1;
}
