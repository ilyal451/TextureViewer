/*
The Texture Viewer Project, http://imagetools.itch.io/texview
Copyright (c) 2011-2024 Ilya Lyutin

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <windows.h>
#include <stdio.h>

#include <ddraw.h>

#include "../../format.h"
#include "../../shared/system.h"
#include "../../shared/plibclient.h"

#include "dds.h"

typedef unsigned char byte_t;
//typedef enum { false, true } bool;

SystemFuncs_t* g_sys;
PLibClientFuncs_t* g_plibclient;

//#define DXGI_FORMAT_ "DXGI_FORMAT_"
#define DXGI_FORMAT_

// DXGI (DX10) formats (via a dx10 header)
PixelFormatInfo_t g_apfiDXGI[] =
{
	// block compression formats
	{ DXGI_FORMAT_"BC1_TYPELESS",				1, {	{ IN_BC1,					PF_B8G8R8A8_UNORM } },					0,	DXGI_FORMAT_BC1_TYPELESS,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 8, 8, 8, 1 } },
	{ DXGI_FORMAT_"BC1_UNORM",					1, {	{ IN_BC1,					PF_B8G8R8A8_UNORM } },					0,	DXGI_FORMAT_BC1_UNORM,					{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 8, 8, 8, 1 } },
	{ DXGI_FORMAT_"BC1_UNORM_SRGB",				1, {	{ IN_BC1,					PF_B8G8R8A8_UNORM } },					0,	DXGI_FORMAT_BC1_UNORM_SRGB,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 8, 8, 8, 1 } },
	{ DXGI_FORMAT_"BC2_TYPELESS",				1, {	{ IN_BC2,					PF_B8G8R8A8_UNORM } },					0,	DXGI_FORMAT_BC2_TYPELESS,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 8, 8, 8, 8 } },
	{ DXGI_FORMAT_"BC2_UNORM",					1, {	{ IN_BC2,					PF_B8G8R8A8_UNORM } },					0,	DXGI_FORMAT_BC2_UNORM,					{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 8, 8, 8, 8 } },
	{ DXGI_FORMAT_"BC2_UNORM_SRGB",				1, {	{ IN_BC2,					PF_B8G8R8A8_UNORM } },					0,	DXGI_FORMAT_BC2_UNORM_SRGB,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 8, 8, 8, 8 } },
	{ DXGI_FORMAT_"BC3_TYPELESS",				1, {	{ IN_BC3,					PF_B8G8R8A8_UNORM } },					0,	DXGI_FORMAT_BC3_TYPELESS,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 8, 8, 8, 8 } },
	{ DXGI_FORMAT_"BC3_UNORM",					1, {	{ IN_BC3,					PF_B8G8R8A8_UNORM } },					0,	DXGI_FORMAT_BC3_UNORM,					{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 8, 8, 8, 8 } },
	{ DXGI_FORMAT_"BC3_UNORM_SRGB",				1, {	{ IN_BC3,					PF_B8G8R8A8_UNORM } },					0,	DXGI_FORMAT_BC3_UNORM_SRGB,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 8, 8, 8, 8 } },
	{ DXGI_FORMAT_"BC4_TYPELESS",				1, {	{ IN_BC4_UNORM,				-1, CL_R, CS_8BIT, CT_UNORM } },		0,	DXGI_FORMAT_BC4_TYPELESS,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 8, 0, 0, 0 } },
	{ DXGI_FORMAT_"BC4_UNORM",					1, {	{ IN_BC4_UNORM,				-1, CL_R, CS_8BIT, CT_UNORM } },		0,	DXGI_FORMAT_BC4_UNORM,					{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 8, 0, 0, 0 } },
	{ DXGI_FORMAT_"BC4_SNORM",					1, {	{ IN_BC4_SNORM,				-1, CL_R, CS_8BIT, CT_SNORM } },		0,	DXGI_FORMAT_BC4_SNORM,					{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 8, 0, 0, 0 } },
	{ DXGI_FORMAT_"BC5_TYPELESS",				1, {	{ IN_BC5_UNORM,				-1, CL_RG, CS_8BIT, CT_UNORM } },		0,	DXGI_FORMAT_BC5_TYPELESS,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 8, 8, 0, 0 } },
	{ DXGI_FORMAT_"BC5_UNORM",					1, {	{ IN_BC5_UNORM,				-1, CL_RG, CS_8BIT, CT_UNORM } },		0,	DXGI_FORMAT_BC5_UNORM,					{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 8, 8, 0, 0 } },
	{ DXGI_FORMAT_"BC5_SNORM",					1, {	{ IN_BC5_SNORM,				-1, CL_RG, CS_8BIT, CT_SNORM } },		0,	DXGI_FORMAT_BC5_SNORM,					{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 8, 8, 0, 0 } },
	{ DXGI_FORMAT_"BC6H_TYPELESS",				1, {	{ IN_BC6H_UF16,				-1, CL_RGB, CS_16BIT, CT_FLOAT } },		0,	DXGI_FORMAT_BC6H_TYPELESS,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 16, 16, 16, 0 } },
	{ DXGI_FORMAT_"BC6H_UF16",					1, {	{ IN_BC6H_UF16,				-1, CL_RGB, CS_16BIT, CT_FLOAT } },		0,	DXGI_FORMAT_BC6H_UF16,					{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 16, 16, 16, 0 } },
	{ DXGI_FORMAT_"BC6H_SF16",					1, {	{ IN_BC6H_SF16,				-1, CL_RGB, CS_16BIT, CT_FLOAT } },		0,	DXGI_FORMAT_BC6H_SF16,					{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 16, 16, 16, 0 } },
	{ DXGI_FORMAT_"BC7_TYPELESS",				1, {	{ IN_BC7,					PF_B8G8R8A8_UNORM } },					0,	DXGI_FORMAT_BC7_TYPELESS,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 8, 8, 8, 8 } },
	{ DXGI_FORMAT_"BC7_UNORM",					1, {	{ IN_BC7,					PF_B8G8R8A8_UNORM } },					0,	DXGI_FORMAT_BC7_UNORM,					{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 8, 8, 8, 8 } },
	{ DXGI_FORMAT_"BC7_UNORM_SRGB",				1, {	{ IN_BC7,					PF_B8G8R8A8_UNORM } },					0,	DXGI_FORMAT_BC7_UNORM_SRGB,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 8, 8, 8, 8 } },

	// generic RGB formats
	{ DXGI_FORMAT_"R32G32B32A32_TYPELESS",		1, {	{ IN_R32G32B32A32,			-1, CL_RGBA, CS_32BIT, CT_UNORM } },	0,	DXGI_FORMAT_R32G32B32A32_TYPELESS,		{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 32, 32, 32, 32 } },
	{ DXGI_FORMAT_"R32G32B32A32_FLOAT",			1, {	{ IN_R32G32B32A32,			-1, CL_RGBA, CS_32BIT, CT_FLOAT } },	0,	DXGI_FORMAT_R32G32B32A32_FLOAT,			{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 32, 32, 32, 32 } },
	{ DXGI_FORMAT_"R32G32B32A32_UINT",			1, {	{ IN_R32G32B32A32,			-1, CL_RGBA, CS_32BIT, CT_UINT } },		0,	DXGI_FORMAT_R32G32B32A32_UINT,			{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 32, 32, 32, 32 } },
	{ DXGI_FORMAT_"R32G32B32A32_SINT",			1, {	{ IN_R32G32B32A32,			-1, CL_RGBA, CS_32BIT, CT_SINT } },		0,	DXGI_FORMAT_R32G32B32A32_SINT,			{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 32, 32, 32, 32 } },
	{ DXGI_FORMAT_"R32G32B32_TYPELESS",			1, {	{ IN_R32G32B32,				-1, CL_RGB, CS_32BIT, CT_UNORM } },		0,	DXGI_FORMAT_R32G32B32_TYPELESS,			{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 32, 32, 32, 0 } },
	{ DXGI_FORMAT_"R32G32B32_FLOAT",			1, {	{ IN_R32G32B32,				-1, CL_RGB, CS_32BIT, CT_FLOAT } },		0,	DXGI_FORMAT_R32G32B32_FLOAT,			{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 32, 32, 32, 0 } },
	{ DXGI_FORMAT_"R32G32B32_UINT",				1, {	{ IN_R32G32B32,				-1, CL_RGB, CS_32BIT, CT_UINT } },		0,	DXGI_FORMAT_R32G32B32_UINT,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 32, 32, 32, 0 } },
	{ DXGI_FORMAT_"R32G32B32_SINT",				1, {	{ IN_R32G32B32,				-1, CL_RGB, CS_32BIT, CT_SINT } },		0,	DXGI_FORMAT_R32G32B32_SINT,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 32, 32, 32, 0 } },
	{ DXGI_FORMAT_"R32G32_TYPELESS",			1, {	{ IN_R32G32,				-1, CL_RG, CS_32BIT, CT_UNORM } },		0,	DXGI_FORMAT_R32G32_TYPELESS,			{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 32, 32, 0, 0 } },
	{ DXGI_FORMAT_"R32G32_FLOAT",				1, {	{ IN_R32G32,				-1, CL_RG, CS_32BIT, CT_FLOAT } },		0,	DXGI_FORMAT_R32G32_FLOAT,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 32, 32, 0, 0 } },
	{ DXGI_FORMAT_"R32G32_UINT",				1, {	{ IN_R32G32,				-1, CL_RG, CS_32BIT, CT_UINT } },		0,	DXGI_FORMAT_R32G32_UINT,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 32, 32, 0, 0 } },
	{ DXGI_FORMAT_"R32G32_SINT",				1, {	{ IN_R32G32,				-1, CL_RG, CS_32BIT, CT_SINT } },		0,	DXGI_FORMAT_R32G32_SINT,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 32, 32, 0, 0 } },
	{ DXGI_FORMAT_"R32_TYPELESS",				1, {	{ IN_R32,					-1, CL_R, CS_32BIT, CT_UNORM } },		0,	DXGI_FORMAT_R32_TYPELESS,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 32, 0, 0, 0 } },
	{ DXGI_FORMAT_"D32_FLOAT",					1, {	{ IN_R32,					-1, CL_R, CS_32BIT, CT_FLOAT } },		0,	DXGI_FORMAT_D32_FLOAT,					{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 32, 0, 0, 0 } },
	{ DXGI_FORMAT_"R32_FLOAT",					1, {	{ IN_R32,					-1, CL_R, CS_32BIT, CT_FLOAT } },		0,	DXGI_FORMAT_R32_FLOAT,					{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 32, 0, 0, 0 } },
	{ DXGI_FORMAT_"R32_UINT",					1, {	{ IN_R32,					-1, CL_R, CS_32BIT, CT_UINT } },		0,	DXGI_FORMAT_R32_UINT,					{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 32, 0, 0, 0 } },
	{ DXGI_FORMAT_"R32_SINT",					1, {	{ IN_R32,					-1, CL_R, CS_32BIT, CT_SINT } },		0,	DXGI_FORMAT_R32_SINT,					{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 32, 0, 0, 0 } },
	{ DXGI_FORMAT_"R16G16B16A16_TYPELESS",		1, {	{ IN_R16G16B16A16,			-1, CL_RGBA, CS_16BIT, CT_UNORM } },	0,	DXGI_FORMAT_R16G16B16A16_TYPELESS,		{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 16, 16, 16, 16 } },
	{ DXGI_FORMAT_"R16G16B16A16_FLOAT",			1, {	{ IN_R16G16B16A16,			-1, CL_RGBA, CS_16BIT, CT_FLOAT } },	0,	DXGI_FORMAT_R16G16B16A16_FLOAT,			{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 16, 16, 16, 16 } },
	{ DXGI_FORMAT_"R16G16B16A16_UNORM",			1, {	{ IN_R16G16B16A16,			-1, CL_RGBA, CS_16BIT, CT_UNORM } },	0,	DXGI_FORMAT_R16G16B16A16_UNORM,			{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 16, 16, 16, 16 } },
	{ DXGI_FORMAT_"R16G16B16A16_UINT",			1, {	{ IN_R16G16B16A16,			-1, CL_RGBA, CS_16BIT, CT_UINT } },		0,	DXGI_FORMAT_R16G16B16A16_UINT,			{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 16, 16, 16, 16 } },
	{ DXGI_FORMAT_"R16G16B16A16_SNORM",			1, {	{ IN_R16G16B16A16,			-1, CL_RGBA, CS_16BIT, CT_SNORM } },	0,	DXGI_FORMAT_R16G16B16A16_SNORM,			{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 16, 16, 16, 16 } },
	{ DXGI_FORMAT_"R16G16B16A16_SINT",			1, {	{ IN_R16G16B16A16,			-1, CL_RGBA, CS_16BIT, CT_SINT } },		0,	DXGI_FORMAT_R16G16B16A16_SINT,			{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 16, 16, 16, 16 } },
	{ DXGI_FORMAT_"R16G16_TYPELESS",			1, {	{ IN_R16G16,				-1, CL_RG, CS_16BIT, CT_UNORM } },		0,	DXGI_FORMAT_R16G16_TYPELESS,			{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 16, 16, 0, 0 } },
	{ DXGI_FORMAT_"R16G16_FLOAT",				1, {	{ IN_R16G16,				-1, CL_RG, CS_16BIT, CT_FLOAT } },		0,	DXGI_FORMAT_R16G16_FLOAT,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 16, 16, 0, 0 } },
	{ DXGI_FORMAT_"R16G16_UNORM",				1, {	{ IN_R16G16,				-1, CL_RG, CS_16BIT, CT_UNORM } },		0,	DXGI_FORMAT_R16G16_UNORM,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 16, 16, 0, 0 } },
	{ DXGI_FORMAT_"R16G16_UINT",				1, {	{ IN_R16G16,				-1, CL_RG, CS_16BIT, CT_UINT } },		0,	DXGI_FORMAT_R16G16_UINT,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 16, 16, 0, 0 } },
	{ DXGI_FORMAT_"R16G16_SNORM",				1, {	{ IN_R16G16,				-1, CL_RG, CS_16BIT, CT_SNORM } },		0,	DXGI_FORMAT_R16G16_SNORM,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 16, 16, 0, 0 } },
	{ DXGI_FORMAT_"R16G16_SINT",				1, {	{ IN_R16G16,				-1, CL_RG, CS_16BIT, CT_SINT } },		0,	DXGI_FORMAT_R16G16_SINT,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 16, 16, 0, 0 } },
	{ DXGI_FORMAT_"R16_TYPELESS",				1, {	{ IN_R16,					-1, CL_R, CS_16BIT, CT_UNORM } },		0,	DXGI_FORMAT_R16_TYPELESS,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 16, 0, 0, 0 } },
	{ DXGI_FORMAT_"R16_FLOAT",					1, {	{ IN_R16,					-1, CL_R, CS_16BIT, CT_FLOAT } },		0,	DXGI_FORMAT_R16_FLOAT,					{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 16, 0, 0, 0 } },
	{ DXGI_FORMAT_"D16_UNORM",					1, {	{ IN_R16,					-1, CL_R, CS_16BIT, CT_UNORM } },		0,	DXGI_FORMAT_D16_UNORM,					{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 16, 0, 0, 0 } },
	{ DXGI_FORMAT_"R16_UNORM",					1, {	{ IN_R16,					-1, CL_R, CS_16BIT, CT_UNORM } },		0,	DXGI_FORMAT_R16_UNORM,					{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 16, 0, 0, 0 } },
	{ DXGI_FORMAT_"R16_UINT",					1, {	{ IN_R16,					-1, CL_R, CS_16BIT, CT_UINT } },		0,	DXGI_FORMAT_R16_UINT,					{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 16, 0, 0, 0 } },
	{ DXGI_FORMAT_"R16_SNORM",					1, {	{ IN_R16,					-1, CL_R, CS_16BIT, CT_SNORM } },		0,	DXGI_FORMAT_R16_SNORM,					{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 16, 0, 0, 0 } },
	{ DXGI_FORMAT_"R16_SINT",					1, {	{ IN_R16,					-1, CL_R, CS_16BIT, CT_SINT } },		0,	DXGI_FORMAT_R16_SINT,					{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 16, 0, 0, 0 } },
	{ DXGI_FORMAT_"R8G8B8A8_TYPELESS",			1, {	{ IN_R8G8B8A8,				-1, CL_RGBA, CS_8BIT, CT_UNORM } },		0,	DXGI_FORMAT_R8G8B8A8_TYPELESS,			{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 8, 8, 8, 8 } },
	{ DXGI_FORMAT_"R8G8B8A8_UNORM",				1, {	{ IN_R8G8B8A8_UNORM,		PF_B8G8R8A8_UNORM } },					0,	DXGI_FORMAT_R8G8B8A8_UNORM,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 8, 8, 8, 8 } },
	{ DXGI_FORMAT_"R8G8B8A8_UNORM_SRGB",		1, {	{ IN_R8G8B8A8_UNORM,		PF_B8G8R8A8_UNORM } },					0,	DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,		{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 8, 8, 8, 8 } },
	{ DXGI_FORMAT_"R8G8B8A8_UINT",				1, {	{ IN_R8G8B8A8,				-1, CL_RGBA, CS_8BIT, CT_UINT } },		0,	DXGI_FORMAT_R8G8B8A8_UINT,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 8, 8, 8, 8 } },
	{ DXGI_FORMAT_"R8G8B8A8_SNORM",				1, {	{ IN_R8G8B8A8,				-1, CL_RGBA, CS_8BIT, CT_SNORM } },		0,	DXGI_FORMAT_R8G8B8A8_SNORM,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 8, 8, 8, 8 } },
	{ DXGI_FORMAT_"R8G8B8A8_SINT",				1, {	{ IN_R8G8B8A8,				-1, CL_RGBA, CS_8BIT, CT_SINT } },		0,	DXGI_FORMAT_R8G8B8A8_SINT,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 8, 8, 8, 8 } },
	{ DXGI_FORMAT_"R8G8_TYPELESS",				1, {	{ IN_R8G8,					-1, CL_RG, CS_8BIT, CT_UNORM } },		0,	DXGI_FORMAT_R8G8_TYPELESS,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 8, 8, 0, 0 } },
	{ DXGI_FORMAT_"R8G8_UNORM",					1, {	{ IN_R8G8,					-1, CL_RG, CS_8BIT, CT_UNORM } },		0,	DXGI_FORMAT_R8G8_UNORM,					{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 8, 8, 0, 0 } },
	{ DXGI_FORMAT_"R8G8_UINT",					1, {	{ IN_R8G8,					-1, CL_RG, CS_8BIT, CT_UINT } },		0,	DXGI_FORMAT_R8G8_UINT,					{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 8, 8, 0, 0 } },
	{ DXGI_FORMAT_"R8G8_SNORM",					1, {	{ IN_R8G8,					-1, CL_RG, CS_8BIT, CT_SNORM } },		0,	DXGI_FORMAT_R8G8_SNORM,					{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 8, 8, 0, 0 } },
	{ DXGI_FORMAT_"R8G8_SINT",					1, {	{ IN_R8G8,					-1, CL_RG, CS_8BIT, CT_SINT } },		0,	DXGI_FORMAT_R8G8_SINT,					{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 8, 8, 0, 0 } },
	{ DXGI_FORMAT_"R8_TYPELESS",				1, {	{ IN_R8,					-1, CL_R, CS_8BIT, CT_UNORM } },		0,	DXGI_FORMAT_R8_TYPELESS,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 8, 0, 0, 0 } },
	{ DXGI_FORMAT_"R8_UNORM",					1, {	{ IN_R8,					-1, CL_R, CS_8BIT, CT_UNORM } },		0,	DXGI_FORMAT_R8_UNORM,					{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 8, 0, 0, 0 } },
	{ DXGI_FORMAT_"R8_UINT",					1, {	{ IN_R8,					-1, CL_R, CS_8BIT, CT_UINT } },			0,	DXGI_FORMAT_R8_UINT,					{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 8, 0, 0, 0 } },
	{ DXGI_FORMAT_"R8_SNORM",					1, {	{ IN_R8,					-1, CL_R, CS_8BIT, CT_SNORM } },		0,	DXGI_FORMAT_R8_SNORM,					{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 8, 0, 0, 0 } },
	{ DXGI_FORMAT_"R8_SINT",					1, {	{ IN_R8,					-1, CL_R, CS_8BIT, CT_SINT } },			0,	DXGI_FORMAT_R8_SINT,					{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 8, 0, 0, 0 } },

	// the native gdi format
	{ DXGI_FORMAT_"B8G8R8A8_TYPELESS",			1, {	{ IN_B8G8R8A8_UNORM,		PF_B8G8R8A8_UNORM } },					0,	DXGI_FORMAT_B8G8R8A8_TYPELESS,			{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 8, 8, 8, 8 } },
	{ DXGI_FORMAT_"B8G8R8A8_UNORM",				1, {	{ IN_B8G8R8A8_UNORM,		PF_B8G8R8A8_UNORM } },					0,	DXGI_FORMAT_B8G8R8A8_UNORM,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 8, 8, 8, 8 } },
	{ DXGI_FORMAT_"B8G8R8A8_UNORM_SRGB",		1, {	{ IN_B8G8R8A8_UNORM,		PF_B8G8R8A8_UNORM } },					0,	DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,		{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 8, 8, 8, 8 } },
	{ DXGI_FORMAT_"B8G8R8X8_TYPELESS",			1, {	{ IN_B8G8R8X8_UNORM,		PF_B8G8R8X8_UNORM } },					0,	DXGI_FORMAT_B8G8R8X8_TYPELESS,			{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 8, 8, 8, 0 } },
	{ DXGI_FORMAT_"B8G8R8X8_UNORM",				1, {	{ IN_B8G8R8X8_UNORM,		PF_B8G8R8X8_UNORM } },					0,	DXGI_FORMAT_B8G8R8X8_UNORM,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 8, 8, 8, 0 } },
	{ DXGI_FORMAT_"B8G8R8X8_UNORM_SRGB",		1, {	{ IN_B8G8R8X8_UNORM,		PF_B8G8R8X8_UNORM } },					0,	DXGI_FORMAT_B8G8R8X8_UNORM_SRGB,		{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 8, 8, 8, 0 } },

	// packed integer formats
	{ DXGI_FORMAT_"R10G10B10A2_TYPELESS",		1, {	{ IN_R10G10B10A2_UNORM,		-1, CL_RGBA, CS_16BIT, CT_UNORM } },	0,	DXGI_FORMAT_R10G10B10A2_TYPELESS,		{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 10, 10, 10, 2 } },
	{ DXGI_FORMAT_"R10G10B10A2_UNORM",			1, {	{ IN_R10G10B10A2_UNORM,		-1, CL_RGBA, CS_16BIT, CT_UNORM } },	0,	DXGI_FORMAT_R10G10B10A2_UNORM,			{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 10, 10, 10, 2 } },
	{ DXGI_FORMAT_"R10G10B10A2_UINT",			1, {	{ IN_R10G10B10A2_UINT,		-1, CL_RGBA, CS_16BIT, CT_UINT } },		0,	DXGI_FORMAT_R10G10B10A2_UINT,			{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 10, 10, 10, 2 } },
	{ DXGI_FORMAT_"B5G6R5_UNORM",				1, {	{ IN_B5G6R5_UNORM,			PF_B8G8R8X8_UNORM } },					0,	DXGI_FORMAT_B5G6R5_UNORM,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 5, 6, 5, 0 } },
	{ DXGI_FORMAT_"B5G5R5A1_UNORM",				1, {	{ IN_B5G5R5A1_UNORM,		PF_B8G8R8A8_UNORM } },					0,	DXGI_FORMAT_B5G5R5A1_UNORM,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 5, 5, 5, 1 } },
	{ DXGI_FORMAT_"B4G4R4A4_UNORM",				1, {	{ IN_B4G4R4A4_UNORM,		PF_B8G8R8A8_UNORM } },					0,	DXGI_FORMAT_B4G4R4A4_UNORM,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 4, 4, 4, 4 } },

	// alpha / luminance
	{ DXGI_FORMAT_"A8_UNORM",					1, {	{ IN_A8,					-1, CL_A, CS_8BIT, CT_UNORM } },		0,	DXGI_FORMAT_A8_UNORM,					{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 0, 0, 0, 8 } },

	// palette
	{ DXGI_FORMAT_"P8",							1, {	{ IN_P8,					-1, CL_P, CS_8BIT, CT_UINT } },			256,	DXGI_FORMAT_P8,						{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 0, 0, 0, 0 } },
	{ DXGI_FORMAT_"P8A8",						2, {	{ IN_P8X8,					-1, CL_P, CS_8BIT, CT_UINT },
														{ IN_X8A8,					-1, CL_A, CS_8BIT, CT_UNORM } },		256,	DXGI_FORMAT_A8P8,					{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 0, 0, 0, 8 } },

	// packed bits
	{ DXGI_FORMAT_"R1_UNORM",					1, {	{ IN_L1L,					-1, CL_R, CS_8BIT, CT_UNORM } },		0,	DXGI_FORMAT_R1_UNORM,					{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 1, 0, 0, 0 } },

	// depth/stencil formats (needed?
	{ DXGI_FORMAT_"R32G8X24_TYPELESS",			2, {	{ IN_D32X8X24,				-1, CL_L, CS_32BIT, CT_UNORM },
														{ IN_X32S8X24,				-1, CL_A, CS_8BIT, CT_UNORM } },		0,	DXGI_FORMAT_R32G8X24_TYPELESS,			{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 32, 32, 32, 8 } },
	{ DXGI_FORMAT_"D32_FLOAT_S8X24_UINT",		2, {	{ IN_D32X8X24,				-1, CL_L, CS_32BIT, CT_FLOAT },
														{ IN_X32S8X24,				-1, CL_A, CS_8BIT, CT_UINT } },			0,	DXGI_FORMAT_D32_FLOAT_S8X24_UINT,		{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 32, 32, 32, 8 } },
	{ DXGI_FORMAT_"R32_FLOAT_X8X24_TYPELESS",	1, {	{ IN_D32X8X24,				-1, CL_L, CS_32BIT, CT_FLOAT } },		0,	DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS,	{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 32, 32, 32, 0 } },
	{ DXGI_FORMAT_"X32_TYPELESS_G8X24_UINT",	1, {	{ IN_X32S8X24,				-1, CL_A, CS_8BIT, CT_UINT } },			0,	DXGI_FORMAT_X32_TYPELESS_G8X24_UINT,	{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 0, 0, 0, 8 } },
	{ DXGI_FORMAT_"R24G8_TYPELESS",				2, {	{ IN_D24X8,					-1, CL_L, CS_32BIT, CT_UNORM },
														{ IN_X24S8,					-1, CL_A, CS_8BIT, CT_UNORM } },		0,	DXGI_FORMAT_R24G8_TYPELESS,				{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 24, 24, 24, 8 } },
	{ DXGI_FORMAT_"D24_UNORM_S8_UINT",			2, {	{ IN_D24X8,					-1, CL_L, CS_32BIT, CT_UNORM },
														{ IN_X24S8,					-1, CL_A, CS_8BIT, CT_UINT } },			0,	DXGI_FORMAT_D24_UNORM_S8_UINT,			{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 24, 24, 24, 8 } },
	{ DXGI_FORMAT_"R24_UNORM_X8_TYPELESS",		1, {	{ IN_D24X8					-1, CL_L, CS_32BIT, CT_UNORM } },		0,	DXGI_FORMAT_R24_UNORM_X8_TYPELESS,		{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 24, 24, 24, 0 } },
	{ DXGI_FORMAT_"X24_TYPELESS_G8_UINT",		1, {	{ IN_X24S8,					-1, CL_A, CS_8BIT, CT_UINT } },			0,	DXGI_FORMAT_X24_TYPELESS_G8_UINT,		{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 0, 0, 0, 8 } },

	// packed float formats
	{ DXGI_FORMAT_"R11G11B10_FLOAT",			1, {	{ IN_R11G11B10_FLOAT,		-1, CL_RGB, CS_16BIT, CT_FLOAT } },		0,	DXGI_FORMAT_R11G11B10_FLOAT,			{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 11, 11, 10, 0 } },
	{ DXGI_FORMAT_"R9G9B9E5_SHAREDEXP",			1, {	{ IN_R9G9B9E5_FLOAT,		-1, CL_RGB, CS_32BIT, CT_FLOAT } },		0,	DXGI_FORMAT_R9G9B9E5_SHAREDEXP,			{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 14, 14, 14, 0 } },

	//{ DXGI_FORMAT_"R10G10B10_XR_BIAS_A2_UNORM",	2, {	{ IN_R10G10B10X2_XR_BIAS,	-1, CL_RGB, CS_16BIT, CT_FLOAT },
	//													{ IN_X10X10X10A2_UNORM,		-1, CL_A, CS_8BIT, CT_UNORM } },		0,	DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM,	{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 10, 10, 10, 2 } },

	// interleaved formats
	{ DXGI_FORMAT_"R8G8_B8G8_UNORM",			1, {	{ IN_RGBG,					PF_B8G8R8X8_UNORM } },					0,	DXGI_FORMAT_R8G8_B8G8_UNORM,			{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 8, 8, 8, 0 } },
	{ DXGI_FORMAT_"G8R8_G8B8_UNORM",			1, {	{ IN_GRGB,					PF_B8G8R8X8_UNORM } },					0,	DXGI_FORMAT_G8R8_G8B8_UNORM,			{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 8, 8, 8, 0 } },
	{ DXGI_FORMAT_"YUY2",						1, {	{ IN_YUY2,					PF_B8G8R8X8_UNORM } },					0,	DXGI_FORMAT_YUY2,						{ 0, 0, 0, 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 8, 8, 8, 0 } },

};

int g_nDXGIFormats = sizeof( g_apfiDXGI ) / sizeof( PixelFormatInfo_t );


// D3DFMT_ (DX9) formats
PixelFormatInfo_t g_apfi[] =
{
	// indicates that there is a dx10 header
	{ NULL,							0, {	{ -1, -1, 0, -1 } },														0,	-1,			{ 0, DDPF_FOURCC, '01XD', 0, 0, 0, 0, 0 }, 0, 0 },

	// block compression formats
	{ "DXT1",						1, {	{ IN_BC1,					PF_B8G8R8A8_UNORM } },							0,	-1,			{ 0, DDPF_FOURCC, '1TXD', 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 8, 8, 8, 1 } },
	{ "DXT2",						1, {	{ IN_BC2,					PF_B8G8R8A8_UNORM } },							0,	-1,			{ 0, DDPF_FOURCC, '2TXD', 0, 0, 0, 0, 0 }, GM_SRGB, IMAGE_PREMULTIPLIED_ALPHA, { 8, 8, 8, 8 } },
	{ "DXT3",						1, {	{ IN_BC2,					PF_B8G8R8A8_UNORM } },							0,	-1,			{ 0, DDPF_FOURCC, '3TXD', 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 8, 8, 8, 8 } },
	{ "DXT4",						1, {	{ IN_BC3,					PF_B8G8R8A8_UNORM } },							0,	-1,			{ 0, DDPF_FOURCC, '4TXD', 0, 0, 0, 0, 0 }, GM_SRGB, IMAGE_PREMULTIPLIED_ALPHA, { 8, 8, 8, 8 } },
	{ "DXT5",						1, {	{ IN_BC3,					PF_B8G8R8A8_UNORM } },							0,	-1,			{ 0, DDPF_FOURCC, '5TXD', 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 8, 8, 8, 8 } },
	{ "DXT5 RxGB",					1, {	{ IN_BC3,					PF_B8G8R8A8_UNORM } },							0,	-1,			{ 0, DDPF_FOURCC, 'BGXR', 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 8, 8, 8, 8 } },
	{ "BC4U",						1, {	{ IN_BC4_UNORM,				-1, CL_R,		CS_8BIT,	CT_UNORM } },		0,	-1,			{ 0, DDPF_FOURCC, 'U4CB', 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 8, 0, 0, 0 } },
	{ "3Dc+ (BC4U)",				1, {	{ IN_BC4_UNORM,				-1, CL_R,		CS_8BIT,	CT_UNORM } },		0,	-1,			{ 0, DDPF_FOURCC, '1ITA', 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 8, 0, 0, 0 } },
	{ "BC4S",						1, {	{ IN_BC4_SNORM,				-1, CL_R,		CS_8BIT,	CT_SNORM } },		0,	-1,			{ 0, DDPF_FOURCC, 'S4CB', 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 8, 0, 0, 0 } },
	{ "BC5U",						1, {	{ IN_BC5_UNORM,				-1, CL_RG,		CS_8BIT,	CT_UNORM } },		0,	-1,			{ 0, DDPF_FOURCC, 'U5CB', 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 8, 8, 0, 0 } },
	{ "3Dc (BC5U)",					1, {	{ IN_BC5_UNORM,				-1, CL_RG,		CS_8BIT,	CT_UNORM } },		0,	-1,			{ 0, DDPF_FOURCC, '2ITA', 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 8, 8, 0, 0 } },
	{ "BC5S",						1, {	{ IN_BC5_SNORM,				-1, CL_RG,		CS_8BIT,	CT_SNORM } },		0,	-1,			{ 0, DDPF_FOURCC, 'S5CB', 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 8, 8, 0, 0 } },

	// fourcc rgb formats
	{ "R16G16B16A16",				1, {	{ IN_R16G16B16A16,			-1, CL_RGBA,	CS_16BIT,	CT_UNORM } },		0,	-1,			{ 0, DDPF_FOURCC, 36, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 16, 16, 16, 16 } },
	{ "U16V16W16Q16",				1, {	{ IN_R16G16B16A16,			-1, CL_RGBA,	CS_16BIT,	CT_SNORM } },		0,	-1,			{ 0, DDPF_FOURCC, 110, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 16, 16, 16, 16 } },
	{ "R16F",						1, {	{ IN_R16,					-1, CL_R,		CS_16BIT,	CT_FLOAT } },		0,	-1,			{ 0, DDPF_FOURCC, 111, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 16, 0, 0, 0 } },
	{ "R16G16F",					1, {	{ IN_R16G16,				-1, CL_RG,		CS_16BIT,	CT_FLOAT } },		0,	-1,			{ 0, DDPF_FOURCC, 112, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 16, 16, 0, 0 } },
	{ "R16G16B16A16F",				1, {	{ IN_R16G16B16A16,			-1, CL_RGBA,	CS_16BIT,	CT_FLOAT } },		0,	-1,			{ 0, DDPF_FOURCC, 113, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 16, 16, 16, 16 } },
	{ "R32F",						1, {	{ IN_R32,					-1, CL_R,		CS_32BIT,	CT_FLOAT } },		0,	-1,			{ 0, DDPF_FOURCC, 114, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 32, 0, 0, 0 } },
	{ "R32G32F",					1, {	{ IN_R32G32,				-1, CL_RG,		CS_32BIT,	CT_FLOAT } },		0,	-1,			{ 0, DDPF_FOURCC, 115, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 32, 32, 0, 0 } },
	{ "R32G32B32A32F",				1, {	{ IN_R32G32B32A32,			-1, CL_RGBA,	CS_32BIT,	CT_FLOAT } },		0,	-1,			{ 0, DDPF_FOURCC, 116, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 32, 32, 32, 32 } },
	{ "U8V8Cx",						1, {	{ IN_U8V8CX,				-1, CL_RG,		CS_8BIT,	CT_SNORM } },		0,	-1,			{ 0, DDPF_FOURCC, 117, 0, 0, 0, 0, 0 }, GM_LINEAR, 0, { 8, 8, 0, 0 } },

	// interleaved fourcc formats
	{ "R8G8_B8G8",					1, {	{ IN_RGBG,					PF_B8G8R8X8_UNORM } },							0,	-1,			{ 0, DDPF_FOURCC, 'GBGR', 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 8, 8, 8, 0 } },
	{ "G8R8_G8B8",					1, {	{ IN_GRGB,					PF_B8G8R8X8_UNORM } },							0,	-1,			{ 0, DDPF_FOURCC, 'BGRG', 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 8, 8, 8, 0 } },
	{ "UYVY",						1, {	{ IN_UYVY,					PF_B8G8R8X8_UNORM } },							0,	-1,			{ 0, DDPF_FOURCC, 'YVYU', 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 8, 8, 8, 0 } },
	{ "YUY2",						1, {	{ IN_YUY2,					PF_B8G8R8X8_UNORM } },							0,	-1,			{ 0, DDPF_FOURCC, '2YUY', 0, 0, 0, 0, 0 }, GM_SRGB, 0, { 8, 8, 8, 0 } },

	// RGB formats
	{ "B2G3R3",						1, {	{ IN_B2G3R3_UNORM,			PF_B8G8R8X8_UNORM } },							0,	-1,			{ 0, DDPF_RGB, 0, 8,  0x000000E0, 0x0000001C, 0x00000003, 0x00000000 }, GM_SRGB, 0, { 3, 3, 2, 0 } },
	{ "B2G3R3A8",					1, {	{ IN_B2G3R3A8_UNORM,		PF_B8G8R8A8_UNORM } },							0,	-1,			{ 0, DDPF_RGB, 0, 16, 0x000000E0, 0x0000001C, 0x00000003, 0x0000FF00 }, GM_SRGB, 0, { 3, 3, 2, 8 } },
	{ "B4G4R4A4",					1, {	{ IN_B4G4R4A4_UNORM,		PF_B8G8R8A8_UNORM } },							0,	-1,			{ 0, DDPF_RGB, 0, 16, 0x00000F00, 0x000000F0, 0x0000000F, 0x0000F000 }, GM_SRGB, 0, { 4, 4, 4, 4 } },
	{ "B4G4R4X4",					1, {	{ IN_B4G4R4X4_UNORM,		PF_B8G8R8X8_UNORM } },							0,	-1,			{ 0, DDPF_RGB, 0, 16, 0x00000F00, 0x000000F0, 0x0000000F, 0x00000000 }, GM_SRGB, 0, { 4, 4, 4, 0 } },
	{ "B5G5R5A1",					1, {	{ IN_B5G5R5A1_UNORM,		PF_B8G8R8A8_UNORM } },							0,	-1,			{ 0, DDPF_RGB, 0, 16, 0x00007C00, 0x000003E0, 0x0000001F, 0x00008000 }, GM_SRGB, 0, { 5, 5, 5, 1 } },
	{ "B5G5R5X1",					1, {	{ IN_B5G5R5X1_UNORM,		PF_B8G8R8X8_UNORM } },							0,	-1,			{ 0, DDPF_RGB, 0, 16, 0x00007C00, 0x000003E0, 0x0000001F, 0x00000000 }, GM_SRGB, 0, { 5, 5, 5, 0 } },
	{ "B5G6R5",						1, {	{ IN_B5G6R5_UNORM,			PF_B8G8R8X8_UNORM } },							0,	-1,			{ 0, DDPF_RGB, 0, 16, 0x0000F800, 0x000007E0, 0x0000001F, 0x00000000 }, GM_SRGB, 0, { 5, 6, 5, 0 } },
	{ "B8G8R8A8",					1, {	{ IN_B8G8R8A8_UNORM,		PF_B8G8R8A8_UNORM } },							0,	-1,			{ 0, DDPF_RGB, 0, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000 }, GM_SRGB, 0, { 8, 8, 8, 8 } },
	{ "B8G8R8X8",					1, {	{ IN_B8G8R8X8_UNORM,		PF_B8G8R8X8_UNORM } },							0,	-1,			{ 0, DDPF_RGB, 0, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0x00000000 }, GM_SRGB, 0, { 8, 8, 8, 0 } },
	{ "R8G8B8A8",					1, {	{ IN_R8G8B8A8_UNORM,		PF_B8G8R8A8_UNORM } },							0,	-1,			{ 0, DDPF_RGB, 0, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000 }, GM_SRGB, 0, { 8, 8, 8, 8 } },
	{ "R8G8B8X8",					1, {	{ IN_R8G8B8X8_UNORM,		PF_B8G8R8X8_UNORM } },							0,	-1,			{ 0, DDPF_RGB, 0, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0x00000000 }, GM_SRGB, 0, { 8, 8, 8, 0 } },
	{ "B8G8R8",						1, {	{ IN_B8G8R8_UNORM,			PF_B8G8R8X8_UNORM } },							0,	-1,			{ 0, DDPF_RGB, 0, 24, 0x00FF0000, 0x0000FF00, 0x000000FF, 0x00000000 }, GM_SRGB, 0, { 8, 8, 8, 0 } },
	{ "R8G8B8",						1, {	{ IN_R8G8B8_UNORM,			PF_B8G8R8X8_UNORM } },							0,	-1,			{ 0, DDPF_RGB, 0, 24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0x00000000 }, GM_SRGB, 0, { 8, 8, 8, 0 } }, // **
	{ "R16G16",						1, {	{ IN_R16G16,				-1, CL_RG,		CS_16BIT,	CT_UNORM } },		0,	-1,			{ 0, DDPF_RGB, 0, 32, 0x0000FFFF, 0xFFFF0000, 0x00000000, 0x00000000 }, GM_LINEAR, 0, { 16, 16, 0, 0 } },

	// XXX: these two are decoded incorrectly (swizzle)
	{ "B10G10R10A2",				1, {	{ IN_B10G10R10A2_UNORM,		-1, CL_RGBA,	CS_16BIT,	CT_UNORM } },		0,	-1,			{ 0, DDPF_RGB, 0, 32, 0x000003FF, 0x000FFC00, 0x3FF00000, 0xC0000000 }, GM_LINEAR, 0, { 10, 10, 10, 2 } },
	{ "R10G10B10A2",				1, {	{ IN_R10G10B10A2_UNORM,		-1, CL_RGBA,	CS_16BIT,	CT_UNORM } },		0,	-1,			{ 0, DDPF_RGB, 0, 32, 0x3FF00000, 0x000FFC00, 0x000003FF, 0xC0000000 }, GM_LINEAR, 0, { 10, 10, 10, 2 } },

	// YUV formats
	//{  "AYUV",						1, { { IN_AYUV, CL_RGBX, CS_8BIT, CT_UNORM } },							0,	-1,			{ 0, DDPF_YUV, 0, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000 }, GM_SRGB, 0 },

	// alpha only
	{ "A8",							1, {	{ IN_A8,					-1, CL_A,		CS_8BIT,	CT_UNORM } },		0,	-1,			{ 0, DDPF_ALPHA, 0, 8, 0x00000000, 0x00000000, 0x00000000, 0x000000FF }, GM_SRGB, 0, { 0, 0, 0, 8 } },

	// luminance
	{ "L4A4",						1, {	{ IN_L4A4,					-1, CL_LA,		CS_8BIT,	CT_UNORM } },		0,	-1,			{ 0, DDPF_LUMINANCE, 0, 8,  0x0000000F, 0x00000000, 0x00000000, 0x000000F0 }, GM_SRGB, IMAGE_GRAYSCALE, { 4, 4, 4, 4 } },
	{ "L8",							1, {	{ IN_L8,					-1, CL_L,		CS_8BIT,	CT_UNORM } },		0,	-1,			{ 0, DDPF_LUMINANCE, 0, 8,  0x000000FF, 0x00000000, 0x00000000, 0x00000000 }, GM_SRGB, IMAGE_GRAYSCALE, { 8, 8, 8, 0 } },
	{ "L8A8",						1, {	{ IN_L8A8,					-1, CL_LA,		CS_8BIT,	CT_UNORM } },		0,	-1,			{ 0, DDPF_LUMINANCE, 0, 16, 0x000000FF, 0x00000000, 0x00000000, 0x0000FF00 }, GM_SRGB, IMAGE_GRAYSCALE, { 8, 8, 8, 8 } },
	{ "L16",						1, {	{ IN_L16,					-1, CL_L,		CS_16BIT,	CT_UNORM } },		0,	-1,			{ 0, DDPF_LUMINANCE, 0, 16, 0x0000FFFF, 0x00000000, 0x00000000, 0x00000000 }, GM_LINEAR, IMAGE_GRAYSCALE, { 16, 16, 16, 0 } },

	// indexed formats
	{ "P4A4",						2, {	{ IN_P4X4,					-1, CL_P,		CS_8BIT,	CT_UINT },
											{ IN_X4A4,					-1, CL_A,		CS_8BIT,	CT_UNORM } },		16,		-1,		{ 0, DDPF_PALETTEINDEXED4, 0, 8,  0x00000000, 0x00000000, 0x00000000, 0x000000F0 }, GM_SRGB, 0, { 8, 8, 8, 4 } },
	{ "P8",							1, {	{ IN_P8,					-1, CL_P,		CS_8BIT,	CT_UINT } },		256,	-1,		{ 0, DDPF_PALETTEINDEXED8, 0, 8,  0x00000000, 0x00000000, 0x00000000, 0x00000000 }, GM_SRGB, 0, { 8, 8, 8, 0 } },
	{ "P8A8",						2, {	{ IN_P8X8,					-1, CL_P,		CS_8BIT,	CT_UINT },
											{ IN_X8A8,					-1, CL_A,		CS_8BIT,	CT_UNORM } },		256,	-1,		{ 0, DDPF_PALETTEINDEXED8, 0, 16, 0x00000000, 0x00000000, 0x00000000, 0x0000FF00 }, GM_SRGB, 0, { 8, 8, 8, 8 } },

	// signed UV formats
	{ "U8V8",						1, {	{ IN_R8G8,					-1, CL_RG,		CS_8BIT,	CT_SNORM } },		0,		-1,		{ 0, DDPF_BUMPDUDV,  0, 16, 0x000000FF, 0x0000FF00, 0x00000000, 0x00000000 }, GM_LINEAR, 0, { 8, 8, 0, 0 } },
	{ "U8V8W8Q8",					1, {	{ IN_R8G8B8A8,				-1, CL_RGBA,	CS_8BIT,	CT_SNORM } },		0,		-1,		{ 0, DDPF_BUMPDUDV,  0, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000 }, GM_LINEAR, 0, { 8, 8, 8, 8 } },
	{ "U16V16",						1, {	{ IN_R16G16,				-1, CL_RG,		CS_16BIT,	CT_SNORM } },		0,		-1,		{ 0, DDPF_BUMPDUDV,  0, 32, 0x0000FFFF, 0xFFFF0000, 0x00000000, 0x00000000 }, GM_LINEAR, 0, { 16, 16, 0, 0 } },
	{ "U10V10W10A2",				2, {	{ IN_U10V10W10X2,			-1, CL_RGB,		CS_16BIT,	CT_SNORM },
											{ IN_X10X10X10A2_UNORM,		-1, CL_A,		CS_8BIT,	CT_UNORM } },		0,		-1,		{ 0, DDPF_BUMPDUDV,  0, 32, 0x3FF00000, 0x000FFC00, 0x000003FF, 0xC0000000 }, GM_LINEAR, 0, { 10, 10, 10, 2 } },

	// mixed signed/unsigned formats
	{ "U5V5L6",						2, {	{ IN_U5V5X6,				-1, CL_RG,		CS_8BIT,	CT_SNORM },
											{ IN_X5X5L6,				-1, CL_A,		CS_8BIT,	CT_UNORM } },		0,		-1,		{ 0, DDPF_BUMPLUMINANCE, 0, 16, 0x0000001F, 0x000003E0, 0x0000FC00, 0x00000000 }, GM_LINEAR, 0, { 5, 5, 0, 6 } },
	{ "U8V8L8X8",					2, {	{ IN_U8V8X8X8,				-1, CL_RG,		CS_8BIT,	CT_SNORM },
											{ IN_X8X8L8X8,				-1, CL_A,		CS_8BIT,	CT_UNORM } },		0,		-1,		{ 0, DDPF_BUMPLUMINANCE, 0, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0x00000000 }, GM_LINEAR, 0, { 8, 8, 8, 8 } },

	// ** - unlisted

};

int g_nPixelFormats = sizeof( g_apfi ) / sizeof( PixelFormatInfo_t );


// these formats provide the pixel description inside the structure
#define DDPF_PIXELDESRC (DDPF_ALPHA | DDPF_RGB | DDPF_YUV | DDPF_LUMINANCE | DDPF_BUMPLUMINANCE | DDPF_BUMPDUDV | DDPF_PALETTEINDEXED8 | DDPF_PALETTEINDEXED4)
#define DDPF_BASE_FORMATS (DDPF_FOURCC | DDPF_PIXELDESRC)


PixelFormatInfo_t* _DDS_GetDXGIFormat( int dxgiFormat )
{
	for ( int i = 0; i < g_nDXGIFormats; i++ )
	{
		if ( g_apfiDXGI[i].dxgiFormat == dxgiFormat )
		{
			return &g_apfiDXGI[i];
		}
	}

	return NULL;
}


PixelFormatInfo_t* _DDS_GetPixelFormat( DDPIXELFORMAT* ppfi, bool* pbDX10 )
{
	int i;

	// check if it's dx10
	if (ppfi->dwFourCC == '01XD')
	{
		*pbDX10 = true;
		return NULL;
	}

	for (i = 0; i < g_nPixelFormats; i++)
	{
		// XXX: fix for Unreal UCC exported DDS
		// it doesn't add any flags at all
		if ( ppfi->dwFlags == 0 )
		{
			if ( ppfi->dwFourCC != 0 )
			{
				// assume is a FOURCC format
				if ( g_apfi[i].pf.dwFlags & DDPF_FOURCC )
				{
					if (ppfi->dwFourCC == g_apfi[i].pf.dwFourCC)
					{
						break;
					}
				}
			}
			else
			{
				// assume RGBA
				if ( g_apfi[i].pf.dwFlags & DDPF_RGB )
				{
					if ((ppfi->dwRBitMask == g_apfi[i].pf.dwRBitMask) &&
						(ppfi->dwGBitMask == g_apfi[i].pf.dwGBitMask) &&
						(ppfi->dwBBitMask == g_apfi[i].pf.dwBBitMask) &&
						(ppfi->dwRGBAlphaBitMask == g_apfi[i].pf.dwRGBAlphaBitMask))
					{
						break;
					}
				}
			}
		}
		
		if (ppfi->dwFlags & g_apfi[i].pf.dwFlags & DDPF_BASE_FORMATS)
		{
			if (ppfi->dwFlags & DDPF_FOURCC)
			{
				if (ppfi->dwFourCC == g_apfi[i].pf.dwFourCC)
				{
					break;
				}
			}
			else if (ppfi->dwFlags & DDPF_PIXELDESRC)
			{
				if (ppfi->dwRGBBitCount == g_apfi[i].pf.dwRGBBitCount)
				{
					if ((ppfi->dwFlags & DDPF_RGB) ||
						(ppfi->dwFlags & DDPF_YUV) ||
						(ppfi->dwFlags & DDPF_BUMPLUMINANCE) ||
						(ppfi->dwFlags & DDPF_BUMPDUDV))
					{
						if ((ppfi->dwRBitMask == g_apfi[i].pf.dwRBitMask) &&
							(ppfi->dwGBitMask == g_apfi[i].pf.dwGBitMask) &&
							(ppfi->dwBBitMask == g_apfi[i].pf.dwBBitMask) &&
							(ppfi->dwRGBAlphaBitMask == g_apfi[i].pf.dwRGBAlphaBitMask))
						{
							break;
						}
					}
					else if (ppfi->dwFlags & DDPF_ALPHA)
					{
						if (ppfi->dwRGBAlphaBitMask == g_apfi[i].pf.dwRGBAlphaBitMask)
						{
							break;
						}
					}
					else if (ppfi->dwFlags & DDPF_LUMINANCE)
					{
						if ((ppfi->dwRBitMask == g_apfi[i].pf.dwRBitMask) &&
							(ppfi->dwRGBAlphaBitMask == g_apfi[i].pf.dwRGBAlphaBitMask))
						{
							break;
						}
					}
					// XXX: probably there should be something else
					else if ((ppfi->dwFlags & DDPF_PALETTEINDEXED8) ||
						(ppfi->dwFlags & DDPF_PALETTEINDEXED4))
					{
						break; // XXX: 
					}
				}
			}
		}
	}

	if (i < g_nPixelFormats)
	{
		return &g_apfi[i];
	}

	return NULL;
}


#define NUM_CUBE_FACES 6

int GetNumCubeFacesDX9(DWORD caps)
{
	int n = 0;
	int i;

	for (i = 0; i < NUM_CUBE_FACES; i++)
	{
		if (caps & (DDSCAPS2_CUBEMAP_POSITIVEX << i))
		{
			n++;
		}
	}
	
	return n;
}


int _GetPixelSize( StreamInfo_t* psi )
{
	PixelFormat_t pf;
	pf.ePixelFormat = psi->ePixelFormat;
	pf.eChannelLayout = psi->eChannelLayout;
	pf.iComponentSize = psi->iComponentSize;
	pf.eDataFormat = psi->eDataType;

	return g_plibclient->pfnGetPixelSize( &pf );
}


bool _DDS_ReadHeader(HF stream, DDSURFACEDESC2* pdds)
{
	int magic;

	if (SYS_fread(&magic, sizeof(int), 1, stream))
	{
		if (magic == DDS_MAGIC)
		{
			if (SYS_fread(pdds, sizeof(DDSURFACEDESC2), 1, stream))
			{
				return true;
			}
		}
	}

	return false;
}


typedef struct MIPMapInfo_s
{
	int iWidth;
	int iHeight;
	int iDepth;
} MIPMapInfo_t;

typedef struct File_s
{
	PixelFormatInfo_t* ppfi;
	int nImages;
	int nMIPMaps;
	MIPMapInfo_t* ammi; // nMIPMaps
	HMEM hPaletteBits;
	HMEM ahImageStream[MAX_STREAMS];
} File_t;


H_FILE LoadFile( HF stream, KEYVALUEBUFFER hkvbufSettings, KEYVALUEBUFFER hkvbufMetadata )
{
	DDSURFACEDESC2 dds;

	if (_DDS_ReadHeader(stream, &dds))
	{
		bool bDX10 = false;
		PixelFormatInfo_t* ppfi = _DDS_GetPixelFormat(&dds.ddpfPixelFormat, &bDX10);

		if (ppfi != NULL || bDX10)
		{
			int iArraySize = 1;
			
			if (bDX10)
			{
				DDS_HEADER_DX10 dx10;

				if (SYS_fread(&dx10, sizeof(DDS_HEADER_DX10), 1, stream))
				{
					ppfi = _DDS_GetDXGIFormat( dx10.dxgiFormat );
					if ( ppfi == NULL )
					{
						goto err1;
					}
					
					iArraySize = max(1, dx10.arraySize);

					// XXX: allow checking for the legacy struct also?
					if (dx10.miscFlag & 0x04) // DDS_RESOURCE_MISC_TEXTURECUBE
					{
						iArraySize *= NUM_CUBE_FACES;
					}
				}
				else
				{
					// XXX: eof
					goto err1;
				}
			}
			else
			{
				// TODO: check dwCaps1 first?
				if (dds.ddsCaps.dwCaps2 & DDSCAPS2_CUBEMAP)
				{
					iArraySize = GetNumCubeFacesDX9(dds.ddsCaps.dwCaps2);
				}
				else
				{
					iArraySize = 1;
				}
			}

			if (ppfi != NULL) // check it again since GetDXGIFormat() can return NULL
			{
				HMEM pal = NULL;
				bool bPalPassed = false;

				// if the format expects a palette
				if (ppfi->iPaletteSize != 0)
				{
					int iSize = ppfi->iPaletteSize * sizeof(DWORD);
					pal = SYS_AllocStreamMemory(iSize);
					if (pal != NULL)
					{
						BYTE* in = (BYTE*)SYS_malloc(iSize*2); // alloc two buffers at once
						if ( in != NULL )
						{
							BYTE* out = &in[iSize];

							if (SYS_fread(in, iSize, 1, stream))
							{
								g_plibclient->pfnDecodePixels(IN_R8G8B8X8_UNORM, out, in, ppfi->iPaletteSize, 1, 0);
								SYS_WriteStreamMemory( pal, 0, iSize, out );

								bPalPassed = true;
							}
							SYS_free(in);
						}
					}
				}
				else
				{
					bPalPassed = true;
				}

				if (bPalPassed)
				{
					int iWidth = max(1, dds.dwWidth);
					int iHeight = max(1, dds.dwHeight);
					int iDepth = (dds.ddsCaps.dwCaps2 & DDSCAPS2_VOLUME)? max(1, dds.dwDepth): 1; // TODO: no arrays and volume textures in one
					int nMIPMaps = (dds.ddsCaps.dwCaps & DDSCAPS_MIPMAP)? max(1, dds.dwMipMapCount): 1;

					File_t* pFile = (File_t*)SYS_malloc(sizeof(File_t));
					if ( pFile != NULL )
					{
						pFile->ppfi = ppfi;
						pFile->hPaletteBits = pal;
						pFile->nImages = iArraySize;
						pFile->nMIPMaps = nMIPMaps;
						pFile->ammi = (MIPMapInfo_t*)SYS_malloc(sizeof(MIPMapInfo_t) * pFile->nMIPMaps);
						if ( pFile->ammi != NULL )
						{
							int iMaxPixelSize = 0;
							int iPixelSize[MAX_STREAMS];
							int iTotalSize[MAX_STREAMS];
							for ( int iStream = 0; iStream < ppfi->nStreams; iStream++ )
							{
								iPixelSize[iStream] = _GetPixelSize( &ppfi->stream[iStream] );
								if ( iMaxPixelSize < iPixelSize[iStream] )
								{
									iMaxPixelSize = iPixelSize[iStream];
								}
								iTotalSize[iStream] = 0;
								pFile->ahImageStream[iStream] = NULL;
							}

							// gather the mip map info
							for (int iMIPMap = 0; iMIPMap < nMIPMaps; iMIPMap++)
							{
								MIPMapInfo_t* pmmi = &pFile->ammi[iMIPMap];
								pmmi->iWidth = iWidth >> iMIPMap;
								pmmi->iHeight = iHeight >> iMIPMap;
								pmmi->iDepth = iDepth >> iMIPMap;
								if (pmmi->iWidth == 0)
								{
									if (pmmi->iHeight == 0) // something went wrong
									{
										goto err2;
									}
									else
									{
										pmmi->iWidth = 1;
									}
								}
								if (pmmi->iHeight == 0)
								{
									if (pmmi->iWidth == 0) // something went wrong
									{
										goto err2;
									}
									else
									{
										pmmi->iHeight = 1;
									}
								}
								if ( pmmi->iDepth == 0 )
								{
									pmmi->iDepth = 1;
								}

								// add this mip map to the total size
								for ( int iStream = 0; iStream < ppfi->nStreams; iStream++ )
								{
									int iImageSize = iPixelSize[iStream] * pmmi->iWidth * pmmi->iHeight * pmmi->iDepth * iArraySize;
									iTotalSize[iStream] += iImageSize;
								}
							}

							// alloc the data
							for ( int iStream = 0; iStream < ppfi->nStreams; iStream++ )
							{
								pFile->ahImageStream[iStream] = SYS_AllocStreamMemory(iTotalSize[iStream]);
								if ( pFile->ahImageStream[iStream] == NULL )
								{
									goto err3;
								}
							}

							// for the DXTn formats it may be tricky to determine it by simple math.. so we have this function
							int iHeightGranularity = g_plibclient->pfnGetHeightGranularity(ppfi->stream[0].eInPixelFormat);

							// read the data
							for (int iImage = 0; iImage < iArraySize; iImage++)
							{
								int iMIPMapOffsetPixels = 0;
								for (int iMIPMap = 0; iMIPMap < nMIPMaps; iMIPMap++)
								{
									MIPMapInfo_t* pmmi = &pFile->ammi[iMIPMap];

									int iPitchPixels = pmmi->iWidth;
									int iSliceSizePixels = iPitchPixels * pmmi->iHeight;
									int iImageSizePixels = iSliceSizePixels * pmmi->iDepth;
									int iImageOffsetPixels = iMIPMapOffsetPixels + iImageSizePixels * iImage;
									iMIPMapOffsetPixels += iImageSizePixels * iArraySize;

									int iInputChunkSize = g_plibclient->pfnGetInputSize(ppfi->stream[0].eInPixelFormat, pmmi->iWidth, iHeightGranularity, 0);
									int iOutputChunkSize = iMaxPixelSize * pmmi->iWidth * iHeightGranularity;
									byte_t* in = (byte_t*)SYS_malloc(iInputChunkSize+iOutputChunkSize); // alloc two buffers at once
									if (in == NULL)
									{
										goto err3;
									}
									byte_t* out = &in[iInputChunkSize];

									for (int iSlice = 0; iSlice < pmmi->iDepth; iSlice++)
									{
										int iSliceOffsetPixels = iImageOffsetPixels + iSlice * iSliceSizePixels;

										for (int y = 0; y < pmmi->iHeight; y += iHeightGranularity)
										{
											int iOffsetPixels = iSliceOffsetPixels + y * iPitchPixels;

											if (SYS_fread(in, iInputChunkSize, 1, stream))
											{
												int nLines = min(iHeightGranularity, pmmi->iHeight - y);
												// decompose
												for ( int iStream = 0; iStream < ppfi->nStreams; iStream++ )
												{
													g_plibclient->pfnDecodePixels( ppfi->stream[iStream].eInPixelFormat, out, in, pmmi->iWidth, nLines, 0 );
													int iOffset = iOffsetPixels * iPixelSize[iStream];
													int iSize = nLines * iPitchPixels * iPixelSize[iStream];
													SYS_WriteStreamMemory( pFile->ahImageStream[iStream], iOffset, iSize, out );
												}
											}
											else
											{
												goto finish; // XXX: on eof just skip everything beyond we've already read
											}
										}
									}
finish:
									SYS_free(in);
								}
							}

							//KEYVALUEKEY hKey = g_sys->pfnAddKey( hkvbufMetadata, "DXGI", bDX10 ? "true" : "false" );
							//g_sys->pfnSetKyeValue( hKey, "true" );

							return ( H_FILE )pFile;

err3:
							for ( int iStream = 0; iStream < ppfi->nStreams; iStream++ )
							{
								if ( pFile->ahImageStream[iStream] != NULL )
								{
									SYS_FreeStreamMemory( pFile->ahImageStream[iStream] );
								}
							}
err2:
							SYS_free( pFile->ammi );
						}

						SYS_free(pFile);
					}

					if (pal != NULL)
					{
						SYS_FreeStreamMemory(pal);
					}
				}
			}
		}
	}

err1:

	return NULL;
}


void FreeFile( H_FILE hFile )
{
	File_t* pFile = ( File_t* )hFile;

	for ( int iStream = 0; iStream < pFile->ppfi->nStreams; iStream++ )
	{
		SYS_FreeStreamMemory( pFile->ahImageStream[iStream] );
	}

	SYS_free( pFile->ammi );

	if ( pFile->hPaletteBits != NULL )
	{
		SYS_FreeStreamMemory( pFile->hPaletteBits );
	}

	SYS_free( pFile );
}


int GetNumSets( H_FILE hFile )
{
	return 1;
}


H_SET LoadSet( H_FILE hFile, int iSet )
{
	// we do not support sets here
	return ( H_SET )hFile;
}


void FreeSet( H_SET hSet )
{
	// do nothing
}


int GetNumImages( H_SET hSet )
{
	File_t* pFile = ( File_t* )hSet;
	return pFile->nImages;
}


int GetNumMIPMaps( H_SET hSet )
{
	File_t* pFile = ( File_t* )hSet;
	return pFile->nMIPMaps;
}


const char* GetFormatStr( H_SET hSet )
{
	File_t* pFile = ( File_t* )hSet;
	return pFile->ppfi->pszPixelFormat;
}


void GetInputGamma( H_SET hSet, Gamma_t* pcs )
{
	File_t* pFile = ( File_t* )hSet;
	pcs->eGamma = pFile->ppfi->eInputGamma;
}


void GetOutputGamma( H_SET hSet, Gamma_t* pcs )
{
	File_t* pFile = ( File_t* )hSet;
	pcs->eGamma = pFile->ppfi->eInputGamma;
}


int GetImageFlags( H_SET hSet )
{
	File_t* pFile = ( File_t* )hSet;
	return pFile->ppfi->flags;
}


int GetOriginalBitDepth( H_SET hSet, int iChannel )
{
	File_t* pFile = ( File_t* )hSet;
	return pFile->ppfi->aiNumChannelBits[iChannel];
}


int GetImageWidth( H_SET hSet, int iMIPMap )
{
	File_t* pFile = ( File_t* )hSet;
	return pFile->ammi[iMIPMap].iWidth;
}


int GetImageHeight( H_SET hSet, int iMIPMap )
{
	File_t* pFile = ( File_t* )hSet;
	return pFile->ammi[iMIPMap].iHeight;
}


int GetImageDepth( H_SET hSet, int iMIPMap )
{
	File_t* pFile = ( File_t* )hSet;
	return pFile->ammi[iMIPMap].iDepth;
}


int GetNumPaletteColors( H_SET hSet )
{
	File_t* pFile = ( File_t* )hSet;
	return pFile->ppfi->iPaletteSize;
}


int GetNumStreams( H_SET hSet, int eStreamType )
{
	File_t* pFile = ( File_t* )hSet;

	if ( eStreamType == ST_PALETTE )
	{
		return pFile->hPaletteBits ? 1 : 0;
	}
	else
	{
		return pFile->ppfi->nStreams;
	}
}


void GetStreamPixelFormat( H_SET hSet, int eStreamType, int iStream, PixelFormat_t* ppfi )
{
	File_t* pFile = ( File_t* )hSet;

	if ( eStreamType == ST_PALETTE )
	{
		ppfi->ePixelFormat = PF_B8G8R8X8_UNORM;
		// ignored here
		//ppfi->eChannelLayout = CL_BGRX;
		//ppfi->iComponentSize = CS_8BIT;
		//ppfi->eDataFormat = CT_UNORM;
	}
	else
	{
		ppfi->ePixelFormat = pFile->ppfi->stream[iStream].ePixelFormat;
		ppfi->eChannelLayout = pFile->ppfi->stream[iStream].eChannelLayout;
		ppfi->iComponentSize = pFile->ppfi->stream[iStream].iComponentSize;
		ppfi->eDataFormat = pFile->ppfi->stream[iStream].eDataType;
	}
}


HMEM GetStreamBits( H_SET hSet, int eStreamType, int iStream )
{
	File_t* pFile = ( File_t* )hSet;

	if ( eStreamType == ST_PALETTE )
	{
		return pFile->hPaletteBits;
	}
	else
	{
		return pFile->ahImageStream[iStream];
	}
}


FormatFuncs_t g_fmt =
{
	LoadFile,
	FreeFile,
	GetNumSets,
	LoadSet,
	FreeSet,
	GetNumImages,
	GetNumMIPMaps,
	GetFormatStr,
	GetInputGamma,
	GetOutputGamma,
	GetImageFlags,
	GetOriginalBitDepth,
	GetImageWidth,
	GetImageHeight,
	GetImageDepth,
	GetNumPaletteColors,
	GetNumStreams,
	GetStreamPixelFormat,
	GetStreamBits,
};


extern "C" __declspec(dllexport) int __cdecl GetInterfaceVersion( int iCurrentInterfaceVersion )
{
	return FORMAT_INTERFACE_VERSION;
}


extern "C" __declspec(dllexport) FormatFuncs_t* __cdecl LoadDll( SystemFuncs_t* psystem, HWND hWndMain )
{
	g_sys = psystem;
	g_plibclient = ( PLibClientFuncs_t* )psystem->pfnGetPLibClientFuncs();

	return &g_fmt;
}


BOOL WINAPI DllMain(       HINSTANCE hinst,
                                DWORD reason,
                                LPVOID lpReserved)
{
    return TRUE;
}

