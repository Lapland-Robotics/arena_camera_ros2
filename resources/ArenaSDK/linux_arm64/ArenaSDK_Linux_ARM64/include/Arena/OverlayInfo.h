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
#pragma once
#include <string>

namespace Arena
{
	/**
	* @class OverlayInfo
	*
	* @brief Contains overlay calibration and geometry information for Chroma + Helios.
	*/
	class ARENA_API OverlayInfo
	{
	public:
		/**
		 * @fn OverlayInfo()
		 *
		 * An empty constructor
		 *
		 * @warning 
		 *  - Does not sufficiently initialize OverlayInfo
		 */
		OverlayInfo();

		/**
		 * @fn OverlayInfo()
		 *
		 * A constructor
		 * 
		 * @param pChroma
		 *  - Type: Arena::IDevice*
		 *  - A Chroma device
		 * 
		 * @param pHelios
		 *  - Type: Arena::IDevice*
		 *  - A Helios device
		 */
		OverlayInfo(Arena::IDevice* pChroma, Arena::IDevice* pHelios);

		/**
		 * @fn OverlayInfo(const OverlayInfo& overlayInfo)
		 *
		 * A copy constructor
		 *
		 * @param overlayInfo
		 *  - Type: const OverlayInfo&
		 *  - Overlay information object to copy
		 */
		OverlayInfo(const OverlayInfo& overlayInfo);

		/**
		 * @fn virtual ~OverlayInfo()
		 *
		 * A destructor
		 */
		virtual ~OverlayInfo();

		/**
		 * @fn virtual OverlayInfo& operator=(OverlayInfo overlayInfo)
		 *
		 * A copy assignment operator
		 *
		 * @param overlayInfo
		 *  - Type: OverlayInfo
		 *  - Overlay information object to copy
		 *
		 * @return 
		 *  - Type: OverlayInfo&
		 *  - Copied overlay information object
		 */
		virtual OverlayInfo& operator=(OverlayInfo overlayInfo);

		/**
		 * @fn virtual void SetInfo(Arena::IDevice* pChroma, Arena::IDevice* pHelios)
		 *
		 * @param pChroma
		 *  - Type: Arena::IDevice*
		 *  - A Chroma device
		 * 
		 * @param pHelios
		 *  - Type: Arena::IDevice*
		 *  - A Helios device
		 * 
		 * @return 
		 *  - none
		 *
		 * <B> SetFPS </B> sets the overlay information from devices.
		 */
		virtual void SetInfo(Arena::IDevice* pChroma, Arena::IDevice* pHelios);

		virtual double GetFocalLengthX();
		virtual double GetFocalLengthY();
		virtual double GetOpticalCenterX();
		virtual double GetOpticalCenterY();
		virtual std::string GetDistortionModel();
		virtual std::vector<double> GetLensDistortionValues();
		virtual std::vector<double> GetRotationValues();
		virtual std::vector<double> GetTranslationValues();
		virtual size_t GetBinningHorizointal();
		virtual size_t GetBinningVertical();
		virtual double GetScale();
		virtual double GetCoordinateOffsetA();
		virtual double GetCoordinateOffsetB();
		virtual double GetCoordinateOffsetC();

	protected:
		void* m_pInternal;
	};
} // namespace Arena