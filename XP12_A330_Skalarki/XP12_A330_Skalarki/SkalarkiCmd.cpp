#include "pch.h"
#include "SkalarkiCmd.h"
#include "SkalarkiComm.h"
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
#include "SkalarkiCmdList.h"

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

XPLMDataRef gFdCptDataref = NULL;
XPLMCommandRef gFdCptToggleDataref = NULL;

XPLMDataRef gLsCptDataref = NULL;
XPLMCommandRef gLsCptToggleDataref = NULL;

XPLMDataRef gFdFoDataref = NULL;
XPLMCommandRef gFdFoToggleDataref = NULL;

XPLMDataRef gLsFoDataref = NULL;
XPLMCommandRef gLsFoToggleDataref = NULL;

XPLMDataRef gLocDataref = NULL;
XPLMCommandRef gLocToggleDataref = NULL;

XPLMDataRef gAltDataref = NULL;
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
XPLMDataRef gBaroFo = NULL;
XPLMCommandRef gBaroFoPull = NULL;
XPLMCommandRef gBaroFoPush = NULL;
XPLMCommandRef gBaroFoUp = NULL;
XPLMCommandRef gBaroFoDown = NULL;
XPLMCommandRef gBaroFoInHg = NULL;
XPLMCommandRef gBaroFoHpa = NULL;
//XPLMDataRef gBaroFoHpaDisplay = NULL;
//XPLMDataRef gBaroFoInHgDisplay = NULL;

XPLMCommandRef gVsPull = NULL;
XPLMCommandRef gVsPush = NULL;
XPLMCommandRef gVsUp = NULL;
XPLMCommandRef gVsDown = NULL;

XPLMCommandRef clearMasterWarningCommand = NULL;
XPLMCommandRef autoPilotServosToggleCmd = NULL;
XPLMCommandRef autoPilotFlightDirDownCmd = NULL;

float Spd = 250.0;
float SpdSet = 250.0;
float Alt = 0.0;
float AltSet = 0.0;
float Hdg = 0.0;
float HdgSet = 0.0;
int Vs = 0;
int VsSet = 0;

float BaroCpt = 29.92;
int BaroCptStd = 1;
int BaroCptStdSet = 1;
int BaroCptMode = 0; // 0 = inHg, 1 = hPa
int BaroCptModeSet = 0;
int BaroCptDisplay = 0;

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

bool SpdMachMode = false;
bool BaroQnhMode = false;

bool SpdManaged = true;
bool HdgManaged = true;
bool AltManaged = true;
bool VsManaged = true;
int AltMode = 0;
int NdRangeSet = 2; // 40 (default value of A330)
int NdRange = 2; // 40 (default value of A330)
int NdModeSet = 0; // LS (default value of A330)
int NdMode = 0; // LS (default value of A330)

int LsState = 0;
int FdState = 0;

int LsState2 = 0;
int FdState2 = 0;

int LocState = 0;

int ApState1 = 0;
int ApState2 = 0;
int AthrState = 0;
int ApprState = 0;

int MasterCautionState = 0;
int MasterWarningState = 0;

int packet_counter = 0;

const float MIN_SPEED = 125.0;
const float IN_HG_TO_HPA = 33.863889532610884;

