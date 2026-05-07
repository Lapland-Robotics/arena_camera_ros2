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

#define TAB1 "  "
#define TAB2 "    "

#include <iostream>
#include <opencv2/opencv.hpp>

// Helios-Chroma: Overlay
//    This example shows how to overlay color image over 3D image using the
//	  Helios-Chroma device group. With the system calibrated, we can grab new images
//    with the grouped Helios and Triton devices and use the calibration values from
//    the device group to find the RGB color for each 3D point measured with the Helios.
//    Based on the output of solvePnP, we can project the 3D points measured by
//    the Helios onto the RGB camera image using the OpenCV function projectPoints.
//    Grab a Helios image with the getImageHelios() function(output: xyz_mm) and
//    a Triton RGB image with the getImageTriton() function(output: triton_rgb).
//    The following code shows how to project the Helios xyz points onto the Triton
//    image, giving a (row, col) position for each 3D point. We can sample the
//    Triton image at that (row, col) position to find the 3D point’s RGB value.

// =-=-=-=-=-=-=-=-=-
// =-=- SETTINGS =-=-
// =-=-=-=-=-=-=-=-=-

// image timeout
#define TIMEOUT 2000

// pixel format for saving
#define PIXEL_FORMAT BGR8
#define PIXEL_FORMAT_3D Coord3D_ABCY16

// file name
#define FILE_NAME "Images\\Cpp_HeliosChroma_Overlay"
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
void SavePLY(Arena::IImage* pImage, const uint8_t* pColorData, const char* filename, float scale = 0.25f, float x = 0.0f, float y = 0.0f, float z = 0.0f)
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
		writer.Save(pImage->GetData(), pColorData);
	else
		writer << pImage->GetData();
}

// Overlay Info struct
//    This struct holds the information needed to overlay depth and RGB images.
struct OverlayInfo
{
public:
	// calibration values
	double focalLengthX = 0.0;
	double focalLengthY = 0.0;
	double opticalCenterX = 0.0;
	double opticalCenterY = 0.0;

	std::string distortionModel;

	double value0 = 0.0;
	double value1 = 0.0;
	double value2 = 0.0;
	double value3 = 0.0;
	double value4 = 0.0;
	double value5 = 0.0;
	double value6 = 0.0;
	double value7 = 0.0;

	// orientation values
	double rotation0 = 0.0;
	double rotation1 = 0.0;
	double rotation2 = 0.0;

	double translationX = 0.0;
	double translationY = 0.0;
	double translationZ = 0.0;

	// binning values
	size_t binningHorizontal = 1;
	size_t binningVertical = 1;

	// 3D
	double scale = 0.25;
	double coordA = -8192.0;
	double coordB = -8192.0;
	double coordC = 0.0;
};

// =-=-=-=-=-=-=-=-=-
// =-=- HELPERS -=-=-
// =-=-=-=-=-=-=-=-=-

// set the selector node value to get a specific node value
double getNodeValueFromSelector(GenApi::INodeMap* pNodeMap, const char* pSelectorId, const char* pSelectorValue, const char* pNodeId)
{
	Arena::SetNodeValue<GenICam::gcstring>(pNodeMap, pSelectorId, pSelectorValue);
	return Arena::GetNodeValue<double>(pNodeMap, pNodeId);
}

