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
#include "OverlayInfo.h"

#define TAB1 "  "
#define TAB2 "    "

#include <iostream>

// Helios-Chroma: Overlay SDK
//    This example shows how to overlay color image over 3D image using the
//	  Helios-Chroma device group. With the system calibrated, we can grab new images
//    with the grouped Helios and Triton devices and use the calibration values from
//    the device group to find the RGB color for each 3D point measured with the Helios.
//    The following code shows how to project the Helios xyz points onto the Triton
//    image using ImageFactory of ArenaSDK.

// =-=-=-=-=-=-=-=-=-
// =-=- SETTINGS =-=-
// =-=-=-=-=-=-=-=-=-

// image timeout
#define TIMEOUT 2000

// pixel format for saving
#define PIXEL_FORMAT BGR8
#define PIXEL_FORMAT_3D Coord3D_ABCY16

// file name
#define FILE_NAME "Images\\Cpp_HeliosChroma_Overlay_SDK"
#define JPG ".jpg"
#define PLY ".ply"

// =-=-=-=-=-=-=-=-=-
// =-=- HELPERS -=-=-
// =-=-=-=-=-=-=-=-=-

// demonstrates saving an jpg image
// (1) converts image to a displayable pixel format
// (2) prepares image parameters
// (3) prepares image writer
// (4) sets image writer to JPEG
// (5) saves image
// (6) destroys converted image
void SaveJPG(Arena::IImage* pImage, const char* filename)
{
	// convert image
	std::cout << TAB2 << "Convert image\n";

	Arena::IImage* pConverted;
	pConverted = Arena::ImageFactory::Convert(pImage, PIXEL_FORMAT);

	// prepare image parameters
	std::cout << TAB2 << "Prepare image parameters\n";

	Save::ImageParams params(
		pConverted->GetWidth(),
		pConverted->GetHeight(),
		pConverted->GetBitsPerPixel());

	// prepare image writer
	std::cout << TAB2 << "Prepare image writer\n";

	Save::ImageWriter writer(params, filename);

	// set image writer to the pixel format
	std::cout << TAB2 << "Set image writer to JPG\n";

	writer.SetJpeg(JPG, 75, false, Save::EJpegSubsampling::NoSubsampling, false);

	// save image
	std::cout << TAB2 << "Save image to " << filename << JPG << "\n";

	writer << pConverted->GetData();

	// destroy converted image
	Arena::ImageFactory::Destroy(pConverted);
}

// demonstrates saving an ply image
// (1) prepares image parameters
// (2) prepares image writers
// (3) sets image writer to PLY
// (4) saves image
void SavePLY(Arena::IImage* pImage, Arena::IImage* pColorData, const char* filename, float scale = 0.25f, float x = 0.0f, float y = 0.0f, float z = 0.0f)
{
	// prepare image parameters
	std::cout << TAB2 << "Prepare image parameters\n";

	Save::ImageParams params(
		pImage->GetWidth(),
		pImage->GetHeight(),
		pImage->GetBitsPerPixel());

	// prepare image writer
	std::cout << TAB2 << "Prepare image writer\n";

	Save::ImageWriter writer(params, (filename + std::string(PLY)).c_str());

	// set image writer to the pixel format
	std::cout << TAB2 << "Set image writer to PLY\n";

	// save .ply with color data
	bool filterPoints = true;
	bool isSigned = pImage->GetPixelFormat() == Coord3D_ABC16s || pImage->GetPixelFormat() == Coord3D_ABCY16s;

	writer.SetPly(PLY, filterPoints, isSigned, scale, x, y, z);

	// save image
	std::cout << TAB2 << "Save image to " << filename << PLY << "\n";

	if (pColorData)
		writer.Save(pImage->GetData(), pColorData->GetData());
	else
		writer << pImage->GetData();
}

// =-=-=-=-=-=-=-=-=-
// =-=- EXAMPLE -=-=-
// =-=-=-=-=-=-=-=-=-