SkalarkiCmd::SkalarkiCmd()
{
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(FdCptRelease, sizeof(FdCptRelease) - 1)), []() { XPLMCommandOnce(gFdCptToggleDataref); });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(LsCptRelease, sizeof(LsCptRelease) - 1)), []() { XPLMCommandOnce(gLsCptToggleDataref); });

    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(FdFoRelease, sizeof(FdFoRelease) - 1)), []() { XPLMCommandOnce(gFdFoToggleDataref); });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(LsFoRelease, sizeof(LsFoRelease) - 1)), []() { XPLMCommandOnce(gLsFoToggleDataref); });

    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(Ap1Release, sizeof(Ap1Release) - 1)), []() { XPLMCommandOnce(ap1_push); });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(Ap2Release, sizeof(Ap2Release) - 1)), []() { XPLMCommandOnce(ap2_push); });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(AthrRelease, sizeof(AthrRelease) - 1)), []() { XPLMCommandOnce(athr_push); });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(ApprRelease, sizeof(ApprRelease) - 1)), []() { XPLMCommandOnce(appr_push); });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(LocRelease, sizeof(LocRelease) - 1)), []() { XPLMCommandOnce(gLocToggleDataref); });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(ExpedRelease, sizeof(ExpedRelease) - 1)), []() { /*XPLMCommandOnce(gLocToggleDataref);*/ });

    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(SpdPress, sizeof(SpdPress) - 1)), []() { XPLMCommandOnce(gSpdPressDataref); });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(SpdPull, sizeof(SpdPull) - 1)), []() { XPLMCommandOnce(gSpdPullDataref); });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(SpdLeft, sizeof(SpdLeft) - 1)), []() { SpdSet -= 1.0; if (SpdSet < MIN_SPEED) { SpdSet = MIN_SPEED; } });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(SpdRight, sizeof(SpdRight) - 1)), []() { SpdSet += 1.0; if (SpdSet > 999.0) { SpdSet = 999.0; } });

    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(HdgPress, sizeof(HdgPress) - 1)), []() { XPLMCommandOnce(gHdgPressDataref); });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(HdgPull, sizeof(HdgPull) - 1)), []() { XPLMCommandOnce(gHdgPullDataref); });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(HdgLeft, sizeof(HdgLeft) - 1)), []() { HdgSet -= 1.0; if (HdgSet < 0.0) { HdgSet += 360.0f; } });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(HdgRight, sizeof(HdgRight) - 1)), []() { HdgSet += 1.0; if (HdgSet > 360.0) { HdgSet -= 360.0f; } });

    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(AltMode100, sizeof(AltMode100) - 1)), []() { XPLMCommandOnce(gAltModeLeftDataref); });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(AltMode1000, sizeof(AltMode1000) - 1)), []() { XPLMCommandOnce(gAltModeRightDataref); });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(AltPress, sizeof(AltPress) - 1)), []() { XPLMCommandOnce(gAltPressDataref); });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(AltPull, sizeof(AltPull) - 1)), []() { XPLMCommandOnce(gAltPullDataref); });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(AltLeft, sizeof(AltLeft) - 1)), []() {
        int AltMode = XPLMGetDatai(gAltModeDataref);
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
    });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(AltRight, sizeof(AltRight) - 1)), []() {
        int AltMode = XPLMGetDatai(gAltModeDataref);
        if (AltMode == 0)
        {
            AltSet = Alt + 100.0;
        }
        else
        {
            AltSet = Alt + 1000.0;
        }
    });

    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(BaroCptInHg, sizeof(BaroCptInHg) - 1)), []() { XPLMCommandOnce(gBaroCptInHg); });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(BaroCptHPa, sizeof(BaroCptHPa) - 1)), []() { XPLMCommandOnce(gBaroCptHpa); });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(BaroCptPull, sizeof(BaroCptPull) - 1)), []() { XPLMCommandOnce(gBaroCptPull); });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(BaroCptPress, sizeof(BaroCptPress) - 1)), []() { XPLMCommandOnce(gBaroCptPush); });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(BaroCptLeft, sizeof(BaroCptLeft) - 1)), []() { XPLMCommandOnce(gBaroCptDown); });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(BaroCptRight, sizeof(BaroCptRight) - 1)), []() {XPLMCommandOnce(gBaroCptUp); });

    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(BaroFoInHg, sizeof(BaroFoInHg) - 1)), []() { XPLMCommandOnce(gBaroFoInHg); });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(BaroFoHpa, sizeof(BaroFoHpa) - 1)), []() { XPLMCommandOnce(gBaroFoHpa); });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(BaroFoPull, sizeof(BaroFoPull) - 1)), []() { XPLMCommandOnce(gBaroFoPull); });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(BaroFoPress, sizeof(BaroFoPress) - 1)), []() { XPLMCommandOnce(gBaroFoPush); });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(BaroFoLeft, sizeof(BaroFoLeft) - 1)), []() { XPLMCommandOnce(gBaroFoDown); });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(BaroFoRight, sizeof(BaroFoRight) - 1)), []() {XPLMCommandOnce(gBaroFoUp); });

    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(NdModeLs, sizeof(NdModeLs) - 1)), []() { NdModeSet = 0; });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(NdModeVor, sizeof(NdModeVor) - 1)), []() { NdModeSet = 1; });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(NdModeNav, sizeof(NdModeNav) - 1)), []() { NdModeSet = 2; });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(NdModeArc, sizeof(NdModeArc) - 1)), []() { NdModeSet = 3; });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(NdModePlan, sizeof(NdModePlan) - 1)), []() { NdModeSet = 4; });

    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(NdRange10, sizeof(NdRange10) - 1)), []() { NdRangeSet = 0; });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(NdRange20, sizeof(NdRange20) - 1)), []() { NdRangeSet = 1; });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(NdRange40, sizeof(NdRange40) - 1)), []() { NdRangeSet = 2; });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(NdRange80, sizeof(NdRange80) - 1)), []() { NdRangeSet = 3; });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(NdRange160, sizeof(NdRange160) - 1)), []() { NdRangeSet = 4; });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(NdRange320, sizeof(NdRange320) - 1)), []() { NdRangeSet = 5; });

    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(VsPullRelease, sizeof(VsPullRelease) - 1)), []() { XPLMCommandOnce(gVsPull); });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(VsPushRelease, sizeof(VsPushRelease) - 1)), []() { XPLMCommandOnce(gVsPush); });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(VsLeft, sizeof(VsLeft) - 1)), []() { XPLMCommandOnce(gVsDown); });
    CommandMap.emplace(std::hash<std::string_view>{}(std::string_view(VsRight, sizeof(VsRight) - 1)), []() { XPLMCommandOnce(gVsUp); });
}