// get information required to overlay images from Helios and Triton
OverlayInfo getOverlayInfoFromDevices(Arena::IDevice* pTriton, Arena::IDevice* pHelios)
{
	OverlayInfo info;

	GenApi::INodeMap* pTritonNodeMap = pTriton->GetNodeMap();
	GenApi::INodeMap* pHeliosNodeMap = pHelios->GetNodeMap();

	// get node values

	// calibration values, from Chroma
	info.focalLengthX = Arena::GetNodeValue<double>(pTritonNodeMap, "CalibFocalLengthX");
	info.focalLengthY = Arena::GetNodeValue<double>(pTritonNodeMap, "CalibFocalLengthY");
	info.opticalCenterX = Arena::GetNodeValue<double>(pTritonNodeMap, "CalibOpticalCenterX");
	info.opticalCenterY = Arena::GetNodeValue<double>(pTritonNodeMap, "CalibOpticalCenterY");
	info.distortionModel = Arena::GetNodeValue<GenICam::gcstring>(pTritonNodeMap, "CalibLensDistortionModel");

	info.value0 = getNodeValueFromSelector(pTritonNodeMap, "CalibLensDistortionValueSelector", "Value0", "CalibLensDistortionValue");
	info.value1 = getNodeValueFromSelector(pTritonNodeMap, "CalibLensDistortionValueSelector", "Value1", "CalibLensDistortionValue");
	info.value2 = getNodeValueFromSelector(pTritonNodeMap, "CalibLensDistortionValueSelector", "Value2", "CalibLensDistortionValue");
	info.value3 = getNodeValueFromSelector(pTritonNodeMap, "CalibLensDistortionValueSelector", "Value3", "CalibLensDistortionValue");
	info.value4 = getNodeValueFromSelector(pTritonNodeMap, "CalibLensDistortionValueSelector", "Value4", "CalibLensDistortionValue");
	info.value5 = getNodeValueFromSelector(pTritonNodeMap, "CalibLensDistortionValueSelector", "Value5", "CalibLensDistortionValue");
	info.value6 = getNodeValueFromSelector(pTritonNodeMap, "CalibLensDistortionValueSelector", "Value6", "CalibLensDistortionValue");
	info.value7 = getNodeValueFromSelector(pTritonNodeMap, "CalibLensDistortionValueSelector", "Value7", "CalibLensDistortionValue");

	// orientation values, from Helios
	info.rotation0 = getNodeValueFromSelector(pHeliosNodeMap, "DeviceGroupMemberTransformValueSelector", "RotationAxisAngleVector0", "DeviceGroupMemberTransformValue");
	info.rotation1 = getNodeValueFromSelector(pHeliosNodeMap, "DeviceGroupMemberTransformValueSelector", "RotationAxisAngleVector1", "DeviceGroupMemberTransformValue");
	info.rotation2 = getNodeValueFromSelector(pHeliosNodeMap, "DeviceGroupMemberTransformValueSelector", "RotationAxisAngleVector2", "DeviceGroupMemberTransformValue");
	info.translationX = getNodeValueFromSelector(pHeliosNodeMap, "DeviceGroupMemberTransformValueSelector", "TranslationX", "DeviceGroupMemberTransformValue");
	info.translationY = getNodeValueFromSelector(pHeliosNodeMap, "DeviceGroupMemberTransformValueSelector", "TranslationY", "DeviceGroupMemberTransformValue");
	info.translationZ = getNodeValueFromSelector(pHeliosNodeMap, "DeviceGroupMemberTransformValueSelector", "TranslationZ", "DeviceGroupMemberTransformValue");

	// binning values, from Chroma
	info.binningHorizontal = Arena::GetNodeValue<int64_t>(pTritonNodeMap, "BinningHorizontal");
	info.binningVertical = Arena::GetNodeValue<int64_t>(pTritonNodeMap, "BinningVertical");

	// 3D, from Helios
	info.scale = Arena::GetNodeValue<double>(pHeliosNodeMap, "Scan3dCoordinateScale");
	info.coordA = getNodeValueFromSelector(pHeliosNodeMap, "Scan3dCoordinateSelector", "CoordinateA", "Scan3dCoordinateOffset");
	info.coordB = getNodeValueFromSelector(pHeliosNodeMap, "Scan3dCoordinateSelector", "CoordinateB", "Scan3dCoordinateOffset");
	info.coordC = getNodeValueFromSelector(pHeliosNodeMap, "Scan3dCoordinateSelector", "CoordinateC", "Scan3dCoordinateOffset");

	return info;
}

