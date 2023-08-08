// A330_Skalarki.cpp : Defines the exported functions for the DLL.
//

#define WIN32_LEAN_AND_MEAN

#include "pch.h"
#include "XPLMPlugin.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"
#include "XPLMUtilities.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"
#include "XPLMScenery.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "SkalarkiCmd.h"
#include "SkalarkiComm.h"

const float MIN_SPEED = 125.0;
const float IN_HG_TO_HPA = 33.863889532610884;

#define MSG_ADD_DATAREF 0x01000000           //  Add dataref to DRE message

bool DatarefsInitialized = false;

SkalarkiComm* gSkalarkiComm = nullptr;

XPLMCommandRef AirbusTakeoverPbCommand = NULL;	//  Our two custom commands
XPLMCommandRef AutothrustOffCommand = NULL;

XPLMDataRef ap1 = NULL;
XPLMDataRef ap2 = NULL;
XPLMDataRef athr_mode = NULL;
XPLMDataRef appr = NULL;

XPLMDataRef master_caution = NULL;
XPLMDataRef master_warning = NULL;

XPLMCommandRef ap1_push = NULL;
XPLMCommandRef ap2_push = NULL;
XPLMCommandRef athr_push = NULL;
XPLMCommandRef appr_push = NULL;

XPLMDataRef gFdDataref = NULL;
XPLMCommandRef gFdToggleDataref = NULL;

XPLMDataRef gLsDataref = NULL;
XPLMCommandRef gLsToggleDataref = NULL;

XPLMDataRef gLocDataref = NULL;
XPLMCommandRef gLocToggleDataref = NULL;

XPLMDataRef gAltDataref = NULL;
XPLMDataRef gAlt1000Dataref = NULL;
XPLMDataRef gSpdDataref = NULL;
XPLMDataRef gHdgDataref = NULL;
XPLMDataRef gVsDataref = NULL;

XPLMCommandRef gAltPullDataref = NULL;
XPLMCommandRef gAltPressDataref = NULL;

XPLMCommandRef gAltModeLeftDataref = NULL;
XPLMCommandRef gAltModeRightDataref = NULL;

XPLMDataRef gAltModeDataref = NULL;

XPLMCommandRef gSpdPullDataref = NULL;
XPLMCommandRef gSpdPressDataref = NULL;

XPLMCommandRef gHdgPullDataref = NULL;
XPLMCommandRef gHdgPressDataref = NULL;

XPLMDataRef gHdgManagedDataref = NULL;
XPLMDataRef gSpdManagedDataref = NULL;
XPLMDataRef gVsManagedDataref = NULL;
XPLMDataRef gAltManagedDataref = NULL;

XPLMDataRef gNdModeDataref = NULL;
XPLMDataRef gNdModeLeftDataref = NULL;
XPLMDataRef gNdModeRightDataref = NULL;
XPLMDataRef gNdRangeDataref = NULL;

XPLMDataRef gBaroCptStd = NULL;
XPLMDataRef gBaroCptMode = NULL;
XPLMDataRef gBaroCpt = NULL;
XPLMCommandRef gBaroCptPull = NULL;
XPLMCommandRef gBaroCptPush = NULL;
XPLMCommandRef gBaroCptUp = NULL;
XPLMCommandRef gBaroCptDown = NULL;
XPLMCommandRef gBaroCptInHg = NULL;
XPLMCommandRef gBaroCptHpa = NULL;
//XPLMDataRef gBaroCptHpaDisplay = NULL;
//XPLMDataRef gBaroCptInHgDisplay = NULL;

XPLMDataRef gBaroFoStd = NULL;
XPLMDataRef gBaroFoMode = NULL;
XPLMDataRef gBaroFoSet = NULL;
XPLMDataRef gBaroFoPull = NULL;
XPLMDataRef gBaroFoPush = NULL;
//XPLMDataRef gBaroFoHpaDisplay = NULL;
//XPLMDataRef gBaroFoInHgDisplay = NULL;

XPLMCommandRef gVsPull = NULL;
XPLMCommandRef gVsPush = NULL;
XPLMCommandRef gVsUp = NULL;
XPLMCommandRef gVsDown = NULL;

XPLMCommandRef clearMasterWarningCommand = NULL;
XPLMCommandRef autoPilotServosToggleCmd = NULL;
XPLMCommandRef autoPilotFlightDirDownCmd = NULL;

float A330_SkalarkiUpdate(float elapsedMe, float elapsedSim, int counter, void* refcon);
void ProcessPacket(char* recvbuf);
int UpdateAlt(int AltNew);
int UpdateSpd(int SpdNew);
int UpdateHdg(int HdgNew);
int UpdateVs(int VsNew);

int UpdateBaroCpt(float BaroNew);
int SetBaroCptStd();
int UpdateBaroFo(float BaroNew);
int SetBaroFoStd();

float Spd = 250.0;
float SpdSet = 250.0;
float Alt = 0.0;
float AltSet = 0.0;
float Hdg = 0.0;
float HdgSet = 0.0;
int Vs = 0;
int VsSet = 0;
float BaroCpt = 29.92;
float BaroCptSet = 29.92;
int BaroCptStd = 1;
int BaroCptStdSet = 1;
int BaroCptMode = 0; // 0 = inHg, 1 = hPa
int BaroCptModeSet = 0;
int BaroCptDisplay = 0;
bool BaroCptLeftSet = false;
bool BaroCptRightSet = false;
bool BaroCptPushSet = false;
bool BaroCptPullSet = false;
bool BaroCaptInHgSet = false;
bool BaroCaptHpaSet = false;

bool UpdateBaroCptValue = true;
bool UpdateBaroCptDisplay = true;

float BaroFo = 29.92;
int BaroFoStd = 1;
int BaroFoStdSet = 1;
int BaroFoMode = 0; // 0 = inHg, 1 = hPa
int BaroFoModeSet = 0;
int BaroFoDisplay = 0;

bool UpdateBaroFoValue = true;
bool UpdateBaroFoDisplay = true;

int SpdDot = 0;
int HdgDot = 0;
int AltDot = 0;

bool AltPullSet = false;
bool AltPressSet = false;

bool SpdPullSet = false;
bool SpdPressSet = false;

bool HdgPullSet = false;
bool HdgPressSet = false;

bool SpdMachMode = false;
bool BaroQnhMode = false;

bool SpdManaged = true;
bool HdgManaged = true;
bool AltManaged = true;
bool VsManaged = true;
int AltMode = 0;
int AltModeSet = 0;
int NdRangeSet = 2; // 40 (default value of A330)
int NdRange = 2; // 40 (default value of A330)
int NdModeSet = 0; // LS (default value of A330)
int NdMode = 0; // LS (default value of A330)

int LsState = 0;
int FdState = 0;
int LocState = 0;

int ApState1 = 0;
int ApState2 = 0;
int AthrState = 0;
int ApprState = 0;

int MasterCautionState = 0;
int MasterWarningState = 0;

bool Ap1Toggle = false;
bool Ap2Toggle = false;
bool AthrToggle = false;
bool FdCptToggle = false;
bool LsCptToggle = false;
bool FdFoToggle = false;
bool LsFoToggle = false;
bool LocToggle = false;
bool ExpedToggle = false;
bool ApprToggle = false;