SkalarkiCmd::~SkalarkiCmd()
{
	
}

void SkalarkiCmd::Init(SkalarkiComm* pSkalarkiComm)
{
    pSkalarkiComm->Init(initBuf, intBufLen, nullptr);

    pSkalarkiComm->PushMessage(sendbufBacklightOn, sendbufLenBacklightOn);
    pSkalarkiComm->PushMessage(AltLvlChngOn, AltLvlChngOnLen);
    pSkalarkiComm->PushMessage(HdgVsOn, HdgVsOnLen);
    pSkalarkiComm->PushMessage(SpdOn, SpdOnLen);
    pSkalarkiComm->PushMessage(LatOn, LatOnLen);

    UpdateHdg(pSkalarkiComm, Hdg);
    UpdateAlt(pSkalarkiComm, Alt);
    UpdateSpd(pSkalarkiComm, Spd);
    UpdateVs(pSkalarkiComm, Vs);
    UpdateBaroCpt(pSkalarkiComm, BaroCpt);
    UpdateBaroFo(pSkalarkiComm, BaroFo);
}

void SkalarkiCmd::InitDatarefs(SkalarkiComm* pSkalariComm)
{
    gFdCptDataref = XPLMFindDataRef("laminar/A333/annun/capt_flight_director_on");
    gFdCptToggleDataref = XPLMFindCommand("sim/autopilot/fdir_toggle"); // sim/autopilot/fdir2_toggle for FO
    gLsCptDataref = XPLMFindDataRef("laminar/A333/annun/captain_ls_bars_on");
    gLsCptToggleDataref = XPLMFindCommand("laminar/A333/buttons/capt_ils_bars_push"); // laminar/A333/buttons/fo_ils_bars_push
    
    gFdFoDataref = XPLMFindDataRef("laminar/A333/annun/fo_flight_director_on");
    gFdFoToggleDataref = XPLMFindCommand("sim/autopilot/fdir2_toggle"); // sim/autopilot/fdir2_toggle for FO
    gLsFoDataref = XPLMFindDataRef("laminar/A333/annun/fo_ls_bars_on");
    gLsFoToggleDataref = XPLMFindCommand("laminar/A333/buttons/fo_ils_bars_push"); // laminar/A333/buttons/fo_ils_bars_push
    
    gLocDataref = XPLMFindDataRef("laminar/A333/annun/autopilot/loc_mode");
    gLocToggleDataref = XPLMFindCommand("sim/autopilot/NAV"); // or try sim/autopilot/back_course

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

    gAltDataref = XPLMFindDataRef("sim/cockpit/autopilot/altitude");
    AltSet = XPLMGetDataf(gAltDataref);

    gSpdDataref = XPLMFindDataRef("sim/cockpit/autopilot/airspeed");
    SpdSet = XPLMGetDataf(gSpdDataref);

    gHdgDataref = XPLMFindDataRef("sim/cockpit/autopilot/heading_mag");
    HdgSet = XPLMGetDataf(gHdgDataref) + 0.5f;

    gVsDataref = XPLMFindDataRef("sim/cockpit/autopilot/vertical_velocity");
    gAltPullDataref = XPLMFindCommand("laminar/A333/autopilot/altitude_knob_pull");
    gAltPressDataref = XPLMFindCommand("laminar/A333/autopilot/altitude_knob_push");
    gSpdPullDataref = XPLMFindCommand("laminar/A333/autopilot/speed_knob_pull");
    gSpdPressDataref = XPLMFindCommand("laminar/A333/autopilot/speed_knob_push");
    gHdgPullDataref = XPLMFindCommand("laminar/A333/autopilot/heading_knob_pull");
    gHdgPressDataref = XPLMFindCommand("laminar/A333/autopilot/heading_knob_push");

    gAltModeDataref = XPLMFindDataRef("laminar/A333/autopilot/alt_step_knob_pos");
    AltMode = XPLMGetDatai(gAltModeDataref);

    gAltModeLeftDataref = XPLMFindCommand("laminar/A333/autopilot/alt_step_left");
    gAltModeRightDataref = XPLMFindCommand("laminar/A333/autopilot/alt_step_right");
    gNdModeDataref = XPLMFindDataRef("sim/cockpit2/EFIS/map_mode");
    gNdModeLeftDataref = XPLMFindCommand("laminar/A333/knobs/capt_EFIS_knob_left");
    gNdModeRightDataref = XPLMFindCommand("laminar/A333/knobs/capt_EFIS_knob_right");
    gNdRangeDataref = XPLMFindDataRef("sim/cockpit2/EFIS/map_range");

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
            pSkalariComm->PushMessage(SpdDotOn, SpdDotOnLen);
        }
        else
        {
            SpdManaged = false;
            pSkalariComm->PushMessage(SpdDotOff, SpdDotOffLen);
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

    ap1 = XPLMFindDataRef("laminar/A333/annun/autopilot/ap1_mode");
    ap1_push = XPLMFindCommand("sim/autopilot/servos_toggle");
    ap2 = XPLMFindDataRef("laminar/A333/annun/autopilot/ap2_mode");
    ap2_push = XPLMFindCommand("sim/autopilot/servos2_toggle");
    athr_mode = XPLMFindDataRef("laminar/A333/annun/autopilot/a_thr_mode");
    athr_push = XPLMFindCommand("laminar/A333/autopilot/a_thr_toggle");
    appr = XPLMFindDataRef("laminar/A333/annun/autopilot/appr_mode");
    appr_push = XPLMFindCommand("sim/autopilot/approach");
    master_caution = XPLMFindDataRef("laminar/A333/annun/master_caution");
    master_warning = XPLMFindDataRef("laminar/A333/annun/master_warning");

    gBaroCptMode = XPLMFindDataRef("laminar/A333/barometer/capt_inHg_hPa_pos");
    BaroCptMode = XPLMGetDatad(gBaroCptMode);
    gBaroCpt = XPLMFindDataRef("sim/cockpit/misc/barometer_setting");
    BaroCpt = XPLMGetDataf(gBaroCpt);
    gBaroCptStd = XPLMFindDataRef("laminar/A333/barometer/capt_mode");
    BaroCptStdSet = XPLMGetDatad(gBaroCptStd);
    gBaroCptPull = XPLMFindCommand("laminar/A333/pull/baro/capt_std");
    gBaroCptPush = XPLMFindCommand("laminar/A333/push/baro/capt_std");
    gBaroCptUp = XPLMFindCommand("sim/instruments/barometer_up");
    gBaroCptDown = XPLMFindCommand("sim/instruments/barometer_down");
    gBaroCptInHg = XPLMFindCommand("laminar/A333/knob/baro/capt_inHg");
    gBaroCptHpa = XPLMFindCommand("laminar/A333/knob/baro/capt_hPa");
    
    gBaroFoMode = XPLMFindDataRef("laminar/A333/barometer/fo_inHg_hPa_pos");
    BaroFoMode = XPLMGetDatad(gBaroFoMode);
    gBaroFo = XPLMFindDataRef("sim/cockpit/misc/barometer_setting2");
    BaroFo = XPLMGetDataf(gBaroFo);
    gBaroFoStd = XPLMFindDataRef("laminar/A333/barometer/fo_mode");
    BaroFoStdSet = XPLMGetDatad(gBaroFoStd);
    gBaroFoPull = XPLMFindCommand("laminar/A333/pull/baro/fo_std");
    gBaroFoPush = XPLMFindCommand("laminar/A333/push/baro/fo_std");
    gBaroFoUp = XPLMFindCommand("sim/instruments/barometer_copilot_up");
    gBaroFoDown = XPLMFindCommand("sim/instruments/barometer_copilot_down");
    gBaroFoInHg = XPLMFindCommand("laminar/A333/knob/baro/fo_inHg");
    gBaroFoHpa = XPLMFindCommand("laminar/A333/knob/baro/fo_hPa");

    gVsPull = XPLMFindCommand("laminar/A333/autopilot/vertical_knob_pull");
    // or try "sim/autopilot/vertical_speed" ?
    gVsPush = XPLMFindCommand("laminar/A333/autopilot/vertical_knob_push");
    gVsUp = XPLMFindCommand("sim/autopilot/vertical_speed_up");
    gVsDown = XPLMFindCommand("sim/autopilot/vertical_speed_down");
}

