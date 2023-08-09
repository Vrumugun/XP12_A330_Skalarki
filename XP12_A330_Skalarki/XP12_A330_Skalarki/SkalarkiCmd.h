#pragma once

#include <map>
#include <functional>
#include <string_view>

class SkalarkiComm;

class SkalarkiCmd
{
public:
	SkalarkiCmd();
	~SkalarkiCmd();

	void Init(SkalarkiComm* pSkalariComm);
	void InitDatarefs(SkalarkiComm* pSkalariComm);
	void Update(SkalarkiComm* pSkalariComm);
	void ProcessPacket(const char* recvbuf, int length);

	int UpdateAlt(SkalarkiComm* pSkalariComm, int AltNew);
	int UpdateSpd(SkalarkiComm* pSkalariComm, int SpdNew);
	int UpdateHdg(SkalarkiComm* pSkalariComm, int HdgNew);
	int UpdateVs(SkalarkiComm* pSkalariComm, int VsNew);

	int UpdateBaroCpt(SkalarkiComm* pSkalariComm, float BaroNew);
	int SetBaroCptStd(SkalarkiComm* pSkalariComm);
	int UpdateBaroFo(SkalarkiComm* pSkalariComm, float BaroNew);
	int SetBaroFoStd(SkalarkiComm* pSkalariComm);

protected:
	std::map<size_t, std::function<void(void)>> CommandMap;
};