bool VsPullSet = false;
bool VsPushSet = false;
bool VsLeftSet = false;
bool VsRightSet = false;

int packet_counter = 0;

bool AltModePrepare = false;

PLUGIN_API int XPluginStart(
    char* outName,
    char* outSig,
    char* outDesc)
{

    // Plugin Info
    strcpy_s(outName, 100, "A330_Skalarki");
    strcpy_s(outSig, 100, "Vrumugun.A330_Skalarki");
    strcpy_s(outDesc, 200, ".");

    XPLMDebugString("A330_Skalarki: Start...\n");

    DatarefsInitialized = false;

    gSkalarkiComm = new SkalarkiComm();

    gSkalarkiComm->Init(initBuf, intBufLen, &ProcessPacket);

    gSkalarkiComm->PushMessage(sendbufBacklightOn, sendbufLenBacklightOn);
    gSkalarkiComm->PushMessage(AltLvlChngOn, AltLvlChngOnLen);
    gSkalarkiComm->PushMessage(HdgVsOn, HdgVsOnLen);
    gSkalarkiComm->PushMessage(SpdOn, SpdOnLen);
    gSkalarkiComm->PushMessage(LatOn, LatOnLen);
    
    UpdateHdg(Hdg);
    UpdateAlt(Alt);
    UpdateSpd(Spd);
    UpdateVs(Vs);
    UpdateBaroCpt(BaroCpt);

    XPLMRegisterFlightLoopCallback(A330_SkalarkiUpdate, 1, NULL);   // This FLCB will register our custom dataref in DRE
    
    return 1;
}

void InitDatarefs()
{
    if (gFdDataref == NULL)
    {
        gFdDataref = XPLMFindDataRef("laminar/A333/annun/capt_flight_director_on");
    }

    if (gFdToggleDataref == NULL)
    {
        gFdToggleDataref = XPLMFindCommand("sim/autopilot/fdir_toggle"); // sim/autopilot/fdir2_toggle for FO
    }

    if (gLsDataref == NULL)
    {
        gLsDataref = XPLMFindDataRef("laminar/A333/annun/captain_ls_bars_on");
    }

    if (gLsToggleDataref == NULL)
    {
        gLsToggleDataref = XPLMFindCommand("laminar/A333/buttons/capt_ils_bars_push"); // laminar/A333/buttons/fo_ils_bars_push
    }

    if (gLocDataref == NULL)
    {
        gLocDataref = XPLMFindDataRef("laminar/A333/annun/autopilot/loc_mode");
    }

    if (gLocToggleDataref == NULL)
    {
        gLocToggleDataref = XPLMFindCommand("sim/autopilot/NAV"); // or try sim/autopilot/back_course
    }

    // sim/cockpit/autopilot/altitude
    // sim/cockpit/autopilot/airspeed

    // laminar/A333/autopilot/speed_knob_pull
    // laminar/A333/autopilot/speed_knob_push

    // laminar/A333/autopilot/alt_step_left
    // laminar/A333/autopilot/alt_step_right
    // laminar/A333/autopilot/altitude_knob_pull
    // laminar/A333/autopilot/altitude_knob_push
    // laminar/A333/autopilot/alt_step_knob_pos

    // laminar/A333/autopilot/heading_knob_pull
    // laminar/A333/autopilot/heading_knob_push
    // sim/cockpit/autopilot/heading_mag

    // laminar/A333/PFD/capt_heading_mode
    // sim/cockpit2/autopilot/heading_mode

    // sim/cockpit2/autopilot/vnav_speed_window_open (int)
    // 0 = > managed
    // 1 = > selected

    if (gAltDataref == NULL)
    {
        gAltDataref = XPLMFindDataRef("sim/cockpit/autopilot/altitude");
        AltSet = XPLMGetDataf(gAltDataref);
    }
    
    if (gSpdDataref == NULL)
    {
        gSpdDataref = XPLMFindDataRef("sim/cockpit/autopilot/airspeed");
        SpdSet = XPLMGetDataf(gSpdDataref);
    }

    if (gHdgDataref == NULL)
    {
        gHdgDataref = XPLMFindDataRef("sim/cockpit/autopilot/heading_mag");
        HdgSet = XPLMGetDataf(gHdgDataref);
    }

    if (gVsDataref == NULL)
    {
        gVsDataref = XPLMFindDataRef("sim/cockpit/autopilot/vertical_velocity");
    }

    if (gAltPullDataref == NULL)
    {
        gAltPullDataref = XPLMFindCommand("laminar/A333/autopilot/altitude_knob_pull");
    }

    if (gAltPressDataref == NULL)
    {
        gAltPressDataref = XPLMFindCommand("laminar/A333/autopilot/altitude_knob_push");
    }

    if (gSpdPullDataref == NULL)
    {
        gSpdPullDataref = XPLMFindCommand("laminar/A333/autopilot/speed_knob_pull");
    }

    if (gSpdPressDataref == NULL)
    {
        gSpdPressDataref = XPLMFindCommand("laminar/A333/autopilot/speed_knob_push");
    }

    if (gHdgPullDataref == NULL)
    {
        gHdgPullDataref = XPLMFindCommand("laminar/A333/autopilot/heading_knob_pull");
    }

    if (gHdgPressDataref == NULL)
    {
        gHdgPressDataref = XPLMFindCommand("laminar/A333/autopilot/heading_knob_push");
    }

    if (gAltModeDataref == NULL)
    {
        gAltModeDataref = XPLMFindDataRef("laminar/A333/autopilot/alt_step_knob_pos");
        AltMode = XPLMGetDatai(gAltModeDataref);
    }

    if (gAltModeLeftDataref == NULL)
    {
        gAltModeLeftDataref = XPLMFindCommand("laminar/A333/autopilot/alt_step_left");
    }

    if (gAltModeRightDataref == NULL)
    {
        gAltModeRightDataref = XPLMFindCommand("laminar/A333/autopilot/alt_step_right");
    }

    if (gNdModeDataref == NULL)
    {
        gNdModeDataref = XPLMFindDataRef("sim/cockpit2/EFIS/map_mode");
    }

    if (gNdModeLeftDataref == NULL)
    {
        gNdModeLeftDataref = XPLMFindCommand("laminar/A333/knobs/capt_EFIS_knob_left");
    }

    if (gNdModeRightDataref == NULL)
    {
        gNdModeRightDataref = XPLMFindCommand("laminar/A333/knobs/capt_EFIS_knob_right");
    }

    if (gNdRangeDataref == NULL)
    {
        gNdRangeDataref = XPLMFindDataRef("sim/cockpit2/EFIS/map_range");
    }

    if (gBaroCptMode == NULL)
    {
        gBaroCptMode = XPLMFindDataRef("laminar/A333/barometer/capt_inHg_hPa_pos");
        BaroCptMode = XPLMGetDatad(gBaroCptMode);
    }

    if (gBaroCpt == NULL)
    {
        gBaroCpt = XPLMFindDataRef("sim/cockpit/misc/barometer_setting");
        BaroCpt = XPLMGetDataf(gBaroCpt);
    }

    if (gHdgManagedDataref == NULL)
    {
        gHdgManagedDataref = XPLMFindDataRef("laminar/A333/autopilot/hdg_window_open");
        int i = XPLMGetDatad(gHdgManagedDataref);
        if (i == 0)
        {
            HdgManaged = true;
        }
        else
        {
            HdgManaged = false;
        }
    }

    if (gSpdManagedDataref == NULL)
    {
        gSpdManagedDataref = XPLMFindDataRef("sim/cockpit2/autopilot/vnav_speed_window_open");
        int i = XPLMGetDatai(gSpdManagedDataref);
        if (i == 0)
        {
            SpdManaged = true;
            gSkalarkiComm->PushMessage(SpdDotOn, SpdDotOnLen);
        }
        else
        {
            SpdManaged = false;
            gSkalarkiComm->PushMessage(SpdDotOff, SpdDotOffLen);
        }
    }

    if (gVsManagedDataref == NULL)
    {
        gVsManagedDataref = XPLMFindDataRef("laminar/A333/autopilot/vvi_fpa_window_open");
        int i = XPLMGetDatad(gVsManagedDataref);
        if (i == 0)
        {
            VsManaged = true;
        }
        else
        {
            VsManaged = false;
        }
    }
        
    if (gAltManagedDataref == NULL)
    {
        gAltManagedDataref = XPLMFindDataRef("sim/cockpit2/autopilot/fms_vnav");
        int i = XPLMGetDatai(gAltManagedDataref);
        if (i == 1)
        {
            AltManaged = true;
        }
        else
        {
            AltManaged = false;
        }
    }

    if (ap1 == NULL)
    {
        ap1 = XPLMFindDataRef("laminar/A333/annun/autopilot/ap1_mode");
    }

    if (ap1_push == NULL)
    {
        ap1_push = XPLMFindCommand("sim/autopilot/servos_toggle");
    }

    if (ap2 == NULL)
    {
        ap2 = XPLMFindDataRef("laminar/A333/annun/autopilot/ap2_mode");
    }

    if (ap2_push == NULL)
    {
        ap2_push = XPLMFindCommand("sim/autopilot/servos2_toggle");
    }

    if (athr_mode == NULL)
    {
        athr_mode = XPLMFindDataRef("laminar/A333/annun/autopilot/a_thr_mode");
    }

    if (athr_push == NULL)
    {
        athr_push = XPLMFindCommand("laminar/A333/autopilot/a_thr_toggle");
    }

    if (appr == NULL)
    {
        appr = XPLMFindDataRef("laminar/A333/annun/autopilot/appr_mode");
    }

    if (appr_push == NULL)
    {
        appr_push = XPLMFindCommand("sim/autopilot/approach");
    }

    if (master_caution == NULL)
    {
        master_caution = XPLMFindDataRef("laminar/A333/annun/master_caution");
    }

    if (master_warning == NULL)
    {
        master_warning = XPLMFindDataRef("laminar/A333/annun/master_warning");
    }

    if (gBaroCptStd == NULL)
    {
        gBaroCptStd = XPLMFindDataRef("laminar/A333/barometer/capt_mode");
        BaroCptStdSet = XPLMGetDatad(gBaroCptStd);
    }

    if (gBaroCptPull == NULL)
    {
        gBaroCptPull = XPLMFindCommand("laminar/A333/pull/baro/capt_std");
    }

    if (gBaroCptPush == NULL)
    {
        gBaroCptPush = XPLMFindCommand("laminar/A333/push/baro/capt_std");
    }

    if (gBaroCptUp == NULL)
    {
        gBaroCptUp = XPLMFindCommand("sim/instruments/barometer_up");
    }

    if (gBaroCptDown == NULL)
    {
        gBaroCptDown = XPLMFindCommand("sim/instruments/barometer_down");
    }

    if (gBaroCptInHg == NULL)
    {
        gBaroCptInHg = XPLMFindCommand("laminar/A333/knob/baro/capt_inHg");
    }

    if (gBaroCptHpa == NULL)
    {
        gBaroCptHpa = XPLMFindCommand("laminar/A333/knob/baro/capt_hPa");
    }

    if (gVsPull == NULL)
    {
        gVsPull = XPLMFindCommand("laminar/A333/autopilot/vertical_knob_pull");
    }

    if (gVsPush == NULL)
    {
        // or try "sim/autopilot/vertical_speed" ?
        gVsPush = XPLMFindCommand("laminar/A333/autopilot/vertical_knob_push");
    }

    if (gVsUp == NULL)
    {
        gVsUp = XPLMFindCommand("sim/autopilot/vertical_speed_up");
    }

    if (gVsDown == NULL)
    {
        gVsDown = XPLMFindCommand("sim/autopilot/vertical_speed_down");
    }

    DatarefsInitialized = true;
}

