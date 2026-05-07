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

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

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

#define MAX_BUF 1024

int CheckForElevatedPermissions()
{
#if defined(__linux__)
	return (getuid() == 0);
#elif defined(_WIN32)
	HANDLE hProcess = GetCurrentProcess();
	HANDLE hToken = NULL;

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
	return 0;
}

void DisplayCurrentState(const SupportedNicEntry* NicList, size_t entryCount)
{
	printf("%-10s%-12s%-14s%s\n", "[Index]", "[State]", "[MAC]", "[Name]");

	for (size_t i = 0; i < entryCount; i++)
	{
		printf("%-10zu%-12s%-14s\"%s\"\n",
			   i + 1,
			   (NicList[i].state ? "[ENABLED]" : "[DISABLED]"),
			   NicList[i].mac_addr,
			   NicList[i].name);
	}
}

int main()
{
	if (!CheckForElevatedPermissions())
	{
		printf("\nProgram must be run with elevated permissions.\n\n");
		return -1;
	}

	printf("\nC_NicPoEControl\n\n");
	printf("Scanning for supported interfaces...\n\n");

	// Here we use an array of 32 entries to retrieve the interface list.
	// It is possible to get only the number of interfaces by passing nullptr for the list argument.
	// The value returned in entryCount can be used to allocate a list of the appropriate size before
	// calling GetListOfSupportedNics() again.
	size_t entryCount = MAX_NUM_NIC_ENTRIES;

	// The SupportedNicEntry struct is defined in LucidNicControlLib.h
	SupportedNicEntry NicList[MAX_NUM_NIC_ENTRIES];

	int result = GetListOfSupportedNics(NicList, &entryCount);

	if (result < 0)
	{
		if (result == LUCID_NIC_ERR_BUFFER_OVERFLOW)
		{
			printf("\nError - Specified array too small. %zu supported interfaces discovered.\n", entryCount);
		}
		else
		{
			printf("\nError - Failed to retrieve list of supported network interfaces!\n");
		}
		return result;
	}

	if (entryCount == 0)
	{
		printf("No supported interfaces detected.\n");
		return 0;
	}

	char selection[MAX_BUF];
	size_t nicIndex = 0;

	// Main loop
	while (1)
	{
		printf("\n");
		DisplayCurrentState(NicList, entryCount);

		printf("\nEnter the index of the interface you wish to toggle the PoE state for ('x' to exit): ");
		
		#ifdef _WIN32
    			if (scanf_s("%31s", selection, (unsigned)_countof(selection)) != 1)
		#else
    			if (scanf("%31s", selection) != 1)
		#endif
		continue;

		// quit
		if (strcmp(selection, "x") == 0)
		{
			printf("\nExample complete\n");
			return 0;
		}

		nicIndex = (size_t)atoi(selection);

		// Toggle PoE state of selected interface
		if (nicIndex >= 1 && nicIndex <= entryCount)
		{
			SupportedNicEntry* nic = &NicList[nicIndex - 1];

			if (nic->state)
			{
				// Disable PoE
				result = PoEDisableStateByMac(nic->mac_addr);
				if (result == 0)
				{
					// Read back current state
					result = PoEQueryStateByMac(nic->mac_addr, &nic->state);
					if (result < 0)
					{
						printf("Error - Failed to query PoE state for interface: \"%s\"\n", nic->name);
					}
				}
				else
				{
					printf("Error - Failed to disable PoE for interface: \"%s\"\n", nic->name);
				}
			}
			else
			{
				// Enable PoE
				result = PoEEnableStateByName(nic->name);
				if (result == 0)
				{
					// Read back current state
					result = PoEQueryStateByName(nic->name, &nic->state);
					if (result < 0)
					{
						printf("Error - Failed to query PoE state for interface: \"%s\"\n", nic->name);
					}
				}
				else
				{
					printf("Error - Failed to enable PoE for interface: \"%s\"\n", nic->name);
				}
			}
		}
		else
		{
			printf("Out of range...\n");
			continue;
		}
	}

	return 0;
}