// get an image from Helios and store the data in xyz_mm matrix
void getImageHelios(Arena::IDevice* pHelios, Arena::IImage** ppOutImage, cv::Mat& xyz_mm, size_t& width, size_t& height, size_t& numCh, OverlayInfo& info)
{
	// set helios device nodes
	std::cout << TAB1 << "Set Helios device nodes\n";

	Arena::SetNodeValue<GenICam::gcstring>(pHelios->GetNodeMap(), "PixelFormat", "Coord3D_ABCY16");

	pHelios->StartStream();
	Arena::IImage* pHeliosImage = pHelios->GetImage(TIMEOUT);

	// copy the input image because original will be delited after function call
	Arena::IImage* pCopyImage = Arena::ImageFactory::Copy(pHeliosImage);
	*ppOutImage = pCopyImage;

	height = pHeliosImage->GetHeight();
	width = pHeliosImage->GetWidth();
	int64_t pf = pHeliosImage->GetPixelFormat();
	numCh = pf == Coord3D_ABCY16 ? 4 : 3;

	xyz_mm = cv::Mat((int)height, (int)width, CV_32FC3);
	const uint16_t* input_data = reinterpret_cast<const uint16_t*>(pHeliosImage->GetData());

	for (unsigned int ir = 0; ir < height; ++ir)
	{
		for (unsigned int ic = 0; ic < width; ++ic)
		{
			// get unsigned 16 bit values for X,Y,Z coordinates
			ushort x_u16 = input_data[0];
			ushort y_u16 = input_data[1];
			ushort z_u16 = input_data[2];

			// convert 16-bit X,Y,Z to float values in mm
			xyz_mm.at<cv::Vec3f>(ir, ic)[0] = (float)(x_u16 * info.scale + info.coordA);
			xyz_mm.at<cv::Vec3f>(ir, ic)[1] = (float)(y_u16 * info.scale + info.coordB);
			xyz_mm.at<cv::Vec3f>(ir, ic)[2] = (float)(z_u16 * info.scale + info.coordC);

			input_data += numCh;
		}
	}

	pHelios->RequeueBuffer(pHeliosImage);
	pHelios->StopStream();
}

// get an image from Triton and store the data in triton_rgb matrix
void getImageTriton(Arena::IDevice* pTriton, Arena::IImage** ppOutImage, cv::Mat& triton_rgb)
{
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

	pTriton->StartStream();
	Arena::IImage* pTritonImage = pTriton->GetImage(TIMEOUT);

	// copy image because original will be deleted after function call
	// convert to RGB for the matrix operation
	Arena::IImage* pRgbImage = Arena::ImageFactory::Convert(pTritonImage, RGB8);
	*ppOutImage = pRgbImage;

	size_t height, width;
	height = pRgbImage->GetHeight();
	width = pRgbImage->GetWidth();
	triton_rgb = cv::Mat((int)height, (int)width, CV_8UC3);
	memcpy(triton_rgb.data, pRgbImage->GetData(), height * width * 3);

	pTriton->RequeueBuffer(pTritonImage);
	pTriton->StopStream();
}

// =-=-=-=-=-=-=-=-=-
// =-=- EXAMPLE -=-=-
// =-=-=-=-=-=-=-=-=-

