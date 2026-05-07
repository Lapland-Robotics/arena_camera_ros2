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

#include <limits>

#include "stdafx.h"
#include "ArenaApi.h"
#include "SaveApi.h"

#define TAB1 "  "
#define TAB2 "    "

// EVS: XYTP Heat Map
//    This example demonstrates saving a BGR heatmap of a XYPT frame. It captures
//    events as XYPT frame from EVS camera, interprets the XYPT data from the frame
//    to retrieve the time value for each event pixel and then converts this data into
//    a BGR buffer. The buffer is then used to create a jpg heatmap image.

// =-=-=-=-=-=-=-=-=-
// =-=- SETTINGS =-=-
// =-=-=-=-=-=-=-=-=-

// file name
#define JPG_FILE_NAME "Images/Cpp_EVS_XYTP_Frame_HeatMap.jpg"

// pixel format
#define PIXEL_FORMAT BGR8

// image timeout
#define IMAGE_TIMEOUT 2000

// =-=-=-=-=-=-=-=-=-
// =-=- EXAMPLE -=-=-
// =-=-=-=-=-=-=-=-=-

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
void AcquireImageAndCreateHeatMapColoring(Arena::IDevice* pDevice)
{
	// get node values that will be changed in order to return their values at
	// the end of the example
	GenICam::gcstring acquisitionModeInitial = Arena::GetNodeValue<GenICam::gcstring>(pDevice->GetNodeMap(), "AcquisitionMode");

	// set acquisition mode
	std::cout << TAB1 << "Set acquisition mode to 'Continuous'\n";

	Arena::SetNodeValue<GenICam::gcstring>(
		pDevice->GetNodeMap(),
		"AcquisitionMode",
		"Continuous");

	// set buffer handling mode
	std::cout << TAB1 << "Set buffer handling mode to 'NewestOnly'\n";

	Arena::SetNodeValue<GenICam::gcstring>(
		pDevice->GetTLStreamNodeMap(),
		"StreamBufferHandlingMode",
		"NewestOnly");

	// The EventFormat node determines whether the camera can use the EVS datastream engine.
	// When set to EVS, Arena switches to the EVS engine. If EVS is not supported,
	// the acquisition mode is restored to its original setting, and the process is exited.

	bool bIsEVSEventFormatImplemented = GenApi::IsImplemented(pDevice->GetNodeMap()->GetNode("EventFormat"));

	if (!bIsEVSEventFormatImplemented)
	{
		std::cout << TAB1 << "\nConnected camera does not support any EventFormats\n";

		// return node to its initial value
		Arena::SetNodeValue<GenICam::gcstring>(pDevice->GetNodeMap(), "AcquisitionMode", acquisitionModeInitial);
		return;
	}

	// get node value
	GenICam::gcstring eventFormatInitial = Arena::GetNodeValue<GenICam::gcstring>(pDevice->GetNodeMap(), "EventFormat");

	std::cout << TAB1 << "Set Event Format to EVT3.0\n";

	Arena::SetNodeValue<GenICam::gcstring>(
		pDevice->GetNodeMap(),
		"EventFormat",
		"EVT3_0");

	// set camera event rate to 10 Mev/s
	std::cout << TAB1 << "Set Camera Event Rate to 10 Mev/s\n";
	bool ercEnableInitial = Arena::GetNodeValue<bool>(pDevice->GetNodeMap(), "ErcEnable");

	Arena::SetNodeValue<bool>(
		pDevice->GetNodeMap(),
		"ErcEnable",
		true);

	double cameraEventRateInitial = Arena::GetNodeValue<double>(pDevice->GetNodeMap(), "ErcRateLimit");
	Arena::SetNodeValue<double>(
		pDevice->GetNodeMap(),
		"ErcRateLimit",
		10.0);

	// set evs output format to XYTPFrame
	std::cout << TAB1 << "Set EVS output format to XYTPFrame\n";

	Arena::SetNodeValue<GenICam::gcstring>(
		pDevice->GetTLStreamNodeMap(),
		"StreamEvsOutputFormat",
		"XYTPFrame");

	// start stream
	std::cout << TAB1 << "Start stream\n";

	pDevice->StartStream();

	std::cout << TAB2 << "Get one image\n";

	// The retrieved image will have a pixel format of LUCID_LucidXYTP128f,
	// where each channel is a 32bit float and xy represents x and y coordinate, t represents time, and p represents p.
	Arena::IImage* pImage = pDevice->GetImage(IMAGE_TIMEOUT);

	if (pImage)
	{
		if (pImage->IsIncomplete())
		{
			std::cout << TAB2 << "Image " << pImage->GetFrameId() << " is incomplete." << std::endl;
			return;
		}
	}
	else
	{
		std::cout << TAB2 << "GetImage() returned a null pointer." << std::endl;
		return;
	}

	// prepare info from input buffer
	const size_t width = pImage->GetWidth();
	const size_t height = pImage->GetHeight();
	const size_t size = width * height;
	const size_t srcBpp = pImage->GetBitsPerPixel();
	const size_t srcPixelSize = srcBpp / 8;
	const uint8_t* srcInput = pImage->GetData();
	const size_t validEventSize = pImage->GetSizeFilled() / srcPixelSize;
	std::cout << TAB2 << "Number of valid event pixel is:" << validEventSize << std::endl;

	// prepare image output buffer
	size_t dstBpp = Arena::GetBitsPerPixel(PIXEL_FORMAT);
	size_t dstPixelSize = dstBpp / 8;
	size_t dstDataSize = size * dstPixelSize;
	uint8_t* pOutput = new uint8_t[dstDataSize];
	memset(pOutput, 0, dstDataSize);

	// manually convert to BGR image
	const float* pIn = reinterpret_cast<const float*>(srcInput);

	const double RGBmin = 0;
	const double RGBmax = 255;

	// because timestamp value during an envet for xytp frame don't have fixed value range for each
	// image captured we have to find t value range to assign color
	float minT = (std::numeric_limits<float>::max)();
	float maxT = (std::numeric_limits<float>::min)();

	// first pass to find the min and max t values
	for (size_t i = 0; i < validEventSize; i++)
	{
		float t = *(pIn + 2);

		if (t > maxT)
			maxT = t;
		if (t < minT)
			minT = t;

		pIn += 4; // move to the next pixel
	}

	// reset pointer to start
	pIn = reinterpret_cast<const float*>(srcInput);

	// set color bound based on the time value range
	double range = maxT - minT;
	double yellowColorBorder = minT + range / 4;
	double greenColorBorder = minT + 2 * range / 4;
	double cyanColorBorder = minT + 3 * range / 4;
	double blueColorBorder = maxT;

	for (size_t i = 0; i < validEventSize; i++)
	{
		float x = *pIn;
		float y = *(pIn + 1);
		float t = *(pIn + 2);
		pIn += 4;

		double coordinateColorBlue, coordinateColorGreen, coordinateColorRed;

		// color mapping logic based on the t value
		if (t <= yellowColorBorder)
		{
			// colors between red and yellow
			double yellowColorPercentage = (t - minT) / (yellowColorBorder - minT);
			coordinateColorRed = RGBmax;
			coordinateColorGreen = RGBmax * yellowColorPercentage;
			coordinateColorBlue = RGBmin;
		}
		else if (t <= greenColorBorder)
		{
			// colors between yellow and green
			double greenColorPercentage = (t - yellowColorBorder) / (greenColorBorder - yellowColorBorder);
			coordinateColorRed = RGBmax - RGBmax * greenColorPercentage;
			coordinateColorGreen = RGBmax;
			coordinateColorBlue = RGBmin;
		}
		else if (t <= cyanColorBorder)
		{
			// colors between green and cyan
			double cyanColorPercentage = (t - greenColorBorder) / (cyanColorBorder - greenColorBorder);
			coordinateColorRed = RGBmin;
			coordinateColorGreen = RGBmax;
			coordinateColorBlue = RGBmax * cyanColorPercentage;
		}
		else if (t <= blueColorBorder)
		{
			// colors between cyan and blue
			double blueColorPercentage = (t - cyanColorBorder) / (blueColorBorder - cyanColorBorder);
			coordinateColorRed = RGBmin;
			coordinateColorGreen = RGBmax - RGBmax * blueColorPercentage;
			coordinateColorBlue = RGBmax;
		}
		else
		{
			coordinateColorRed = RGBmin;
			coordinateColorGreen = RGBmin;
			coordinateColorBlue = RGBmax;
		}

		// calculate the offset using x and y
		int xPos = static_cast<int>(x);
		int yPos = static_cast<int>(y);
		size_t offset = (yPos * width + xPos) * dstPixelSize;

		// set color in BGR format for JPG
		pOutput[offset] = static_cast<uint8_t>(coordinateColorBlue);
		pOutput[offset + 1] = static_cast<uint8_t>(coordinateColorGreen);
		pOutput[offset + 2] = static_cast<uint8_t>(coordinateColorRed);
	}

	// create jpg image from buffer and save
	std::cout << TAB2 << "Create BGR heatmap from XYPT Frame\n";

	Arena::IImage* pCreate = Arena::ImageFactory::Create(pOutput, dstDataSize, width, height, PIXEL_FORMAT);
	Save::ImageParams jpgParams(width, height, dstBpp);
	Save::ImageWriter jpgWriter(jpgParams, JPG_FILE_NAME);
	jpgWriter << pCreate->GetData();
	std::cout << TAB2 << "Save heatmap image as jpg to " << jpgWriter.GetLastFileName() << "\n";

	// clean up
	Arena::ImageFactory::Destroy(pCreate);
	delete[] pOutput;
	pOutput = NULL;

	// stop stream
	std::cout << TAB1 << "Stop stream\n";
	pDevice->RequeueBuffer(pImage);
	pDevice->StopStream();

	// reset node to its initial value
	Arena::SetNodeValue<bool>(pDevice->GetNodeMap(), "ErcEnable", ercEnableInitial);
	Arena::SetNodeValue<double>(pDevice->GetNodeMap(), "ErcRateLimit", cameraEventRateInitial);
	Arena::SetNodeValue<GenICam::gcstring>(pDevice->GetNodeMap(), "EventFormat", eventFormatInitial);
	Arena::SetNodeValue<GenICam::gcstring>(pDevice->GetNodeMap(), "AcquisitionMode", acquisitionModeInitial);
}