void SkalarkiCmd::ProcessPacket(const char* recvbuf, int length)
{
    packet_counter++;

    std::string_view rx = std::string_view(recvbuf, length);

    size_t hash = std::hash<std::string_view>{}(rx);

    if (CommandMap.find(hash) != CommandMap.end())
    {
        CommandMap[hash]();
    }
    else
    {
        XPLMDebugString("A330_Skalarki: Error, unknown command.\n");
    }
}

void SkalarkiCmd::Update(SkalarkiComm* pSkalarkiComm)
{
    int vsnew = XPLMGetDataf(gVsDataref);
    if (vsnew != Vs)
    {
        Vs = vsnew;
        if (VsManaged == false)
        {
            UpdateVs(pSkalarkiComm, Vs);
        }
    }

    int VsStateNew = XPLMGetDatad(gVsManagedDataref);
    if (VsStateNew == 0)
    {
        if (VsManaged == false)
        {
            VsManaged = true;
            UpdateVs(pSkalarkiComm, Vs);
        }
    }
    else
    {
        if (VsManaged == true)
        {
            VsManaged = false;
            UpdateVs(pSkalarkiComm, Vs);
        }
    }

    int AltManagedNew = XPLMGetDatai(gAltManagedDataref);
    if (AltManagedNew == 1)
    {
        if (AltManaged == false)
        {
            AltManaged = true;
            pSkalarkiComm->PushMessage(AltDotOn, AltDotOnLen);
        }
    }
    else
    {
        if (AltManaged == true)
        {
            AltManaged = false;
            pSkalarkiComm->PushMessage(AltDotOff, AltDotOffLen);
        }
    }

    int AthrStateNew = (XPLMGetDatad(athr_mode) > 0.001) ? 1 : 0;
    if (AthrState != AthrStateNew)
    {
        AthrState = AthrStateNew;
        if (AthrState == 1)
        {
            pSkalarkiComm->PushMessage(AthrOn, AthrOnLen);
        }
        else
        {
            pSkalarkiComm->PushMessage(AthrOff, AthrOffLen);
        }
    }

    int ApprStateNew = (XPLMGetDatad(appr) > 0.001) ? 1 : 0;
    if (ApprState != ApprStateNew)
    {
        ApprState = ApprStateNew;
        if (ApprState == 1)
        {
            pSkalarkiComm->PushMessage(ApprOn, ApprOnLen);
        }
        else
        {
            pSkalarkiComm->PushMessage(ApprOff, ApprOffLen);
        }
    }

    int ApStateNew1 = (XPLMGetDatad(ap1) > 0.001) ? 1 : 0;
    if (ApState1 != ApStateNew1)
    {
        ApState1 = ApStateNew1;
        if (ApState1 == 1)
        {
            pSkalarkiComm->PushMessage(Ap1On, Ap1OnLen);
        }
        else
        {
            pSkalarkiComm->PushMessage(Ap1Off, Ap1OffLen);
        }
    }

    int ApStateNew2 = (XPLMGetDatad(ap2) > 0.001) ? 1 : 0;
    if (ApState2 != ApStateNew2)
    {
        ApState2 = ApStateNew2;
        if (ApState2 == 1)
        {
            pSkalarkiComm->PushMessage(Ap2On, Ap2OnLen);
        }
        else
        {
            pSkalarkiComm->PushMessage(Ap2Off, Ap2OffLen);
        }
    }

    int MasterCautionStateNew = (XPLMGetDatad(master_caution) > 0.001) ? 1 : 0;
    if (MasterCautionState != MasterCautionStateNew)
    {
        MasterCautionState = MasterCautionStateNew;
        if (MasterCautionState == 1)
        {
            pSkalarkiComm->PushMessage(MasterCautionOn, MasterCautionOnLen);
        }
        else
        {
            pSkalarkiComm->PushMessage(MasterCautionOff, MasterCautionOffLen);
        }
    }

    int MasterWarningStateNew = (XPLMGetDatad(master_warning) > 0.001) ? 1 : 0;
    if (MasterWarningState != MasterWarningStateNew)
    {
        MasterWarningState = MasterWarningStateNew;
        if (MasterWarningState == 1)
        {
            pSkalarkiComm->PushMessage(MasterWarningOn, MasterCautionOnLen);
        }
        else
        {
            pSkalarkiComm->PushMessage(MasterWarningOff, MasterWarningOffLen);
        }
    }

    int i = XPLMGetDatad(gHdgManagedDataref);
    if (i == 0)
    {
        if (HdgManaged == false)
        {
            HdgManaged = true;
            UpdateHdg(pSkalarkiComm, Hdg);
            pSkalarkiComm->PushMessage(HdgDotOn, HdgDotOnLen);
        }
    }
    else
    {
        if (HdgManaged == true)
        {
            HdgManaged = false;
            UpdateHdg(pSkalarkiComm, Hdg);
            pSkalarkiComm->PushMessage(HdgDotOff, HdgDotOffLen);
        }
    }

    i = XPLMGetDatai(gSpdManagedDataref);
    if (i == 0)
    {
        if (SpdManaged == false)
        {
            SpdManaged = true;
            UpdateSpd(pSkalarkiComm, Spd);
            pSkalarkiComm->PushMessage(SpdDotOn, SpdDotOnLen);
        }
    }
    else
    {
        if (SpdManaged == true)
        {
            SpdManaged = false;
            SpdSet = XPLMGetDataf(gSpdDataref);
            Spd = SpdSet;
            UpdateSpd(pSkalarkiComm, Spd);
            pSkalarkiComm->PushMessage(SpdDotOff, SpdDotOffLen);
        }
    }

    int Ls = (XPLMGetDatad(gLsCptDataref) > 0.001) ? 1 : 0; //XPLMGetDatai(gLsDataref);
    if (Ls != LsState)
    {
        LsState = Ls;
        if (LsState == 1)
        {
            pSkalarkiComm->PushMessage(LsCptOn, LsCptOnLen);
        }
        else
        {
            pSkalarkiComm->PushMessage(LsCptOff, LsCptOffLen);
        }
    }

    int Fd = (XPLMGetDatad(gFdCptDataref) > 0.001) ? 1 : 0; //XPLMGetDatai(gFdDataref);
    if (Fd != FdState)
    {
        FdState = Fd;
        if (FdState == 1)
        {
            pSkalarkiComm->PushMessage(FdCptOn, FdCptOnLen);
        }
        else
        {
            pSkalarkiComm->PushMessage(FdCptOff, FdCptOffLen);
        }
    }

    int Ls2 = (XPLMGetDatad(gLsFoDataref) > 0.001) ? 1 : 0; //XPLMGetDatai(gLsDataref);
    if (Ls2 != LsState2)
    {
        LsState2 = Ls2;
        if (LsState2 == 1)
        {
            pSkalarkiComm->PushMessage(LsFoOn, LsFoOnLen);
        }
        else
        {
            pSkalarkiComm->PushMessage(LsFoOff, LsFoOffLen);
        }
    }

    int Fd2 = (XPLMGetDatad(gFdFoDataref) > 0.001) ? 1 : 0; //XPLMGetDatai(gFdDataref);
    if (Fd2 != FdState2)
    {
        FdState2 = Fd2;
        if (FdState2 == 1)
        {
            pSkalarkiComm->PushMessage(FdFoOn, FdFoOnLen);
        }
        else
        {
            pSkalarkiComm->PushMessage(FdFoOff, FdFoOffLen);
        }
    }

    int loc = (XPLMGetDatad(gLocDataref) > 0.001) ? 1 : 0; //XPLMGetDatai(gFdDataref);
    if (loc != LocState)
    {
        LocState = loc;
        if (LocState == 1)
        {
            pSkalarkiComm->PushMessage(LocOn, LocOnLen);
        }
        else
        {
            pSkalarkiComm->PushMessage(LocOff, LocOffLen);
        }
    }

    if (NdMode != NdModeSet)
    {
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
    }

    if (NdRange != NdRangeSet)
    {
        NdRange = NdRangeSet;
        XPLMSetDatai(gNdRangeDataref, NdRange);
    }

    if (HdgSet != Hdg)
    {
        Hdg = HdgSet;
        UpdateHdg(pSkalarkiComm, (int)(Hdg + 0.5));
        XPLMSetDataf(gHdgDataref, Hdg);
    }

    if (SpdSet != Spd)
    {
        Spd = SpdSet;
        UpdateSpd(pSkalarkiComm, (int)Spd);
        XPLMSetDataf(gSpdDataref, Spd);
    }

    if (Alt != AltSet)
    {
        Alt = AltSet;
        UpdateAlt(pSkalarkiComm, Alt);

        XPLMSetDataf(gAltDataref, Alt);
    }
    
    /*
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
    */
    BaroCptModeSet = XPLMGetDatad(gBaroCptMode);
    if (BaroCptMode != BaroCptModeSet)
    {
        BaroCptMode = BaroCptModeSet;
        BaroCptDisplay = 0;
        UpdateBaroCptDisplay = true;
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
            SetBaroCptStd(pSkalarkiComm);
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
                UpdateBaroCpt(pSkalarkiComm, BaroCptDisplay / 100.0);
            }
        }
        else
        {
            float baro = (XPLMGetDataf(gBaroCpt) * IN_HG_TO_HPA) + 0.5;
            int BaroDisplayNew = baro;
            if (BaroCptDisplay != BaroDisplayNew)
            {
                BaroCptDisplay = BaroDisplayNew;
                UpdateBaroCpt(pSkalarkiComm, BaroCptDisplay);
            }
        }
    }

    BaroFoModeSet = XPLMGetDatad(gBaroFoMode);
    if (BaroFoMode != BaroFoModeSet)
    {
        BaroFoMode = BaroFoModeSet;
        BaroFoDisplay = 0;
        UpdateBaroFoDisplay = true;
    }

    if (UpdateBaroFoValue == true)
    {
        UpdateBaroFoValue = false;
        UpdateBaroFoDisplay = true;
    }

    BaroFoStdSet = XPLMGetDatad(gBaroFoStd);
    if (BaroFoStd != BaroFoStdSet)
    {
        BaroFoStd = BaroFoStdSet;
        if (BaroFoStd == 1)
        {
            SetBaroFoStd(pSkalarkiComm);
        }
        else
        {
            BaroFoDisplay = 0;
        }
    }

    if (BaroFoStd == 0)
    {
        if (BaroFoMode == 0)
        {
            float baro = XPLMGetDataf(gBaroFo) + 0.005;
            int BaroDisplayNew = baro * 100;
            if (BaroFoDisplay != BaroDisplayNew)
            {
                BaroFoDisplay = BaroDisplayNew;
                UpdateBaroFo(pSkalarkiComm, BaroFoDisplay / 100.0);
            }
        }
        else
        {
            float baro = (XPLMGetDataf(gBaroFo) * IN_HG_TO_HPA) + 0.5;
            int BaroDisplayNew = baro;
            if (BaroFoDisplay != BaroDisplayNew)
            {
                BaroFoDisplay = BaroDisplayNew;
                UpdateBaroFo(pSkalarkiComm, BaroFoDisplay);
            }
        }
    }
}

