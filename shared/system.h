/*
Texture Viewer, the game developer's image viewer (site: imagetools.itch.io)
Copyright (C) 2010-2024 Ilya Lyutin (lyutinilya551@gmail.com)
 
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the
use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it 
freely, subject to the following restrictions:
1. The origin of this software must not be misrepresented; you must not claim
   that you wrote the original software. If you use this software in a
   product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifdef __cplusplus
extern "C" SystemFuncs_t* g_sys;
#else
extern SystemFuncs_t* g_sys;
#endif

#define SYS_OpenFile (g_sys->pfnOpenFile)
#define SYS_CloseFile (g_sys->pfnCloseFile)
#define SYS_GetFileSize (g_sys->pfnGetFileSize)
#define SYS_SetFilePointer (g_sys->pfnSetFilePointer)
#define SYS_GetFilePointer (g_sys->pfnGetFilePointer)
#define SYS_ReadFile (g_sys->pfnReadFile)

#define SYS_malloc (g_sys->pfnAllocMemory)
#define SYS_free (g_sys->pfnFreeMemory)

#define SYS_AllocStreamMemory (g_sys->pfnAllocStreamMemory)
#define SYS_FreeStreamMemory (g_sys->pfnFreeStreamMemory)
#define SYS_ReadStreamMemory (g_sys->pfnReadStreamMemory)
#define SYS_WriteStreamMemory (g_sys->pfnWriteStreamMemory)

#define SYS_CallThread (g_sys->pfnCallThread)
#define SYS_WaitForThreadsToComplete (g_sys->pfnWaitForThreadsToComplete)

#define SYS_FindKey (g_sys->pfnFindKey)
#define SYS_AddKey (g_sys->pfnAddKey)
#define SYS_GetKeyValue (g_sys->pfnGetKeyValue)
#define SYS_SetKeyValue (g_sys->pfnSetKeyValue)

#ifdef __cplusplus
extern "C" {
#endif

HF SYS_wfopen(const wchar_t *filename, const wchar_t *mode);
int SYS_fclose(HF stream);
int SYS_fseek(HF stream, long offset, int whence);
long SYS_ftell(HF stream);
size_t SYS_fread(void *ptr, size_t size, size_t n, HF stream);

void* SYS_MapFile(HF stream );
void SYS_UnmapFile(void* p);

#ifdef __cplusplus
}
#endif