float A330_SkalarkiUpdate(float elapsedMe, float elapsedSim, int counter, void* refcon)
{
    if (DatarefsInitialized == false)
    {
        InitDatarefs();
    }

    if (FdCptToggle == true)
    {
        XPLMCommandOnce(gFdToggleDataref);
		FdCptToggle = false;
    }

    if (LsCptToggle == true)
    {
		XPLMCommandOnce(gLsToggleDataref);
		LsCptToggle = false;
    }

    if (VsPullSet == true)
    {
        XPLMCommandOnce(gVsPull);
		VsPullSet = false;
    }

    if (VsPushSet == true)
    {
		XPLMCommandOnce(gVsPush);
		VsPushSet = false;
    }

    if (VsLeftSet == true)
    {
        XPLMCommandOnce(gVsDown);
		VsLeftSet = false;
    }

    if (VsRightSet == true)
    {
        XPLMCommandOnce(gVsUp);
        VsRightSet = false;
    }

    if (LocToggle == true)
    {
		XPLMCommandOnce(gLocToggleDataref);
		LocToggle = false;
    }

	int vsnew = XPLMGetDataf(gVsDataref);
    if (vsnew != Vs)
    {
		Vs = vsnew;
        if (VsManaged == false)
        {
            UpdateVs(Vs);
        }
    }

    int VsStateNew = XPLMGetDatad(gVsManagedDataref);
    if (VsStateNew == 0)
    {
        if (VsManaged == false)
        {
            VsManaged = true;
            UpdateVs(Vs);
        }
    }
    else
    {
        if (VsManaged == true)
        {
            VsManaged = false;
            UpdateVs(Vs);
        }
    }

    int AltManagedNew = XPLMGetDatai(gAltManagedDataref);
    if (AltManagedNew == 1)
    {
        if (AltManaged == false)
        {
            AltManaged = true;
            gSkalarkiComm->PushMessage(AltDotOn, AltDotOnLen);
        }
    }
    else
    {
        if (AltManaged == true)
        {
            AltManaged = false;
            gSkalarkiComm->PushMessage(AltDotOff, AltDotOffLen);
        }
    }

    if (Ap1Toggle == true)
    {
		if (ap1_push != NULL)
		{
			XPLMCommandOnce(ap1_push);
		}
		Ap1Toggle = false;
    }

    if (Ap2Toggle == true)
    {
        if (ap2_push != NULL)
        {
            XPLMCommandOnce(ap2_push);
        }
        Ap2Toggle = false;
    }

	if (AthrToggle == true)
	{
		if (athr_push != NULL)
		{
			XPLMCommandOnce(athr_push);
		}
		AthrToggle = false;
	}

	if (ApprToggle == true)
	{
		if (appr_push != NULL)
		{
			XPLMCommandOnce(appr_push);
		}
		ApprToggle = false;
	}

    int AthrStateNew = (XPLMGetDatad(athr_mode) > 0.001) ? 1 : 0;
    if (AthrState != AthrStateNew)
    {
        AthrState = AthrStateNew;
        if (AthrState == 1)
        {
            gSkalarkiComm->PushMessage(AthrOn, AthrOnLen);
        }
        else
        {
            gSkalarkiComm->PushMessage(AthrOff, AthrOffLen);
        }
    }

    int ApprStateNew = (XPLMGetDatad(appr) > 0.001) ? 1 : 0;
    if (ApprState != ApprStateNew)
    {
        ApprState = ApprStateNew;
        if (ApprState == 1)
        {
            gSkalarkiComm->PushMessage(ApprOn, ApprOnLen);
        }
        else
        {
            gSkalarkiComm->PushMessage(ApprOff, ApprOffLen);
        }
    }

    int ApStateNew1 = (XPLMGetDatad(ap1) > 0.001) ? 1 : 0;
    if (ApState1 != ApStateNew1)
    {
        ApState1 = ApStateNew1;
        if (ApState1 == 1)
        {
            gSkalarkiComm->PushMessage(Ap1On, Ap1OnLen);
        }
        else
        {
            gSkalarkiComm->PushMessage(Ap1Off, Ap1OffLen);
        }
    }

    int ApStateNew2 = (XPLMGetDatad(ap2) > 0.001) ? 1 : 0;
    if (ApState2 != ApStateNew2)
    {
        ApState2 = ApStateNew2;
        if (ApState2 == 1)
        {
            gSkalarkiComm->PushMessage(Ap2On, Ap2OnLen);
        }
        else
        {
            gSkalarkiComm->PushMessage(Ap2Off, Ap2OffLen);
        }
    }

	int MasterCautionStateNew = (XPLMGetDatad(master_caution) > 0.001) ? 1 : 0;
	if (MasterCautionState != MasterCautionStateNew)
	{
		MasterCautionState = MasterCautionStateNew;
		if (MasterCautionState == 1)
		{
            gSkalarkiComm->PushMessage(MasterCautionOn, MasterCautionOnLen);
		}
		else
		{
            gSkalarkiComm->PushMessage(MasterCautionOff, MasterCautionOffLen);
		}
	}
    
	int MasterWarningStateNew = (XPLMGetDatad(master_warning) > 0.001) ? 1 : 0;
	if (MasterWarningState != MasterWarningStateNew)
	{
		MasterWarningState = MasterWarningStateNew;
		if (MasterWarningState == 1)
		{
            gSkalarkiComm->PushMessage(MasterWarningOn, MasterCautionOnLen);
		}
		else
		{
            gSkalarkiComm->PushMessage(MasterWarningOff, MasterWarningOffLen);
		}
	}
    
    int i = XPLMGetDatad(gHdgManagedDataref);
    if (i == 0)
    {
        if (HdgManaged == false)
        {
            HdgManaged = true;
            UpdateHdg(Hdg);
            gSkalarkiComm->PushMessage(HdgDotOn, HdgDotOnLen);
        }
    }
    else
    {
        if (HdgManaged == true)
        {
            HdgManaged = false;
            UpdateHdg(Hdg);
            gSkalarkiComm->PushMessage(HdgDotOff, HdgDotOffLen);
        }
    }

    i = XPLMGetDatai(gSpdManagedDataref);
    if (i == 0)
    {
        if (SpdManaged == false)
        {
            SpdManaged = true;
            UpdateSpd(Spd);
            gSkalarkiComm->PushMessage(SpdDotOn, SpdDotOnLen);
        }
    }
    else
    {
        if (SpdManaged == true)
        {
            SpdManaged = false;
            SpdSet = XPLMGetDataf(gSpdDataref);
            Spd = SpdSet;
            UpdateSpd(Spd);
            gSkalarkiComm->PushMessage(SpdDotOff, SpdDotOffLen);
        }
    }
    
    int Ls = (XPLMGetDatad(gLsDataref) > 0.001) ? 1 : 0; //XPLMGetDatai(gLsDataref);
    if (Ls != LsState)
    {
        LsState = Ls;
        if (LsState == 1)
        {
            gSkalarkiComm->PushMessage(LsCptOn, LsCptOnLen);
        }
        else
        {
            gSkalarkiComm->PushMessage(LsCptOff, LsCptOffLen);
        }
    }

    int Fd = (XPLMGetDatad(gFdDataref) > 0.001) ? 1 : 0; //XPLMGetDatai(gFdDataref);
    if (Fd != FdState)
    {
        FdState = Fd;
        if (FdState == 1)
        {
            gSkalarkiComm->PushMessage(FdCptOn, FdCptOnLen);
        }
        else
        {
            gSkalarkiComm->PushMessage(FdCptOff, FdCptOffLen);
        }
    }

    int loc = (XPLMGetDatad(gLocDataref) > 0.001) ? 1 : 0; //XPLMGetDatai(gFdDataref);
    if (loc != LocState)
    {
        LocState = loc;
        if (LocState == 1)
        {
            gSkalarkiComm->PushMessage(LocOn, LocOnLen);
        }
        else
        {
            gSkalarkiComm->PushMessage(LocOff, LocOffLen);
        }
    }

    if (NdMode != NdModeSet)
    {
        //NdMode = NdModeSet;
		while (NdMode != NdModeSet)
		{
			if (NdMode < NdModeSet)
			{
				NdMode++;
                XPLMCommandOnce(gNdModeRightDataref);
			}
			else
			{
				NdMode--;
                XPLMCommandOnce(gNdModeLeftDataref);
			}
		}

        //XPLMSetDatai(gNdModeDataref, NdMode);
    }

    if (NdRange != NdRangeSet)
    {
        NdRange = NdRangeSet;
        XPLMSetDatai(gNdRangeDataref, NdRange);
    }

    if (HdgSet != Hdg)
    {
        Hdg = HdgSet;
        UpdateHdg((int)(Hdg + 0.5));
        XPLMSetDataf(gHdgDataref, Hdg);
    }

    if (SpdSet != Spd)
    {
        Spd = SpdSet;
        UpdateSpd((int)Spd);
        XPLMSetDataf(gSpdDataref, Spd);
    }

    if (Alt != AltSet)
    {
        Alt = AltSet;
        UpdateAlt(Alt);

        XPLMSetDataf(gAltDataref, Alt);
    }

    if (AltMode != AltModeSet)
    {
        AltMode = AltModeSet;
        if (AltMode == 0)
        {
            XPLMCommandOnce(gAltModeLeftDataref);
        }
        else
        {
            XPLMCommandOnce(gAltModeRightDataref);
        }
    }

    if (AltPullSet == true)
    {
        XPLMCommandOnce(gAltPullDataref);
        AltPullSet = false;
    }
    else if (AltPressSet == true)
    {
        XPLMCommandOnce(gAltPressDataref);
        AltPressSet = false;
    }
    else if (HdgPressSet == true)
    {
        XPLMCommandOnce(gHdgPressDataref);
        HdgPressSet = false;
    }
    else if (HdgPullSet == true)
    {
        XPLMCommandOnce(gHdgPullDataref);
        HdgPullSet = false;
    }
    else if (SpdPressSet == true)
    {
        XPLMCommandOnce(gSpdPressDataref);
        SpdPressSet = false;
    }
    else if (SpdPullSet == true)
    {
        XPLMCommandOnce(gSpdPullDataref);
        SpdPullSet = false;
    }

    if (BaroCptPushSet == true)
    {
        BaroCptPushSet = false;
        
        if (gBaroCptPush != NULL)
        {
            XPLMCommandOnce(gBaroCptPush);
        }
    }
    
    if (BaroCptPullSet == true)
    {
        BaroCptPullSet = false;
        
        if (gBaroCptPull != NULL)
        {
            XPLMCommandOnce(gBaroCptPull);
        }
    }

    if (BaroCaptInHgSet == true)
    {
        BaroCaptInHgSet = false;

		if (gBaroCptInHg != NULL)
		{
			XPLMCommandOnce(gBaroCptInHg);
		}
    }

    if (BaroCaptHpaSet == true)
    {
        BaroCaptHpaSet = false;
        
        if (gBaroCptHpa != NULL)
        {
			XPLMCommandOnce(gBaroCptHpa);
        }
    }

    BaroCptModeSet = XPLMGetDatad(gBaroCptMode);
    if (BaroCptMode != BaroCptModeSet)
    {
        BaroCptMode = BaroCptModeSet;
        BaroCptDisplay = 0;
        UpdateBaroCptDisplay = true;
    }
    
    if (BaroCptStd == 0)
    {
        if (BaroCptLeftSet == true)
        {
            BaroCptLeftSet = false;

            if (gBaroCptDown != NULL)
            {
                XPLMCommandOnce(gBaroCptDown);
            }
        }
        
        if (BaroCptRightSet == true)
        {
            BaroCptRightSet = false;

            if (gBaroCptUp != NULL)
            {
                XPLMCommandOnce(gBaroCptUp);
            }
        }
    }
    
    if (UpdateBaroCptValue == true)
    {
        UpdateBaroCptValue = false;
        UpdateBaroCptDisplay = true;
    }
    
    BaroCptStdSet = XPLMGetDatad(gBaroCptStd);
    if (BaroCptStd != BaroCptStdSet)
    {
        BaroCptStd = BaroCptStdSet;
        if (BaroCptStd == 1)
        {
            SetBaroCptStd();
        }
        else
        {
            BaroCptDisplay = 0;
        }
    }
    
    if (BaroCptStd == 0)
    {
        if (BaroCptMode == 0)
        {
            float baro = XPLMGetDataf(gBaroCpt) + 0.005;
            int BaroDisplayNew = baro * 100;
            if (BaroCptDisplay != BaroDisplayNew)
            {
                BaroCptDisplay = BaroDisplayNew;
                UpdateBaroCpt(BaroCptDisplay / 100.0);
            }
        }
        else
        {
            float baro = (XPLMGetDataf(gBaroCpt) * IN_HG_TO_HPA) + 0.5;
            int BaroDisplayNew = baro;
            if (BaroCptDisplay != BaroDisplayNew)
            {
                BaroCptDisplay = BaroDisplayNew;
                UpdateBaroCpt(BaroCptDisplay);
            }
        }
    }

    return 0.1f;
}

