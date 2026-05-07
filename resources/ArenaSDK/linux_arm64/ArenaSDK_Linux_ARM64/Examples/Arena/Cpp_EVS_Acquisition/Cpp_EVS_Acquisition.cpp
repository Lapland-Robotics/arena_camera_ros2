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
#include "ArenaApi.h"
#include "SaveApi.h"
#include <iomanip>

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
#define TIMEOUT 2000

// number of images to grab
#define NUM_IMAGES 25

// =-=-=-=-=-=-=-=-=-
// =-=- EXAMPLE -=-=-
// =-=-=-=-=-=-=-=-=-

std::string GetEvsEventRate(double rate)
{
	std::ostringstream oss;
	if (rate < 1000)
	{
		oss << std::setprecision(0) << std::fixed << rate << " ev/s";
	}
	else if (rate < 1000 * 1000)
	{
		oss << std::setprecision(1) << std::fixed << (rate / 1000) << " Kev/s";
	}
	else if (rate < 1000 * 1000 * 1000)
	{
		oss << std::setprecision(1) << std::fixed << (rate / (1000 * 1000)) << " Mev/s";
	}
	else
	{
		oss << std::setprecision(1) << std::fixed << (rate / (1000 * 1000 * 1000)) << " Gev/s";
	}
	return oss.str();
}

std::string GetEvsGvspFrameRate(double rate)
{
	std::ostringstream oss;
	oss << std::setprecision(0) << std::fixed << rate << " Bid/s";
	return oss.str();
}

std::string GetEvsLinkThroughput(double rate)
{
	std::ostringstream oss;
	if (rate < 1000)
	{
		oss << std::setprecision(0) << std::fixed << rate << " Bps";
	}
	else if (rate < 1000 * 1000)
	{
		oss << std::setprecision(1) << std::fixed << (rate / 1000) << " KBps";
	}
	else if (rate < 1000 * 1000 * 1000)
	{
		oss << std::setprecision(1) << std::fixed << (rate / (1000 * 1000)) << " MBps";
	}
	else
	{
		oss << std::setprecision(1) << std::fixed << (rate / (1000 * 1000 * 1000)) << " GBps";
	}
	return oss.str();
}


void PrintInfo(Arena::IDevice* pDevice)
{
	int64_t width = Arena::GetNodeValue<int64_t>(pDevice->GetNodeMap(), "Width");
	int64_t height = Arena::GetNodeValue<int64_t>(pDevice->GetNodeMap(), "Height");

	std::cout << TAB1 << "Image (w,h) = (" << width << "," << height << ") " << std::endl;
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
void AcquireImages(Arena::IDevice* pDevice)
{
	PrintInfo(pDevice);
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

	// set evs output format to CDFrame
	std::cout << TAB1 << "Set EVS output format to CDFrame\n";

	Arena::SetNodeValue<GenICam::gcstring>(
		pDevice->GetTLStreamNodeMap(),
		"StreamEvsOutputFormat",
		"CDFrame");

	// Set evs accumulation time to auto
	std::cout << TAB1 << "Set EVS Accumulation Time to auto \n";

	double frameGenFPS = Arena::GetNodeValue<double>(pDevice->GetTLStreamNodeMap(), "StreamFrameGeneratorFPS");

	Arena::SetNodeValue<int64_t>(pDevice->GetTLStreamNodeMap(), "StreamFrameGeneratorAccumTime", int(1000000 / frameGenFPS));

	// start stream
	std::cout << TAB1 << "Start stream\n";

	pDevice->StartStream();

	GenApi::INodeMap* pNodemap = pDevice->GetTLStreamNodeMap();

	// get images
	std::cout << TAB1 << "Getting " << NUM_IMAGES << " images\n";

	for (int i = 0; i < NUM_IMAGES; i++)
	{
		// get image
		std::cout << TAB2 << "Get image " << i << "\n";

		Arena::IImage* pImage = pDevice->GetImage(TIMEOUT);
			
		if (pImage)
		{
			// Get EVS event rate
			double eventRate = GenApi::CFloatPtr(pNodemap->GetNode("StreamEvsEventRate"))->GetValue();
			std::cout << TAB3 << "Event Rate: " << GetEvsEventRate(eventRate) << std::endl;

			// Get EVS GVSP frame rate
			double gvspFrameRate = GenApi::CFloatPtr(pNodemap->GetNode("StreamEvsGvspFrameRate"))->GetValue();
			std::cout << TAB3 << "GVSP Frame Rate: " << GetEvsGvspFrameRate(gvspFrameRate) << std::endl;

			// Get EVS link throughput
			double linkThroughput = GenApi::CFloatPtr(pNodemap->GetNode("StreamEvsLinkThroughput"))->GetValue();
			std::cout << TAB3 << "Link Throughput: " << GetEvsLinkThroughput(linkThroughput) << std::endl;


			if (pImage->IsIncomplete())
			{
				std::cout << TAB3 << "Image " << pImage->GetFrameId() << " is incomplete." << std::endl;
			}
		}

		pDevice->RequeueBuffer(pImage);
	}

	// stop stream
	std::cout << TAB1 << "Stop stream\n";

	pDevice->StopStream();

	// return node to its initial value
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

	std::cout << TAB1 << "\nSelect device:\n";
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

	std::cout << "Cpp_EVS_Acquisition";

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

		// run example
		std::cout << "Commence example\n\n";
		AcquireImages(pDevice);
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
