/*
 * Copyright (C) 2012-2019 Motion Systems
 * 
 * This file is part of ForceSeat motion system.
 *
 * www.motionsystems.eu
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <chrono>
//#include <ros/ros.h>
//#include <std_msgs/String.h>
#include "ForceSeatMI_Functions.h"
#define M_PI 3.14159265358979323846264338327950288

static void append(std::wstring& str, const wchar_t* text)
{
	if (! str.empty())
	{
		str += L" ";
	}
	str += text;
}

static const wchar_t* statusToString(FSMI_UINT8 status)
{
	switch (status)
	{
		case FSMI_ModuleStatus_Ok:                   return L"Ok";
		case FSMI_ModuleStatus_Overheated:           return L"Overheated";
		case FSMI_ModuleStatus_Communication_Error:  return L"Communication Error";
		case FSMI_ModuleStatus_Config_Error:         return L"Config Error";
		case FSMI_ModuleStatus_RefPointSwitch_Error: return L"Limit Switch Error";
		case FSMI_ModuleStatus_Calibration_Error:    return L"Calibration Error";
		case FSMI_ModuleStatus_General_Error:        return L"General Error";
		case FSMI_ModuleStatus_NotConnected_Error:   return L"Not connected";
		case FSMI_ModuleStatus_NoPowerSupply_Error:  return L"No power supply error";
		case FSMI_ModuleStatus_FanSpeedTooLow_Error: return L"Fan speed too low error";
		case FSMI_ModuleStatus_OutOfRange_Error:     return L"Out of range error";
		case FSMI_ModuleStatus_StallDetected_Error:  return L"Stall detected error";
		case FSMI_ModuleStatus_EMCY_Error:           return L"EMCY error";
		default:                                     return L"Unknown";
	}
}

static std::wstring stateToString(FSMI_UINT8 state)
{
	std::wstring result;
	if ((state & FSMI_PlatformCurrentState_AnyPaused) != 0)
	{
		append(result, L"paused");
	}
	if ((state & FSMI_PlatformCurrentState_Offline) != 0)
	{
		append(result, L"offline");
	}
	auto parkMode = state & FSMI_PlatformCurrentState_ParkModeMask;
	switch (parkMode)
	{
		case FSMI_PlatformCurrentState_SoftParkToCenter:
			append(result, L"soft center");
			break;
		case FSMI_PlatformCurrentState_SoftParkNormal:
			append(result, L"soft normal");
			break;
		case FSMI_PlatformCurrentState_SoftParkForTransport:
			append(result, L"soft transport");
			break;
		case FSMI_PlatformCurrentState_HardParkToCenter:
			append(result, L"hard center");
			break;
		case FSMI_PlatformCurrentState_HardParkNormal:
			append(result, L"hard normal");
			break;
		case FSMI_PlatformCurrentState_HardParkForTransport:
			append(result, L"hard transport");
			break;
		default:
			break;
	}

	if (parkMode != FSMI_PlatformCurrentState_NoParking && (state & FSMI_PlatformCurrentState_ParkingCompleted) != 0)
	{
		append(result, L"(completed)");
	}

	return result.empty() ? L"running" : result;
}

static void work(FSMI_Handle api, std::vector<double>& stim, std::vector<double>& time)//, ros::Publisher chatter_pub)
{
	///*ROS MESSAGE*/
	//std_msgs::String msg;
	//std::stringstream ss;
	///*Trigger sent when the loop starts*/
	printf("Giving time for the client to connect...\n");
	Sleep(3000);

	// Communication structure
	FSMI_TopTablePositionPhysical pos;
	FSMI_PlatformInfo platformInfo;

	// Demo data
	float x = 0;
	float step = 0.01f;
	float tm = 0;
	
	
	int t = 0; //discrete time, sample index
	std::chrono::system_clock::time_point start;
	std::chrono::system_clock::time_point end;
	std::chrono::system_clock::duration elapsed;
	std::chrono::system_clock::duration fstep; 
	// Prepare structure by clearing it and setting correct size
	memset(&pos, 0, sizeof(pos));
	pos.structSize = sizeof(pos);

	// The demo program is able to provide pause, position and max speed
	pos.mask = FSMI_POS_BIT_STATE | FSMI_POS_BIT_MATRIX | FSMI_POS_BIT_MAX_SPEED;

	// Let's start the motion
	ForceSeatMI_BeginMotionControl(api);

	printf("SIM started...\n");
	printf("Press 'q' to exit\n");

	FSMI_UINT64 recentMark = 0;
	
	// zero position
	pos.state = FSMI_STATE_NO_PAUSE;
	pos.maxSpeed = 64000;

	pos.pitch = x; // in rad
	pos.roll = 0; // in rad
	pos.yaw = 0; // in rad
	pos.sway = 0; // in mm
	pos.surge = 0; // in mm
	pos.heave = 0; // in mm
	

	printf("zero position...\n");

	printf(" *Press S to start*\n");
	while (GetKeyState('S') == 0)
	{
		ForceSeatMI_SendTopTablePosPhy(api, &pos);
		Sleep(25);
	}
	start = std::chrono::system_clock::now();
	//// <ROS trigger...
	//std::time_t time_now_t = std::chrono::system_clock::to_time_t(start);
	//ss << "Start: " << std::ctime(&time_now_t) << std::endl;
	//msg.data = ss.str();
	//ROS_INFO("%s", msg.data.c_str());
	//chatter_pub.publish(msg);
	//ros::spinOnce();
	//// ...ROS trigger>
	while (GetKeyState('Q') == 0)
	{

		x = stim[t];
		tm = time[t];
		printf("t = %d; time = %f; FS = %f\n", t,tm, x);

		t = t + 1;
		
		if (t >= stim.size())
		{
			t = 0;
			start = std::chrono::system_clock::now();
		}

		// Fill demo dataq
		pos.pitch = x; // in rad
		pos.roll  = 0; // in rad
		pos.yaw   = 0; // in rad
		pos.sway  = 0; // in mm
		pos.surge = 0; // in mm
		pos.heave = 0; // in mm

		ForceSeatMI_SendTopTablePosPhy(api, &pos);

		// Change values somehow
		//if (step > 0)
		//{
		//	if (x < 0.3f)
		//	{
		//		x += step;
		//	}
		//	else
		//	{
		//		step = -step;
		//	}
		//}
		//else
		//{
		//	if (x > -0.3f)
		//	{
		//		x += step;
		//	}
		//	else
		//	{
		//		step = -step;
		//	}
		//}

		// Get current status
		if (ForceSeatMI_GetPlatformInfoEx(api, &platformInfo, sizeof(platformInfo), 100))
		{
			if (platformInfo.structSize != sizeof(platformInfo))
			{
				wprintf(L"Incorrect structure size: %d vs %d\n", (int)platformInfo.structSize, (int)sizeof(platformInfo));
				break;
			}
			else if (platformInfo.timemark == recentMark)
			{
				//wprintf(L"No new platform info\n");
			}
			else
			{
				//wprintf(L"Connected: %ls, State: %ls, Pos: %u, %u, %u, %u, %u, %u, Time: %llu, Module %u status: %ls\n",
				//	platformInfo.isConnected != FSMI_False ? L"yes" : L"no",
				//	stateToString(platformInfo.state).c_str(),
				//	platformInfo.actualMotorPosition[0],
				//	platformInfo.actualMotorPosition[1],
				//	platformInfo.actualMotorPosition[2],
				//	platformInfo.actualMotorPosition[3],
				//	platformInfo.actualMotorPosition[4],
				//	platformInfo.actualMotorPosition[5],
				//	platformInfo.timemark,
				//	platformInfo.worstModuleStatusIndex,
				//	statusToString(platformInfo.worstModuleStatus));

				//wprintf(L"    Inverse Kinematics: supported: %ls, precision: %f %f %f %f %f %f, status: %ls\n",
				//	platformInfo.isKinematicsSupported ? L"yes" : L"no",
				//	platformInfo.ikPrecision[0],
				//	platformInfo.ikPrecision[1],
				//	platformInfo.ikPrecision[2],
				//	platformInfo.ikPrecision[3],
				//	platformInfo.ikPrecision[4],
				//	platformInfo.ikPrecision[5],
				//	platformInfo.ikRecentStatus ? L"ok" : L"failed");

				//wprintf(L"    Actual Top Frame: %ls roll %f, pitch %f, yaw %f, heave %f, surge %f, sway %f\n",
				//	platformInfo.fkRecentStatus ? L"ok" : L"failed",
				//	platformInfo.fkRoll,
				//	platformInfo.fkPitch,
				//	platformInfo.fkYaw,
				//	platformInfo.fkHeave,
				//	platformInfo.fkSurge,
				//	platformInfo.fkSway);

				recentMark = platformInfo.timemark;
			}
		}
		else
		{
			wprintf(L"Failed to get platform info\n");
		}

		//timing
		//fstep = (std::chrono::milliseconds)(tm*1000);
		for (;;)
		{
			end = std::chrono::system_clock::now();
			elapsed = end-start;
			if (std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() >= tm * 1000)
				break;
		}
	}
	//// <ROS trigger...
	//time_now_t = std::chrono::system_clock::to_time_t(end);
	//ss << "End: " << std::ctime(&time_now_t) << std::endl;
	//msg.data = ss.str();
	//ROS_INFO("%s", msg.data.c_str());
	//chatter_pub.publish(msg);
	//ros::spinOnce();
	//// ...ROS trigger>
	ForceSeatMI_EndMotionControl(api);
	printf("Game ended...\n");
}