PLUGIN_API void XPluginStop(void)
{
    XPLMUnregisterFlightLoopCallback(A330_SkalarkiUpdate, NULL);

    DatarefsInitialized = false;
    
    if (gSkalarkiComm != nullptr)
    {
        gSkalarkiComm->Shutdown();
        delete gSkalarkiComm;
    }
}

PLUGIN_API void XPluginDisable(void)
{

}

PLUGIN_API int XPluginEnable(void)
{
    return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID    inFromWho,
    long             inMessage,
    void* inParam)
{

}

int SetBaroCptStd()
{
    char sendbufBaroLeft[200] = "\x2e\x00\x00\x00\x64\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00" \
        "\x04\x00\x00\x00\x10\x00\x00\x00\x11\x00\x00\x00\x12\x00\x00\x00" \
        "\x13\x00\x00\x00\x0b\x41\x4c\x54\x50\x52\x45\x53\x53\x55\x52\x45" \
        "\x0e\x00\x00\x00\x01\x04\x31\x30\x31\x33";
    
    int sendBufBaroLen = 58;

    sendbufBaroLeft[sendBufBaroLen - 3 - 1] = ' ';
    sendbufBaroLeft[sendBufBaroLen - 2 - 1] = 'S';
    sendbufBaroLeft[sendBufBaroLen - 1 - 1] = 't';
    sendbufBaroLeft[sendBufBaroLen - 0 - 1] = 'd';
    
    gSkalarkiComm->PushMessage(sendbufBaroLeft, sendBufBaroLen);
    
    return 0;
}

