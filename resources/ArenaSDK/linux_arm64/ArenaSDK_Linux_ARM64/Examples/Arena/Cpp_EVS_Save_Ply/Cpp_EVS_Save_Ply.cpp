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

// Save: EVS Save
//    This example demonstrates how to acquire and save an image using the Event Stream (EVS) format.
//    It covers setting up acquisition mode, configuring the camera for EVS,
//    and saving the acquired image in the PLY format using the save library. 

// =-=-=-=-=-=-=-=-=-
// =-=- SETTINGS =-=-
// =-=-=-=-=-=-=-=-=-

// image timeout
#define TIMEOUT 2000

// File name
//    The relative path and file name to save the image to. The file format is
//    determined by the file extension. For this example, the image will be
//    saved as a PLY file.
#define FILE_NAME "Images/Cpp_EVS_Save_Ply/Cpp_EVS_Save_Ply.ply"

// =-=-=-=-=-=-=-=-=-
// =-=- EXAMPLE -=-=-
// =-=-=-=-=-=-=-=-=-

void SaveImage(Arena::IImage* pImage, const char* filename)
{
	// Prepare image parameters
	//    An image's width, height, and bits per pixel are required to save to
	//    disk. Its size and stride (i.e. pitch) can be calculated from those 3
	//    inputs. Notice that an image's size and stride use bytes as a unit
	//    while the bits per pixel uses bits.
	std::cout << TAB2 << "Prepare image parameters\n";

	Save::ImageParams params(
		pImage->GetWidth(),
		pImage->GetHeight(),
		pImage->GetBitsPerPixel(),
		true,
		pImage->GetSizeFilled() / (pImage->GetBitsPerPixel() / 8));

	// Prepare image writer
	//    The image writer requires three arguments to save an image:
	//    the image parameters, the file name, and the image data.
	//    These parameters are passed to the image writer's constructor.
	std::cout << TAB2 << "Prepare image writer\n";

	Save::ImageWriter writer(
		params,
		filename);

	// set default parameters for SetPly()
	bool filterPoints = true;
	bool isSignedPixelFormat = false;
	float scale = 0.25f;
	float offsetA = 0.0f;
	float offsetB = 0.0f;
	float offsetC = 0.0f;

	// set the output file format of the image writer to .ply
	writer.SetPly(".ply", filterPoints, isSignedPixelFormat, scale, offsetA, offsetB, offsetC);

	// Save image
	//    Passing image data into the image writer using the cascading I/O
	//    operator (<<) triggers a save. Notice that the << operator accepts the
	//    image data as a constant unsigned 8-bit integer pointer (const
	//    uint8_t*) and the file name as a character string (const char*).
	std::cout << TAB2 << "Save image to ";

	writer << pImage->GetData();

	std::cout << writer.GetLastFileName() << "\n";
}

// demonstrates acquisition and save 
// (1) sets acquisition mode
// (2) sets buffer handling mode
// (3) sets Event Format to EVS and camera event rate to 10 Mev/s
// (4) sets EVS output format to XYTPFrame
// (5) starts the stream
// (6) acquires one image 
// (7) saves the image
// (8) requeues the buffer
// (9) stops the stream
void AcquireAndSaveImage(Arena::IDevice* pDevice)
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

	// Configure EVS output format to XYTPFrame
	// Setting the XYTPFrame format will instruct Arena to generate a 3D Arena::IImage object
	// with a pixel format of ABCY16.
	std::cout << TAB1 << "Set EVS output format to XYTPFrame\n";

	Arena::SetNodeValue<GenICam::gcstring>(
		pDevice->GetTLStreamNodeMap(),
		"StreamEvsOutputFormat",
		"XYTPFrame");

	// start stream
	std::cout << TAB1 << "Start stream\n";

	pDevice->StartStream();

	std::cout << TAB2 << "Get one image\n";

	// The retrieved image will have a pixel format of ABCY16,
	// where A represents x, B represents y, C represents t, and Y represents p.
	Arena::IImage* pImage = pDevice->GetImage(TIMEOUT);

	if (pImage)
	{
		if (pImage->IsIncomplete())
		{
			std::cout << TAB2 << "Image " << pImage->GetFrameId() << " is incomplete." << std::endl;
		}
		else
		{
			SaveImage(pImage, FILE_NAME);
		}
	}

	// clean up example
	pDevice->RequeueBuffer(pImage);

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

	std::cout << "Cpp_EVS_Save_Ply";

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
		AcquireAndSaveImage(pDevice);
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
