/*
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/

#include "audio/Sample.h"

#include "audio/AudioGlobal.h"
#include "audio/Stream.h"
#include "audio/AudioBackend.h"
#include "audio/AudioSource.h"

using namespace std;

namespace audio {

Sample::Sample(const string & _name) : ResourceHandle(), name(_name), length(0) {
}

Sample::~Sample() {
	
	// Delete sources referencing this sample.
	for(Backend::source_iterator p = backend->sourcesBegin(); p != backend->sourcesEnd();) {
		if(*p && (*p)->getSample() == this) {
			p = backend->deleteSource(p);
		} else {
			++p;
		}
	}
	
}

aalError Sample::load() {
	
	if(length != 0) {
		return AAL_ERROR_INIT;
	}
	
	Stream * stream = CreateStream(name);
	if(!stream) {
		return AAL_ERROR_FILEIO;
	}
	
	stream->GetFormat(format);
	stream->GetLength(length);
	DeleteStream(stream);
	
	return AAL_OK;
}

aalError Sample::setCallback(aalSampleCallback func, void * _data, size_t time, TimeUnit unit) {
	
	callbacks.resize(callbacks.size() + 1);
	
	callbacks.back().func = func;
	callbacks.back().data = _data;
	callbacks.back().time = UnitsToBytes(time, format, unit);
	if(callbacks.back().time > length) {
		callbacks.back().time = length;
	}
	
	return AAL_OK;
}

} // namespace audio