int SetBaroFoStd()
{
    char sendBufBaroRight[200] = "\x2e\x00\x00\x00\x64\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00" \
        "\x04\x00\x00\x00\x14\x00\x00\x00\x15\x00\x00\x00\x16\x00\x00\x00" \
        "\x17\x00\x00\x00\x0b\x41\x4c\x54\x50\x52\x45\x53\x53\x55\x52\x45" \
        "\x0f\x00\x00\x00\x01\x04\x31\x30\x31\x33";

    int sendBufBaroLen = 58;
    
    sendBufBaroRight[sendBufBaroLen - 3 - 1] = ' ';
    sendBufBaroRight[sendBufBaroLen - 2 - 1] = 'S';
    sendBufBaroRight[sendBufBaroLen - 1 - 1] = 't';
    sendBufBaroRight[sendBufBaroLen - 0 - 1] = 'd';

    gSkalarkiComm->PushMessage(sendBufBaroRight, sendBufBaroLen);
    
    return 0;
}

int UpdateBaroCpt(float BaroNew)
{
    if (BaroCptMode == 0)
    {
        int baro_int = BaroNew * 100;

        char sendbufBaroLeft[200] = "\x2f\x00\x00\x00\x64\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00" \
            "\x04\x00\x00\x00\x10\x00\x00\x00\x11\x00\x00\x00\x12\x00\x00\x00" \
            "\x13\x00\x00\x00\x0b\x41\x4c\x54\x50\x52\x45\x53\x53\x55\x52\x45" \
            "\x0e\x00\x00\x00\x01\x05\x32\x39\x2e\x31\x30";
        
        int sendBufBaroLen = 59;

        int alt_0 = (baro_int / 1000) % 10;
        int alt_1 = (baro_int / 100) % 10;
        int alt_2 = (baro_int / 10) % 10;
        int alt_3 = baro_int % 10;

        sendbufBaroLeft[sendBufBaroLen - 4 - 1] = alt_0 + '0';
        sendbufBaroLeft[sendBufBaroLen - 3 - 1] = alt_1 + '0';
        sendbufBaroLeft[sendBufBaroLen - 2 - 1] = '.';
        sendbufBaroLeft[sendBufBaroLen - 1 - 1] = alt_2 + '0';
        sendbufBaroLeft[sendBufBaroLen - 0 - 1] = alt_3 + '0';
        
        gSkalarkiComm->PushMessage(sendbufBaroLeft, sendBufBaroLen);
    }
    else
    {
        char sendbufBaroLeft[200] = "\x2e\x00\x00\x00\x64\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00" \
            "\x04\x00\x00\x00\x10\x00\x00\x00\x11\x00\x00\x00\x12\x00\x00\x00" \
            "\x13\x00\x00\x00\x0b\x41\x4c\x54\x50\x52\x45\x53\x53\x55\x52\x45" \
            "\x0e\x00\x00\x00\x01\x04\x31\x30\x31\x33";
        
        int sendBufBaroLen = 58;

        int baro_int = BaroNew;

        int alt_0 = (baro_int / 1000) % 10;
        int alt_1 = (baro_int / 100) % 10;
        int alt_2 = (baro_int / 10) % 10;
        int alt_3 = baro_int % 10;

        sendbufBaroLeft[sendBufBaroLen - 3 - 1] = alt_0 + '0';
        sendbufBaroLeft[sendBufBaroLen - 2 - 1] = alt_1 + '0';
        sendbufBaroLeft[sendBufBaroLen - 1 - 1] = alt_2 + '0';
        sendbufBaroLeft[sendBufBaroLen - 0 - 1] = alt_3 + '0';
        
        gSkalarkiComm->PushMessage(sendbufBaroLeft, sendBufBaroLen);
    }
    
    return 0;
}

