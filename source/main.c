#include <psp2/appmgr.h>
#include <psp2/kernel/iofilemgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/modulemgr.h> 
#include <psp2/kernel/clib.h>
#include <psp2/shellsvc.h>
#include <psp2/sysmodule.h>
#include <psp2/pvf.h>

#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "fs.h"
#include "menu_displayfiles.h"
#include "textures.h"
#include "touch.h"
#include "utils.h"

#define CLIB_HEAP_SIZE 1 * 1024 * 1024			// |sceClibMspace| Used for libvita2d_sys
//#define LIBC_HEAP_SIZE 1 * 1024 * 1024		// |sceLibc|       Used for general memory stuff (file browser, small temp allocation etc.), this value is embedded in the module
int _newlib_heap_size_user = 1 * 1024 * 1024;	// |newlib|        Used for decoders

SceUID main_thread_uid;
SceUID event_flag_uid;

#ifdef DEBUG
SceAppMgrBudgetInfo budget_info;
#endif

int main(int argc, char *argv[]) {

#ifdef DEBUG
	sceClibMemset(&budget_info, 0, sizeof(SceAppMgrBudgetInfo));
	budget_info.size = 0x88;
	sceAppMgrGetBudgetInfo(&budget_info);
	sceClibPrintf("----- EMPA-A INITIAL BUDGET -----");
	sceClibPrintf("LPDDR2: %d MB\n", budget_info.freeLPDDR2 / 1024 / 1024);
#endif

	void* clibm_base, *mspace;
	SceUID clib_heap = sceKernelAllocMemBlock("ClibHeap", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, CLIB_HEAP_SIZE, NULL);
	sceKernelGetMemBlockBase(clib_heap, &clibm_base);
	mspace = sceClibMspaceCreate(clibm_base, CLIB_HEAP_SIZE);

	main_thread_uid = sceKernelGetThreadId();
	sceKernelChangeThreadPriority(main_thread_uid, 160);

	event_flag_uid = sceKernelCreateEventFlag("ElevenMPVA_thread_event_flag", SCE_KERNEL_ATTR_MULTI, FLAG_ELEVENMPVA_IS_FG | FLAG_ELEVENMPVA_IS_DECODER_USED, NULL);

	vita2d_clib_pass_mspace(mspace);
	vita2d_init_with_msaa_and_memsize(0, 4 * 1024, 128 * 1024, 64 * 1024, 4 * 1024, 0);
	vita2d_set_vblank_wait(0);

	vita2d_system_pvf_config config;
	sceClibMemset(&config, 0, sizeof(vita2d_system_pvf_config));
	config.language = SCE_PVF_LANGUAGE_CJK;
	config.family = SCE_PVF_FAMILY_SANSERIF;
	config.style = SCE_PVF_STYLE_REGULAR;
	font = vita2d_load_system_pvf(1, &config, 13, 13);

	Textures_Load();

	Utils_InitAppUtil();

	Config_Load();
	Config_GetLastDirectory();

	SCE_CTRL_ENTER = Utils_GetEnterButton();
	SCE_CTRL_CANCEL = Utils_GetCancelButton();

	Touch_Init();

	sceShellUtilInitEvents(0);
	Utils_InitPowerTick();

	Menu_DisplayFiles();

	return 0;
}
