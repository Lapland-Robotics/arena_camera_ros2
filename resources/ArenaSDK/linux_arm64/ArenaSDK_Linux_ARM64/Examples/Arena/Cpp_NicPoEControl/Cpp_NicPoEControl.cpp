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

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

#include "LucidNicControlLib.h"

// Nic PoE Control: Introduction
//    This example shows how to control the PoE feature on supported Lucid network cards.
//    Note that the system cannot control the network card unless a driver is installed.
//    This utility must be run with admin rights.

// =-=-=-=-=-=-=-=-=-
// =-=- EXAMPLE -=-=-
// =-=-=-=-=-=-=-=-=-

// Demonstrates usage of the Lucid PoE Control Library:
// (1) Getting a list of supported interfaces on the system
// (2) Enabling/Disabling/Querying PoE state for an interface given it's name or MAC address

#define MAX_NUM_NIC_ENTRIES 32


bool CheckForElevatedPermissions()
{
#if defined(__linux__)

	return (getuid() == 0);

#elif defined(_WIN32)
	HANDLE hProcess = GetCurrentProcess();
	HANDLE hToken = nullptr;

	if (OpenProcessToken(hProcess, TOKEN_QUERY, &hToken) != 0)
	{
		TOKEN_ELEVATION tokenInfo;
		DWORD returnLength = 0;
		if (GetTokenInformation(hToken, TokenElevation, &tokenInfo, sizeof(tokenInfo), &returnLength) != 0)
		{
			return tokenInfo.TokenIsElevated != 0;
		}
	}

#endif
	return false;
}

void DisplayCurrentState(const SupportedNicEntry* NicList, const size_t entryCount)
{
	std::cout << std::setw(10) << std::left << "[Index]";
	std::cout << std::setw(12) << "[State]";
	std::cout << std::setw(14) << "[MAC]";
	std::cout << "[Name]\n";

	for (uint32_t i = 0; i < entryCount; i++)
	{
		std::stringstream ss;
		ss << "[" << (NicList[i].state ? "ENABLED" : "DISABLED") << "]";

		std::cout << std::setw(10) << std::left << (i + 1)
				  << std::setw(12) << std::left << ss.str()
				  << std::setw(14) << NicList[i].mac_addr
				  << "\"" << NicList[i].name << "\"\n";
	}
}

int main()
{
	if (!CheckForElevatedPermissions())
	{
		std::cout << "\nProgram must be run with elevated permissions.\n\n";
		return -1;
	}

	std::cout << "\nCpp_NicPoEControl\n\n";
	std::cout << "Scanning for supported interfaces...\n\n";

	// Here we use an array of 32 entries to retrieve the interface list.
	// It is possible to get only the number of interfaces by passing nullptr for the list argument.
	// The value returned in entryCount can be used to allocate a list of the appropriate size before
	// calling GetListOfSupportedNics() again.
	size_t entryCount = MAX_NUM_NIC_ENTRIES;

	// The SupportedNicEntry struct is defined in LucidNicControlLib.h
	SupportedNicEntry NicList[MAX_NUM_NIC_ENTRIES];

	int result = LucidNicPoEControl::GetListOfSupportedNics(NicList, &entryCount);

	if (result < 0)
	{
		if (result == LUCID_NIC_ERR_BUFFER_OVERFLOW)
		{
			std::cout << "\nError - Specified array too small. " << entryCount << " supported interfaces discovered.\n";
		}
		else
		{
			std::cout << "\nError - Failed to retrieve list of supported network interfaces!\n";
		}
		
		return result;
	}

	if (entryCount == 0)
	{
		std::cout << "No supported interfaces detected.\n";
		return 0;
	}

	// Main loop
	std::string selection = "";
	size_t nicIndex = 0;

	while (selection != "x")
	{
		std::cout << "\n";

		DisplayCurrentState(NicList, entryCount);

		std::cout << "\nEnter the index of the interface you wish to toggle the PoE state for ('x' to exit): ";
		std::cin >> selection;

		if (selection == "x")
		{
			// quit
			std::cout << "\nExample complete\n";
			return 0;
		}

		try
		{
			nicIndex = std::stoi(selection);
		}
		catch(...)
		{
			std::cout << "Invalid selection...\n";
			continue;
		}

		// Toggle PoE state of selected interface
		if (nicIndex >= 1 && nicIndex <= entryCount)
		{
			if (NicList[nicIndex-1].state)
			{
				// Disable PoE
				result = LucidNicPoEControl::PoEDisableStateByMac(NicList[nicIndex - 1].mac_addr);
				if (result == 0)
				{
					// Read back current state
					result = LucidNicPoEControl::PoEQueryStateByMac(NicList[nicIndex - 1].mac_addr, &NicList[nicIndex - 1].state);
					if (result < 0)
					{
						std::cout << "Error - Failed to query PoE state for interface: \"" << NicList[nicIndex - 1].name << "\"\n";
					}
				}
				else
				{
					std::cout << "Error - Failed to disable PoE for interface: \"" << NicList[nicIndex - 1].name << "\"\n";
				}
			}
			else
			{
				// Enable PoE
				result = LucidNicPoEControl::PoEEnableStateByName(NicList[nicIndex - 1].name);
				if (result == 0)
				{
					// Read back current state
					result = LucidNicPoEControl::PoEQueryStateByName(NicList[nicIndex - 1].name, &NicList[nicIndex - 1].state);
					if (result < 0)
					{
						std::cout << "Error - Failed to query PoE state for interface: \"" << NicList[nicIndex - 1].name << "\"\n";
					}
				}
				else
				{
					std::cout << "Error - Failed to enable PoE for interface: \"" << NicList[nicIndex - 1].name << "\"\n";
				}
			}
		}
		else
		{
			std::cout << "Out of range...\n";
			continue;
		}
	}

	std::cout << "\n\n";

	return 0;
}