// =-=-=-=-=-=-=-=-=-
// =- PREPARATION -=-
// =- & CLEAN UP =-=-
// =-=-=-=-=-=-=-=-=-

Arena::DeviceInfo SelectDevice(std::vector<Arena::DeviceInfo>& deviceInfos)
{
	if (deviceInfos.size() == 1)
	{
		std::cout << "\n"
				  << TAB1 << "Only one device detected: " << deviceInfos[0].ModelName() << TAB1 << deviceInfos[0].SerialNumber() << TAB1 << deviceInfos[0].IpAddressStr() << ".\n";
		std::cout << TAB1 << "Automatically selecting this device.\n";
		return deviceInfos[0];
	}

	std::cout << "\nSelect device:\n";
	for (size_t i = 0; i < deviceInfos.size(); i++)
	{
		std::cout << TAB1 << i + 1 << ". " << deviceInfos[i].ModelName() << TAB1 << deviceInfos[i].SerialNumber() << TAB1 << deviceInfos[i].IpAddressStr() << "\n";
	}
	size_t selection = 0;

	do
	{
		std::cout << TAB1 << "Make selection (1-" << deviceInfos.size() << "): ";
		std::cin >> selection;

		if (std::cin.fail())
		{
			std::cin.clear();
			while (std::cin.get() != '\n')
				;
			std::cout << TAB1 << "Invalid input. Please enter a number.\n";
		}
		else if (selection <= 0 || selection > deviceInfos.size())
		{
			std::cout << TAB1 << "Invalid device selected. Please select a device in the range (1-" << deviceInfos.size() << ").\n";
		}

	} while (selection <= 0 || selection > deviceInfos.size());

	return deviceInfos[selection - 1];
}

