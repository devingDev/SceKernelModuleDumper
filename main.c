#include <vitasdkkern.h>
#include <taihen.h>
#include <libk/stdio.h>
#include <libk/stdarg.h>
#include <libk/string.h>

#define BASE_DUMP_PATH "ur0:os0_modules/"

#define LOG(...) \
	do { \
		char buffer[256]; \
		snprintf(buffer, sizeof(buffer), ##__VA_ARGS__); \
		log_write(buffer, strlen(buffer)); \
} while (0)


void log_write(const char *buffer, size_t length){

	SceUID fd = ksceIoOpen("ur0:os0_modules/log.txt", SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 6);
	if (fd < 0)
		return;

	ksceIoWrite(fd, buffer, length);
	ksceIoClose(fd);
}



int LoadModule(char *load_path, char *load_module_name, int flags, char *load_module_write_path){


	int uid = 0;
	int ret = 0;
	int fd = 0;



	char write_path[0x100];
	char read_path[0x100];


	sprintf(write_path, "%s%s", load_module_write_path, load_module_name);
	sprintf(read_path, "%s%s", load_path, load_module_name);

	LOG("module path\n%s\n", read_path);


	SceUID fdc1 = ksceIoOpen(write_path, SCE_O_RDONLY, 0);

	if(fdc1 >= 0){

		LOG("Not module Dumped.\n\n");

		return ksceIoClose(fdc1);

	}




	ret = uid = ksceKernelLoadModule(read_path, flags, NULL);
	if (ret < 0) {
		ret = -1;
		goto skip;
	}


	SceKernelModuleInfo info = {0};
	info.size = sizeof(info);
	ret = ksceKernelGetModuleInfo(KERNEL_PID, uid, &info);
	if (ret < 0) {
		ret = -1;
		goto skip;
	}


	LOG("RAM Address : 0x%08X - 0x%08X\n\n", (unsigned int)info.segments[0].vaddr, (unsigned int)(info.segments[0].vaddr + info.segments[0].memsz));

	ret = fd = ksceIoOpen(write_path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
	if (ret < 0) {
		ret = -1;
		goto skip;
	}

	ksceIoWrite(fd, (char*)info.segments[0].vaddr, info.segments[0].memsz);



	ret = 0;

skip:
	if (fd > 0){
		ksceIoClose(fd);
	}

	if (uid > 0){
		ksceKernelUnloadModule(uid, 0, NULL);
	}

	return ret;

}


int SceGetErrorCode(int eroor_code){

	switch (eroor_code){

		case 0x90010002:LOG("It is a module which is not loaded.\n\n");break;

		default:LOG("unknown Error Code : [%08X]\n\n",eroor_code);break;

	}

	return eroor_code;

}


int DumpRamModule(char *module_name){

	LOG("%s\n", module_name);



	static char dump_path[0x100];

	sprintf(dump_path, "%sRAM_modules/%s", BASE_DUMP_PATH, module_name);


	SceUID fdc = ksceIoOpen(dump_path, SCE_O_RDONLY, 0);

	if(fdc >= 0){

		LOG("Not RAM module Dumped.\n\n");

		return ksceIoClose(fdc);
	}



	tai_module_info_t info;
	info.size = sizeof(tai_module_info_t);

	int tgmifk = taiGetModuleInfoForKernel(KERNEL_PID, module_name, &info);

	if (tgmifk < 0){
		return SceGetErrorCode(tgmifk);
	}






	SceKernelModuleInfo m_info = {0};
	m_info.size = sizeof(m_info);
	int kgmi = ksceKernelGetModuleInfo(KERNEL_PID, info.modid, &m_info);

	if (kgmi < 0){
		return SceGetErrorCode(kgmi);
	}

	LOG("NID : 0x%08X\n", (unsigned int)info.module_nid);

	LOG("RAM Address : 0x%08X - 0x%08X\n\n", (unsigned int)m_info.segments[0].vaddr, (unsigned int)(m_info.segments[0].vaddr + m_info.segments[0].memsz));


	int fd = ksceIoOpen(dump_path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);

	ksceIoWrite(fd, (char*)m_info.segments[0].vaddr, m_info.segments[0].memsz);

	ksceIoClose(fd);

	return 0;

}



int NotExistMkdir(char *path){

	int dfd = ksceIoDopen(path);
	if(dfd >= 0){

		ksceIoDclose(dfd);

	}else{

		ksceIoMkdir(path, 0777);

	}

	return 0;

}


void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(SceSize argc, const void *args) {

	NotExistMkdir("ur0:os0_modules/");
	NotExistMkdir("ur0:os0_modules/RAM_modules/");

	/*

	NotExistMkdir("ur0:os0_modules/kd/");
	NotExistMkdir("ur0:os0_modules/sm/");
	NotExistMkdir("ur0:os0_modules/ue/");

	*/


	LOG("Start ScePsp2BootConfig Dump\n\n");

	LoadModule("os0:", "psp2bootconfig.skprx", 0, "ur0:os0_modules/");
	LoadModule("os0:", "psp2config_vita.skprx", 0, "ur0:os0_modules/");
	LoadModule("os0:", "psp2config_dolce.skprx", 0, "ur0:os0_modules/");


	/* Test

	LoadModule("os0:", "PDEL_psp2bootconfig.skprx", 0, "ur0:os0_modules/");
	LoadModule("os0:", "PDEL_psp2config_vita.skprx", 0, "ur0:os0_modules/");
	LoadModule("os0:", "PDEL_psp2config_dolce.skprx", 0, "ur0:os0_modules/");

	LoadModule("os0:", "PTEL_psp2bootconfig.skprx", 0, "ur0:os0_modules/");
	LoadModule("os0:", "PTEL_psp2config_vita.skprx", 0, "ur0:os0_modules/");
	LoadModule("os0:", "PTEL_psp2config_dolce.skprx", 0, "ur0:os0_modules/");

	*/


	LOG("Success ScePsp2BootConfig Dump\n\n\n");




	//LoadModule("os0:kd/", "bsod.skprx", 0, "ur0:os0_modules/kd/");
	//LoadModule("os0:kd/", "syslibtrace.skprx", 0, "ur0:os0_modules/kd/");


	/* Boot Loop */
	//LoadModule("os0:kd/", "bootimage.skprx", 0, "ur0:os0_modules/kd/");

	LOG("Start RAM module Dumper\n\n");

	DumpRamModule("SceVshBridge");
	DumpRamModule("SceAppMgr");
	DumpRamModule("SceMarlinHci");
	DumpRamModule("SceKrm");
	DumpRamModule("SceSysmodule");
	DumpRamModule("ScePfsMgr");
	DumpRamModule("SceFios2Kernel");
	DumpRamModule("SceAVConfig");
	DumpRamModule("SceCoredump");
	DumpRamModule("SceCompat");
	DumpRamModule("SceGpuEs4");
	DumpRamModule("SceHid");
	DumpRamModule("SceNgs");
	DumpRamModule("SceAudioin");
	DumpRamModule("SceAvcodec");
	DumpRamModule("SceMgVideo");
	DumpRamModule("SceUsbstorMg");
	DumpRamModule("SceMagicGate");
	DumpRamModule("SceUsbPspcm");
	DumpRamModule("SceUsbstorVStorDriver");
	DumpRamModule("SceUsbstorDriver");
	DumpRamModule("SceBt");
	DumpRamModule("SceUsbEtherRtl");
	DumpRamModule("SceUsbEtherSmsc");
	DumpRamModule("SceWlanBt");
	DumpRamModule("SceGps");
	DumpRamModule("SceNetPs");
	DumpRamModule("SceUlobjMgr");
	DumpRamModule("SceNpDrm");
	DumpRamModule("SceError");
	DumpRamModule("SceSblMgKeyMgr");
	DumpRamModule("SceCodecEngineWrapper");
	DumpRamModule("SceRegistryMgr");
	DumpRamModule("SceSblUpdateMgr");
	DumpRamModule("SceSblPostSsMgr");
	DumpRamModule("SceMtpIfDriver");
	DumpRamModule("SceUsbMtp");
	DumpRamModule("SceUsbSerial");
	DumpRamModule("SceUsbServ");
	DumpRamModule("SceUdcd");
	DumpRamModule("SceUsbd");
	DumpRamModule("ScePower");
	DumpRamModule("SceHpremote");
	DumpRamModule("SceAudio");
	DumpRamModule("SceCodec");
	DumpRamModule("SceCtrl");
	DumpRamModule("SceIdStorage");
	DumpRamModule("SceClockgen");
	DumpRamModule("SceSysStateMgr");
	DumpRamModule("SceExfatfs");
	DumpRamModule("SceRtc");
	DumpRamModule("SceSdstor");
	DumpRamModule("SceSblGcAuthMgr");
	DumpRamModule("SceMsif");
	DumpRamModule("SceSdif");
	DumpRamModule("SceSblSsMgr");
	DumpRamModule("SceSblSsSmComm");
	DumpRamModule("SceDisplay");
	DumpRamModule("SceLcd");
	DumpRamModule("SceSyscon");
	DumpRamModule("SceLowio");
	DumpRamModule("SceKernelModulemgr");
	DumpRamModule("SceIofilemgr");
	DumpRamModule("SceProcessmgr");
	DumpRamModule("SceSblAuthMgr");
	DumpRamModule("SceSblSmschedProxy");
	DumpRamModule("SceKernelDmacMgr");
	DumpRamModule("SceKernelThreadMgr");
	DumpRamModule("SceSblACMgr");
	DumpRamModule("SceSystimer");
	DumpRamModule("SceKernelBusError");
	DumpRamModule("SceKernelIntrMgr");
	DumpRamModule("SceExcpmgr");
	DumpRamModule("SceSysmem");



	/* PS Vita, Kernel module */

	DumpRamModule("SceMotionDev");
	DumpRamModule("SceTouch");
	DumpRamModule("SceCamera");



	/* PS TV, Kernel module */

	DumpRamModule("SceMotionDevDummy");
	DumpRamModule("SceTouchDummy");
	DumpRamModule("SceCameraDummy");
	DumpRamModule("SceUsbAudio");
	DumpRamModule("SceHdmi");
	DumpRamModule("SceDs3");



	/* Error Test (LOL) */

	DumpRamModule("SceErrorTest");

	LOG("Success RAM module Dumper\n\n");



	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {

	return SCE_KERNEL_STOP_SUCCESS;
	
}