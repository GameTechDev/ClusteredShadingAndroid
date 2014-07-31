//--------------------------------------------------------------------------------------
// Copyright 2012 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.
//--------------------------------------------------------------------------------------
#ifndef CPUTPARSER_H
#define CPUTPARSER_H

class CommandParser {
public:
	~CommandParser();
	void ParseConfigurationOptions(cString arguments, cString delimiter = _L("-"));
	void CleanConfigurationOptions(void);

	bool GetParameter(cString arg);
	bool GetParameter(cString arg, double *pOut);
	bool GetParameter(cString arg, int *pOut);
	bool GetParameter(cString arg, unsigned int *pOut);
	bool GetParameter(cString arg, cString *pOut);
	bool GetParameter(cString arg, char *pOut);

private:
	std::map<cString, cString> m_ArgumentMap;
};

#endif // CPUTPARSER_H