int main()
{
	// flag to track when an exception has been thrown
	bool exceptionThrown = false;

	std::cout << "Cpp_EVS_XYTP_Frame_HeatMap";

	try
	{
		// prepare example
		Arena::ISystem* pSystem = Arena::OpenSystem();
		pSystem->UpdateDevices(100);
		std::vector<Arena::DeviceInfo> deviceInfos = pSystem->GetDevices();
		if (deviceInfos.size() == 0)
		{
			std::cout << "\nNo camera connected\nPress enter to complete\n";
			std::getchar();
			return 0;
		}
		Arena::DeviceInfo selectedDeviceInfo = SelectDevice(deviceInfos);
		Arena::IDevice* pDevice = pSystem->CreateDevice(selectedDeviceInfo);

		std::cout << "Commence example\n\n";

		// run example
		AcquireImageAndCreateHeatMapColoring(pDevice);

		std::cout << "\nExample complete\n";

		// clean up example
		pSystem->DestroyDevice(pDevice);
		Arena::CloseSystem(pSystem);
	}
	catch (GenICam::GenericException& ge)
	{
		std::cout << "\nGenICam exception thrown: " << ge.what() << "\n";
		exceptionThrown = true;
	}
	catch (std::exception& ex)
	{
		std::cout << "\nStandard exception thrown: " << ex.what() << "\n";
		exceptionThrown = true;
	}
	catch (...)
	{
		std::cout << "\nUnexpected exception thrown\n";
		exceptionThrown = true;
	}

	std::cout << "Press enter to complete\n";
	std::cin.ignore();
	std::getchar();

	if (exceptionThrown)
		return -1;
	else
		return 0;
}