int UpdateBaroFo(float BaroNew)
{
    if (BaroFoMode == 0)
    {
        int baro_int = BaroNew * 100;
        
        char sendBufBaroRight[200] = "\x2f\x00\x00\x00\x64\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00" \
            "\x04\x00\x00\x00\x14\x00\x00\x00\x15\x00\x00\x00\x16\x00\x00\x00" \
            "\x17\x00\x00\x00\x0b\x41\x4c\x54\x50\x52\x45\x53\x53\x55\x52\x45" \
            "\x0f\x00\x00\x00\x01\x05\x33\x33\x2e\x33\x33";

        int sendBufBaroLen = 59;

        int alt_0 = (baro_int / 1000) % 10;
        int alt_1 = (baro_int / 100) % 10;
        int alt_2 = (baro_int / 10) % 10;
        int alt_3 = baro_int % 10;
        
        sendBufBaroRight[sendBufBaroLen - 4 - 1] = alt_0 + '0';
        sendBufBaroRight[sendBufBaroLen - 3 - 1] = alt_1 + '0';
        sendBufBaroRight[sendBufBaroLen - 2 - 1] = '.';
        sendBufBaroRight[sendBufBaroLen - 1 - 1] = alt_2 + '0';
        sendBufBaroRight[sendBufBaroLen - 0 - 1] = alt_3 + '0';
        
        gSkalarkiComm->PushMessage(sendBufBaroRight, sendBufBaroLen);
    }
    else
    {
        char sendBufBaroRight[200] = "\x2e\x00\x00\x00\x64\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00" \
            "\x04\x00\x00\x00\x14\x00\x00\x00\x15\x00\x00\x00\x16\x00\x00\x00" \
            "\x17\x00\x00\x00\x0b\x41\x4c\x54\x50\x52\x45\x53\x53\x55\x52\x45" \
            "\x0f\x00\x00\x00\x01\x04\x31\x30\x31\x33";

        int sendBufBaroLen = 58;

        int baro_int = BaroNew;

        int alt_0 = (baro_int / 1000) % 10;
        int alt_1 = (baro_int / 100) % 10;
        int alt_2 = (baro_int / 10) % 10;
        int alt_3 = baro_int % 10;
        
        sendBufBaroRight[sendBufBaroLen - 3 - 1] = alt_0 + '0';
        sendBufBaroRight[sendBufBaroLen - 2 - 1] = alt_1 + '0';
        sendBufBaroRight[sendBufBaroLen - 1 - 1] = alt_2 + '0';
        sendBufBaroRight[sendBufBaroLen - 0 - 1] = alt_3 + '0';
        
        gSkalarkiComm->PushMessage(sendBufBaroRight, sendBufBaroLen);
    }
    
    return 0;
}

int UpdateVs(int VsNew)
{
    char sendbufVs[200] = "\x2a\x00\x00\x00\x64\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00" \
        "\x05\x00\x00\x00\x0b\x00\x00\x00\x0c\x00\x00\x00\x0d\x00\x00\x00" \
        "\x0e\x00\x00\x00\x0f\x00\x00\x00\x02\x56\x53\x14\x00\x00\x00\x01" \
        "\x05\x20\x30\x30\x30\x30";

    int sendBufVsLen = 54;

    if (VsManaged == false)
    {
        if (VsNew < 0)
        {
            sendbufVs[sendBufVsLen - 4 - 1] = '-';
            VsNew = -1 * VsNew;
        }
        else
        {
            sendbufVs[sendBufVsLen - 4 - 1] = '+';
        }

        int alt_0 = (VsNew / 1000) % 10;
        int alt_1 = (VsNew / 100) % 10;
        int alt_2 = (VsNew / 10) % 10;
        int alt_3 = VsNew % 10;

        sendbufVs[sendBufVsLen - 3 - 1] = alt_0 + '0';
        sendbufVs[sendBufVsLen - 2 - 1] = alt_1 + '0';
        sendbufVs[sendBufVsLen - 1 - 1] = 'o'; //alt_2 + '0';
        sendbufVs[sendBufVsLen - 0 - 1] = 'o'; //alt_3 + '0';
    }
    else
    {
        sendbufVs[sendBufVsLen - 4 - 1] = '-';
        sendbufVs[sendBufVsLen - 3 - 1] = '-';
        sendbufVs[sendBufVsLen - 2 - 1] = '-';
        sendbufVs[sendBufVsLen - 1 - 1] = '-';
        sendbufVs[sendBufVsLen - 0 - 1] = '-';
    }

    gSkalarkiComm->PushMessage(sendbufVs, sendBufVsLen);
    
    return 0;
}

int UpdateSpd(int SpdNew)
{
    char sendbufSpd[200] = "\x21\x00\x00\x00\x64\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00" \
        "\x03\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x02\x00\x00\x00" \
        "\x03\x53\x50\x44\x14\x00\x00\x00\x01\x03\x30\x30\x30";

    int sendBufSpdLen = 45;

    if (SpdManaged == false)
    {
        int alt_0 = (SpdNew / 100) % 10;
        int alt_1 = (SpdNew / 10) % 10;
        int alt_2 = SpdNew % 10;

        sendbufSpd[sendBufSpdLen - 2 - 1] = alt_0 + '0';
        sendbufSpd[sendBufSpdLen - 1 - 1] = alt_1 + '0';
        sendbufSpd[sendBufSpdLen - 0 - 1] = alt_2 + '0';
    }
    else
    {
        sendbufSpd[sendBufSpdLen - 2 - 1] = '-';
        sendbufSpd[sendBufSpdLen - 1 - 1] = '-';
        sendbufSpd[sendBufSpdLen - 0 - 1] = '-';
    }

    gSkalarkiComm->PushMessage(sendbufSpd, sendBufSpdLen);

    return 0;
}