// demonstrates how to overlay a color image onto a 3D image using Helios-Chroma device group
// (1) Obtain overlay information from device nodes of the Helios and Triton devices
// (2) Get images from Helios and Triton device
// (3) Overlay RGB color data onto 3D XYZ points
// (4) Save the resulting image
void OverlayColorOnto3DAndSave(Arena::IDevice* pTriton, Arena::IDevice* pHelios)
{
	// get node values that will be changed in order to return their values at
	// the end of the example
	GenICam::gcstring pfTriton = Arena::GetNodeValue<GenICam::gcstring>(pTriton->GetNodeMap(), "PixelFormat");
	GenICam::gcstring pfHelios = Arena::GetNodeValue<GenICam::gcstring>(pHelios->GetNodeMap(), "PixelFormat");

	// get overlay information from the devices
	std::cout << TAB1 << "Get and prepare information for overlay\n";

	OverlayInfo overlayInfo = getOverlayInfoFromDevices(pTriton, pHelios);

	// prepare camera matrix, distance coefficients, and
	// rotation and translation vectors
	cv::Mat cameraMatrix,
		distCoeffs,
		rotationVector,
		translationVector;

	// calibration, camera matrix
	cameraMatrix = cv::Mat((int)3, (int)3, CV_64F);
	cameraMatrix.at<double>(0) = overlayInfo.focalLengthX;
	cameraMatrix.at<double>(1) = 0.0;
	cameraMatrix.at<double>(2) = overlayInfo.opticalCenterX;
	cameraMatrix.at<double>(3) = 0.0;
	cameraMatrix.at<double>(4) = overlayInfo.focalLengthY;
	cameraMatrix.at<double>(5) = overlayInfo.opticalCenterY;
	cameraMatrix.at<double>(6) = 0.0;
	cameraMatrix.at<double>(7) = 0.0;
	cameraMatrix.at<double>(8) = 1.0;

	// calibration, distortion coefficients
	distCoeffs = cv::Mat((int)8, (int)1, CV_64F);
	distCoeffs.at<double>(0) = overlayInfo.value0;
	distCoeffs.at<double>(1) = overlayInfo.value1;
	distCoeffs.at<double>(2) = overlayInfo.value2;
	distCoeffs.at<double>(3) = overlayInfo.value3;
	distCoeffs.at<double>(4) = overlayInfo.value4;
	distCoeffs.at<double>(5) = overlayInfo.value5;
	distCoeffs.at<double>(6) = overlayInfo.value6;
	distCoeffs.at<double>(7) = overlayInfo.value7;

	// orientation, rotation
	rotationVector = cv::Mat((int)3, (int)1, CV_64F);
	rotationVector.at<double>(0) = overlayInfo.rotation0;
	rotationVector.at<double>(1) = overlayInfo.rotation1;
	rotationVector.at<double>(2) = overlayInfo.rotation2;

	// orientation, translation
	translationVector = cv::Mat((int)3, (int)1, CV_64F);
	translationVector.at<double>(0) = overlayInfo.translationX;
	translationVector.at<double>(1) = overlayInfo.translationY;
	translationVector.at<double>(2) = overlayInfo.translationZ;

	// get an image from Triton
	std::cout << TAB1 << "Get and prepare Triton image\n";

	Arena::IImage* pImageTriton = nullptr;
	cv::Mat imageMatrixRGB, imageMatrixBGR;

	getImageTriton(
		pTriton,
		&pImageTriton,
		imageMatrixRGB);

	// get an image from Triton
	std::cout << TAB1 << "Save Triton image\n";

	std::cout << TAB2 << "Save image to " << FILE_NAME << "_triton_matrix.jpg\n";

	cv::cvtColor(imageMatrixRGB, imageMatrixBGR, cv::COLOR_RGB2BGR);
	cv::imwrite(FILE_NAME "_triton_matrix.jpg", imageMatrixBGR);

	SaveJPG(pImageTriton, (FILE_NAME + std::string("_triton")).c_str());

	// get an image from Helios
	std::cout << TAB1 << "Get and prepare Helios image\n";

	Arena::IImage* pImageHelios = nullptr;
	cv::Mat imageMatrixXYZ;
	size_t width = 0, height = 0, numCh = 4;

	getImageHelios(
		pHelios,
		&pImageHelios,
		imageMatrixXYZ,
		width,
		height,
		numCh,
		overlayInfo);

	// get an image from Helios
	std::cout << TAB1 << "Save Helios image\n";

	std::cout << TAB2 << "Save image to " << FILE_NAME << "_helios_matrix.jpg\n ";

	cv::imwrite(FILE_NAME "_helios_matrix.jpg", imageMatrixXYZ);

	SavePLY(pImageHelios, nullptr, (FILE_NAME + std::string("_helios")).c_str());

	// overlay RGB color data onto 3D XYZ points
	std::cout << TAB1 << "Overlay the RGB color data onto the 3D XYZ points\n";

	// convert the Helios xyz values from 640x480 to a Nx1 matrix to feed into projectPoints
	std::cout << TAB2 << "Reshape XYZ matrix\n";

	int N = imageMatrixXYZ.rows * imageMatrixXYZ.cols;
	cv::Mat xyzPoints = imageMatrixXYZ.reshape(3, N);

	// use projectPoints to find the position in
	// the Triton image (row,col) of each Helios 3d point
	std::cout << TAB2 << "Project points\n";

	cv::Mat projectedPointsTriton;

	cv::projectPoints(
		xyzPoints,
		rotationVector,
		translationVector,
		cameraMatrix,
		distCoeffs,
		projectedPointsTriton);

	// finally, loop through the set of points and access the Triton RGB image at the positions
	// calculated by projectPoints to find the RGB value of each 3D point
	std::cout << TAB2 << "Get values at projected points\n";

	uint8_t* pColorData = new uint8_t[width * height * 3];

	for (int i = 0; i < N; i++)
	{
		unsigned int colTriton = (unsigned int)std::round(projectedPointsTriton.at<cv::Vec2f>(i)[0]);
		unsigned int rowTriton = (unsigned int)std::round(projectedPointsTriton.at<cv::Vec2f>(i)[1]);

		colTriton /= overlayInfo.binningHorizontal;
		rowTriton /= overlayInfo.binningVertical;

		// only handle appropriate points
		if (rowTriton > 0 &&
			colTriton > 0 &&
			rowTriton < static_cast<unsigned int>(imageMatrixRGB.rows) &&
			colTriton < static_cast<unsigned int>(imageMatrixRGB.cols))
		{
			// access corresponding XYZ and RGB data
			uchar R = imageMatrixRGB.at<cv::Vec3b>(rowTriton, colTriton)[0];
			uchar G = imageMatrixRGB.at<cv::Vec3b>(rowTriton, colTriton)[1];
			uchar B = imageMatrixRGB.at<cv::Vec3b>(rowTriton, colTriton)[2];

			// Now you have the RGB values of a measured 3D Point at location (X,Y,Z).
			// Depending on your application you can do different things with these values,
			// for example, feed them into a point cloud rendering engine to view a 3D RGB image.
			pColorData[i * 3 + 0] = R;
			pColorData[i * 3 + 1] = G;
			pColorData[i * 3 + 2] = B;
		}
	}

	// save result
	std::cout << TAB1 << "Save 3D image\n";

	SavePLY(pImageHelios, pColorData, FILE_NAME, overlayInfo.scale, overlayInfo.coordA, overlayInfo.coordB, overlayInfo.coordC);
	Arena::IImage* pImageOverlay = Arena::ImageFactory::Create(pColorData, width * height * 3, width, height, RGB8);
	SaveJPG(pImageOverlay, FILE_NAME);

	// clean up
	delete[] pColorData;
	pColorData = NULL;
	Arena::ImageFactory::Destroy(pImageTriton);
	Arena::ImageFactory::Destroy(pImageHelios);
	Arena::ImageFactory::Destroy(pImageOverlay);

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
	return deviceInfo.ModelName().find("TRI") != GenICam::gcstring::npos && deviceInfo.ModelName().find("-C") != GenICam::gcstring::npos && deviceInfo.DeviceGroupEnable();
}

bool isApplicableDeviceHelios(Arena::DeviceInfo deviceInfo)
{
	// helios device with device group membership needed
	return (deviceInfo.ModelName().find("HLT") != GenICam::gcstring::npos || deviceInfo.ModelName().find("HT") != GenICam::gcstring::npos) && deviceInfo.DeviceGroupEnable();
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

	std::cout << "Cpp_HeliosChroma_Overlay\n";

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
			throw std::exception("Devices not created");
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
