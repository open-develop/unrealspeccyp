/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2010 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef _DINGOO

#include "../platform.h"
#include "../io.h"
#include "../../options_common.h"
//#include "../../tools/log.h"

namespace xPlatform
{

void VideoUpdate();
void VideoFlip();
void UpdateAudio();
void UpdateKeys();

bool Init(const char* path)
{
	const char* lf = "A:\\GAME\\";
	OPTION_GET(op_last_file)->Set(lf);
//	xIo::SetResourcePath(OpLastFolder());
	xIo::SetProfilePath(OpLastFolder());
//	xLog::SetLogPath(OpLastFolder());
	Handler()->OnInit();
#ifdef SIM_VERSION
	Handler()->OnOpenFile(path);
#endif//SIM_VERSION
	return true;
}
void Done()
{
	Handler()->OnDone();
}
void Loop()
{
	while(!*OPTION_GET(op_quit) && _sys_judge_event(NULL) >= 0)
	{
		UpdateAudio(); //sync by audio
		VideoFlip();
		UpdateKeys();
		Handler()->OnLoop();
		VideoUpdate();
	}
}

}
//namespace xPlatform

extern "C" int main(int argc, char** argv)
{
	xPlatform::Init(argv[0]); // argv[0] contain path with .app name
	xPlatform::Loop();
	xPlatform::Done();
	return 0;
}

extern "C" int dingoo_fprintf(FILE * f, const char* fmt, const char* s1, const char* s2)
{
	char buf[1024];
	sprintf(buf, fmt, s1, s2);
	fputs(buf, f);
	return strlen(buf);
}
extern "C" int dingoo_sscanf(const char * str, const char * format, ...)
{
	return 0;
}

#endif//_DINGOO