// demonstrates how to overlay a color image onto a 3D image using Helios-Chroma device group
// (1) sets device node values
// (2) gets images from Helios and Triton device in a device group
// (3) overlays RGB color data onto 3D XYZ points
// (4) saves the resulting images
void OverlayColorOnto3DAndSave(Arena::IDevice* pTriton, Arena::IDevice* pHelios)
{
	// get node values that will be changed in order to return their values at
	// the end of the example
	GenICam::gcstring pfTriton = Arena::GetNodeValue<GenICam::gcstring>(pTriton->GetNodeMap(), "PixelFormat");
	GenICam::gcstring pfHelios = Arena::GetNodeValue<GenICam::gcstring>(pHelios->GetNodeMap(), "PixelFormat");

	// set triton device nodes
	std::cout << TAB1 << "Set Triton device nodes\n";
	GenApi::INodeMap* pTritonNodeMap = pTriton->GetNodeMap();
	Arena::SetNodeValue<int64_t>(pTritonNodeMap, "OffsetX", 0);
	Arena::SetNodeValue<int64_t>(pTritonNodeMap, "OffsetY", 0);
	Arena::SetNodeValue<int64_t>(pTritonNodeMap, "Width", Arena::GetNodeMax<int64_t>(pTritonNodeMap, "Width"));
	Arena::SetNodeValue<int64_t>(pTritonNodeMap, "Height", Arena::GetNodeMax<int64_t>(pTritonNodeMap, "Height"));
	Arena::SetNodeValue<int64_t>(pTritonNodeMap, "BinningHorizontal", Arena::GetNodeMax<int64_t>(pTritonNodeMap, "BinningHorizontal"));
	Arena::SetNodeValue<int64_t>(pTritonNodeMap, "BinningVertical", Arena::GetNodeMax<int64_t>(pTritonNodeMap, "BinningVertical"));
	Arena::SetNodeValue<GenICam::gcstring>(pTritonNodeMap, "PixelFormat", "RGB8"); // ensure colour pixel format

	// set helios device nodes
	std::cout << TAB1 << "Set Helios device nodes\n";

	Arena::SetNodeValue<GenICam::gcstring>(pHelios->GetNodeMap(), "PixelFormat", "Coord3D_ABCY16");

	// get and save an image from Triton
	std::cout << TAB1 << "Get Triton image\n";

	pTriton->StartStream();
	Arena::IImage* pImageTriton = pTriton->GetImage(TIMEOUT);

	std::cout << TAB1 << "Save Triton image\n";

	SaveJPG(pImageTriton, (FILE_NAME + std::string("_triton")).c_str());

	// get and save an image from Helios
	std::cout << TAB1 << "Get Helios image\n";

	pHelios->StartStream();
	Arena::IImage* pImageHelios = pHelios->GetImage(TIMEOUT);

	std::cout << TAB1 << "Save Helios image\n";

	SavePLY(pImageHelios, nullptr, (FILE_NAME + std::string("_helios")).c_str());

	// get overlay information from the devices
	std::cout << TAB1 << "Get Overlay information\n";

	Arena::OverlayInfo* pOverlayInfo = Arena::ImageFactory::CreateOverlayInfo(pTriton, pHelios);

	// overlay RGB color data onto 3D XYZ points and save the resulting image
	std::cout << TAB1 << "Overlay the RGB color data onto the 3D XYZ points\n";

	Arena::IImage* pImageOverlay = Arena::ImageFactory::Overlay(pImageHelios, pImageTriton, pOverlayInfo);

	std::cout << TAB1 << "Save 3D image\n";

	SaveJPG(pImageOverlay, FILE_NAME);
	SavePLY(pImageHelios, pImageOverlay, FILE_NAME, 
		pOverlayInfo->GetScale(), 
		pOverlayInfo->GetCoordinateOffsetA(), 
		pOverlayInfo->GetCoordinateOffsetB(), 
		pOverlayInfo->GetCoordinateOffsetC());

	// clean up
	pTriton->RequeueBuffer(pImageTriton);
	pHelios->RequeueBuffer(pImageHelios);
	pTriton->StopStream();
	pHelios->StopStream();
	Arena::ImageFactory::Destroy(pImageOverlay);
	Arena::ImageFactory::DestroyOverlayInfo(pOverlayInfo);

	// set pixel format back to the original values
	Arena::SetNodeValue<GenICam::gcstring>(pTriton->GetNodeMap(), "PixelFormat", pfTriton);
	Arena::SetNodeValue<GenICam::gcstring>(pHelios->GetNodeMap(), "PixelFormat", pfHelios);
}