int loadCsv(const std::string fname,std::vector<double> &stim, std::vector<double> &time )
{
	std::fstream fin;
	std::string linestr;
	double tmp;
	int count = 0;
	fin.open(fname, std::fstream::in);

	while (std::getline(fin, linestr, ','))
	{
		count++;
		std::istringstream(linestr) >> tmp;
		time.push_back(tmp);
		std::getline(fin, linestr, '\n');
		std::istringstream(linestr) >> tmp;
		stim.push_back((tmp *  M_PI)/180.0);
	}
	std::cout<<" - "<<count<<" samples loaded from "<<fname<<std::endl;
	// Open an existing file 
	//
	return 0;
}

int ChoseCSV(const std::string fname, std::string& CSVname)
{
	std::fstream fin;
	double tmp;
	int count = 0;
	fin.open(fname, std::fstream::in);

	std::getline(fin, CSVname);

	return 0;
}
// wchar_t*
int main(int argc,char* argv[])
//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	std::vector<double> stim;
	std::vector<double> time;
	std::string CSV;

	///*A ROS node is launched for synchronization*/
	//ros::init(argc, argv, "Platform");
	//ros::NodeHandle n;
	//ros::Publisher chatter_pub = n.advertise<std_msgs::String>("trigger", 1000);

	// ros::Rate loop_rate(10); The timing implemented for the control will be used. 
	/*                                          */
	FSMI_Handle api = ForceSeatMI_Create();
	if (! api)
	{
		wprintf(L"ForceSeatMI DLL was not loaded, so the game will not send anything to the ForceSeatPM\n");
		return -1;
	}

	ChoseCSV("Signal.txt",CSV);
	loadCsv(CSV, stim, time);
	printf("Profile Loaded\n");
	// Activate appropriate ForceSeatPM profile
	ForceSeatMI_SetAppID(api, ""); // If you have dedicated app id, remove ActivateProfile calls from your code
	ForceSeatMI_ActivateProfile(api, "SDK - Positioning");

	work(api, stim, time);// , chatter_pub);
	ForceSeatMI_Delete(api);
	return 0;
}