int UpdateHdg(int HdgNew)
{
    char sendbufHdg[200] = "\x21\x00\x00\x00\x64\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00" \
        "\x03\x00\x00\x00\x03\x00\x00\x00\x04\x00\x00\x00\x05\x00\x00\x00" \
        "\x03\x48\x44\x47\x14\x00\x00\x00\x01\x03\x30\x30\x30";
    int sendBufHdgLen = 45;

    if (HdgManaged == false)
    {
        int alt_0 = (HdgNew / 100) % 10;
        int alt_1 = (HdgNew / 10) % 10;
        int alt_2 = HdgNew % 10;

        sendbufHdg[sendBufHdgLen - 2 - 1] = alt_0 + '0';
        sendbufHdg[sendBufHdgLen - 1 - 1] = alt_1 + '0';
        sendbufHdg[sendBufHdgLen - 0 - 1] = alt_2 + '0';
    }
    else
    {
        sendbufHdg[sendBufHdgLen - 2 - 1] = '-';
        sendbufHdg[sendBufHdgLen - 1 - 1] = '-';
        sendbufHdg[sendBufHdgLen - 0 - 1] = '-';
    }

    gSkalarkiComm->PushMessage(sendbufHdg, sendBufHdgLen);
    
    return 0;
}

int UpdateAlt(int AltNew)
{

    char sendbufAlt[200] = "\x2b\x00\x00\x00\x64\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00" \
        "\x05\x00\x00\x00\x06\x00\x00\x00\x07\x00\x00\x00\x08\x00\x00\x00" \
        "\x09\x00\x00\x00\x0a\x00\x00\x00\x03\x41\x4c\x54\x14\x00\x00\x00" \
        "\x01\x05\x33\x30\x30\x30\x30";
    int sendbufLenAlt = 55;// sendbufAlt[0] + 4;

    //

    int alt_0 = (AltNew / 10000) % 10;
    int alt_1 = (AltNew / 1000) % 10;
    int alt_2 = (AltNew / 100) % 10;
    int alt_3 = (AltNew / 10) % 10;
    int alt_4 = AltNew % 10;

    sendbufAlt[sendbufLenAlt - 4 - 1] = alt_0 + '0';
    sendbufAlt[sendbufLenAlt - 3 - 1] = alt_1 + '0';
    sendbufAlt[sendbufLenAlt - 2 - 1] = alt_2 + '0';
    sendbufAlt[sendbufLenAlt - 1 - 1] = alt_3 + '0';
    sendbufAlt[sendbufLenAlt - 0 - 1] = alt_4 + '0';

    gSkalarkiComm->PushMessage(sendbufAlt, sendbufLenAlt);

    return 0;
}

