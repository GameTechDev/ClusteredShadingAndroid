/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////
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

