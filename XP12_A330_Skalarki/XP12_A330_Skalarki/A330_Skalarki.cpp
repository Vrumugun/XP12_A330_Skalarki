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
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

using namespace std::literals;

#include "SkalarkiCmd.h"
#include "SkalarkiComm.h"

#define MSG_ADD_DATAREF 0x01000000           //  Add dataref to DRE message

bool DatarefsInitialized = false;

SkalarkiComm* gSkalarkiComm = nullptr;
SkalarkiCmd* gSkalarkiCmd = nullptr;

float A330_SkalarkiUpdate(float elapsedMe, float elapsedSim, int counter, void* refcon);

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

    gSkalarkiCmd = new SkalarkiCmd();

    gSkalarkiCmd->Init(gSkalarkiComm);

    XPLMRegisterFlightLoopCallback(A330_SkalarkiUpdate, 1, NULL);   // This FLCB will register our custom dataref in DRE
    
    return 1;
}

float A330_SkalarkiUpdate(float elapsedMe, float elapsedSim, int counter, void* refcon)
{
    if (DatarefsInitialized == false)
    {
        gSkalarkiCmd->InitDatarefs(gSkalarkiComm);
        DatarefsInitialized = true;
    }

	SkalarkiMessage rx_message = gSkalarkiComm->PopMessage();
    while(rx_message._len > 0)
    {
        gSkalarkiCmd->ProcessPacket(rx_message._msg, rx_message._len);
        rx_message = gSkalarkiComm->PopMessage();
    }
    
    gSkalarkiCmd->Update(gSkalarkiComm);

    return 0.1f;
}

PLUGIN_API void XPluginStop(void)
{
    XPLMUnregisterFlightLoopCallback(A330_SkalarkiUpdate, NULL);

    DatarefsInitialized = false;

    if (gSkalarkiCmd != nullptr)
    {
        delete gSkalarkiCmd;
    }
    
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