int SkalarkiCmd::SetBaroCptStd(SkalarkiComm* pSkalariComm)
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

    pSkalariComm->PushMessage(sendbufBaroLeft, sendBufBaroLen);

    return 0;
}

int SkalarkiCmd::SetBaroFoStd(SkalarkiComm* pSkalariComm)
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

    pSkalariComm->PushMessage(sendBufBaroRight, sendBufBaroLen);

    return 0;
}

int SkalarkiCmd::UpdateBaroCpt(SkalarkiComm* pSkalariComm, float BaroNew)
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

        pSkalariComm->PushMessage(sendbufBaroLeft, sendBufBaroLen);
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

        pSkalariComm->PushMessage(sendbufBaroLeft, sendBufBaroLen);
    }

    return 0;
}

int SkalarkiCmd::UpdateBaroFo(SkalarkiComm* pSkalariComm, float BaroNew)
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

        pSkalariComm->PushMessage(sendBufBaroRight, sendBufBaroLen);
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

        pSkalariComm->PushMessage(sendBufBaroRight, sendBufBaroLen);
    }

    return 0;
}

int SkalarkiCmd::UpdateVs(SkalarkiComm* pSkalariComm, int VsNew)
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

    pSkalariComm->PushMessage(sendbufVs, sendBufVsLen);

    return 0;
}

int SkalarkiCmd::UpdateSpd(SkalarkiComm* pSkalariComm, int SpdNew)
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

    pSkalariComm->PushMessage(sendbufSpd, sendBufSpdLen);

    return 0;
}


int SkalarkiCmd::UpdateHdg(SkalarkiComm* pSkalariComm, int HdgNew)
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

    pSkalariComm->PushMessage(sendbufHdg, sendBufHdgLen);

    return 0;
}

int SkalarkiCmd::UpdateAlt(SkalarkiComm* pSkalariComm, int AltNew)
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

    pSkalariComm->PushMessage(sendbufAlt, sendbufLenAlt);

    return 0;
}