// =-=-=-=-=-=-=-=-=-
// =- PREPARATION -=-
// =- & CLEAN UP =-=-
// =-=-=-=-=-=-=-=-=-

bool isApplicableDeviceTriton(Arena::DeviceInfo deviceInfo)
{
	// color Triton device with device group membership needed
	return deviceInfo.ModelName().find("TRI") != GenICam::gcstring::npos 
		&& deviceInfo.ModelName().find("-C") != GenICam::gcstring::npos 
		&& deviceInfo.DeviceGroupEnable();
}

bool isApplicableDeviceHelios(Arena::DeviceInfo deviceInfo)
{
	// helios device with device group membership needed
	return (deviceInfo.ModelName().find("HLT") != GenICam::gcstring::npos 
		|| deviceInfo.ModelName().find("HT") != GenICam::gcstring::npos) 
		&& deviceInfo.DeviceGroupEnable();
}

bool isApplicableDeviceGroup(Arena::DeviceInfo tritonInfo, Arena::DeviceInfo heliosInfo)
{
	// helios and Triton devices need to be in the same device group
	return tritonInfo.DeviceGroupSerial() == heliosInfo.DeviceGroupSerial();
}

int main()
{
	// flag to track when an exception has been thrown
	bool exceptionThrown = false;

	std::cout << "Cpp_HeliosChroma_Overlay_SDK\n";

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

		// identify Helios-Chroma device group
		Arena::DeviceInfo* pInfoTriton = nullptr;
		Arena::DeviceInfo* pInfoHelios = nullptr;
		for (auto& deviceInfo : deviceInfos)
		{
			if (!pInfoTriton && isApplicableDeviceTriton(deviceInfo))
			{
				pInfoTriton = &deviceInfo;
			}
			else if (isApplicableDeviceTriton(deviceInfo))
			{
				throw std::logic_error("Too many Triton devices connected");
			}
			else if (!pInfoHelios && isApplicableDeviceHelios(deviceInfo))
			{
				pInfoHelios = &deviceInfo;
			}
			else if (isApplicableDeviceHelios(deviceInfo))
			{
				throw std::logic_error("Too many Helios devices connected");
			}
		}

		if (!pInfoTriton)
			throw std::logic_error("No applicable Triton devices");

		if (!pInfoHelios)
			throw std::logic_error("No applicable Helios devices");

		if (!isApplicableDeviceGroup(*pInfoTriton, *pInfoHelios))
			throw std::logic_error("No applicable Helios-Chroma device group");

		// create devices
		Arena::IDevice* pDeviceTriton = nullptr;
		Arena::IDevice* pDeviceHelios = nullptr;

		pDeviceTriton = pSystem->CreateDevice(*pInfoTriton);

		// enable stream auto negotiate packet size
		Arena::SetNodeValue<bool>(
			pDeviceTriton->GetTLStreamNodeMap(),
			"StreamAutoNegotiatePacketSize",
			true);

		// enable stream packet resend
		//Arena::SetNodeValue<bool>(
		//	pDeviceTriton->GetTLStreamNodeMap(),
		//	"StreamPacketResendEnable",
		//	true);

		pDeviceHelios = pSystem->CreateDevice(*pInfoHelios);

		// enable stream auto negotiate packet size
		Arena::SetNodeValue<bool>(
			pDeviceHelios->GetTLStreamNodeMap(),
			"StreamAutoNegotiatePacketSize",
			true);

		// enable stream packet resend
		//Arena::SetNodeValue<bool>(
		//	pDeviceHelios->GetTLStreamNodeMap(),
		//	"StreamPacketResendEnable",
		//	true);

		// run example
		if (pDeviceTriton && pDeviceHelios)
		{
			std::cout << "Commence example\n\n";
			OverlayColorOnto3DAndSave(pDeviceTriton, pDeviceHelios);
			std::cout << "\nExample complete\n";
		}
		else
		{
			throw std::runtime_error(std::string("Devices not connected"));
		}

		if (pDeviceTriton)
			pSystem->DestroyDevice(pDeviceTriton);
		if (pDeviceHelios)
			pSystem->DestroyDevice(pDeviceHelios);

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