// Called from receiver thread, don't call XPLM functions from here! It will crash X-Plane.
void ProcessPacket(char* recvbuf)
{
    packet_counter++;

    int msgLen = recvbuf[0] + 4;

    bool foundAltRight = true;
    bool foundAltLeft = true;
    bool foundHdgRight = true;
    bool foundHdgLeft = true;
    bool foundSpdRight = true;
    bool foundSpdLeft = true;
    bool foundVsRight = true;
    bool foundVsLeft = true;
    bool foundBaroCptLeft = true;
    bool foundBaroCptRight = true;
    bool foundBaroFoLeft = true;
    bool foundBaroFoRight = true;

    bool foundBaroCptPress = true;
    bool foundBaroCptPull = true;

    bool foundAltPull = true;
    bool foundAltPress = true;

    bool foundHdgPull = true;
    bool foundHdgPress = true;

    bool foundSpdPull = true;
    bool foundSpdPress = true;

    bool foundVsPull = true;
    bool foundVsPush = true;

    bool foundAltMode100 = true;
    bool foundAltMode1000 = true;

    bool foundNdRange320 = true;
    bool foundNdRange160 = true;
    bool foundNdRange80 = true;
    bool foundNdRange40 = true;
    bool foundNdRange20 = true;
    bool foundNdRange10 = true;

    bool foundNdModeLs = true;
    bool foundNdModeVor = true;
    bool foundNdModeNav = true;
    bool foundNdModeArc = true;
    bool foundNdModePlan = true;

    bool foundBaroInHg = true;
    bool foundBaroHpa = true;

    bool foundAp1Release = true;
    bool foundAp2Release = true;
    bool foundAthrRelease = true;
    bool foundExpedRelease = true;
    bool foundApprRelease = true;
    bool foundLocRelease = true;
    bool foundFdCptRelease = true;
    bool foundLsCptRelease = true;
    bool foundFdFoRelease = true;
    bool foundLsFoRelease = true;

    for (int i = 0; i < msgLen; i++)
    {
        if (recvbuf[i] != NdModeLs[i])
        {
            foundNdModeLs = false;
        }
        if (recvbuf[i] != NdModeVor[i])
        {
            foundNdModeVor = false;
        }
        if (recvbuf[i] != NdModeNav[i])
        {
            foundNdModeNav = false;
        }
        if (recvbuf[i] != NdModeArc[i])
        {
            foundNdModeArc = false;
        }
        if (recvbuf[i] != NdModePlan[i])
        {
            foundNdModePlan = false;
        }
        if (recvbuf[i] != Ap1Release[i])
        {
            foundAp1Release = false;
        }
        if (recvbuf[i] != Ap2Release[i])
        {
            foundAp2Release = false;
        }
        if (recvbuf[i] != AthrRelease[i])
        {
            foundAthrRelease = false;
        }
        if (recvbuf[i] != FdCptRelease[i])
        {
            foundFdCptRelease = false;
        }
        if (recvbuf[i] != LsCptRelease[i])
        {
            foundLsCptRelease = false;
        }
        if (recvbuf[i] != FdFoRelease[i])
        {
            foundFdFoRelease = false;
        }
        if (recvbuf[i] != LsFoRelease[i])
        {
            foundLsFoRelease = false;
        }
        if (recvbuf[i] != LocRelease[i])
        {
            foundLocRelease = false;
        }
        if (recvbuf[i] != ExpedRelease[i])
        {
            foundExpedRelease = false;
        }
        if (recvbuf[i] != ApprRelease[i])
        {
            foundApprRelease = false;
        }

        if (recvbuf[i] != BaroInHg[i])
        {
            foundBaroInHg = false;
        }
        if (recvbuf[i] != BaroHPa[i])
        {
            foundBaroHpa = false;
        }
        if (recvbuf[i] != NdRange10[i])
        {
            foundNdRange10 = false;
        }
        if (recvbuf[i] != NdRange20[i])
        {
            foundNdRange20 = false;
        }
        if (recvbuf[i] != NdRange40[i])
        {
            foundNdRange40 = false;
        }
        if (recvbuf[i] != NdRange80[i])
        {
            foundNdRange80 = false;
        }
        if (recvbuf[i] != NdRange160[i])
        {
            foundNdRange160 = false;
        }
        if (recvbuf[i] != NdRange320[i])
        {
            foundNdRange320 = false;
        }
        if (recvbuf[i] != AltMode100[i])
        {
            foundAltMode100 = false;
        }
        if (recvbuf[i] != AltMode1000[i])
        {
            foundAltMode1000 = false;
        }

        if (recvbuf[i] != VsPullRelease[i])
        {
            foundVsPull = false;
        }
        if (recvbuf[i] != VsPushRelease[i])
        {
            foundVsPush = false;
        }

        if (recvbuf[i] != SpdPull[i])
        {
            foundSpdPull = false;
        }
        if (recvbuf[i] != SpdPress[i])
        {
            foundSpdPress = false;
        }

        if (recvbuf[i] != HdgPull[i])
        {
            foundHdgPull = false;
        }
        if (recvbuf[i] != HdgPress[i])
        {
            foundHdgPress = false;
        }

        if (recvbuf[i] != AltPull[i])
        {
            foundAltPull = false;
        }
        if (recvbuf[i] != AltPress[i])
        {
            foundAltPress = false;
        }

        if (recvbuf[i] != BaroCptPress[i])
        {
            foundBaroCptPress = false;
        }
        if (recvbuf[i] != BaroCptPull[i])
        {
            foundBaroCptPull = false;
        }

        if (recvbuf[i] != AltRight[i])
        {
            foundAltRight = false;
        }
        if (recvbuf[i] != AltLeft[i])
        {
            foundAltLeft = false;
        }

        if (recvbuf[i] != HdgLeft[i])
        {
            foundHdgLeft = false;
        }
        if (recvbuf[i] != HdgRight[i])
        {
            foundHdgRight = false;
        }

        if (recvbuf[i] != SpdLeft[i])
        {
            foundSpdLeft = false;
        }
        if (recvbuf[i] != SpdRight[i])
        {
            foundSpdRight = false;
        }

        if (recvbuf[i] != VsLeft[i])
        {
            foundVsLeft = false;
        }
        if (recvbuf[i] != VsRight[i])
        {
            foundVsRight = false;
        }

        if (recvbuf[i] != BaroCptLeft[i])
        {
            foundBaroCptLeft = false;
        }
        if (recvbuf[i] != BaroCptRight[i])
        {
            foundBaroCptRight = false;
        }

        if (recvbuf[i] != BaroFoLeft[i])
        {
            foundBaroFoLeft = false;
        }
        if (recvbuf[i] != BaroFoRight[i])
        {
            foundBaroFoRight = false;
        }
    }

    if (foundNdModeLs == true)
    {
        NdModeSet = 0;
    }
    else if (foundNdModeVor == true)
    {
        NdModeSet = 1;
    }
    else if (foundNdModeNav == true)
    {
        NdModeSet = 2;
    }
    else if (foundNdModeArc == true)
    {
        NdModeSet = 3;
    }
    else if (foundNdModePlan == true)
    {
        NdModeSet = 4;
    }

    if (foundAp1Release == true)
    {
        Ap1Toggle = true;
    }

    if (foundAp2Release == true)
    {
        Ap2Toggle = true;
    }

    if (foundAthrRelease == true)
    {
        AthrToggle = true;
    }

    if (foundExpedRelease == true)
    {
        ExpedToggle = true;
    }

    if (foundApprRelease == true)
    {
        ApprToggle = true;
    }

    if (foundLocRelease == true)
    {
        LocToggle = true;
    }

    if (foundFdCptRelease == true /*|| foundFdFoRelease == true*/)
    {
        FdCptToggle = true;
    }

    if (foundLsCptRelease == true /*|| foundLsFoRelease == true*/)
    {
        LsCptToggle = true;
    }

    if (foundBaroInHg == true)
    {
        BaroCaptInHgSet = true;
    }
    else if (foundBaroHpa == true)
    {
        BaroCaptHpaSet = true;
    }
    else if (foundNdRange320 == true)
    {
        NdRangeSet = 5;// 320;
    }
    else if (foundNdRange160 == true)
    {
        NdRangeSet = 4; // 160;
    }
    else if (foundNdRange80 == true)
    {
        NdRangeSet = 3; // 80;
    }
    else if (foundNdRange40 == true)
    {
        NdRangeSet = 2; // 40;
    }
    else if (foundNdRange20 == true)
    {
        NdRangeSet = 1; // 20;
    }
    else if (foundNdRange10 == true)
    {
        NdRangeSet = 0; // 10;
    }
    else if (foundAltMode100 == true)
    {
        AltModeSet = 0;
    }
    else if (foundAltMode1000 == true)
    {
        AltModeSet = 1;
    }
    else if (foundBaroCptPress == true)
    {
        BaroCptPushSet = true;
    }
    else if (foundBaroCptPull == true)
    {
        BaroCptPullSet = true;
    }
    else if (foundSpdPull == true)
    {
        SpdPullSet = true;
    }
    else if (foundSpdPress == true)
    {
        SpdPressSet = true;
    }
    if (foundHdgPull == true)
    {
        HdgPullSet = true;
    }
    else if (foundHdgPress == true)
    {
        HdgPressSet = true;
    }
    if (foundAltPull == true)
    {
        AltPullSet = true;
    }
    else if (foundAltPress == true)
    {
        AltPressSet = true;
    }
    else if (foundVsPull == true)
    {
        VsPullSet = true;
    }
    else if (foundVsPush == true)
    {
        VsPushSet = true;
    }
    else if (foundBaroCptLeft == true /*|| foundBaroFoLeft == true*/)
    {
        BaroCptLeftSet = true;
    }
    else if (foundBaroCptRight == true /*|| foundBaroFoRight == true*/)
    {
        BaroCptRightSet = true;
    }
    else if (foundAltRight == true)
    {
        if (AltMode == 0)
        {
            AltSet = Alt + 100.0;
        }
        else
        {
			AltSet = Alt + 1000.0;
        }
    }
    else if (foundAltLeft == true)
    {
        if (AltMode == 0)
        {
            AltSet = Alt - 100.0;
        }
        else
        {
            AltSet = Alt - 1000.0;
        }
        
        if (AltSet <= 0.0)
        {
            AltSet = 0.0;
        }
    }
    else if (foundHdgRight == true)
    {
        HdgSet += 1.0;
        if (HdgSet >= 360.0)
        {
            HdgSet -= 360.0;
        }
    }
    else if (foundHdgLeft == true)
    {
        HdgSet -= 1.0;
        if (HdgSet < 0.0)
        {
            HdgSet += 360.0;
        }
    }
    else if (foundSpdLeft == true)
    {
        SpdSet -= 1.0;
        if (SpdSet < MIN_SPEED)
        {
            SpdSet = MIN_SPEED;
        }
    }
    else if (foundSpdRight == true)
    {
        SpdSet += 1.0;
        if (SpdSet > 999.0)
        {
            SpdSet = 999.0;
        }
    }
    else if (foundVsLeft == true)
    {
        VsLeftSet = true;
    }
    else if (foundVsRight == true)
    {
        VsRightSet = true;
    }
}
